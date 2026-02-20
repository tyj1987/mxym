#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <string>
#include <map>
#include "../4DyuchiGRX_common/typedef.h"
#include "I4DyuchiGXRenderer.h"

// 前向声明
class CoD3D12Device;
class D3D12FontManager;

// DirectX 12 Font 对象类
class D3D12FontObject : public IDIFontObject
{
public:
    D3D12FontObject();
    virtual ~D3D12FontObject();

    // ========== 初始化 ==========

    // 初始化字体
    HRESULT Initialize(CoD3D12Device* pDevice, LOGFONT* pLogFont, DWORD dwFlag);

    // ========== 文本渲染 ==========

    // 渲染文本
    BOOL RenderText(ID3D12GraphicsCommandList* pCommandList,
                    TCHAR* str, DWORD dwLen, RECT* pRect,
                    DWORD dwColor, CHAR_CODE_TYPE type,
                    int iZOrder, DWORD dwFlag);

    // ========== 字体信息 ==========

    // 获取字体信息
    LOGFONT GetLogFont() const { return m_logFont; }
    const char* GetFontName() const { return m_szFontName; }
    int GetHeight() const { return m_nHeight; }
    int GetWidth() const { return m_nWidth; }

    // ========== 字符数据 ==========

    // 获取字符 UV 坐标
    struct CharUV
    {
        float u0, v0, u1, v1;
        float width, height;
        float advance;
    };

    const CharUV* GetCharUV(TCHAR ch) const;

    // ========== 字体纹理访问 ==========

    // 获取字体纹理资源
    ID3D12Resource* GetTexture() const { return m_pFontTextureAtlas.Get(); }
    // 获取字体纹理SRV句柄
    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSRV() const { return m_hTextureAtlasSRV; }

    // ========== 字符信息 ==========

    // 获取字符宽度（默认）
    float GetCharWidth() const { return static_cast<float>(m_nWidth); }
    // 获取字符宽度（ANSI）
    float GetCharWidthA(TCHAR ch) const;
    // 获取字符高度（默认）
    float GetCharHeight() const { return static_cast<float>(m_nHeight); }
    // 获取字符高度
    float GetCharHeightEx(TCHAR ch) const;
    // 获取行间距
    float GetLineSpacing() const { return static_cast<float>(m_nHeight) * 1.2f; }
    // 获取字符纹理坐标（引用版本）
    BOOL GetCharTexCoords(TCHAR ch, float& u0, float& v0, float& u1, float& v1) const;

    // ========== IUnknown 接口 ==========

    STDMETHODIMP QueryInterface(REFIID riid, PPVOID ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ========== IDIFontObject 接口 ==========

    void __stdcall BeginRender() override;
    void __stdcall EndRender() override;
    BOOL __stdcall DrawText(TCHAR* str, DWORD dwLen, RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type, DWORD dwFlag) override;

private:
    // ========== 字体信息 ==========
    LOGFONT m_logFont;
    char m_szFontName[MAX_NAME_LEN];
    int m_nHeight;
    int m_nWidth;
    DWORD dwFlag;

    // ========== 字符 UV 映射 ==========
    std::map<TCHAR, CharUV> m_mCharUVMap;

    // ========== 字体纹理集 ==========
    ComPtr<ID3D12Resource> m_pFontTextureAtlas;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hTextureAtlasSRV;

    // ========== 纹理尺寸 ==========
    UINT m_nAtlasWidth;
    UINT m_nAtlasHeight;

    // ========== 引用计数 ==========
    DWORD m_dwRefCount;

    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;
};
