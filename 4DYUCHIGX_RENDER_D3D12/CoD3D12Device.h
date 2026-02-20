#pragma once

// 必须首先包含预编译头
#include "D3D12PCH.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <map>
#include "../4DyuchiGRX_common/typedef.h"
#include "../4DyuchiGRX_common/IFileStorage.h"
#include "I4DyuchiGXRenderer.h"

// 前向声明
class D3D12MaterialManager;
class D3D12LightManager;
class D3D12SpriteManager;
class D3D12FontManager;
class D3D12MeshManager;
class D3D12ShadowManager;
class D3D12PostProcessManager;
class D3D12TerrainManager;
class D3D12RenderScheduler;

// DirectX 12 渲染设备类（DX8 兼容实现）
class CoD3D12Device : public I4DyuchiGXRenderer
{
public:
    CoD3D12Device();
    virtual ~CoD3D12Device();

    // 清理所有资源
    void Cleanup();

    // ========== IUnknown 接口 ==========
    STDMETHODIMP QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ========== I4DyuchiGXRenderer 接口 ==========

    // 设备创建和初始化
    BOOL __stdcall Create(HWND hWnd, DISPLAY_INFO* pInfo, I4DyuchiFileStorage* pFileStorage, ErrorHandleProc pErrorHandleFunc) override;

    // 对象创建
    IDISpriteObject* __stdcall CreateSpriteObject(char* szFileName, DWORD dwFlag) override;
    IDISpriteObject* __stdcall CreateSpriteObject(char* szFileName, DWORD dwXPos, DWORD dwYPos, DWORD dwWidth, DWORD dwHeight, DWORD dwFlag) override;
    IDISpriteObject* __stdcall CreateEmptySpriteObject(DWORD dwWidth, DWORD dwHeight, TEXTURE_FORMAT TexFormat, DWORD dwFlag) override;
    IDIMeshObject* __stdcall CreateMeshObject(CMeshFlag flag) override;
    IDIFontObject* __stdcall CreateFontObject(LOGFONT* pLogFont, DWORD dwFlag) override;
    IDIHeightField* __stdcall CreateHeightField(DWORD dwFlag) override;
    IDIMeshObject* __stdcall CreateImmMeshObject(IVERTEX* piv3Tri, DWORD dwTriCount, void* pMtlHandle, DWORD dwFlag) override;

    // 帧控制
    void __stdcall BeginRender(SHORT_RECT* pRect, DWORD dwColor, DWORD dwFlag) override;
    void __stdcall EndRender() override;

    // 渲染标志
    void __stdcall SetShadowFlag(DWORD dwFlag) override;
    DWORD __stdcall GetShadowFlag() override;
    void __stdcall SetLightMapFlag(DWORD dwFlag) override;
    DWORD __stdcall GetLightMapFlag() override;
    void __stdcall SetRenderMode(DWORD dwFlag) override;
    DWORD __stdcall GetRenderMode() override;

    // 雾效
    void __stdcall EnableFog(float fStart, float fEnd, float fDensity, DWORD dwColor, DWORD dwFlag) override;
    void __stdcall DisableFog() override;

    // 阴影
    BOOL __stdcall BeginShadowMap() override;
    void __stdcall EndShadowMap() override;

    // 客户端信息
    void __stdcall GetClientRect(SHORT_RECT* pRect, WORD* pwWidth, WORD* pwHeight) override;

    // 动态光照
    DWORD __stdcall CreateDynamicLight(DWORD dwRS, DWORD dwColor, char* szFileName) override;
    BOOL __stdcall DeleteDynamicLight(DWORD dwIndex) override;

    // 特效着色器
    BOOL __stdcall CreateEffectShaderPaletteFromFile(char* szFileName) override;
    BOOL __stdcall CreateEffectShaderPalette(CUSTOM_EFFECT_DESC* pEffectDescList, DWORD dwNum) override;
    void __stdcall DeleteEffectShaderPalette() override;

