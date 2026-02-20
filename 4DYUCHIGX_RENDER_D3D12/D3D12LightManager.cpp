// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12LightManager.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <cstring>

// 辅助宏

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12LightManager"); \
    }

// ============================================
// 构造函数和析构函数
// ============================================

D3D12LightManager::D3D12LightManager()
{
    m_pDevice = nullptr;
    m_numDirectionalLights = 0;
    m_numPointLights = 0;
    m_numSpotLights = 0;
    m_dwAmbientColor = 0xFF202020; // 默认深灰色环境光
    m_pMappedLightConstants = nullptr;

    // 初始化所有光源为禁用状态
    for (UINT i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
    {
        m_directionalLights[i] = D3D12DirectionalLight();
        m_directionalLights[i].bEnable = FALSE;
    }

    for (UINT i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        m_pointLights[i] = D3D12PointLight();
        m_pointLights[i].bEnable = FALSE;
    }

    for (UINT i = 0; i < MAX_SPOT_LIGHTS; ++i)
    {
        m_spotLights[i] = D3D12SpotLight();
        m_spotLights[i].bEnable = FALSE;
    }

    // 初始化常量数据
    ZeroMemory(&m_lightConstants, sizeof(m_lightConstants));
}

D3D12LightManager::~D3D12LightManager()
{
    Release();
}

// ============================================
// 初始化和释放
// ============================================

HRESULT D3D12LightManager::Initialize(CoD3D12Device* pDevice)
{
    if (pDevice == nullptr)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    try {
        // 创建光照常量缓冲区
        DX12_THROW_IF_FAILED(CreateConstantBuffer());

        // 初始化常量数据
        UpdateLightConstants();

        // 上传初始数据
        UploadConstantBufferData();

    }
    catch (const std::exception& e) {
        // 记录错误到调试输出
        OutputDebugStringA("D3D12LightManager::Initialize failed: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
        return E_FAIL;
    }

    return S_OK;
}

void D3D12LightManager::Release()
{
    // 取消映射常量缓冲区
    if (m_pMappedLightConstants != nullptr)
    {
        m_pLightConstantBuffer->Unmap(0, nullptr);
        m_pMappedLightConstants = nullptr;
    }

    // 释放常量缓冲区
    m_pLightConstantBuffer.Reset();

    // 重置状态
    m_numDirectionalLights = 0;
    m_numPointLights = 0;
    m_numSpotLights = 0;
}

// ============================================
// 方向光
// ============================================

HRESULT D3D12LightManager::SetDirectionalLight(UINT index, const DIRECTIONAL_LIGHT_DESC* pDesc)
{
    if (pDesc == nullptr || index >= MAX_DIRECTIONAL_LIGHTS)
        return E_INVALIDARG;

    D3D12DirectionalLight& light = m_directionalLights[index];

    // 复制光源属性
    light.v3Direction = DirectX::XMFLOAT3(pDesc->v3Dir.x, pDesc->v3Dir.y, pDesc->v3Dir.z);
    light.v4Ambient = DirectX::XMFLOAT4(
        ((pDesc->dwAmbient >> 16) & 0xFF) / 255.0f,
        ((pDesc->dwAmbient >> 8) & 0xFF) / 255.0f,
        ((pDesc->dwAmbient) & 0xFF) / 255.0f,
        1.0f);
    light.v4Diffuse = DirectX::XMFLOAT4(
        ((pDesc->dwDiffuse >> 16) & 0xFF) / 255.0f,
        ((pDesc->dwDiffuse >> 8) & 0xFF) / 255.0f,
        ((pDesc->dwDiffuse) & 0xFF) / 255.0f,
        1.0f);
    light.v4Specular = DirectX::XMFLOAT4(
        ((pDesc->dwSpecular >> 16) & 0xFF) / 255.0f,
        ((pDesc->dwSpecular >> 8) & 0xFF) / 255.0f,
        ((pDesc->dwSpecular) & 0xFF) / 255.0f,
        1.0f);
    light.bEnable = pDesc->bEnable;

    // 更新计数
    if (light.bEnable && index >= m_numDirectionalLights)
    {
        m_numDirectionalLights = index + 1;
    }

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();

    return S_OK;
}

void D3D12LightManager::EnableDirectionalLight(const DIRECTIONAL_LIGHT_DESC* pDesc, DWORD dwFlag)
{
    if (pDesc == nullptr)
        return;

    // 查找空闲槽位
    UINT index = 0;
    for (index = 0; index < MAX_DIRECTIONAL_LIGHTS; ++index)
    {
        if (!m_directionalLights[index].bEnable)
            break;
    }

    if (index < MAX_DIRECTIONAL_LIGHTS)
    {
        SetDirectionalLight(index, pDesc);
    }
}

void D3D12LightManager::DisableDirectionalLight()
{
    // 禁用所有方向光
    for (UINT i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
    {
        m_directionalLights[i].bEnable = FALSE;
    }
    m_numDirectionalLights = 0;

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();
}

// ============================================
// 点光源
// ============================================

DWORD D3D12LightManager::CreateDynamicLight(DWORD dwRS, DWORD dwColor, char* szFileName)
{
    // 查找空闲槽位
    UINT index = 0;
    for (index = 0; index < MAX_POINT_LIGHTS; ++index)
    {
        if (!m_pointLights[index].bEnable)
            break;
    }

    if (index >= MAX_POINT_LIGHTS)
        return 0xFFFFFFFF;

    // 创建点光源
    D3D12PointLight& light = m_pointLights[index];

    light.v3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);  // 默认位置
    light.fRange = dwRS;  // 使用提供的范围

    // 设置颜色
    light.v4Diffuse = DirectX::XMFLOAT4(
        ((dwColor >> 16) & 0xFF) / 255.0f,
        ((dwColor >> 8) & 0xFF) / 255.0f,
        ((dwColor) & 0xFF) / 255.0f,
        1.0f);
    light.v4Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    light.v4Specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    // 设置衰减
    light.v3Attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

    light.bEnable = TRUE;

    // 更新计数
    if (index >= m_numPointLights)
    {
        m_numPointLights = index + 1;
    }

    // 从文件加载光照纹理
    if (szFileName != nullptr)
    {
        LoadLightTexture(index, szFileName);
    }

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();

    return index;
}

BOOL D3D12LightManager::DeleteDynamicLight(DWORD dwIndex)
{
    if (dwIndex >= MAX_POINT_LIGHTS)
        return FALSE;

    m_pointLights[dwIndex].bEnable = FALSE;

    // 重新计算数量
    m_numPointLights = 0;
    for (UINT i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (m_pointLights[i].bEnable)
            m_numPointLights = i + 1;
    }

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();

    return TRUE;
}

BOOL D3D12LightManager::SetRTLight(LIGHT_DESC* pLightDesc, DWORD dwLightIndex, DWORD dwFlag)
{
    if (pLightDesc == nullptr || dwLightIndex >= MAX_POINT_LIGHTS)
        return FALSE;

    D3D12PointLight& light = m_pointLights[dwLightIndex];

    // 复制光源属性
    light.v3Position = DirectX::XMFLOAT3(
        pLightDesc->v3Point.x,
        pLightDesc->v3Point.y,
        pLightDesc->v3Point.z);
    light.fRange = pLightDesc->fRs;

    XMStoreFloat4(&light.v4Ambient, ColorToXMVECTOR(pLightDesc->dwAmbient));
    XMStoreFloat4(&light.v4Diffuse, ColorToXMVECTOR(pLightDesc->dwDiffuse));
    XMStoreFloat4(&light.v4Specular, ColorToXMVECTOR(pLightDesc->dwSpecular));

    // 实现衰减参数
    // 标准衰减公式：Att = 1 / (A0 + A1*d + A2*d^2)
    // A0 = 常数衰减, A1 = 线性衰减, A2 = 二次衰减
    float fRange = light.fRange;
    if (fRange > 0.0f)
    {
        // 根据范围计算合适的衰减参数
        light.v3Attenuation.x = 1.0f;           // 常数衰减
        light.v3Attenuation.y = 0.0f;           // 线性衰减
        light.v3Attenuation.z = 1.0f / (fRange * fRange);  // 二次衰减
    }
    else
    {
        light.v3Attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    }

    light.bEnable = TRUE;

    // 更新计数
    if (dwLightIndex >= m_numPointLights)
    {
        m_numPointLights = dwLightIndex + 1;
    }

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();

    return TRUE;
}

// ============================================
// 聚光灯
// ============================================

void D3D12LightManager::SetSpotLightDesc(VECTOR3* pv3From, VECTOR3* pv3To, VECTOR3* pv3Up,
                                         float fFov, float fNear, float fFar, float fWidth,
                                         BOOL bOrtho, void* pMtlHandle, DWORD dwColorOP,
                                         DWORD dwLightIndex, SPOT_LIGHT_TYPE type)
{
    if (pv3From == nullptr || pv3To == nullptr || pv3Up == nullptr)
        return;

    if (dwLightIndex >= MAX_SPOT_LIGHTS)
        return;

    D3D12SpotLight& light = m_spotLights[dwLightIndex];

    // 设置位置和方向
    light.v3Position = DirectX::XMFLOAT3(pv3From->x, pv3From->y, pv3From->z);

    DirectX::XMFLOAT3 to = DirectX::XMFLOAT3(pv3To->x, pv3To->y, pv3To->z);
    DirectX::XMFLOAT3 from = DirectX::XMFLOAT3(pv3From->x, pv3From->y, pv3From->z);

    // 计算方向
    DirectX::XMVECTOR vFrom = XMLoadFloat3(&from);
    DirectX::XMVECTOR vTo = XMLoadFloat3(&to);
    DirectX::XMVECTOR vDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vTo, vFrom));
    XMStoreFloat3(&light.v3Direction, vDir);

    // 设置锥角
    light.fSpot = cosf(fFov * 0.5f);

    // 实现其他参数
    // 根据远平面设置范围
    light.fRange = fFar > 0.0f ? fFar : 100.0f;

    // 计算衰减参数
    if (light.fRange > 0.0f)
    {
        light.v3Attenuation.x = 1.0f;
        light.v3Attenuation.y = 0.0f;
        light.v3Attenuation.z = 1.0f / (light.fRange * light.fRange);
    }
    else
    {
        light.v3Attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    }

    // 设置颜色（从 dwColorOP）
    XMStoreFloat4(&light.v4Diffuse, ColorToXMVECTOR(dwColorOP));
    light.v4Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    light.v4Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    light.bEnable = TRUE;

    // 更新计数
    if (dwLightIndex >= m_numSpotLights)
    {
        m_numSpotLights = dwLightIndex + 1;
    }

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();
}

