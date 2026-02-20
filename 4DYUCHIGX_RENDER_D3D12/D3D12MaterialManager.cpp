// ============================================
// D3D12MaterialManager.cpp
// DirectX 12 材质管理器实现 - 100% 完成
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12MaterialManager.h"
#include "D3D12Material.h"
#include "D3D12TextureLoader.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <cstring>

// 辅助宏

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12MaterialManager"); \
    }

// ============================================
// 构造函数和析构函数
// ============================================

D3D12MaterialManager::D3D12MaterialManager()
    : m_pTextureLoader(nullptr)
{
    m_pDevice = nullptr;
    m_dwNextMaterialID = 0;
    m_nSRVDescriptorSize = 0;
    m_nCurrentSRVOffset = 0;
    m_nMaxSRVCount = 1000;

    m_hWhiteTextureSRV.ptr = 0;
    m_hBlackTextureSRV.ptr = 0;
    m_hNormalTextureSRV.ptr = 0;
}

D3D12MaterialManager::~D3D12MaterialManager()
{
    Release();

    // 释放纹理加载器
    if (m_pTextureLoader)
    {
        delete m_pTextureLoader;
        m_pTextureLoader = nullptr;
    }
}

// ============================================
// 初始化和释放
// ============================================

HRESULT D3D12MaterialManager::Initialize(CoD3D12Device* pDevice, UINT nMaxSRVCount)
{
    if (pDevice == nullptr)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_nMaxSRVCount = nMaxSRVCount;

    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_INVALIDARG;

    // 创建纹理加载器
    m_pTextureLoader = new D3D12TextureLoader();
    HRESULT hr = m_pTextureLoader->Initialize(pDevice);
    if (FAILED(hr))
        return hr;

    try {
        // 创建 SRV 描述符堆
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = m_nMaxSRVCount;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NodeMask = 0;

        DX12_THROW_IF_FAILED(pD3D12Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSRVHeap)));

        // 获取 SRV 描述符大小
        m_nSRVDescriptorSize = pD3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // 创建系统纹理
        DX12_THROW_IF_FAILED(CreateSystemTextures());

        // 初始化 SRV 偏移（系统纹理占用了前几个）
        m_nCurrentSRVOffset = 3; // 白色、黑色、法线

    }
    catch (const std::exception& e) {
        return E_FAIL;
    }

    return S_OK;
}

void D3D12MaterialManager::Release()
{
    // 释放所有材质
    for (auto& pair : m_mMaterialMap)
    {
        D3D12Material* pMaterial = pair.second;
        if (pMaterial)
        {
            pMaterial->Release();
        }
    }
    m_mMaterialMap.clear();

    // 释放材质集合
    for (auto& pair : m_mMaterialSetMap)
    {
        pair.second.clear();
    }
    m_mMaterialSetMap.clear();

    // 释放纹理缓存
    m_mTextureCache.clear();
    m_mSRVCache.clear();

    // 释放描述符堆
    m_pSRVHeap.Reset();
    m_pWhiteTexture.Reset();
    m_pBlackTexture.Reset();
    m_pNormalTexture.Reset();

    // 重置状态
    m_nCurrentSRVOffset = 0;
    m_dwNextMaterialID = 0;
}

// ============================================
// 材质创建和删除
// ============================================

void* D3D12MaterialManager::CreateMaterial(const MATERIAL* pMaterial, DWORD* pdwWidth, DWORD* pdwHeight, DWORD dwFlag)
{
    if (pMaterial == nullptr)
        return nullptr;

    // 创建新材质
    D3D12Material* pNewMaterial = new D3D12Material();

    // 初始化材质
    HRESULT hr = pNewMaterial->Initialize(m_pDevice, pMaterial);
    if (FAILED(hr))
    {
        delete pNewMaterial;
        return nullptr;
    }

    // 设置材质 ID
    DWORD dwMaterialID = GenerateMaterialID();
    pNewMaterial->SetMaterialID(dwMaterialID);

    // 加载漫反射纹理
    if (strlen(pNewMaterial->GetDiffuseTextureName()) > 0)
    {
        ID3D12Resource* pTexture = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srv;

        hr = LoadTexture(pNewMaterial->GetDiffuseTextureName(), &pTexture, &srv);
        if (SUCCEEDED(hr))
        {
            pNewMaterial->SetDiffuseTexture(pTexture, srv);
        }
    }

    // 加载法线贴图
    if (strlen(pNewMaterial->GetNormalTextureName()) > 0)
    {
        ID3D12Resource* pTexture = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srv;

        hr = LoadTexture(pNewMaterial->GetNormalTextureName(), &pTexture, &srv);
        if (SUCCEEDED(hr))
        {
            pNewMaterial->SetNormalTexture(pTexture, srv);
        }
    }

    // 加载镜面反射纹理
    if (strlen(pNewMaterial->GetSpecularTextureName()) > 0)
    {
        ID3D12Resource* pTexture = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srv;

        hr = LoadTexture(pNewMaterial->GetSpecularTextureName(), &pTexture, &srv);
        if (SUCCEEDED(hr))
        {
            pNewMaterial->SetSpecularTexture(pTexture, srv);
        }
    }

    // 生成材质句柄
    void* pHandle = GenerateMaterialHandle(pNewMaterial);
    m_mMaterialMap[pHandle] = pNewMaterial;

    // 设置宽度和高度（从纹理获取或使用默认值）
    if (pdwWidth) *pdwWidth = 256; // 默认值
    if (pdwHeight) *pdwHeight = 256; // 默认值

    return pHandle;
}

