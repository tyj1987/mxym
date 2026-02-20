#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <string>
#include "../4DyuchiGRX_common/typedef.h"
#include "I4DyuchiGXRenderer.h"

// 前向声明
class CoD3D12Device;
class D3D12SpriteManager;

// DirectX 12 Sprite 对象类
class D3D12SpriteObject : public IDISpriteObject
{
public:
    D3D12SpriteObject();
    virtual ~D3D12SpriteObject();

    // ========== 初始化 ==========

    // 从文件初始化
    HRESULT Initialize(CoD3D12Device* pDevice, const char* szFileName, DWORD dwFlag);

    // 初始化空 Sprite（用于动态纹理）
    HRESULT InitializeEmpty(CoD3D12Device* pDevice, DWORD dwWidth, DWORD dwHeight, TEXTURE_FORMAT TexFormat, DWORD dwFlag);

    // ========== 纹理管理 ==========

    // 加载纹理
    HRESULT LoadTexture(CoD3D12Device* pDevice, const char* szFileName);

    // ========== 变换 ==========

    // 设置位置
    void SetPosition(float x, float y);
    void GetPosition(float* pX, float* pY) const;

    // 设置缩放
    void SetScaling(float x, float y);
    void GetScaling(float* pX, float* pY) const;

    // 设置旋转（弧度）
    void SetRotation(float fRadians);
    float GetRotation() const { return m_fRotation; }

    // ========== 纹理区域 ==========

    // 设置纹理矩形
    void SetTextureRect(const RECT* pRect);
    void GetTextureRect(RECT* pRect) const;

    // 设置屏幕矩形
    void SetScreenRect(const RECT* pRect);
    void GetScreenRect(RECT* pRect) const;

    // ========== 颜色 ==========

    // 设置颜色调制
    void SetColor(DWORD dwColor);
    DWORD GetColor() const { return m_dwColor; }

    // ========== 尺寸 ==========

    // 获取宽度
    UINT GetWidth() const { return m_nWidth; }

    // 获取高度
    UINT GetHeight() const { return m_nHeight; }

    // ========== Z 顺序 ==========

    // 设置 Z 顺序
    void SetZOrder(int iZOrder);
    int GetZOrder() const { return m_iZOrder; }

    // ========== 纹理资源 ==========

    // 获取纹理资源
    ID3D12Resource* GetTexture() const { return m_pTexture.Get(); }

    // 获取纹理 SRV
    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSRV() const { return m_hTextureSRV; }

    // 获取纹理名称
    const char* GetTextureName() const { return m_szTextureName; }

    // ========== 便捷访问方法（用于兼容DX8接口） ==========

    // 获取X坐标
    float GetX() const { return m_v2Position.x; }

    // 获取Y坐标
    float GetY() const { return m_v2Position.y; }

    // 获取X缩放
    float GetScaleX() const { return m_v2Scaling.x; }

    // 获取Y缩放
    float GetScaleY() const { return m_v2Scaling.y; }

    // 获取可见性
    bool IsVisible() const { return m_bVisible; }

    // 设置可见性
    void SetVisible(bool bVisible) { m_bVisible = bVisible; }

    // ========== 渲染支持 ==========

    // 计算变换矩阵
    DirectX::XMFLOAT4X4 CalculateTransformMatrix() const;

    // ========== IUnknown 接口 ==========

    STDMETHODIMP QueryInterface(REFIID riid, PPVOID ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ========== IDISpriteObject 接口 ==========

    BOOL __stdcall Draw(VECTOR2* pv2Scaling, float fRot, VECTOR2* pv2Trans, RECT* pRect, DWORD dwColor, DWORD dwFlag) override;
    BOOL __stdcall Resize(float fWidth, float fHeight) override;
    BOOL __stdcall GetImageHeader(IMAGE_HEADER* pImgHeader, DWORD dwFrameIndex) override;
    BOOL __stdcall LockRect(LOCKED_RECT* pOutLockedRect, RECT* pRect, TEXTURE_FORMAT TexFormat) override;
    BOOL __stdcall UnlockRect() override;

private:
    // ========== 纹理资源 ==========
    ComPtr<ID3D12Resource> m_pTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hTextureSRV;

    // ========== 变换 ==========
    DirectX::XMFLOAT2 m_v2Position;
    DirectX::XMFLOAT2 m_v2Scaling;
    float m_fRotation;

    // ========== 纹理矩形 ==========
    RECT m_rcTextureRect;
    RECT m_rcScreenRect;

    // ========== 颜色 ==========
    DWORD m_dwColor;

    // ========== Z 顺序 ==========
    int m_iZOrder;

    // ========== 标志 ==========
    DWORD m_dwFlag;

    // ========== 可见性 ==========
    bool m_bVisible;

    // ========== 纹理名称 ==========
    char m_szTextureName[MAX_NAME_LEN];

    // ========== 尺寸 ==========
    UINT m_nWidth;
    UINT m_nHeight;

    // ========== 引用计数 ==========
    DWORD m_dwRefCount;
};