    // 渲染方法
    BOOL __stdcall RenderMeshObject(
        IDIMeshObject* pMeshObj,
        DWORD dwRefIndex,
        float fDistance,
        DWORD dwAlpha,
        LIGHT_INDEX_DESC* pDynamicLightIndexList, DWORD dwLightNum,
        LIGHT_INDEX_DESC* pSpotLightIndexList, DWORD dwSpotLightNum,
        DWORD dwMtlSetIndex,
        DWORD dwEffectIndex,
        DWORD dwFlag) override;

    BOOL __stdcall RenderSprite(IDISpriteObject* pSprite, VECTOR2* pv2Scaling, float fRot, VECTOR2* pv2Trans, RECT* pRect, DWORD dwColor, int iZOrder, DWORD dwFlag) override;
    BOOL __stdcall RenderFont(IDIFontObject* pFont, TCHAR* str, DWORD dwLen, RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type, int iZOrder, DWORD dwFlag) override;

    // 调试渲染
    void __stdcall RenderBox(VECTOR3* pv3Oct, DWORD dwColor) override;
    void __stdcall RenderPoint(VECTOR3* pv3Point, DWORD dwColor) override;
    void __stdcall RenderCircle(VECTOR2* pv2Point, float fRs, DWORD dwColor) override;
    void __stdcall RenderLine(VECTOR2* pv2Point0, VECTOR2* pv2Point1, DWORD dwColor) override;
    void __stdcall RenderGrid(VECTOR3* pv3Quad, DWORD dwColor) override;
    BOOL __stdcall RenderTriIvertex(IVERTEX* piv3Tri, void* pMtlHandle, DWORD dwFacesNum, DWORD dwFlag) override;
    BOOL __stdcall RenderTriVector3(VECTOR3* pv3Tri, DWORD dwFacesNum, DWORD dwFlag) override;

    // 三角形缓冲区
    void* __stdcall AllocRenderTriBuffer(IVERTEX** ppIVList, DWORD dwFacesNum, DWORD dwRenderFlag) override;
    void __stdcall EnableRenderTriBuffer(void* pTriBufferHandle, void* pMtlHandle, DWORD dwRenderFacesNum) override;
    void __stdcall DisableRenderTriBuffer(void* pTriBufferHandle) override;
    void __stdcall FreeRenderTriBuffer(void* pTriBufferHandle) override;

    // 光照
    BOOL __stdcall SetRTLight(LIGHT_DESC* pLightDesc, DWORD dwLightIndex, DWORD dwFlag) override;
    void __stdcall EnableDirectionalLight(DIRECTIONAL_LIGHT_DESC* pLightDesc, DWORD dwFlag) override;
    void __stdcall DisableDirectionalLight() override;
    void __stdcall SetSpotLightDesc(VECTOR3* pv3From, VECTOR3* pv3To, VECTOR3* pv3Up, float fFov, float fNear, float fFar, float fWidth, BOOL bOrtho, void* pMtlHandle, DWORD dwColorOP, DWORD dwLightIndex, SPOT_LIGHT_TYPE type) override;
    void __stdcall SetShadowLightSenderPosition(BOUNDING_SPHERE* pSphere, DWORD dwLightIndex) override;
    void __stdcall SetViewFrusturm(VIEW_VOLUME* pViewVolume, CAMERA_DESC* camera, MATRIX4* pMatView, MATRIX4* pMatProj, MATRIX4* pMatForBilboard) override;

    // 系统信息
    void __stdcall GetSystemStatus(SYSTEM_STATUS* pStatus) override;
    void __stdcall UpdateWindowSize() override;
    void __stdcall Present(HWND hWnd) override;

    // 颜色
    void __stdcall SetAmbientColor(DWORD dwColor) override;
    DWORD __stdcall GetAmbientColor() override;
    void __stdcall SetEmissiveColor(DWORD dwColor) override;
    DWORD __stdcall GetEmissiveColor() override;

    // 性能分析
    void __stdcall BeginPerformanceAnalyze() override;
    void __stdcall EndPerformanceAnalyze() override;
    BOOL __stdcall CaptureScreen(char* szFileName) override;