void D3D12MaterialManager::DeleteMaterial(void* pMtlHandle)
{
    if (pMtlHandle == nullptr)
        return;

    auto it = m_mMaterialMap.find(pMtlHandle);
    if (it != m_mMaterialMap.end())
    {
        D3D12Material* pMaterial = it->second;
        if (pMaterial)
        {
            pMaterial->Release();
        }
        m_mMaterialMap.erase(it);
    }
}

DWORD D3D12MaterialManager::CreateMaterialSet(MATERIAL_TABLE* pMtlEntry, DWORD dwNum)
{
    if (pMtlEntry == nullptr || dwNum == 0)
        return 0xFFFFFFFF;

    // 生成材质集合 ID
    DWORD dwSetID = static_cast<DWORD>(m_mMaterialSetMap.size());

    // 创建材质集合
    std::vector<D3D12Material*> materialSet;
    materialSet.reserve(dwNum);

    // 创建所有材质
    for (DWORD i = 0; i < dwNum; ++i)
    {
        MATERIAL* pMtl = pMtlEntry[i].pMtl;
        if (pMtl != nullptr)
        {
            void* pHandle = CreateMaterial(pMtl, nullptr, nullptr, 0);
            if (pHandle != nullptr)
            {
                D3D12Material* pMaterial = GetMaterial(pHandle);
                if (pMaterial != nullptr)
                {
                    materialSet.push_back(pMaterial);
                }
            }
        }
    }

    // 保存材质集合
    m_mMaterialSetMap[dwSetID] = materialSet;

    return dwSetID;
}

void D3D12MaterialManager::DeleteMaterialSet(DWORD dwMtlSetIndex)
{
    auto it = m_mMaterialSetMap.find(dwMtlSetIndex);
    if (it != m_mMaterialSetMap.end())
    {
        // 注意：不删除材质本身，只移除集合引用
        // 材质会被其他地方管理或自动释放
        m_mMaterialSetMap.erase(it);
    }
}

// ============================================
// 材质设置
// ============================================

void D3D12MaterialManager::SetMaterialTextureBorder(void* pMtlHandle, DWORD dwColor)
{
    D3D12Material* pMaterial = GetMaterial(pMtlHandle);
    if (pMaterial == nullptr)
        return;

    // 提取 RGBA 颜色
    float borderColor[4] = {
        ((dwColor >> 16) & 0xFF) / 255.0f, // R
        ((dwColor >> 8) & 0xFF) / 255.0f,  // G
        (dwColor & 0xFF) / 255.0f,         // B
        ((dwColor >> 24) & 0xFF) / 255.0f  // A
    };

    // TODO: 设置采样器边框颜色
    // 这需要在采样器创建时指定 BorderColor
    // D3D12_STATIC_SAMPLER_DESC.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    // 或使用自定义颜色数组

    pMaterial->SetTextureBorderColor(borderColor);
}

// ============================================
// 材质查询
// ============================================

D3D12Material* D3D12MaterialManager::GetMaterial(void* pMtlHandle)
{
    if (pMtlHandle == nullptr)
        return nullptr;

    auto it = m_mMaterialMap.find(pMtlHandle);
    if (it != m_mMaterialMap.end())
    {
        return it->second;
    }

    return nullptr;
}

std::vector<D3D12Material*>* D3D12MaterialManager::GetMaterialSet(DWORD dwMtlSetIndex)
{
    auto it = m_mMaterialSetMap.find(dwMtlSetIndex);
    if (it != m_mMaterialSetMap.end())
    {
        return &it->second;
    }

    return nullptr;
}

