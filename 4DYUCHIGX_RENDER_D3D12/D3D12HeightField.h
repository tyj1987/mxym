// ============================================
// D3D12HeightField.h
// DirectX 12 高度场地形类
// 第 7 阶段 - HeightField 地形系统实现
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <vector>

#include <string>
#include "../4DyuchiGRX_common/typedef.h"
#include "I4DyuchiGXRenderer.h"



namespace DirectX { using namespace DirectX; }

// 前向声明
class CoD3D12Device;

// ========== 高度图配置 ==========
struct HeightMapConfig
{
    UINT nWidth;              // 宽度（顶点数）
    UINT nHeight;             // 高度（顶点数）
    float fHeightScale;       // 高度缩放
    float fHeightOffset;      // 高度偏移
    float fTextureScale;      // 纹理坐标缩放
    float fWorldScale;        // 世界空间缩放

    HeightMapConfig()
        : nWidth(512)
        , nHeight(512)
        , fHeightScale(1.0f)
        , fHeightOffset(0.0f)
        , fTextureScale(1.0f)
        , fWorldScale(1.0f)
    {}
};

// ========== 地形顶点 ==========
struct TerrainVertex
{
    XMFLOAT3 position;         // 位置
    XMFLOAT3 normal;           // 法线
    XMFLOAT2 texCoord;         // 纹理坐标
    XMFLOAT2 texCoordDetail;   // 细节纹理坐标

    TerrainVertex()
        : position(0, 0, 0)
        , normal(0, 1, 0)
        , texCoord(0, 0)
        , texCoordDetail(0, 0)
    {}
};

// ========== 地形分块 ==========
struct TerrainPatch
{
    UINT nX, nZ;              // 分块索引
    UINT nSize;               // 分块大小
    UINT nLOD;                // 当前 LOD 层级

    ComPtr<ID3D12Resource> pVertexBuffer;
    ComPtr<ID3D12Resource> pIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    UINT nVertexCount;
    UINT nIndexCount;

    XMFLOAT3 v3Center;        // 分块中心
    float fRadius;            // 包围球半径
    BOOL bVisible;            // 是否可见

    TerrainPatch()
        : nX(0), nZ(0)
        , nSize(0)
        , nLOD(0)
        , nVertexCount(0)
        , nIndexCount(0)
        , fRadius(0.0f)
        , bVisible(FALSE)
    {
        v3Center = XMFLOAT3(0, 0, 0);
        ZeroMemory(&vertexBufferView, sizeof(vertexBufferView));
        ZeroMemory(&indexBufferView, sizeof(indexBufferView));
    }
};

// ========== 地形纹理层 ==========
struct TerrainLayer
{
    std::string strName;
    float fMinHeight;
    float fMaxHeight;
    float fMinSlope;
    float fMaxSlope;

    ComPtr<ID3D12Resource> pDiffuseTexture;
    ComPtr<ID3D12Resource> pNormalTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE hDiffuseSRV;
    D3D12_GPU_DESCRIPTOR_HANDLE hNormalSRV;

    TerrainLayer()
        : fMinHeight(0.0f)
        , fMaxHeight(1.0f)
        , fMinSlope(0.0f)
        , fMaxSlope(1.0f)
    {
        ZeroMemory(&hDiffuseSRV, sizeof(hDiffuseSRV));
        ZeroMemory(&hNormalSRV, sizeof(hNormalSRV));
    }
};

// ============================================
// D3D12HeightField 类
// 基于高度场的地形渲染
// ============================================

class D3D12HeightField : public IDIHeightField
{
public:
    D3D12HeightField();
    virtual ~D3D12HeightField();

    // ========== IUnknown 接口 ==========
    STDMETHODIMP QueryInterface(REFIID riid, PPVOID ppv) override;
    STDMETHODIMP_(ULONG) AddRef(void) override;
    STDMETHODIMP_(ULONG) Release(void) override;

    // ========== IDIHeightField 接口 ==========

    // 开始初始化高度场
    BOOL __stdcall StartInitialize(HFIELD_DESC* pDesc) override;

    // 结束初始化
    void __stdcall EndInitialize() override;

    // 创建高度场网格对象
    IDIMeshObject* __stdcall CreateHeightFieldObject(HFIELD_OBJECT_DESC* pDesc) override;

    // 初始化索引缓冲池
    BOOL __stdcall InitiallizeIndexBufferPool(DWORD dwDetailLevel, DWORD dwIndicesNum, DWORD dwNum) override;

