#include "D3D12PCH.h"
#include "D3D12SpriteObject.h"
#include "IRenderer_GUID.h"
#include "CoD3D12Device.h"
#include <cstring>

// 辅助宏
#include "D3D12PCH.h"

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12SpriteObject"); \
    }

// ============================================
// 构造函数和析构函数
// ============================================

D3D12SpriteObject::D3D12SpriteObject()
{
    // 变换
    m_v2Position = DirectX::XMFLOAT2(0.0f, 0.0f);
    m_v2Scaling = DirectX::XMFLOAT2(1.0f, 1.0f);
    m_fRotation = 0.0f;

    // 纹理矩形
    m_rcTextureRect = { 0, 0, 0, 0 };
    m_rcScreenRect = { 0, 0, 0, 0 };

    // 颜色
    m_dwColor = 0xFFFFFFFF;

    // Z 顺序
    m_iZOrder = 0;

    // 标志
    m_dwFlag = 0;

    // 可见性
    m_bVisible = true;

    // 纹理名称
    memset(m_szTextureName, 0, sizeof(m_szTextureName));

    // 尺寸
    m_nWidth = 0;
    m_nHeight = 0;

    // SRV
    m_hTextureSRV.ptr = 0;

    // 引用计数
    m_dwRefCount = 1;
}

D3D12SpriteObject::~D3D12SpriteObject()
{
    // ComPtr 自动释放
}

// ============================================
// 初始化
// ============================================

HRESULT D3D12SpriteObject::Initialize(CoD3D12Device* pDevice, const char* szFileName, DWORD dwFlag)
{
    if (pDevice == nullptr || szFileName == nullptr)
        return E_INVALIDARG;

    m_dwFlag = dwFlag;

    // 加载纹理
    HRESULT hr = LoadTexture(pDevice, szFileName);
    if (FAILED(hr))
        return hr;

    // 设置默认纹理矩形（全纹理）
    if (m_nWidth > 0 && m_nHeight > 0)
    {
        m_rcTextureRect = { 0, 0, (LONG)m_nWidth, (LONG)m_nHeight };
    }

    return S_OK;
}

HRESULT D3D12SpriteObject::InitializeEmpty(CoD3D12Device* pDevice, DWORD dwWidth, DWORD dwHeight, TEXTURE_FORMAT TexFormat, DWORD dwFlag)
{
    if (pDevice == nullptr)
        return E_INVALIDARG;

    m_nWidth = dwWidth;
    m_nHeight = dwHeight;
    m_dwFlag = dwFlag;

    // TODO: 创建空纹理
    // 1. 创建 D3D12 纹理资源
    // 2. 创建 SRV
    // 3. 初始化为黑色或白色

    return S_OK;
}

// ============================================
// 纹理管理
// ============================================

HRESULT D3D12SpriteObject::LoadTexture(CoD3D12Device* pDevice, const char* szFileName)
{
    if (pDevice == nullptr || szFileName == nullptr)
        return E_INVALIDARG;

    // TODO: 第 4 周实现 - 完整的纹理加载
    // 1. 从文件加载纹理
    // 2. 创建 D3D12 纹理资源
    // 3. 创建 SRV
    // 4. 获取纹理尺寸

    // 临时：保存文件名
    strcpy_s(m_szTextureName, MAX_NAME_LEN, szFileName);

    return S_OK;
}

// ============================================
// 变换
// ============================================

void D3D12SpriteObject::SetPosition(float x, float y)
{
    m_v2Position = DirectX::XMFLOAT2(x, y);
}

void D3D12SpriteObject::GetPosition(float* pX, float* pY) const
{
    if (pX) *pX = m_v2Position.x;
    if (pY) *pY = m_v2Position.y;
}

void D3D12SpriteObject::SetScaling(float x, float y)
{
    m_v2Scaling = DirectX::XMFLOAT2(x, y);
}

void D3D12SpriteObject::GetScaling(float* pX, float* pY) const
{
    if (pX) *pX = m_v2Scaling.x;
    if (pY) *pY = m_v2Scaling.y;
}

void D3D12SpriteObject::SetRotation(float fRadians)
{
    m_fRotation = fRadians;
}

// ============================================
// 纹理区域
// ============================================

void D3D12SpriteObject::SetTextureRect(const RECT* pRect)
{
    if (pRect != nullptr)
    {
        m_rcTextureRect = *pRect;
    }
}

void D3D12SpriteObject::GetTextureRect(RECT* pRect) const
{
    if (pRect != nullptr)
    {
        *pRect = m_rcTextureRect;
    }
}

void D3D12SpriteObject::SetScreenRect(const RECT* pRect)
{
    if (pRect != nullptr)
    {
        m_rcScreenRect = *pRect;
    }
}

void D3D12SpriteObject::GetScreenRect(RECT* pRect) const
{
    if (pRect != nullptr)
    {
        *pRect = m_rcScreenRect;
    }
}

// ============================================
// 颜色
// ============================================

