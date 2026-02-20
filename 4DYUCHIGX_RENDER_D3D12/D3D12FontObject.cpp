#include "D3D12PCH.h"
#include "D3D12FontObject.h"
#include "IRenderer_GUID.h"
#include "CoD3D12Device.h"
#include <cstring>

// ============================================
// 构造函数和析构函数
// ============================================

#include "D3D12PCH.h"

D3D12FontObject::D3D12FontObject()
{
    memset(&m_logFont, 0, sizeof(m_logFont));
    memset(m_szFontName, 0, sizeof(m_szFontName));
    m_nHeight = 0;
    m_nWidth = 0;
    dwFlag = 0;

    m_nAtlasWidth = 0;
    m_nAtlasHeight = 0;
    m_hTextureAtlasSRV.ptr = 0;

    m_dwRefCount = 1;
    m_pDevice = nullptr;
}

D3D12FontObject::~D3D12FontObject()
{
    // ComPtr 自动释放
}

// ============================================
// 初始化
// ============================================

HRESULT D3D12FontObject::Initialize(CoD3D12Device* pDevice, LOGFONT* pLogFont, DWORD dwFlag)
{
    if (pDevice == nullptr || pLogFont == nullptr)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_logFont = *pLogFont;
    dwFlag = dwFlag;

    // 获取字体信息
    m_nHeight = pLogFont->lfHeight;
    m_nWidth = pLogFont->lfWidth;

    // TODO: 第 4 周实现
    // 1. 创建字体
    // 2. 生成所有字符的纹理
    // 3. 打包到纹理集
    // 4. 生成 UV 坐标映射

    return S_OK;
}

// ============================================
// 文本渲染
// ============================================

BOOL D3D12FontObject::RenderText(ID3D12GraphicsCommandList* pCommandList,
                                TCHAR* str, DWORD dwLen, RECT* pRect,
                                DWORD dwColor, CHAR_CODE_TYPE type,
                                int iZOrder, DWORD dwFlag)
{
    if (pCommandList == nullptr || str == nullptr)
        return FALSE;

    // TODO: 第 4 周实现 - 完整的文本渲染
    // 1. 测量文本尺寸
    // 2. 计算字符位置
    // 3. 生成顶点数据
    // 4. 绘制字符

    return TRUE;
}

// ============================================
// 字符数据
// ============================================

const D3D12FontObject::CharUV* D3D12FontObject::GetCharUV(TCHAR ch) const
{
    auto it = m_mCharUVMap.find(ch);
    if (it != m_mCharUVMap.end())
    {
        return &it->second;
    }
    return nullptr;
}

// ============================================
// IUnknown 接口
// ============================================

STDMETHODIMP D3D12FontObject::QueryInterface(REFIID riid, PPVOID ppv)
{
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IDIFontObject))
    {
        *ppv = static_cast<IDIFontObject*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) D3D12FontObject::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

STDMETHODIMP_(ULONG) D3D12FontObject::Release(void)
{
    DWORD dwRefCount = InterlockedDecrement(&m_dwRefCount);
    if (dwRefCount == 0)
    {
        delete this;
    }
    return dwRefCount;
}

// ============================================
// IDIFontObject 接口实现
// ============================================

void __stdcall D3D12FontObject::BeginRender()
{
    // TODO: 开始字体渲染批处理
    // 设置批处理状态、绑定纹理集等
}

void __stdcall D3D12FontObject::EndRender()
{
    // TODO: 结束字体渲染批处理
    // 提交批处理命令
}

BOOL __stdcall D3D12FontObject::DrawText(TCHAR* str, DWORD dwLen, RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type, DWORD dwFlag)
{
    if (str == nullptr)
        return FALSE;

    if (dwLen == 0)
        dwLen = static_cast<DWORD>(_tcslen(str));

    // TODO: 实现完整的文本渲染
    // 1. 测量文本尺寸
    // 2. 计算字符位置
    // 3. 生成顶点数据
    // 4. 绘制字符

    return TRUE;
}

// ============================================
// 字符信息方法
// ============================================

float D3D12FontObject::GetCharWidthA(TCHAR ch) const
{
    const CharUV* pUV = GetCharUV(ch);
    if (pUV != nullptr)
        return pUV->width;
    return 0.0f;
}

float D3D12FontObject::GetCharHeightEx(TCHAR ch) const
{
    const CharUV* pUV = GetCharUV(ch);
    if (pUV != nullptr)
        return pUV->height;
    return static_cast<float>(m_nHeight);
}

BOOL D3D12FontObject::GetCharTexCoords(TCHAR ch, float& u0, float& v0, float& u1, float& v1) const
{
    const CharUV* pUV = GetCharUV(ch);
    if (pUV == nullptr)
        return FALSE;

    u0 = pUV->u0;
    v0 = pUV->v0;
    u1 = pUV->u1;
    v1 = pUV->v1;

    return TRUE;
}
