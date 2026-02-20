// 必须首先包含预编译头,以确保NOMINMAX等宏正确设置
#include "D3D12PCH.h"

#include "D3D12Texture.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "DirectXTex.h" // DirectX Tool Library for texture loading

D3D12Texture::D3D12Texture()
{
    m_width = 0;
    m_height = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_mipLevels = 0;
    m_isRenderTarget = FALSE;
    m_isDepthStencil = FALSE;
}

D3D12Texture::~D3D12Texture()
{
    Release();
}

void D3D12Texture::Release()
{
    m_textureResource.Reset();
    m_uploadHeap.Reset();
    m_width = 0;
    m_height = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_mipLevels = 0;
    m_isRenderTarget = FALSE;
    m_isDepthStencil = FALSE;
}

HRESULT D3D12Texture::LoadFromFile(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const wchar_t* filename)
{
    if (device == nullptr || filename == nullptr)
        return E_INVALIDARG;

    // 根据文件扩展名选择加载方法
    std::wstring ext = filename;
    size_t dotPos = ext.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
    {
        ext = ext.substr(dotPos);
    }

    HRESULT hr = S_OK;

    // 转换为小写进行比较
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    if (ext == L".dds")
    {
        hr = LoadDDS(device, commandList, filename);
    }
    else if (ext == L".bmp")
    {
        hr = LoadBMP(device, commandList, filename);
    }
    else if (ext == L".tga")
    {
        hr = LoadTGA(device, commandList, filename);
    }
    else
    {
        // Use DirectXTex WIC loader for PNG, JPG, and other formats
        DirectX::ScratchImage image;
        DirectX::TexMetadata metadata = {};

        HRESULT hr = DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, image);
        if (SUCCEEDED(hr))
        {
            const DirectX::Image* img = image.GetImage(0, 0, 0);
            if (img)
            {
                hr = CreateFromMemory(
                    device,
                    commandList,
                    img->pixels,
                    img->width,
                    img->height,
                    DXGI_FORMAT_R8G8B8A8_UNORM,
                    1);
                return hr;
            }
        }
        return E_FAIL;
    }

    return hr;
}