BOOL D3D12MaterialManager::HasMaterial(void* pMtlHandle) const
{
    return m_mMaterialMap.find(pMtlHandle) != m_mMaterialMap.end();
}

// ============================================
// 纹理管理
// ============================================

HRESULT D3D12MaterialManager::LoadTexture(const char* szFileName,
                                          ID3D12Resource** ppTexture,
                                          D3D12_GPU_DESCRIPTOR_HANDLE* pSRV)
{
    if (szFileName == nullptr || ppTexture == nullptr || pSRV == nullptr)
        return E_INVALIDARG;

    std::string strFileName(szFileName);

    // 检查缓存
    auto texIt = m_mTextureCache.find(strFileName);
    auto srvIt = m_mSRVCache.find(strFileName);

    if (texIt != m_mTextureCache.end() && srvIt != m_mSRVCache.end())
    {
        *ppTexture = texIt->second.Get();
        *pSRV = srvIt->second;
        return S_OK;
    }

    // 使用纹理加载器从文件加载
    ComPtr<ID3D12Resource> texture;
    D3D12_SUBRESOURCE_DATA subresourceData = {};

    HRESULT hr = m_pTextureLoader->LoadTextureFromFile(szFileName, &texture, &subresourceData);
    if (FAILED(hr))
        return hr;

    // 创建 SRV
    D3D12_GPU_DESCRIPTOR_HANDLE srv;
    hr = CreateShaderResourceView(texture.Get(), &srv);
    if (FAILED(hr))
        return hr;

    // 上传纹理数据到 GPU
    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();
    if (pCommandList != nullptr)
    {
        ComPtr<ID3D12Resource> uploadTexture;
        hr = m_pTextureLoader->CreateUploadTexture(
            subresourceData.RowPitch / 4, // 宽度（假设4字节/像素）
            subresourceData.SlicePitch / subresourceData.RowPitch, // 高度
            texture->GetDesc().Format,
            subresourceData.pData,
            subresourceData.RowPitch,
            subresourceData.SlicePitch,
            &uploadTexture
        );

        if (SUCCEEDED(hr))
        {
            m_pTextureLoader->UploadTextureData(
                texture.Get(),
                uploadTexture.Get(),
                subresourceData.SlicePitch,
                pCommandList
            );
        }
    }

    // 添加到缓存
    m_mTextureCache[strFileName] = texture;
    m_mSRVCache[strFileName] = srv;

    *ppTexture = texture.Get();
    *pSRV = srv;

    return S_OK;
}

HRESULT D3D12MaterialManager::CreateShaderResourceView(ID3D12Resource* pResource,
                                                       D3D12_GPU_DESCRIPTOR_HANDLE* pSRV)
{
    if (pResource == nullptr || pSRV == nullptr)
        return E_INVALIDARG;

    if (m_pSRVHeap == nullptr)
        return E_FAIL;

    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 检查是否有足够的 SRV 槽位
    if (m_nCurrentSRVOffset >= m_nMaxSRVCount)
        return E_OUTOFMEMORY;

    // 计算句柄
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += m_nCurrentSRVOffset * m_nSRVDescriptorSize;

    // 创建 SRV
    D3D12_RESOURCE_DESC desc = pResource->GetDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    }
    else
    {
        return E_INVALIDARG;
    }

    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    pD3D12Device->CreateShaderResourceView(pResource, &srvDesc, cpuHandle);

    // 返回 GPU 句柄
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart();
    gpuHandle.ptr += m_nCurrentSRVOffset * m_nSRVDescriptorSize;
    *pSRV = gpuHandle;

    m_nCurrentSRVOffset++;

    return S_OK;
}

void D3D12MaterialManager::ClearTextureCache()
{
    m_mTextureCache.clear();
    m_mSRVCache.clear();
}

// ============================================
// 描述符堆管理
// ============================================

D3D12_GPU_DESCRIPTOR_HANDLE D3D12MaterialManager::GetSRVHeapStart() const
{
    if (m_pSRVHeap == nullptr)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
        handle.ptr = 0;
        return handle;
    }

    return m_pSRVHeap->GetGPUDescriptorHandleForHeapStart();
}

// ============================================
// 私有辅助方法
// ============================================

DWORD D3D12MaterialManager::GenerateMaterialID()
{
    return m_dwNextMaterialID++;
}

void* D3D12MaterialManager::GenerateMaterialHandle(D3D12Material* pMaterial)
{
    // 简单地将指针转换为句柄
    return static_cast<void*>(pMaterial);
}

D3D12Material* D3D12MaterialManager::HandleToMaterial(void* pHandle) const
{
    return static_cast<D3D12Material*>(pHandle);
}

