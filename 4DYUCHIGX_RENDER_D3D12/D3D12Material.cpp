#include "D3D12Material.h"
#include "CoD3D12Device.h"
#include <algorithm>

// 辅助宏
#include "D3D12PCH.h"

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12Material"); \
    }

// ============================================
// 构造函数和析构函数
// ============================================

D3D12Material::D3D12Material()
{
    // 材质属性
    m_dwDiffuse = 0xFFFFFFFF;       // 白色
    m_dwAmbient = 0xFF202020;       // 深灰色
    m_dwSpecular = 0xFFFFFFFF;      // 白色
    m_fTransparency = 1.0f;         // 不透明
    m_fShine = 0.0f;
    m_fShineStrength = 0.0f;

    m_dwFlag = 0;
    m_dwMaterialID = 0xFFFFFFFF;

    // 纹理名称
    memset(m_szDiffuseTexName, 0, sizeof(m_szDiffuseTexName));
    memset(m_szNormalTexName, 0, sizeof(m_szNormalTexName));
    memset(m_szSpecularTexName, 0, sizeof(m_szSpecularTexName));

    // SRV 句柄
    m_hDiffuseSRV.ptr = 0;
    m_hNormalSRV.ptr = 0;
    m_hSpecularSRV.ptr = 0;

    // 引用计数
    m_dwRefCount = 1;

    // 初始化常量
    UpdateConstants();
}

D3D12Material::~D3D12Material()
{
    // ComPtr 自动释放资源
}

// ============================================
// 初始化
// ============================================

HRESULT D3D12Material::Initialize(CoD3D12Device* pDevice, const MATERIAL* pMtl)
{
    if (pMtl == nullptr)
        return E_INVALIDARG;

    // 复制材质属性（使用const_cast因为MATERIAL的方法不是const的）
    MATERIAL* pMtlNonConst = const_cast<MATERIAL*>(pMtl);
    m_dwDiffuse = pMtlNonConst->GetDiffuse();
    m_dwAmbient = pMtlNonConst->GetAmbient();
    m_dwSpecular = pMtlNonConst->GetSpecular();
    m_fTransparency = pMtlNonConst->GetTransparency();
    m_fShine = pMtlNonConst->GetShine();
    m_fShineStrength = pMtlNonConst->GetShineStrength();
    m_dwFlag = pMtlNonConst->GetFlag();

    // 复制纹理名称
    if (strlen(pMtlNonConst->GetDiffuseTexmapName()) > 0)
    {
        strcpy_s(m_szDiffuseTexName, MAX_NAME_LEN, pMtlNonConst->GetDiffuseTexmapName());
    }
    if (strlen(pMtlNonConst->GetReflectTexmapName()) > 0)
    {
        strcpy_s(m_szSpecularTexName, MAX_NAME_LEN, pMtlNonConst->GetReflectTexmapName());
    }
    if (strlen(pMtlNonConst->GetBumpTexmapName()) > 0)
    {
        strcpy_s(m_szNormalTexName, MAX_NAME_LEN, pMtlNonConst->GetBumpTexmapName());
    }

    // 更新常量数据
    UpdateConstants();

    // 创建常量缓冲区资源
    ID3D12Device* pD3D12Device = pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_INVALIDARG;

    // 创建上传堆常量缓冲区（256字节，对齐到256字节边界）
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = 256;  // 256字节对齐
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = pD3D12Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pConstantBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 映射常量缓冲区并上传初始数据
    D3D12_RANGE readRange = {0, 0};
    void* pData = nullptr;
    hr = m_pConstantBuffer->Map(0, &readRange, &pData);
    if (SUCCEEDED(hr))
    {
        memcpy(pData, &m_constants, sizeof(MaterialConstants));
        m_pConstantBuffer->Unmap(0, nullptr);
    }

    return hr;
}

// ============================================
// 纹理加载
// ============================================

HRESULT D3D12Material::LoadDiffuseTexture(CoD3D12Device* pDevice, const char* szFileName)
{
    if (pDevice == nullptr || szFileName == nullptr)
        return E_INVALIDARG;

    // TODO: 第 2 周实现 - 纹理加载
    // 1. 加载纹理文件（DDS, TGA, BMP 等）
    // 2. 创建 D3D12 纹理资源
    // 3. 创建 SRV
    // 4. 保存纹理和 SRV 句柄

    strcpy_s(m_szDiffuseTexName, MAX_NAME_LEN, szFileName);

    return S_OK;
}

HRESULT D3D12Material::LoadNormalTexture(CoD3D12Device* pDevice, const char* szFileName)
{
    if (pDevice == nullptr || szFileName == nullptr)
        return E_INVALIDARG;

    // TODO: 第 2 周实现 - 法线贴图加载

    strcpy_s(m_szNormalTexName, MAX_NAME_LEN, szFileName);

    return S_OK;
}