    // 加载纹理调色板
    BOOL __stdcall LoadTilePalette(TEXTURE_TABLE* pTexTable, DWORD dwTileTextureNum) override;

    // 替换纹理
    BOOL __stdcall ReplaceTile(char* szFileName, DWORD dwTexIndex) override;

    // 创建索引缓冲
    BOOL __stdcall CreateIndexBuffer(DWORD dwIndicesNum, DWORD dwDetailLevel, DWORD dwPositionMask, DWORD dwNum) override;

    // 锁定索引缓冲指针
    BOOL __stdcall LockIndexBufferPtr(WORD** ppWord, DWORD dwDetailLevel, DWORD dwPositionMask) override;

    // 解锁索引缓冲指针
    void __stdcall UnlcokIndexBufferPtr(DWORD dwDetailLevel, DWORD dwPositionMask) override;

    // 渲染网格
    BOOL __stdcall RenderGrid(VECTOR3* pv3Quad, DWORD dwTexTileIndex, DWORD dwAlpha) override;

    // 设置高度场纹理混合
    void __stdcall SetHFieldTileBlend(BOOL bSwitch) override;

    // 检查是否启用高度场纹理混合
    BOOL __stdcall IsEnableHFieldTileBlend() override;

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);

    // ========== 高度图加载 ==========
    HRESULT LoadHeightMap(const char* szFileName, const HeightMapConfig& config);
    HRESULT GenerateFromFunction(UINT nWidth, UINT nHeight, float (*HeightFunc)(float x, float z), const HeightMapConfig& config);

    // ========== 顶点/索引生成 ==========
    HRESULT GenerateVertices();
    HRESULT GenerateIndices();
    HRESULT CalculateNormals();
    HRESULT CreateBuffers();

    // ========== 地形分块 ==========
    HRESULT CreatePatches(UINT nPatchSize);
    void UpdatePatches(const XMFLOAT3& cameraPos, float fLODDistance);
    void RenderPatch(ID3D12GraphicsCommandList* pCommandList, UINT nPatchIndex);

    // ========== 纹理层 ==========
    HRESULT AddLayer(const TerrainLayer& layer);
    void RemoveLayer(UINT nIndex);
    TerrainLayer* GetLayer(UINT nIndex);
    UINT GetLayerCount() const;

    // ========== 渲染 ==========
    void Render(ID3D12GraphicsCommandList* pCommandList);
    void RenderAllPatches(ID3D12GraphicsCommandList* pCommandList);

    // ========== 查询 ==========
    float GetHeight(UINT x, UINT z) const;
    float GetHeight(float x, float z) const;
    XMFLOAT3 GetNormal(float x, float z) const;
    BOOL IsOnTerrain(float x, float z) const;

    // ========== 配置 ==========
    const HeightMapConfig& GetConfig() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetPatchCount() const;

private:
    // ========== 内部方法 ==========
    HRESULT LoadRawHeightMap(const char* szFileName, const HeightMapConfig& config);
    HRESULT LoadR16HeightMap(const char* szFileName, const HeightMapConfig& config);
    HRESULT LoadTGAHeightMap(const char* szFileName, const HeightMapConfig& config);
    void CalculatePatchBoundingBox(TerrainPatch& patch, UINT nPatchX, UINT nPatchZ);

private:
    // ========== COM 引用计数 ==========
    DWORD m_dwRefCount;

    // ========== 高度数据 ==========
    std::vector<float> m_vHeightData;
    UINT m_nWidth, m_nHeight;
    HeightMapConfig m_config;

    // ========== 顶点和索引 ==========
    std::vector<TerrainVertex> m_vVertices;
    std::vector<UINT> m_vIndices;
    ComPtr<ID3D12Resource> m_pVertexBuffer;
    ComPtr<ID3D12Resource> m_pIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_nVertexCount, m_nIndexCount;

    // ========== 地形分块 ==========
    std::vector<TerrainPatch> m_vPatches;
    UINT m_nPatchSize;

    // ========== 纹理层 ==========
    std::vector<TerrainLayer> m_vLayers;

    // ========== 纹理混合 ==========
    BOOL m_bTileBlendEnabled;

    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;
};

// ============================================
// 辅助宏
// ============================================

#define DEFAULT_TERRAIN_SIZE 512
#define DEFAULT_PATCH_SIZE 64
#define MAX_TERRAIN_SIZE 4096