void D3D12LightManager::SetShadowLightSenderPosition(BOUNDING_SPHERE* pSphere, DWORD dwLightIndex)
{
    if (pSphere == nullptr || dwLightIndex >= MAX_SPOT_LIGHTS)
        return;

    D3D12SpotLight& light = m_spotLights[dwLightIndex];

    // 设置光源位置
    light.v3Position = DirectX::XMFLOAT3(
        pSphere->v3Point.x,
        pSphere->v3Point.y,
        pSphere->v3Point.z);

    // 设置范围
    light.fRange = pSphere->fRs;

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();
}

// ============================================
// 全局环境光
// ============================================

void D3D12LightManager::SetAmbientColor(DWORD dwColor)
{
    m_dwAmbientColor = dwColor;

    // 更新常量数据
    m_lightConstants.globalAmbient = DirectX::XMFLOAT3(
        ((dwColor >> 16) & 0xFF) / 255.0f,
        ((dwColor >> 8) & 0xFF) / 255.0f,
        ((dwColor) & 0xFF) / 255.0f);

    // 上传到 GPU
    UploadConstantBufferData();
}

// ============================================
// 渲染支持
// ============================================

void D3D12LightManager::UpdateConstantBuffer()
{
    // 更新常量数据
    UpdateLightConstants();

    // 上传到 GPU
    UploadConstantBufferData();
}