    // 材质
    DWORD __stdcall CreateMaterialSet(MATERIAL_TABLE* pMtlEntry, DWORD dwNum) override;
    void __stdcall DeleteMaterialSet(DWORD dwMtlSetIndex) override;
    void* __stdcall CreateMaterial(MATERIAL* pMaterial, DWORD* pdwWidth, DWORD* pdwHeight, DWORD dwFlag) override;
    void __stdcall SetMaterialTextureBorder(void* pMtlHandle, DWORD dwColor) override;
    void __stdcall DeleteMaterial(void* pMtlHandle) override;

    // 其他设置
    void __stdcall SetAttentuation0(float att) override;
    float __stdcall GetAttentuation0() override;
    BOOL __stdcall ConvertCompressedTexture(char* szFileName, DWORD dwFlag) override;
    void __stdcall EnableSpecular(float fVal) override;
    void __stdcall DisableSpecular() override;
    void __stdcall SetVerticalSync(BOOL bSwitch) override;
    BOOL __stdcall IsSetVerticalSync() override;
    void __stdcall ResetDevice(BOOL bTest) override;
    void __stdcall SetFreeVBCacheRate(float fVal) override;
    float __stdcall GetFreeVBCacheRate() override;
    DWORD __stdcall ClearVBCacheWithIDIMeshObject(IDIMeshObject* pObject) override;
    DWORD __stdcall ClearCacheWithMotionUID(void* pMotionUID) override;
    void __stdcall SetTickCount(DWORD dwTickCount, BOOL bGameFrame) override;
    BOOL __stdcall GetD3DDevice(REFIID refiid, void** ppVoid) override;
    BOOL __stdcall InitializeRenderTarget(DWORD dwTexelSize, DWORD dwMaxTexNum) override;
    void __stdcall SetRenderTextureMustUpdate(BOOL bMustUpdate) override;
    void __stdcall SetAlphaRefValue(DWORD dwRefVaule) override;

    // 纹理加载失败处理
    BOOL __stdcall SetLoadFailedTextureTable(TEXTURE_TABLE* pLoadFailedTextureTable, DWORD dwLoadFailedTextureTableSize) override;
    VOID __stdcall GetLoadFailedTextureTable(TEXTURE_TABLE** ppoutLoadFailedTextureTable, DWORD* poutdwLoadFailedTextureTableSize, DWORD* poutdwFailedTextureCount) override;

    // 渲染模式
    VOID __stdcall SetRenderWireSolidBothMode(BOOL bMode) override;
    BOOL __stdcall GetRenderWireSolidBothMode(VOID) override;

    // ========== DX12 特定的辅助方法 ==========

    // 初始化和释放
    HRESULT Initialize(HWND hWnd, UINT width, UINT height, BOOL fullscreen);

    // 帧控制
    HRESULT BeginFrame();
    HRESULT EndFrame();
    HRESULT PresentDX12(UINT syncInterval = 1, UINT flags = 0);

    // 设备访问
    ID3D12Device* GetD3D12Device() const { return m_device.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

    // 获取窗口大小
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }

    // 渲染目标
    HRESULT SetRenderTarget(ID3D12Resource* renderTarget);
    HRESULT ClearRenderTarget(const FLOAT colorRGBA[4]);

    // 视口和剪裁
    void SetViewport(UINT x, UINT y, UINT width, UINT height);
    void SetScissorRect(LONG left, LONG top, LONG right, LONG bottom);

    // 获取渲染目标视图
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();
    ID3D12Resource* GetCurrentRenderTargetResource();

    // 等待 GPU 完成
    void WaitForGpu();

    // ========== 基础管理器访问 ==========
    D3D12MeshManager* GetMeshManager() const;

    // ========== 第6-7阶段：高级管理器访问 ==========
    D3D12ShadowManager* GetShadowManager() const;
    D3D12PostProcessManager* GetPostProcessManager() const;
    D3D12TerrainManager* GetTerrainManager() const;
    D3D12RenderScheduler* GetRenderScheduler() const;

private:
    // D3D12 创建方法
    HRESULT CreateDevice();
    HRESULT CreateCommandQueue();
    HRESULT CreateCommandAllocator();
    HRESULT CreateCommandList();
    HRESULT CreateSwapChain(HWND hWnd);
    HRESULT CreateRenderTargetViews();
    HRESULT CreateDepthStencil();
    HRESULT CreateDescriptorHeaps();

