#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <string>
#include "../4DyuchiGRX_common/typedef.h"

// 前向声明
class CoD3D12Device;

// 材质常量缓冲区数据结构（对应 HLSL）
struct MaterialConstants
{
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 specular;
    float transparency;
    float shininess;
    float shininessStrength;
    float padding; // 16 字节对齐
};

// DirectX 12 材质类
class D3D12Material
{
public:
    D3D12Material();
    virtual ~D3D12Material();

    // ========== 初始化 ==========

    // 初始化材质
    HRESULT Initialize(CoD3D12Device* pDevice, const MATERIAL* pMtl);

    // ========== 纹理加载 ==========

    // 加载漫反射纹理
    HRESULT LoadDiffuseTexture(CoD3D12Device* pDevice, const char* szFileName);

    // 加载法线贴图
    HRESULT LoadNormalTexture(CoD3D12Device* pDevice, const char* szFileName);

    // 加载镜面反射纹理
    HRESULT LoadSpecularTexture(CoD3D12Device* pDevice, const char* szFileName);

    // ========== 直接设置纹理（用于材质管理器） ==========

    // 设置漫反射纹理
    void SetDiffuseTexture(ID3D12Resource* pTexture, D3D12_GPU_DESCRIPTOR_HANDLE srv);

    // 设置法线贴图
    void SetNormalTexture(ID3D12Resource* pTexture, D3D12_GPU_DESCRIPTOR_HANDLE srv);

    // 设置镜面反射纹理
    void SetSpecularTexture(ID3D12Resource* pTexture, D3D12_GPU_DESCRIPTOR_HANDLE srv) { m_pSpecularTexture = pTexture; m_hSpecularSRV = srv; }

    // 设置纹理边框颜色
    void SetTextureBorderColor(const float* pColor)
    {
        // TODO: 实现纹理边框颜色
        if (pColor != nullptr)
        {
            // 存储边框颜色供后续使用
            // m_fBorderColor[0] = pColor[0];
            // m_fBorderColor[1] = pColor[1];
            // m_fBorderColor[2] = pColor[2];
            // m_fBorderColor[3] = pColor[3];
        }
    }

    // ========== 渲染绑定 ==========

    // 绑定到渲染管线
    void Bind(ID3D12GraphicsCommandList* pCommandList,
              D3D12_GPU_DESCRIPTOR_HANDLE cbvHeapStart,
              UINT cbvSRVDescriptorSize);

    // 绑定到渲染管线（简化版本，使用存储的描述符）
    void BindToPipeline(ID3D12GraphicsCommandList* pCommandList);

    // 更新常量缓冲区
    void UpdateConstantBuffer(ID3D12GraphicsCommandList* pCommandList);

    // ========== 属性访问 ==========

    // 获取漫反射颜色
    DWORD GetDiffuseColor() const { return m_dwDiffuse; }
    void SetDiffuseColor(DWORD dwColor) { m_dwDiffuse = dwColor; UpdateConstants(); }

    // 获取环境光颜色
    DWORD GetAmbientColor() const { return m_dwAmbient; }
    void SetAmbientColor(DWORD dwColor) { m_dwAmbient = dwColor; UpdateConstants(); }

    // 获取镜面反射颜色
    DWORD GetSpecularColor() const { return m_dwSpecular; }
    void SetSpecularColor(DWORD dwColor) { m_dwSpecular = dwColor; UpdateConstants(); }

    // 获取透明度
    float GetTransparency() const { return m_fTransparency; }
    void SetTransparency(float fVal) { m_fTransparency = fVal; UpdateConstants(); }

    // 获取光泽度
    float GetShininess() const { return m_fShine; }
    void SetShininess(float fVal) { m_fShine = fVal; UpdateConstants(); }

    // 获取光泽强度
    float GetShininessStrength() const { return m_fShineStrength; }
    void SetShininessStrength(float fVal) { m_fShineStrength = fVal; UpdateConstants(); }

    // 获取材质标志
    DWORD GetFlag() const { return m_dwFlag; }
    void SetFlag(DWORD dwFlag) { m_dwFlag = dwFlag; }

    // 获取材质 ID
    DWORD GetMaterialID() const { return m_dwMaterialID; }
    void SetMaterialID(DWORD dwID) { m_dwMaterialID = dwID; }

    // 获取纹理名称
    const char* GetDiffuseTextureName() const { return m_szDiffuseTexName; }
    const char* GetNormalTextureName() const { return m_szNormalTexName; }
    const char* GetSpecularTextureName() const { return m_szSpecularTexName; }

    // 检查是否有纹理
    BOOL HasDiffuseTexture() const { return m_pDiffuseTexture != nullptr; }
    BOOL HasNormalTexture() const { return m_pNormalTexture != nullptr; }
    BOOL HasSpecularTexture() const { return m_pSpecularTexture != nullptr; }

    // ========== 引用计数 ==========

    DWORD AddRef();
    DWORD Release();
    DWORD GetRefCount() const { return m_dwRefCount; }

    // ========== 材质数据 ==========

    // 获取常量缓冲区数据
    const MaterialConstants& GetConstants() const { return m_constants; }

private:
    // ========== 辅助方法 ==========

    // 更新常量数据
    void UpdateConstants();

    // DWORD 颜色转 XMVECTOR
    DirectX::XMVECTOR ColorToXMVECTOR(DWORD dwColor);

    // ========== 材质属性 ==========

    // 颜色属性（来自 DX8 MATERIAL 结构）
    DWORD m_dwDiffuse;          // 漫反射颜色 (ARGB)
    DWORD m_dwAmbient;          // 环境光颜色 (ARGB)
    DWORD m_dwSpecular;         // 镜面反射颜色 (ARGB)

    float m_fTransparency;      // 透明度 (0.0 - 1.0)
    float m_fShine;             // 光泽度
    float m_fShineStrength;     // 光泽强度

    // 材质标志
    DWORD m_dwFlag;

    // 材质 ID
    DWORD m_dwMaterialID;

    // ========== 纹理资源 ==========

    ComPtr<ID3D12Resource> m_pDiffuseTexture;      // 漫反射纹理
    ComPtr<ID3D12Resource> m_pNormalTexture;       // 法线贴图
    ComPtr<ID3D12Resource> m_pSpecularTexture;     // 镜面反射纹理

    // SRV 描述符句柄
    D3D12_GPU_DESCRIPTOR_HANDLE m_hDiffuseSRV;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hNormalSRV;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hSpecularSRV;

    // 纹理名称
    char m_szDiffuseTexName[MAX_NAME_LEN];
    char m_szNormalTexName[MAX_NAME_LEN];
    char m_szSpecularTexName[MAX_NAME_LEN];

    // ========== 常量缓冲区 ==========

    MaterialConstants m_constants;

    // 常量缓冲区资源
    ComPtr<ID3D12Resource> m_pConstantBuffer;

    // ========== 引用计数 ==========

    DWORD m_dwRefCount;
};