// ============================================
// 私有辅助方法
// ============================================

void D3D12LightManager::UpdateLightConstants()
{
    // 复制方向光数据
    for (UINT i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
    {
        m_lightConstants.directionalDirection[i] = m_directionalLights[i].v3Direction;
        m_lightConstants.directionalAmbient[i] = m_directionalLights[i].v4Ambient;
        m_lightConstants.directionalDiffuse[i] = m_directionalLights[i].v4Diffuse;
        m_lightConstants.directionalSpecular[i] = m_directionalLights[i].v4Specular;
        m_lightConstants.directionalEnabled[i] = m_directionalLights[i].bEnable ? 1 : 0;
    }

    // 复制点光源数据
    for (UINT i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        m_lightConstants.pointPosition[i] = m_pointLights[i].v3Position;
        m_lightConstants.pointRange[i] = m_pointLights[i].fRange;
        m_lightConstants.pointAmbient[i] = m_pointLights[i].v4Ambient;
        m_lightConstants.pointDiffuse[i] = m_pointLights[i].v4Diffuse;
        m_lightConstants.pointSpecular[i] = m_pointLights[i].v4Specular;
        m_lightConstants.pointAttenuation[i] = m_pointLights[i].v3Attenuation;
        m_lightConstants.pointEnabled[i] = m_pointLights[i].bEnable ? 1 : 0;
    }

    // 复制聚光灯数据
    for (UINT i = 0; i < MAX_SPOT_LIGHTS; ++i)
    {
        m_lightConstants.spotPosition[i] = m_spotLights[i].v3Position;
        m_lightConstants.spotRange[i] = m_spotLights[i].fRange;
        m_lightConstants.spotDirection[i] = m_spotLights[i].v3Direction;
        m_lightConstants.spotSpot[i] = m_spotLights[i].fSpot;
        m_lightConstants.spotAmbient[i] = m_spotLights[i].v4Ambient;
        m_lightConstants.spotDiffuse[i] = m_spotLights[i].v4Diffuse;
        m_lightConstants.spotSpecular[i] = m_spotLights[i].v4Specular;
        m_lightConstants.spotAttenuation[i] = m_spotLights[i].v3Attenuation;
        m_lightConstants.spotEnabled[i] = m_spotLights[i].bEnable ? 1 : 0;
    }

    // 复制全局环境光
    m_lightConstants.globalAmbient = DirectX::XMFLOAT3(
        ((m_dwAmbientColor >> 16) & 0xFF) / 255.0f,
        ((m_dwAmbientColor >> 8) & 0xFF) / 255.0f,
        ((m_dwAmbientColor) & 0xFF) / 255.0f);

    // 复制光源数量
    m_lightConstants.numDirectionalLights = m_numDirectionalLights;
    m_lightConstants.numPointLights = m_numPointLights;
    m_lightConstants.numSpotLights = m_numSpotLights;
}

HRESULT D3D12LightManager::CreateConstantBuffer()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 计算常量缓冲区大小（256 字节对齐）
    UINT bufferSize = sizeof(LightConstants);
    bufferSize = (bufferSize + 255) & ~255;  // 对齐到 256 字节

    // 创建常量缓冲区
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = pD3D12Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pLightConstantBuffer));

    if (FAILED(hr))
        return hr;

    // 映射常量缓冲区
    D3D12_RANGE readRange = { 0, 0 };
    hr = m_pLightConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pMappedLightConstants));

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void D3D12LightManager::UploadConstantBufferData()
{
    if (m_pMappedLightConstants == nullptr)
        return;

    // 复制数据到映射的常量缓冲区
    memcpy(m_pMappedLightConstants, &m_lightConstants, sizeof(LightConstants));
}

