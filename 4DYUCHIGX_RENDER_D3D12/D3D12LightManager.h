#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <vector>

#include "../4DyuchiGRX_common/typedef.h"



// 前向声明
class CoD3D12Device;

// 光照常量定义
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 16

// ============================================
// 光源结构体（C++ 端）
// ============================================

// 方向光
struct D3D12DirectionalLight
{
    DirectX::XMFLOAT3 v3Direction;
    float padding1;
    DirectX::XMFLOAT4 v4Ambient;
    DirectX::XMFLOAT4 v4Diffuse;
    DirectX::XMFLOAT4 v4Specular;
    BOOL bEnable;
    DirectX::XMFLOAT3 padding2;

    D3D12DirectionalLight()
    {
        v3Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
        padding1 = 0.0f;
        v4Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        v4Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        v4Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        bEnable = FALSE;
        padding2 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
};

// 点光源
struct D3D12PointLight
{
    DirectX::XMFLOAT3 v3Position;
    float fRange;
    DirectX::XMFLOAT4 v4Ambient;
    DirectX::XMFLOAT4 v4Diffuse;
    DirectX::XMFLOAT4 v4Specular;
    DirectX::XMFLOAT3 v3Attenuation;  // x=常数, y=线性, z=二次
    BOOL bEnable;
    BOOL bHasTexture;               // 是否有纹理附件
    Microsoft::WRL::ComPtr<ID3D12Resource> pLightTexture;  // 光照纹理

    D3D12PointLight()
    {
        v3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        fRange = 100.0f;
        v4Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        v4Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        v4Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        v3Attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        bEnable = FALSE;
        bHasTexture = FALSE;
    }
};

// 聚光灯
struct D3D12SpotLight
{
    DirectX::XMFLOAT3 v3Position;
    float fRange;
    DirectX::XMFLOAT3 v3Direction;
    float fSpot;        // cos(spotAngle/2)
    DirectX::XMFLOAT4 v4Ambient;
    DirectX::XMFLOAT4 v4Diffuse;
    DirectX::XMFLOAT4 v4Specular;
    DirectX::XMFLOAT3 v3Attenuation;
    BOOL bEnable;

    D3D12SpotLight()
    {
        v3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        fRange = 100.0f;
        v3Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
        fSpot = 0.9f;  // 约 25 度
        v4Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        v4Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        v4Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        v3Attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        bEnable = FALSE;
    }
};

// 光照常量缓冲区数据（与 HLSL 对齐）
struct LightConstants
{
    // 方向光数组
    DirectX::XMFLOAT3 directionalDirection[MAX_DIRECTIONAL_LIGHTS];
    float directionalPadding1[MAX_DIRECTIONAL_LIGHTS];
    DirectX::XMFLOAT4 directionalAmbient[MAX_DIRECTIONAL_LIGHTS];
    DirectX::XMFLOAT4 directionalDiffuse[MAX_DIRECTIONAL_LIGHTS];
    DirectX::XMFLOAT4 directionalSpecular[MAX_DIRECTIONAL_LIGHTS];
    int directionalEnabled[MAX_DIRECTIONAL_LIGHTS];
    DirectX::XMFLOAT3 directionalPadding2[MAX_DIRECTIONAL_LIGHTS];

    // 点光源数组
    DirectX::XMFLOAT3 pointPosition[MAX_POINT_LIGHTS];
    float pointRange[MAX_POINT_LIGHTS];
    DirectX::XMFLOAT4 pointAmbient[MAX_POINT_LIGHTS];
    DirectX::XMFLOAT4 pointDiffuse[MAX_POINT_LIGHTS];
    DirectX::XMFLOAT4 pointSpecular[MAX_POINT_LIGHTS];
    DirectX::XMFLOAT3 pointAttenuation[MAX_POINT_LIGHTS];
    int pointEnabled[MAX_POINT_LIGHTS];

    // 聚光灯数组
    DirectX::XMFLOAT3 spotPosition[MAX_SPOT_LIGHTS];
    float spotRange[MAX_SPOT_LIGHTS];
    DirectX::XMFLOAT3 spotDirection[MAX_SPOT_LIGHTS];
    float spotSpot[MAX_SPOT_LIGHTS];
    DirectX::XMFLOAT4 spotAmbient[MAX_SPOT_LIGHTS];
    DirectX::XMFLOAT4 spotDiffuse[MAX_SPOT_LIGHTS];
    DirectX::XMFLOAT4 spotSpecular[MAX_SPOT_LIGHTS];
    DirectX::XMFLOAT3 spotAttenuation[MAX_SPOT_LIGHTS];
    int spotEnabled[MAX_SPOT_LIGHTS];

    // 全局环境光
    DirectX::XMFLOAT3 globalAmbient;
    float padding1;

