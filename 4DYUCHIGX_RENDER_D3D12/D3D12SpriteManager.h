#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <map>
#include <vector>

#include "../4DyuchiGRX_common/typedef.h"



// 前向声明
class CoD3D12Device;
class D3D12SpriteObject;

// Sprite 顶点
struct SpriteVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texCoord;
    DirectX::XMFLOAT4 color;
};

// Sprite 实例数据（用于批量渲染）
struct SpriteInstanceData
{
    DirectX::XMFLOAT4X4 transform;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 texRect;  // left, top, right, bottom
    float zOrder;
    float padding[3];
};

// DirectX 12 Sprite 管理器
class D3D12SpriteManager
{
public:
    D3D12SpriteManager();
    virtual ~D3D12SpriteManager();

    // ========== 初始化 ==========

    // 初始化管理器
    HRESULT Initialize(CoD3D12Device* pDevice);

    // 释放所有资源
    void Release();

    // ========== Sprite 管理 ==========

    // 添加 Sprite
    void* AddSprite(D3D12SpriteObject* pSprite);

    // 移除 Sprite
    void RemoveSprite(void* pHandle);

    // 获取 Sprite
    D3D12SpriteObject* GetSprite(void* pHandle);

    // 检查 Sprite 是否存在
    BOOL HasSprite(void* pHandle) const;

    // ========== 渲染 ==========

    // 渲染单个 Sprite
    BOOL RenderSprite(D3D12SpriteObject* pSprite, VECTOR2* pv2Scaling,
                      float fRot, VECTOR2* pv2Trans, RECT* pRect,
                      DWORD dwColor, int iZOrder, DWORD dwFlag);

    // 批量渲染所有 Sprite
    void RenderAllSprites(ID3D12GraphicsCommandList* pCommandList);

    // ========== 统计 ==========

    // 获取 Sprite 数量
    size_t GetSpriteCount() const { return m_mSpriteMap.size(); }

    // ========== 顶点缓冲区管理 ==========

    // 获取顶点缓冲区
    ID3D12Resource* GetVertexBuffer() const { return m_pVertexBuffer.Get(); }

    // 获取根签名
    ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }

    // 获取 PSO
    ID3D12PipelineState* GetPSO() const { return m_pPSO.Get(); }

private:
    // ========== 辅助方法 ==========

    // 创建顶点缓冲区
    HRESULT CreateVertexBuffer();

    // 创建索引缓冲区
    HRESULT CreateIndexBuffer();

    // 创建根签名
    HRESULT CreateRootSignature();

    // 创建 PSO
    HRESULT CreatePipelineState();

    // ========== Sprite 映射 ==========

    // Sprite 映射（句柄 -> Sprite）
    std::map<void*, D3D12SpriteObject*> m_mSpriteMap;

    // ========== 顶点缓冲区 ==========

    ComPtr<ID3D12Resource> m_pVertexBuffer;
    ComPtr<ID3D12Resource> m_pUploadVertexBuffer;  // 上传堆
    UINT m_nVertexBufferSize;

    ComPtr<ID3D12Resource> m_pIndexBuffer;
    ComPtr<ID3D12Resource> m_pUploadIndexBuffer;  // 上传堆
    UINT m_nIndexBufferSize;

    // ========== 配置 ==========

    UINT m_nMaxSprites;  // 最大精灵数量

    // ========== 渲染管线 ==========

    ComPtr<ID3D12RootSignature> m_pRootSignature;
    ComPtr<ID3D12PipelineState> m_pPSO;

    // ========== 设备指针 ==========

    CoD3D12Device* m_pDevice;
};