HRESULT D3D12Material::LoadSpecularTexture(CoD3D12Device* pDevice, const char* szFileName)
{
    if (pDevice == nullptr || szFileName == nullptr)
        return E_INVALIDARG;

    // TODO: 第 2 周实现 - 镜面反射纹理加载

    strcpy_s(m_szSpecularTexName, MAX_NAME_LEN, szFileName);

    return S_OK;
}

// ============================================
// 直接设置纹理（用于材质管理器）
// ============================================

void D3D12Material::SetDiffuseTexture(ID3D12Resource* pTexture, D3D12_GPU_DESCRIPTOR_HANDLE srv)
{
    m_pDiffuseTexture = pTexture;
    m_hDiffuseSRV = srv;
}

void D3D12Material::SetNormalTexture(ID3D12Resource* pTexture, D3D12_GPU_DESCRIPTOR_HANDLE srv)
{
    m_pNormalTexture = pTexture;
    m_hNormalSRV = srv;
}

// ============================================
// 渲染绑定
// ============================================

void D3D12Material::Bind(ID3D12GraphicsCommandList* pCommandList,
                         D3D12_GPU_DESCRIPTOR_HANDLE cbvHeapStart,
                         UINT cbvSRVDescriptorSize)
{
    if (pCommandList == nullptr)
        return;

    // TODO: 第 2 周实现 - 绑定材质到管线
    // 1. 设置常量缓冲区
    // 2. 设置纹理 SRV
    // 3. 设置采样器

    // 示例：绑定漫反射纹理（如果有）
    if (m_pDiffuseTexture != nullptr && m_hDiffuseSRV.ptr != 0)
    {
        // pCommandList->SetGraphicsRootDescriptorTable(0, m_hDiffuseSRV);
    }
}

void D3D12Material::UpdateConstantBuffer(ID3D12GraphicsCommandList* pCommandList)
{
    if (m_pConstantBuffer == nullptr)
        return;

    // 映射并更新常量缓冲区
    D3D12_RANGE readRange = {0, 0};
    void* pData = nullptr;

    if (SUCCEEDED(m_pConstantBuffer->Map(0, &readRange, &pData)))
    {
        memcpy(pData, &m_constants, sizeof(MaterialConstants));
        m_pConstantBuffer->Unmap(0, nullptr);
    }
}

void D3D12Material::BindToPipeline(ID3D12GraphicsCommandList* pCommandList)
{
    if (pCommandList == nullptr)
        return;

    // 更新常量缓冲区数据
    UpdateConstantBuffer(pCommandList);

    // 绑定材质常量缓冲区（根签名参数索引1）
    if (m_pConstantBuffer != nullptr)
    {
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = m_pConstantBuffer->GetGPUVirtualAddress();
        pCommandList->SetGraphicsRootConstantBufferView(1, cbvAddress);
    }

    // 注意：纹理SRV绑定需要在外部根据根签名布局设置
    // 这里我们只设置常量缓冲区
}

// ============================================
// 引用计数
// ============================================

DWORD D3D12Material::AddRef()
{
    return InterlockedIncrement(&m_dwRefCount);
}

DWORD D3D12Material::Release()
{
    DWORD dwRefCount = InterlockedDecrement(&m_dwRefCount);
    if (dwRefCount == 0)
    {
        delete this;
    }
    return dwRefCount;
}

// ============================================
// 私有辅助方法
// ============================================

void D3D12Material::UpdateConstants()
{
    // 漫反射颜色
    XMStoreFloat4(&m_constants.diffuse, ColorToXMVECTOR(m_dwDiffuse));

    // 环境光颜色
    XMStoreFloat4(&m_constants.ambient, ColorToXMVECTOR(m_dwAmbient));

    // 镜面反射颜色
    XMStoreFloat4(&m_constants.specular, ColorToXMVECTOR(m_dwSpecular));

    // 其他属性
    m_constants.transparency = m_fTransparency;
    m_constants.shininess = m_fShine;
    m_constants.shininessStrength = m_fShineStrength;
    m_constants.padding = 0.0f;
}

DirectX::XMVECTOR D3D12Material::ColorToXMVECTOR(DWORD dwColor)
{
    // DWORD 颜色格式：ARGB
    float a = ((dwColor >> 24) & 0xFF) / 255.0f;
    float r = ((dwColor >> 16) & 0xFF) / 255.0f;
    float g = ((dwColor >> 8) & 0xFF) / 255.0f;
    float b = (dwColor & 0xFF) / 255.0f;

    return DirectX::XMVectorSet(r, g, b, a);
}