HRESULT D3D12Texture::CreateFromMemory(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const void* data,
    UINT width,
    UINT height,
    DXGI_FORMAT format,
    UINT mipLevels)
{
    if (device == nullptr || data == nullptr)
        return E_INVALIDARG;

    m_width = width;
    m_height = height;
    m_format = format;
    m_mipLevels = mipLevels;

    // 如果未指定 mipmap 层级，则生成完整的 mipmap 链
    if (m_mipLevels == 0)
    {
        m_mipLevels = 1;
        UINT maxDim = std::max(width, height);
        while (maxDim > 1)
        {
            m_mipLevels++;
            maxDim /= 2;
        }
    }

    HRESULT hr = S_OK;

    try {
        // 计算每个 mipmap 层级的大小
        UINT64 totalSize = 0;
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(m_mipLevels);

        for (UINT mip = 0; mip < m_mipLevels; ++mip)
        {
            UINT mipWidth = width >> mip;
            UINT mipHeight = height >> mip;
            if (mipWidth == 0) mipWidth = 1;
            if (mipHeight == 0) mipHeight = 1;

            UINT64 rowPitch = (mipWidth * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
            UINT64 mipSize = rowPitch * mipHeight;

            subresources[mip].pData = nullptr; // 稍后设置
            subresources[mip].RowPitch = static_cast<UINT>(rowPitch);
            subresources[mip].SlicePitch = mipSize;
            totalSize += mipSize;
        }

        // 创建上传堆
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Alignment = 0;
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = m_mipLevels;
        textureDesc.Format = format;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadHeap)
        );

        if (FAILED(hr))
            return hr;

        // 创建默认堆纹理资源
        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC textureDesc2 = textureDesc;
        textureDesc2.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        hr = device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc2,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_textureResource)
        );

        if (FAILED(hr))
        {
            m_uploadHeap.Reset();
            return hr;
        }

        // 映射上传堆并复制数据
        UINT8* pData = nullptr;
        hr = m_uploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            m_uploadHeap.Reset();
            m_textureResource.Reset();
            return hr;
        }

        // 复制每个 mipmap 层级
        const UINT8* srcData = static_cast<const UINT8*>(data);
        UINT64 offset = 0;

        for (UINT mip = 0; mip < m_mipLevels; ++mip)
        {
            UINT mipWidth = width >> mip;
            UINT mipHeight = height >> mip;
            if (mipWidth == 0) mipWidth = 1;
            if (mipHeight == 0) mipHeight = 1;

            UINT rowPitch = (mipWidth * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
            UINT64 mipSize = rowPitch * mipHeight;

            // 复制数据
            const UINT8* srcRow = srcData;
            UINT8* dstRow = pData + offset;

            for (UINT row = 0; row < mipHeight; ++row)
            {
                memcpy(dstRow, srcRow, mipWidth * 4);
                srcRow += mipWidth * 4;
                dstRow += rowPitch;
            }

            subresources[mip].pData = reinterpret_cast<BYTE*>(pData + offset);

            srcData += rowPitch * mipHeight;
            offset += mipSize;
        }

        m_uploadHeap->Unmap(0, nullptr);

        // 上传纹理数据 - 使用正确的 D3D12 API
        offset = 0;
        for (UINT mip = 0; mip < m_mipLevels; ++mip)
        {
            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = m_textureResource.Get();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dstLocation.SubresourceIndex = mip;

            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = m_uploadHeap.Get();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

            // 计算当前 mipmap 的尺寸
            UINT mipWidth = std::max(1u, width >> mip);
            UINT mipHeight = std::max(1u, height >> mip);
            UINT64 rowPitch = (mipWidth * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

            srcLocation.PlacedFootprint.Offset = offset;
            srcLocation.PlacedFootprint.Footprint.Format = m_format;
            srcLocation.PlacedFootprint.Footprint.Width = mipWidth;
            srcLocation.PlacedFootprint.Footprint.Height = mipHeight;
            srcLocation.PlacedFootprint.Footprint.Depth = 1;
            srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(rowPitch);

            commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

            offset += rowPitch * mipHeight;
        }

        // 创建着色器资源视图
        hr = CreateShaderResourceView(device);
        if (FAILED(hr))
        {
            m_uploadHeap.Reset();
            m_textureResource.Reset();
        }
    }
    catch (const std::exception& e)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12Texture::LoadDDS(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const wchar_t* filename)
{
    if (!device || !filename)
        return E_INVALIDARG;

    DirectX::ScratchImage image;
    DirectX::TexMetadata metadata = {};

    HRESULT hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &metadata, image);
    if (FAILED(hr))
        return hr;

    // Get the first mip level
    const DirectX::Image* img = image.GetImage(0, 0, 0);
    if (!img)
        return E_FAIL;

    // Create texture from loaded image data
    hr = CreateFromMemory(
        device,
        commandList,
        img->pixels,
        img->width,
        img->height,
        metadata.format,
        metadata.mipLevels);

    return hr;
}

HRESULT D3D12Texture::LoadBMP(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const wchar_t* filename)
{
    if (!device || !filename)
        return E_INVALIDARG;

    DirectX::ScratchImage image;
    DirectX::TexMetadata metadata = {};

    HRESULT hr = DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, image);
    if (FAILED(hr))
        return hr;

    const DirectX::Image* img = image.GetImage(0, 0, 0);
    if (!img)
        return E_FAIL;

    hr = CreateFromMemory(
        device,
        commandList,
        img->pixels,
        img->width,
        img->height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        1);

    return hr;
}

HRESULT D3D12Texture::LoadTGA(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const wchar_t* filename)
{
    if (!device || !filename)
        return E_INVALIDARG;

    DirectX::ScratchImage image;
    DirectX::TexMetadata metadata = {};

    HRESULT hr = DirectX::LoadFromTGAFile(filename, DirectX::TGA_FLAGS_NONE, &metadata, image);
    if (FAILED(hr))
        return hr;

    const DirectX::Image* img = image.GetImage(0, 0, 0);
    if (!img)
        return E_FAIL;

    hr = CreateFromMemory(
        device,
        commandList,
        img->pixels,
        img->width,
        img->height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        1);

    return hr;
}