    // 光源数量
    UINT numDirectionalLights;
    UINT numPointLights;
    UINT numSpotLights;
    float padding2;
};

// ============================================
// 光照管理器
// ============================================

class D3D12LightManager
{
public:
    D3D12LightManager();
    virtual ~D3D12LightManager();

    // ========== 初始化 ==========

    // 初始化光照管理器
    HRESULT Initialize(CoD3D12Device* pDevice);

    // 释放所有资源
    void Release();

    // ========== 方向光 ==========

    // 设置方向光
    HRESULT SetDirectionalLight(UINT index, const DIRECTIONAL_LIGHT_DESC* pDesc);

    // 启用方向光
    void EnableDirectionalLight(const DIRECTIONAL_LIGHT_DESC* pDesc, DWORD dwFlag);

    // 禁用方向光
    void DisableDirectionalLight();

    // 获取方向光数量
    UINT GetDirectionalLightCount() const { return m_numDirectionalLights; }

    // ========== 点光源 ==========

    // 创建动态点光源
    DWORD CreateDynamicLight(DWORD dwRS, DWORD dwColor, char* szFileName);

    // 删除动态光源
    BOOL DeleteDynamicLight(DWORD dwIndex);

    // 设置实时光源
    BOOL SetRTLight(LIGHT_DESC* pLightDesc, DWORD dwLightIndex, DWORD dwFlag);

    // 获取点光源数量
    UINT GetPointLightCount() const { return m_numPointLights; }

    // ========== 聚光灯 ==========

    // 设置聚光灯描述
    void SetSpotLightDesc(VECTOR3* pv3From, VECTOR3* pv3To, VECTOR3* pv3Up,
                          float fFov, float fNear, float fFar, float fWidth,
                          BOOL bOrtho, void* pMtlHandle, DWORD dwColorOP,
                          DWORD dwLightIndex, SPOT_LIGHT_TYPE type);

    // 设置阴影光源位置
    void SetShadowLightSenderPosition(BOUNDING_SPHERE* pSphere, DWORD dwLightIndex);

    // 获取聚光灯数量
    UINT GetSpotLightCount() const { return m_numSpotLights; }

    // ========== 全局环境光 ==========

    // 设置环境光颜色
    void SetAmbientColor(DWORD dwColor);

    // 获取环境光颜色
    DWORD GetAmbientColor() const { return m_dwAmbientColor; }

    // ========== 渲染支持 ==========

    // 更新常量缓冲区
    void UpdateConstantBuffer();

    // 获取常量缓冲区资源
    ID3D12Resource* GetConstantBuffer() const { return m_pLightConstantBuffer.Get(); }

    // 获取常量数据
    const LightConstants& GetLightConstants() const { return m_lightConstants; }

    // ========== 辅助方法 ==========

    // 颜色转换：DWORD to XMVECTOR
    static DirectX::XMVECTOR ColorToXMVECTOR(DWORD dwColor);

    // 颜色转换：XMVECTOR to DWORD
    static DWORD XMVECTORToColor(DirectX::XMVECTOR color);

    // ========== 光照纹理管理 ==========

    // 加载光照纹理
    void LoadLightTexture(UINT lightIndex, const char* szFileName);

    // 释放光照纹理
    void ReleaseLightTexture(UINT lightIndex);

    // 获取光照纹理
    ID3D12Resource* GetLightTexture(UINT lightIndex) const;

    // 检查是否有纹理
    BOOL HasLightTexture(UINT lightIndex) const;

    // ========== 衰减参数 ==========

    // 设置衰减参数
    void SetAttenuationParams(UINT lightIndex, float fConstant, float fLinear, float fQuadratic);

    // 获取衰减参数
    void GetAttenuationParams(UINT lightIndex, float* pConstant, float* pLinear, float* pQuadratic) const;

private:
    // ========== 辅助方法 ==========

    // 更新常量数据结构
    void UpdateLightConstants();

    // 创建常量缓冲区资源
    HRESULT CreateConstantBuffer();

    // 上传常量数据到 GPU
    void UploadConstantBufferData();

    // ========== 光源实例 ==========

    // 方向光数组
    D3D12DirectionalLight m_directionalLights[MAX_DIRECTIONAL_LIGHTS];
    UINT m_numDirectionalLights;

    // 点光源数组
    D3D12PointLight m_pointLights[MAX_POINT_LIGHTS];
    UINT m_numPointLights;

    // 聚光灯数组
    D3D12SpotLight m_spotLights[MAX_SPOT_LIGHTS];
    UINT m_numSpotLights;

    // 全局环境光
    DWORD m_dwAmbientColor;

    // ========== 常量缓冲区 ==========

    // 光照常量缓冲区资源
    ComPtr<ID3D12Resource> m_pLightConstantBuffer;

    // 常量数据（CPU 端）
    LightConstants m_lightConstants;

    // 映射的常量数据指针
    LightConstants* m_pMappedLightConstants;

    // ========== 设备指针 ==========

    CoD3D12Device* m_pDevice;
};