    HRESULT WaitForPreviousFrame();
    HRESULT ResetCommandList();

private:
    // ========== COM 引用计数 ==========
    DWORD m_dwRefCount;

    // ========== 核心 D3D12 对象 ==========
    ComPtr<ID3D12Device>                  m_device;
    ComPtr<IDXGISwapChain3>               m_swapChain;
    ComPtr<ID3D12CommandQueue>            m_commandQueue;
    ComPtr<ID3D12CommandAllocator>        m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList>     m_commandList;

    // ========== 描述符堆 ==========
    ComPtr<ID3D12DescriptorHeap>          m_rtvHeap;      // Render Target View
    ComPtr<ID3D12DescriptorHeap>          m_dsvHeap;      // Depth Stencil View
    ComPtr<ID3D12DescriptorHeap>          m_cbvSrvHeap;   // CBV/SRV/UAV

    // ========== 渲染目标 ==========
    ComPtr<ID3D12Resource>                m_renderTargets[2]; // 双缓冲
    ComPtr<ID3D12Resource>                m_depthStencil;

    // ========== 同步对象 ==========
    ComPtr<ID3D12Fence>                   m_fence;
    UINT64                                 m_fenceValue = 0;

    // ========== 描述符大小 ==========
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
    UINT m_cbvSrvDescriptorSize = 0;

    // ========== 帧索引 ==========
    UINT m_frameIndex = 0;

    // ========== 窗口参数 ==========
    UINT m_width = 0;
    UINT m_height = 0;
    BOOL m_fullscreen = FALSE;
    HWND m_hWnd = NULL;

    // ========== 视口 ==========
    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT m_scissorRect = {};

    // ========== 渲染状态 ==========
    DWORD m_dwRenderMode = 0;
    DWORD m_dwShadowFlag = 0;
    DWORD m_dwLightMapFlag = 0;
    DWORD m_dwAmbientColor = 0xFFFFFFFF;
    DWORD m_dwEmissiveColor = 0xFF000000;

    // ========== 雾效参数 ==========
    BOOL m_bFogEnabled = FALSE;
    float m_fFogStart = 0.0f;
    float m_fFogEnd = 0.0f;
    float m_fFogDensity = 0.0f;
    DWORD m_dwFogColor = 0;

    // ========== 垂直同步 ==========
    BOOL m_bVSync = TRUE;

    // ========== 文件存储 ==========
    I4DyuchiFileStorage* m_pFileStorage;
    ErrorHandleProc m_pErrorHandleFunc;

    // ========== 性能统计 ==========
    SYSTEM_STATUS m_systemStatus;

    // ========== 材质管理器（第 2 阶段） ==========
    D3D12MaterialManager* m_pMaterialManager;

    // ========== 光照管理器（第 3 阶段） ==========
    D3D12LightManager* m_pLightManager;

    // ========== Sprite 管理器（第 4 阶段） ==========
    D3D12SpriteManager* m_pSpriteManager;

    // ========== Font 管理器（第 4 阶段） ==========
    D3D12FontManager* m_pFontManager;

    // ========== Mesh 管理器（第 5 阶段） ==========
    D3D12MeshManager* m_pMeshManager;

    // ========== Shadow 管理器（第 6 阶段） ==========
    D3D12ShadowManager* m_pShadowManager;

    // ========== PostProcess 管理器（第 6 阶段） ==========
    D3D12PostProcessManager* m_pPostProcessManager;

    // ========== Terrain 管理器（第 7 阶段） ==========
    D3D12TerrainManager* m_pTerrainManager;

    // ========== Render Scheduler（第 8 阶段） ==========
    D3D12RenderScheduler* m_pRenderScheduler;
};

// 辅助宏
#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error"); \
    }

#define DX12_RELEASE(p) \
    if ((p) != nullptr) { \
        (p)->Release(); \
        (p) = nullptr; \
    }