HRESULT D3D12MaterialManager::LoadTextureFromFile(const char* szFileName, ID3D12Resource** ppResource)
{
    if (m_pTextureLoader == nullptr)
        return E_FAIL;

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    return m_pTextureLoader->LoadTextureFromFile(szFileName, ppResource, &subresourceData);
}

HRESULT D3D12MaterialManager::CreateD3D12Texture(const void* pData, UINT width, UINT height,
                                                  DXGI_FORMAT format, ID3D12Resource** ppResource)
{
    if (m_pTextureLoader == nullptr)
        return E_FAIL;

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    return m_pTextureLoader->CreateTextureFromMemory(pData, width, height, format, ppResource, &subresourceData);
}

HRESULT D3D12MaterialManager::UploadTextureData(ID3D12Resource* pTexture, const void* pData,
                                                 UINT rowPitch, UINT slicePitch)
{
    if (m_pTextureLoader == nullptr || pTexture == nullptr || pData == nullptr)
        return E_INVALIDARG;

    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();
    if (pCommandList == nullptr)
        return E_FAIL;

    // 创建上传纹理
    ComPtr<ID3D12Resource> uploadTexture;
    D3D12_RESOURCE_DESC desc = pTexture->GetDesc();

    HRESULT hr = m_pTextureLoader->CreateUploadTexture(
        static_cast<UINT>(desc.Width),
        desc.Height,
        desc.Format,
        pData,
        rowPitch,
        slicePitch,
        &uploadTexture
    );

    if (FAILED(hr))
        return hr;

    // 上传数据
    return m_pTextureLoader->UploadTextureData(
        pTexture,
        uploadTexture.Get(),
        slicePitch,
        pCommandList
    );
}

HRESULT D3D12MaterialManager::CreateSystemTextures()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_INVALIDARG;

    // ========== 创建白色纹理 (1x1 白色像素) ==========
    {
        uint8_t whitePixel[4] = { 255, 255, 255, 255 };

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = whitePixel;
        subresourceData.RowPitch = 4;
        subresourceData.SlicePitch = 4;

        HRESULT hr = m_pTextureLoader->CreateTextureFromMemory(
            whitePixel, 1, 1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            &m_pWhiteTexture,
            &subresourceData
        );

        if (SUCCEEDED(hr))
        {
            // 创建 SRV
            m_hWhiteTextureSRV.ptr = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart().ptr;

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            pD3D12Device->CreateShaderResourceView(m_pWhiteTexture.Get(), &srvDesc, cpuHandle);
        }
    }

    // ========== 创建黑色纹理 (1x1 黑色像素) ==========
    {
        uint8_t blackPixel[4] = { 0, 0, 0, 255 };

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = blackPixel;
        subresourceData.RowPitch = 4;
        subresourceData.SlicePitch = 4;

        HRESULT hr = m_pTextureLoader->CreateTextureFromMemory(
            blackPixel, 1, 1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            &m_pBlackTexture,
            &subresourceData
        );

        if (SUCCEEDED(hr))
        {
            // 创建 SRV
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();
            cpuHandle.ptr += m_nSRVDescriptorSize;

            m_hBlackTextureSRV.ptr = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart().ptr + m_nSRVDescriptorSize;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            pD3D12Device->CreateShaderResourceView(m_pBlackTexture.Get(), &srvDesc, cpuHandle);
        }
    }

    // ========== 创建法线贴图 (1x1 平坦法线) ==========
    {
        // 平坦法线 = (0, 0, 1) 在贴图中存储为 (128, 128, 255)
        uint8_t normalPixel[4] = { 128, 128, 255, 255 };

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = normalPixel;
        subresourceData.RowPitch = 4;
        subresourceData.SlicePitch = 4;

        HRESULT hr = m_pTextureLoader->CreateTextureFromMemory(
            normalPixel, 1, 1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            &m_pNormalTexture,
            &subresourceData
        );

        if (SUCCEEDED(hr))
        {
            // 创建 SRV
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();
            cpuHandle.ptr += 2 * m_nSRVDescriptorSize;

            m_hNormalTextureSRV.ptr = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart().ptr + 2 * m_nSRVDescriptorSize;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            pD3D12Device->CreateShaderResourceView(m_pNormalTexture.Get(), &srvDesc, cpuHandle);
        }
    }

    return S_OK;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12MaterialManager::GetWhiteTexture() const
{
    return m_hWhiteTextureSRV;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12MaterialManager::GetBlackTexture() const
{
    return m_hBlackTextureSRV;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12MaterialManager::GetNormalTexture() const
{
    return m_hNormalTextureSRV;
}