// ============================================
// 静态辅助方法
// ============================================

DirectX::XMVECTOR D3D12LightManager::ColorToXMVECTOR(DWORD dwColor)
{
    // DWORD 颜色格式：ARGB
    float a = ((dwColor >> 24) & 0xFF) / 255.0f;
    float r = ((dwColor >> 16) & 0xFF) / 255.0f;
    float g = ((dwColor >> 8) & 0xFF) / 255.0f;
    float b = (dwColor & 0xFF) / 255.0f;

    return DirectX::XMVectorSet(r, g, b, a);
}

DWORD D3D12LightManager::XMVECTORToColor(DirectX::XMVECTOR color)
{
    float r, g, b, a;
    DirectX::XMStoreFloat4(&DirectX::XMFLOAT4(r, g, b, a), color);

    DWORD dwR = static_cast<DWORD>(r * 255.0f);
    DWORD dwG = static_cast<DWORD>(g * 255.0f);
    DWORD dwB = static_cast<DWORD>(b * 255.0f);
    DWORD dwA = static_cast<DWORD>(a * 255.0f);

    return (dwA << 24) | (dwR << 16) | (dwG << 8) | dwB;
}

// ============================================
// 光照纹理加载
// ============================================

void D3D12LightManager::LoadLightTexture(UINT lightIndex, const char* szFileName)
{
    if (lightIndex >= MAX_POINT_LIGHTS || szFileName == nullptr)
        return;

    D3D12PointLight& light = m_pointLights[lightIndex];

    // 加载光照纹理（使用纹理加载器）
    // 这里简化处理，实际应该通过D3D12TextureLoader加载
    // 并存储在light.pLightTexture中

    // 标记纹理已加载
    light.bHasTexture = TRUE;

    // 记录文件名
    // size_t len = strlen(szFileName) + 1;
    // light.szTextureFileName = new char[len];
    // strcpy_s(light.szTextureFileName, len, szFileName);

    // 注意：实际纹理加载应该在渲染管线中处理
    // 这里只是标记光源有纹理附件
}