void D3D12SpriteObject::SetColor(DWORD dwColor)
{
    m_dwColor = dwColor;
}

// ============================================
// Z 顺序
// ============================================

void D3D12SpriteObject::SetZOrder(int iZOrder)
{
    m_iZOrder = iZOrder;
}

// ============================================
// 渲染支持
// ============================================

DirectX::XMFLOAT4X4 D3D12SpriteObject::CalculateTransformMatrix() const
{
    using namespace DirectX;

    // 1. 平移
    XMMATRIX translation = XMMatrixTranslation(m_v2Position.x, m_v2Position.y, 0.0f);

    // 2. 旋转
    XMMATRIX rotation = XMMatrixRotationZ(m_fRotation);

    // 3. 缩放
    XMMATRIX scaling = XMMatrixScaling(m_v2Scaling.x, m_v2Scaling.y, 1.0f);

    // 4. 组合变换：S * R * T
    XMMATRIX transform = scaling * rotation * translation;

    // 转置（因为 HLSL 是列主序）
    XMMATRIX result = XMMatrixTranspose(transform);

    XMFLOAT4X4 outMatrix;
    XMStoreFloat4x4(&outMatrix, result);

    return outMatrix;
}

// ============================================
// IUnknown 接口
// ============================================

STDMETHODIMP D3D12SpriteObject::QueryInterface(REFIID riid, PPVOID ppv)
{
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IDISpriteObject))
    {
        *ppv = static_cast<IDISpriteObject*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) D3D12SpriteObject::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

STDMETHODIMP_(ULONG) D3D12SpriteObject::Release(void)
{
    DWORD dwRefCount = InterlockedDecrement(&m_dwRefCount);
    if (dwRefCount == 0)
    {
        delete this;
    }
    return dwRefCount;
}

// ============================================
// IDISpriteObject 接口实现
// ============================================

BOOL __stdcall D3D12SpriteObject::Draw(VECTOR2* pv2Scaling, float fRot, VECTOR2* pv2Trans, RECT* pRect, DWORD dwColor, DWORD dwFlag)
{
    // 绘制由SpriteManager处理，此函数用于更新参数
    if (pv2Scaling != nullptr)
    {
        SetScaling(pv2Scaling->x, pv2Scaling->y);
    }
    if (pv2Trans != nullptr)
    {
        SetPosition(pv2Trans->x, pv2Trans->y);
    }
    SetRotation(fRot);
    if (pRect != nullptr)
    {
        SetTextureRect(pRect);
    }
    SetColor(dwColor);
    return TRUE;
}

BOOL __stdcall D3D12SpriteObject::Resize(float fWidth, float fHeight)
{
    m_nWidth = static_cast<UINT>(fWidth);
    m_nHeight = static_cast<UINT>(fHeight);

    // 更新屏幕矩形
    m_rcScreenRect.right = m_rcScreenRect.left + m_nWidth;
    m_rcScreenRect.bottom = m_rcScreenRect.top + m_nHeight;

    return TRUE;
}

BOOL __stdcall D3D12SpriteObject::GetImageHeader(IMAGE_HEADER* pImgHeader, DWORD dwFrameIndex)
{
    if (pImgHeader == nullptr)
        return FALSE;

    pImgHeader->dwWidth = m_nWidth;
    pImgHeader->dwHeight = m_nHeight;
    pImgHeader->dwFormat = static_cast<DWORD>(DXGI_FORMAT_B8G8R8A8_UNORM); // 32位BGRA格式
    pImgHeader->dwMipLevels = 1;

    return TRUE;
}

BOOL __stdcall D3D12SpriteObject::LockRect(LOCKED_RECT* pOutLockedRect, RECT* pRect, TEXTURE_FORMAT TexFormat)
{
    if (pOutLockedRect == nullptr || m_pTexture == nullptr)
        return FALSE;

    // 检查纹理是否是上传堆纹理（可映射）
    D3D12_RESOURCE_DESC desc = m_pTexture->GetDesc();
    if (desc.Layout != D3D12_TEXTURE_LAYOUT_ROW_MAJOR)
    {
        // 非上传堆纹理，无法直接映射
        // 返回默认值
        pOutLockedRect->pBits = nullptr;
        pOutLockedRect->Pitch = static_cast<INT>(m_nWidth * 4);
        return FALSE;
    }

    // 映射纹理资源
    D3D12_RANGE readRange = {0, 0};
    void* pData = nullptr;

    if (FAILED(m_pTexture->Map(0, &readRange, &pData)))
    {
        pOutLockedRect->pBits = nullptr;
        pOutLockedRect->Pitch = 0;
        return FALSE;
    }

    pOutLockedRect->pBits = pData;
    pOutLockedRect->Pitch = static_cast<INT>(m_nWidth * 4); // 假设32位RGBA格式

    return TRUE;
}

BOOL __stdcall D3D12SpriteObject::UnlockRect()
{
    if (m_pTexture == nullptr)
        return FALSE;

    // 解除纹理映射
    m_pTexture->Unmap(0, nullptr);

    return TRUE;
}