HRESULT D3D12Texture::CreateRenderTarget(
    ID3D12Device* device,
    UINT width,
    UINT height,
    DXGI_FORMAT format)
{
    if (device == nullptr)
        return E_INVALIDARG;

    m_width = width;
    m_height = height;
    m_format = format;
    m_mipLevels = 1;
    m_isRenderTarget = TRUE;

    HRESULT hr = S_OK;

    try
    {
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = format;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_textureResource)
        );

        if (FAILED(hr))
            return hr;

        // 创建 RTV
        hr = CreateRenderTargetView(device);
    }
    catch (...)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12Texture::CreateDepthStencil(
    ID3D12Device* device,
    UINT width,
    UINT height,
    DXGI_FORMAT format)
{
    if (device == nullptr)
        return E_INVALIDARG;

    m_width = width;
    m_height = height;
    m_format = format;
    m_mipLevels = 1;
    m_isDepthStencil = TRUE;

    HRESULT hr = S_OK;

    try
    {
        D3D12_RESOURCE_DESC depthDesc = {};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Alignment = 0;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.DepthOrArraySize = 1;
        depthDesc.MipLevels = 1;
        depthDesc.Format = format;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depthClearValue = {};
        depthClearValue.Format = format;
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.DepthStencil.Stencil = 0;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClearValue,
            IID_PPV_ARGS(&m_textureResource)
        );

        if (FAILED(hr))
            return hr;

        // 创建 DSV
        hr = CreateDepthStencilView(device);
    }
    catch (...)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12Texture::CreateShaderResourceView(ID3D12Device* device)
{
    if (device == nullptr || m_textureResource == nullptr)
        return E_INVALIDARG;

    // 创建 SRV 描述符
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = m_mipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    // 这里需要从描述符堆分配 SRV
    // 调用者应该提供描述符堆

    return S_OK;
}

HRESULT D3D12Texture::CreateRenderTargetView(ID3D12Device* device)
{
    if (device == nullptr || m_textureResource == nullptr)
        return E_INVALIDARG;

    // 创建 RTV 描述符
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = m_format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    // 这里需要从描述符堆分配 RTV
    // 调用者应该提供描述符堆

    return S_OK;
}

HRESULT D3D12Texture::CreateDepthStencilView(ID3D12Device* device)
{
    if (device == nullptr || m_textureResource == nullptr)
        return E_INVALIDARG;

    // 创建 DSV 描述符
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = m_format;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    // 这里需要从描述符堆分配 DSV
    // 调用者应该提供描述符堆

    return S_OK;
}

HRESULT D3D12Texture::GenerateMipmaps(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList)
{
    // 生成 mipmap 链
    // 这里可以使用 DirectX Tool Library 或手动实现

    // TODO: 实现自动 mipmap 生成
    return E_NOTIMPL;
}

// 纹理管理器实现
D3D12TextureManager::D3D12TextureManager(
    ID3D12Device* device,
    ID3D12DescriptorHeap* srvHeap)
    : m_device(device), m_srvHeap(srvHeap)
{
    if (device == nullptr || srvHeap == nullptr)
        throw std::invalid_argument("Invalid parameters");

    m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

D3D12TextureManager::~D3D12TextureManager()
{
    ReleaseAll();
}

D3D12Texture* D3D12TextureManager::LoadTexture(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const wchar_t* filename)
{
    // 检查缓存
    auto it = m_textureCache.find(filename);
    if (it != m_textureCache.end())
    {
        return it->second;
    }

    // 加载新纹理
    D3D12Texture* texture = new D3D12Texture();
    if (texture == nullptr)
        return nullptr;

    HRESULT hr = texture->LoadFromFile(device, commandList, filename);
    if (FAILED(hr))
    {
        delete texture;
        return nullptr;
    }

    // 创建 SRV (需要在描述符堆中分配)
    // TODO: 分配 SRV

    m_textureCache[filename] = texture;
    return texture;
}

void D3D12TextureManager::ReleaseAll()
{
    for (auto& pair : m_textureCache)
    {
        if (pair.second)
        {
            pair.second->Release();
            delete pair.second;
        }
    }
    m_textureCache.clear();
}

D3D12Texture* D3D12TextureManager::GetTexture(const std::wstring& filename)
{
    auto it = m_textureCache.find(filename);
    if (it != m_textureCache.end())
    {
        return it->second;
    }
    return nullptr;
}