void D3D12LightManager::ReleaseLightTexture(UINT lightIndex)
{
    if (lightIndex >= MAX_POINT_LIGHTS)
        return;

    D3D12PointLight& light = m_pointLights[lightIndex];

    // 释放纹理资源
    if (light.pLightTexture)
    {
        light.pLightTexture.Reset();
        light.bHasTexture = FALSE;
    }
}

ID3D12Resource* D3D12LightManager::GetLightTexture(UINT lightIndex) const
{
    if (lightIndex >= MAX_POINT_LIGHTS)
        return nullptr;

    return m_pointLights[lightIndex].pLightTexture.Get();
}

BOOL D3D12LightManager::HasLightTexture(UINT lightIndex) const
{
    if (lightIndex >= MAX_POINT_LIGHTS)
        return FALSE;

    return m_pointLights[lightIndex].bHasTexture;
}

void D3D12LightManager::SetAttenuationParams(UINT lightIndex, float fConstant, float fLinear, float fQuadratic)
{
    if (lightIndex >= MAX_POINT_LIGHTS)
        return;

    D3D12PointLight& light = m_pointLights[lightIndex];

    // 设置衰减参数
    light.v3Attenuation.x = fConstant;
    light.v3Attenuation.y = fLinear;
    light.v3Attenuation.z = fQuadratic;

    // 更新常量缓冲区
    UpdateLightConstants();
    UploadConstantBufferData();
}

void D3D12LightManager::GetAttenuationParams(UINT lightIndex, float* pConstant, float* pLinear, float* pQuadratic) const
{
    if (lightIndex >= MAX_POINT_LIGHTS)
        return;

    const D3D12PointLight& light = m_pointLights[lightIndex];

    if (pConstant)
        *pConstant = light.v3Attenuation.x;
    if (pLinear)
        *pLinear = light.v3Attenuation.y;
    if (pQuadratic)
        *pQuadratic = light.v3Attenuation.z;
}
