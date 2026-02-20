// 必须首先包含预编译头
#include "D3D12PCH.h"

#include "CoD3D12Device.h"
#include "D3D12MaterialManager.h"
#include "D3D12LightManager.h"
#include "D3D12SpriteManager.h"
#include "D3D12FontManager.h"
#include "D3D12MeshManager.h"
#include "D3D12MeshObject.h"
#include "D3D12ImmMeshObject.h"
#include "D3D12ShadowManager.h"
#include "D3D12PostProcessManager.h"
#include "D3D12TerrainManager.h"
#include "D3D12RenderScheduler.h"
#include "D3D12SpriteObject.h"
#include "D3D12FontObject.h"
#include "d3dx12.h"
#include <iostream>
#include <stdexcept>

CoD3D12Device::CoD3D12Device()
{
    // COM 引用计数
    m_dwRefCount = 1;

    // 核心状态
    m_fenceValue = 0;
    m_frameIndex = 0;
    m_width = 0;
    m_height = 0;
    m_fullscreen = FALSE;
    m_hWnd = NULL;

    // 渲染状态
    m_dwRenderMode = 0;
    m_dwShadowFlag = 0;
    m_dwLightMapFlag = 0;
    m_dwAmbientColor = 0xFFFFFFFF;
    m_dwEmissiveColor = 0xFF000000;

    // 雾效
    m_bFogEnabled = FALSE;
    m_fFogStart = 0.0f;
    m_fFogEnd = 0.0f;
    m_fFogDensity = 0.0f;
    m_dwFogColor = 0;

    // 垂直同步
    m_bVSync = TRUE;

    // 文件存储
    m_pFileStorage = nullptr;
    m_pErrorHandleFunc = nullptr;

    // 性能统计
    ZeroMemory(&m_systemStatus, sizeof(m_systemStatus));

    // 材质管理器（第 2 阶段）
    m_pMaterialManager = nullptr;

    // 光照管理器（第 3 阶段）
    m_pLightManager = nullptr;

    // Sprite 管理器（第 4 阶段）
    m_pSpriteManager = nullptr;

    // Font 管理器（第 4 阶段）
    m_pFontManager = nullptr;

    // Mesh 管理器（第 5 阶段）
    m_pMeshManager = nullptr;

    // Shadow 管理器（第 6 阶段）
    m_pShadowManager = nullptr;

    // PostProcess 管理器（第 6 阶段）
    m_pPostProcessManager = nullptr;

    // Terrain 管理器（第 7 阶段）
    m_pTerrainManager = nullptr;

    // Render Scheduler（第 8 阶段）
    m_pRenderScheduler = nullptr;
}

CoD3D12Device::~CoD3D12Device()
{
    Cleanup();
}

void CoD3D12Device::Cleanup()
{
    // 释放 Render Scheduler（第 8 阶段）
    if (m_pRenderScheduler != nullptr)
    {
        m_pRenderScheduler->Release();
        delete m_pRenderScheduler;
        m_pRenderScheduler = nullptr;
    }

    // 释放 Terrain 管理器（第 7 阶段）
    if (m_pTerrainManager != nullptr)
    {
        m_pTerrainManager->Release();
        delete m_pTerrainManager;
        m_pTerrainManager = nullptr;
    }

    // 释放 PostProcess 管理器（第 6 阶段）
    if (m_pPostProcessManager != nullptr)
    {
        m_pPostProcessManager->Release();
        delete m_pPostProcessManager;
        m_pPostProcessManager = nullptr;
    }

    // 释放 Shadow 管理器（第 6 阶段）
    if (m_pShadowManager != nullptr)
    {
        m_pShadowManager->Release();
        delete m_pShadowManager;
        m_pShadowManager = nullptr;
    }

    // 释放 Mesh 管理器（第 5 阶段）
    if (m_pMeshManager != nullptr)
    {
        m_pMeshManager->Release();
        delete m_pMeshManager;
        m_pMeshManager = nullptr;
    }

    // 释放 Font 管理器（第 4 阶段）
    if (m_pFontManager != nullptr)
    {
        m_pFontManager->Release();
        delete m_pFontManager;
        m_pFontManager = nullptr;
    }

    // 释放 Sprite 管理器（第 4 阶段）
    if (m_pSpriteManager != nullptr)
    {
        m_pSpriteManager->Release();
        delete m_pSpriteManager;
        m_pSpriteManager = nullptr;
    }

    // 释放光照管理器（第 3 阶段）
    if (m_pLightManager != nullptr)
    {
        m_pLightManager->Release();
        delete m_pLightManager;
        m_pLightManager = nullptr;
    }

    // 释放材质管理器（第 2 阶段）
    if (m_pMaterialManager != nullptr)
    {
        m_pMaterialManager->Release();
        delete m_pMaterialManager;
        m_pMaterialManager = nullptr;
    }

    // 等待 GPU 完成
    WaitForPreviousFrame();

    m_fence.Reset();
    m_commandList.Reset();
    m_commandAllocator.Reset();
    m_commandQueue.Reset();
    m_swapChain.Reset();

    for (UINT i = 0; i < 2; ++i)
    {
        m_renderTargets[i].Reset();
    }
    m_depthStencil.Reset();

    m_rtvHeap.Reset();
    m_dsvHeap.Reset();
    m_cbvSrvHeap.Reset();

    m_device.Reset();
}

HRESULT CoD3D12Device::Initialize(HWND hWnd, UINT width, UINT height, BOOL fullscreen)
{
    if (hWnd == NULL)
        return E_INVALIDARG;

    m_hWnd = hWnd;
    m_width = width;
    m_height = height;
    m_fullscreen = fullscreen;

    HRESULT hr = S_OK;

    try {
        // 启用调试层
        #ifdef _DEBUG
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
        #endif

        // 1. 创建设备
        DX12_THROW_IF_FAILED(CreateDevice());

        // 2. 创建命令队列
        DX12_THROW_IF_FAILED(CreateCommandQueue());

        // 3. 创建命令分配器
        DX12_THROW_IF_FAILED(CreateCommandAllocator());

        // 4. 创建命令列表
        DX12_THROW_IF_FAILED(CreateCommandList());

        // 5. 创建交换链
        DX12_THROW_IF_FAILED(CreateSwapChain(hWnd));

        // 6. 创建渲染目标视图
        DX12_THROW_IF_FAILED(CreateRenderTargetViews());

        // 7. 创建深度模板
        DX12_THROW_IF_FAILED(CreateDepthStencil());

        // 8. 创建描述符堆
        DX12_THROW_IF_FAILED(CreateDescriptorHeaps());

        // 9. 创建同步对象
        DX12_THROW_IF_FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // 10. 设置视口和剪裁矩形
        SetViewport(0, 0, width, height);
        SetScissorRect(0, 0, width, height);

        // 11. 初始化材质管理器（第 2 阶段）
        m_pMaterialManager = new D3D12MaterialManager();
        DX12_THROW_IF_FAILED(m_pMaterialManager->Initialize(this, 1000));

        // 12. 初始化光照管理器（第 3 阶段）
        m_pLightManager = new D3D12LightManager();
        DX12_THROW_IF_FAILED(m_pLightManager->Initialize(this));

        // 13. 初始化 Sprite 管理器（第 4 阶段）
        m_pSpriteManager = new D3D12SpriteManager();
        DX12_THROW_IF_FAILED(m_pSpriteManager->Initialize(this));

        // 14. 初始化 Font 管理器（第 4 阶段）
        m_pFontManager = new D3D12FontManager();
        DX12_THROW_IF_FAILED(m_pFontManager->Initialize(this));

        // 15. 初始化 Mesh 管理器（第 5 阶段）
        m_pMeshManager = new D3D12MeshManager();
        DX12_THROW_IF_FAILED(m_pMeshManager->Initialize(this));

        // 16. 初始化 Terrain 管理器（第 7 阶段）
        m_pTerrainManager = new D3D12TerrainManager();
        DX12_THROW_IF_FAILED(m_pTerrainManager->Initialize(this));

        // 17. 初始化 Shadow 管理器（第 6 阶段）
        m_pShadowManager = new D3D12ShadowManager();
        DX12_THROW_IF_FAILED(m_pShadowManager->Initialize(this));

        // 18. 初始化 PostProcess 管理器（第 6 阶段）
        m_pPostProcessManager = new D3D12PostProcessManager();
        DX12_THROW_IF_FAILED(m_pPostProcessManager->Initialize(this, width, height));

        // 19. 初始化 Render Scheduler（第 8 阶段）
        m_pRenderScheduler = new D3D12RenderScheduler();
        DX12_THROW_IF_FAILED(m_pRenderScheduler->Initialize(this));

    }
    catch (const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CoD3D12Device::CreateDevice()
{
    HRESULT hr = S_OK;

    #ifdef _DEBUG
    // 调试模式：启用 DX12 调试层
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
    #endif

    // 创建 DXGI Factory
    ComPtr<IDXGIFactory4> factory;
    DX12_THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    // 查找适配器
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // 跳过软件渲染设备
            continue;
        }

        // 创建 D3D12 设备
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        for (D3D_FEATURE_LEVEL level : featureLevels)
        {
            hr = D3D12CreateDevice(
                adapter.Get(),
                level,
                IID_PPV_ARGS(&m_device)
            );

            if (SUCCEEDED(hr))
            {
                // 成功创建设备
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    if (FAILED(hr))
    {
        // 如果没有找到硬件设备，尝试 WARP 设备
        ComPtr<IDXGIAdapter> warpAdapter;
        hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        if (SUCCEEDED(hr))
        {
            hr = D3D12CreateDevice(
                warpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_device)
            );
        }
    }

    return hr;
}

HRESULT CoD3D12Device::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    return m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
}

HRESULT CoD3D12Device::CreateCommandAllocator()
{
    return m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_commandAllocator)
    );
}

HRESULT CoD3D12Device::CreateCommandList()
{
    return m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    );
}

HRESULT CoD3D12Device::CreateSwapChain(HWND hWnd)
{
    // 创建 DXGI Factory
    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;

    // 交换链描述
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; // 双缓冲
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    // 创建交换链
    ComPtr<IDXGISwapChain1> swapChain1;
    hr = factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    if (FAILED(hr)) return hr;

    // 获取 IDXGISwapChain3 接口
    return swapChain1.As(&m_swapChain);
}

HRESULT CoD3D12Device::CreateRenderTargetViews()
{
    HRESULT hr = S_OK;

    // 创建 RTV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    DX12_THROW_IF_FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 创建渲染目标视图
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < 2; n++)
    {
        // 获取交换链缓冲区
        DX12_THROW_IF_FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));

        // 创建 RTV
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    return S_OK;
}

HRESULT CoD3D12Device::CreateDepthStencil()
{
    // 创建 DSV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    DX12_THROW_IF_FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // 创建深度模板资源
    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_width;
    depthStencilDesc.Height = m_height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    DX12_THROW_IF_FAILED(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencil))
    );

    // 创建 DSV
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    m_device->CreateDepthStencilView(
        m_depthStencil.Get(),
        &dsvDesc,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    return S_OK;
}

HRESULT CoD3D12Device::CreateDescriptorHeaps()
{
    // 创建 CBV/SRV/UAV 堆
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = 1000; // 根据需要调整
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvSrvHeapDesc.NodeMask = 0;

    HRESULT hr = m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap));
    if (SUCCEEDED(hr))
    {
        m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    return hr;
}

HRESULT CoD3D12Device::BeginFrame()
{
    HRESULT hr = S_OK;

    try {
        // 重置命令分配器
        DX12_THROW_IF_FAILED(m_commandAllocator->Reset());

        // 重置命令列表
        DX12_THROW_IF_FAILED(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

        // 设置资源屏障 - 将当前渲染目标从呈现转换到渲染目标
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        m_commandList->ResourceBarrier(1, &barrier);

        // 设置渲染目标
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        rtvHandle.Offset(m_frameIndex, m_rtvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        // 设置视口和剪裁矩形
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);
    }
    catch (const std::exception& e) {
        std::cerr << "BeginFrame 失败: " << e.what() << std::endl;
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CoD3D12Device::EndFrame()
{
    HRESULT hr = S_OK;

    try {
        // 设置资源屏障 - 将渲染目标转换回呈现状态
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        m_commandList->ResourceBarrier(1, &barrier);

        // 关闭命令列表
        DX12_THROW_IF_FAILED(m_commandList->Close());

        // 执行命令列表
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // 呈现
        DX12_THROW_IF_FAILED(m_swapChain->Present(1, 0));

        // 等待 GPU 完成
        WaitForPreviousFrame();

        // 切换帧索引
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }
    catch (const std::exception& e) {
        std::cerr << "EndFrame 失败: " << e.what() << std::endl;
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CoD3D12Device::PresentDX12(UINT syncInterval, UINT flags)
{
    return m_swapChain->Present(syncInterval, flags);
}

HRESULT CoD3D12Device::SetRenderTarget(ID3D12Resource* renderTarget)
{
    if (renderTarget == nullptr)
        return E_INVALIDARG;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
    m_device->CreateRenderTargetView(renderTarget, nullptr, rtvHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    return S_OK;
}

HRESULT CoD3D12Device::ClearRenderTarget(const FLOAT colorRGBA[4])
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(m_frameIndex, m_rtvDescriptorSize);

    m_commandList->ClearRenderTargetView(rtvHandle, colorRGBA, 0, nullptr);

    // 清除深度模板
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    return S_OK;
}

void CoD3D12Device::SetViewport(UINT x, UINT y, UINT width, UINT height)
{
    m_viewport.TopLeftX = static_cast<float>(x);
    m_viewport.TopLeftY = static_cast<float>(y);
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
}

void CoD3D12Device::SetScissorRect(LONG left, LONG top, LONG right, LONG bottom)
{
    m_scissorRect.left = left;
    m_scissorRect.top = top;
    m_scissorRect.right = right;
    m_scissorRect.bottom = bottom;
}

HRESULT CoD3D12Device::WaitForPreviousFrame()
{
    HRESULT hr = S_OK;

    // 信号并等待
    UINT64 fenceValue = m_fenceValue;
    DX12_THROW_IF_FAILED(m_commandQueue->Signal(m_fence.Get(), fenceValue));
    m_fenceValue++;

    // 等待上一帧完成
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        DX12_THROW_IF_FAILED(m_fence->SetEventOnCompletion(fenceValue, NULL));
        // 使用 HANDLE 进行等待
    }

    return S_OK;
}

HRESULT CoD3D12Device::ResetCommandList()
{
    return m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

// ============================================
// IUnknown 接口实现
// ============================================

STDMETHODIMP CoD3D12Device::QueryInterface(REFIID riid, PPVOID ppv)
{
    if (IsEqualGUID(riid, __uuidof(IUnknown)) || IsEqualGUID(riid, IID_I4DyuchiGXRenderer))
    {
        *ppv = static_cast<I4DyuchiGXRenderer*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CoD3D12Device::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

STDMETHODIMP_(ULONG) CoD3D12Device::Release(void)
{
    ULONG refCount = InterlockedDecrement(&m_dwRefCount);
    if (refCount == 0)
    {
        Cleanup();
        delete this;
    }
    return refCount;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - P0 关键方法
// ============================================

BOOL __stdcall CoD3D12Device::Create(HWND hWnd, DISPLAY_INFO* pInfo, I4DyuchiFileStorage* pFileStorage, ErrorHandleProc pErrorHandleFunc)
{
    if (hWnd == nullptr || pInfo == nullptr)
        return FALSE;

    m_pFileStorage = pFileStorage;
    m_pErrorHandleFunc = pErrorHandleFunc;

    // 转换到 DX12 初始化
    HRESULT hr = Initialize(hWnd, pInfo->dwWidth, pInfo->dwHeight, pInfo->bWindowed ? FALSE : TRUE);

    return SUCCEEDED(hr);
}

void __stdcall CoD3D12Device::BeginRender(SHORT_RECT* pRect, DWORD dwColor, DWORD dwFlag)
{
    // 开始一帧
    BeginFrame();

    // 设置视口和剪裁矩形
    if (pRect)
    {
        SetViewport(pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
        SetScissorRect(pRect->left, pRect->top, pRect->right, pRect->bottom);
    }
    else
    {
        SetViewport(0, 0, m_width, m_height);
        SetScissorRect(0, 0, (LONG)m_width, (LONG)m_height);
    }

    // 清屏
    FLOAT clearColor[4] = {
        ((dwColor >> 16) & 0xFF) / 255.0f,
        ((dwColor >> 8) & 0xFF) / 255.0f,
        (dwColor & 0xFF) / 255.0f,
        ((dwColor >> 24) & 0xFF) / 255.0f
    };
    ClearRenderTarget(clearColor);
}

void __stdcall CoD3D12Device::EndRender()
{
    // 结束一帧
    EndFrame();
}

void __stdcall CoD3D12Device::Present(HWND hWnd)
{
    PresentDX12(m_bVSync ? 1 : 0, 0);
}

void __stdcall CoD3D12Device::GetClientRect(SHORT_RECT* pRect, WORD* pwWidth, WORD* pwHeight)
{
    if (pRect)
    {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = static_cast<SHORT>(m_width);
        pRect->bottom = static_cast<SHORT>(m_height);
    }
    if (pwWidth) *pwWidth = static_cast<WORD>(m_width);
    if (pwHeight) *pwHeight = static_cast<WORD>(m_height);
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 渲染标志
// ============================================

void __stdcall CoD3D12Device::SetShadowFlag(DWORD dwFlag)
{
    m_dwShadowFlag = dwFlag;
}

DWORD __stdcall CoD3D12Device::GetShadowFlag()
{
    return m_dwShadowFlag;
}

void __stdcall CoD3D12Device::SetLightMapFlag(DWORD dwFlag)
{
    m_dwLightMapFlag = dwFlag;
}

DWORD __stdcall CoD3D12Device::GetLightMapFlag()
{
    return m_dwLightMapFlag;
}

void __stdcall CoD3D12Device::SetRenderMode(DWORD dwFlag)
{
    m_dwRenderMode = dwFlag;
}

DWORD __stdcall CoD3D12Device::GetRenderMode()
{
    return m_dwRenderMode;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 雾效
// ============================================

void __stdcall CoD3D12Device::EnableFog(float fStart, float fEnd, float fDensity, DWORD dwColor, DWORD dwFlag)
{
    m_bFogEnabled = TRUE;
    m_fFogStart = fStart;
    m_fFogEnd = fEnd;
    m_fFogDensity = fDensity;
    m_dwFogColor = dwColor;
}

void __stdcall CoD3D12Device::DisableFog()
{
    m_bFogEnabled = FALSE;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 阴影
// ============================================

BOOL __stdcall CoD3D12Device::BeginShadowMap()
{
    // TODO: 实现阴影贴图渲染
    return TRUE;
}

void __stdcall CoD3D12Device::EndShadowMap()
{
    // TODO: 结束阴影贴图渲染
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 动态光照
// ============================================

DWORD __stdcall CoD3D12Device::CreateDynamicLight(DWORD dwRS, DWORD dwColor, char* szFileName)
{
    if (m_pLightManager == nullptr)
        return 0xFFFFFFFF;

    return m_pLightManager->CreateDynamicLight(dwRS, dwColor, szFileName);
}

BOOL __stdcall CoD3D12Device::DeleteDynamicLight(DWORD dwIndex)
{
    if (m_pLightManager == nullptr)
        return FALSE;

    return m_pLightManager->DeleteDynamicLight(dwIndex);
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 特效着色器
// ============================================

BOOL __stdcall CoD3D12Device::CreateEffectShaderPaletteFromFile(char* szFileName)
{
    // TODO: 加载特效着色器
    return TRUE;
}

BOOL __stdcall CoD3D12Device::CreateEffectShaderPalette(CUSTOM_EFFECT_DESC* pEffectDescList, DWORD dwNum)
{
    // TODO: 创建特效着色器
    return TRUE;
}

void __stdcall CoD3D12Device::DeleteEffectShaderPalette()
{
    // TODO: 删除特效着色器
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 对象创建（第 4 阶段部分实现）
// ============================================

IDISpriteObject* __stdcall CoD3D12Device::CreateSpriteObject(char* szFileName, DWORD dwFlag)
{
    if (m_pSpriteManager == nullptr)
        return nullptr;

    D3D12SpriteObject* pSprite = new D3D12SpriteObject();
    if (FAILED(pSprite->Initialize(this, szFileName, dwFlag)))
    {
        delete pSprite;
        return nullptr;
    }

    void* pHandle = m_pSpriteManager->AddSprite(pSprite);
    return static_cast<IDISpriteObject*>(pHandle);
}

IDISpriteObject* __stdcall CoD3D12Device::CreateSpriteObject(char* szFileName, DWORD dwXPos, DWORD dwYPos, DWORD dwWidth, DWORD dwHeight, DWORD dwFlag)
{
    if (m_pSpriteManager == nullptr)
        return nullptr;

    D3D12SpriteObject* pSprite = new D3D12SpriteObject();
    if (FAILED(pSprite->Initialize(this, szFileName, dwFlag)))
    {
        delete pSprite;
        return nullptr;
    }

    // 设置位置和大小
    pSprite->SetPosition((float)dwXPos, (float)dwYPos);

    void* pHandle = m_pSpriteManager->AddSprite(pSprite);
    return static_cast<IDISpriteObject*>(pHandle);
}

IDISpriteObject* __stdcall CoD3D12Device::CreateEmptySpriteObject(DWORD dwWidth, DWORD dwHeight, TEXTURE_FORMAT TexFormat, DWORD dwFlag)
{
    if (m_pSpriteManager == nullptr)
        return nullptr;

    D3D12SpriteObject* pSprite = new D3D12SpriteObject();
    if (FAILED(pSprite->InitializeEmpty(this, dwWidth, dwHeight, TexFormat, dwFlag)))
    {
        delete pSprite;
        return nullptr;
    }

    void* pHandle = m_pSpriteManager->AddSprite(pSprite);
    return static_cast<IDISpriteObject*>(pHandle);
}

IDIMeshObject* __stdcall CoD3D12Device::CreateMeshObject(CMeshFlag flag)
{
    if (!m_pMeshManager)
        return nullptr;

    D3D12MeshObject* pMesh = m_pMeshManager->CreateMeshObject();
    if (!pMesh)
        return nullptr;

    pMesh->SetMeshFlag(flag);

    void* pHandle = m_pMeshManager->AddMesh(pMesh);
    return static_cast<IDIMeshObject*>(pHandle);
}

IDIFontObject* __stdcall CoD3D12Device::CreateFontObject(LOGFONT* pLogFont, DWORD dwFlag)
{
    if (m_pFontManager == nullptr)
        return nullptr;

    D3D12FontObject* pFont = new D3D12FontObject();
    if (FAILED(pFont->Initialize(this, pLogFont, dwFlag)))
    {
        delete pFont;
        return nullptr;
    }

    void* pHandle = m_pFontManager->AddFont(pFont);
    return static_cast<IDIFontObject*>(pHandle);
}

IDIHeightField* __stdcall CoD3D12Device::CreateHeightField(DWORD dwFlag)
{
    // TODO: 第 16-17 周实现
    return nullptr;
}

IDIMeshObject* __stdcall CoD3D12Device::CreateImmMeshObject(IVERTEX* piv3Tri, DWORD dwTriCount, void* pMtlHandle, DWORD dwFlag)
{
    if (!m_pMeshManager)
        return nullptr;

    // 创建立即网格对象
    D3D12ImmMeshObject* pImmMesh = m_pMeshManager->CreateImmMeshObject(dwTriCount * 3, dwTriCount * 3);
    if (!pImmMesh)
        return nullptr;

    // 更新顶点数据
    if (piv3Tri && dwTriCount > 0)
    {
        pImmMesh->UpdateVertices(piv3Tri, dwTriCount * 3);
    }

    // 设置材质
    pImmMesh->SetMaterial(pMtlHandle);

    void* pHandle = m_pMeshManager->AddMesh(pImmMesh);
    return static_cast<IDIMeshObject*>(pHandle);
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 渲染方法（存根）
// ============================================

BOOL __stdcall CoD3D12Device::RenderMeshObject(
    IDIMeshObject* pMeshObj,
    DWORD dwRefIndex,
    float fDistance,
    DWORD dwAlpha,
    LIGHT_INDEX_DESC* pDynamicLightIndexList, DWORD dwLightNum,
    LIGHT_INDEX_DESC* pSpotLightIndexList, DWORD dwSpotLightNum,
    DWORD dwMtlSetIndex,
    DWORD dwEffectIndex,
    DWORD dwFlag)
{
    if (!pMeshObj || !m_pMeshManager)
        return FALSE;

    D3D12MeshObject* pMesh = static_cast<D3D12MeshObject*>(pMeshObj);

    // TODO: 第 11 周设置材质集
    // TODO: 第 11 周设置动态光照
    // TODO: 第 11 周设置特效
    // TODO: 第 11 周 应用 Alpha

    pMesh->Render(m_commandList.Get());

    return TRUE;
}

BOOL __stdcall CoD3D12Device::RenderSprite(IDISpriteObject* pSprite, VECTOR2* pv2Scaling, float fRot, VECTOR2* pv2Trans, RECT* pRect, DWORD dwColor, int iZOrder, DWORD dwFlag)
{
    if (m_pSpriteManager == nullptr || pSprite == nullptr)
        return FALSE;

    D3D12SpriteObject* pSpriteObj = static_cast<D3D12SpriteObject*>(pSprite);
    return m_pSpriteManager->RenderSprite(pSpriteObj, pv2Scaling, fRot, pv2Trans, pRect, dwColor, iZOrder, dwFlag);
}

BOOL __stdcall CoD3D12Device::RenderFont(IDIFontObject* pFont, TCHAR* str, DWORD dwLen, RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type, int iZOrder, DWORD dwFlag)
{
    if (m_pFontManager == nullptr || pFont == nullptr)
        return FALSE;

    D3D12FontObject* pFontObj = static_cast<D3D12FontObject*>(pFont);
    return m_pFontManager->RenderFont(pFontObj, str, dwLen, pRect, dwColor, type, iZOrder, dwFlag);
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 调试渲染（存根）
// ============================================

void __stdcall CoD3D12Device::RenderBox(VECTOR3* pv3Oct, DWORD dwColor)
{
    // TODO: 实现调试盒渲染
}

void __stdcall CoD3D12Device::RenderPoint(VECTOR3* pv3Point, DWORD dwColor)
{
    // TODO: 实现点渲染
}

void __stdcall CoD3D12Device::RenderCircle(VECTOR2* pv2Point, float fRs, DWORD dwColor)
{
    // TODO: 实现圆渲染
}

void __stdcall CoD3D12Device::RenderLine(VECTOR2* pv2Point0, VECTOR2* pv2Point1, DWORD dwColor)
{
    // TODO: 实现线渲染
}

void __stdcall CoD3D12Device::RenderGrid(VECTOR3* pv3Quad, DWORD dwColor)
{
    // TODO: 实现网格渲染
}

BOOL __stdcall CoD3D12Device::RenderTriIvertex(IVERTEX* piv3Tri, void* pMtlHandle, DWORD dwFacesNum, DWORD dwFlag)
{
    if (!piv3Tri || dwFacesNum == 0 || !m_pMeshManager)
        return FALSE;

    // 创建立即网格对象
    D3D12ImmMeshObject* pImmMesh = m_pMeshManager->CreateImmMeshObject(dwFacesNum * 3, 0);
    if (!pImmMesh)
        return FALSE;

    // 更新顶点数据
    pImmMesh->UpdateVertices(piv3Tri, dwFacesNum * 3);

    // 设置材质
    pImmMesh->SetMaterial(pMtlHandle);

    // 立即渲染
    pImmMesh->RenderImm(m_commandList.Get(), pMtlHandle);

    // 删除临时网格对象
    pImmMesh->Release();

    return TRUE;
}

BOOL __stdcall CoD3D12Device::RenderTriVector3(VECTOR3* pv3Tri, DWORD dwFacesNum, DWORD dwFlag)
{
    if (!pv3Tri || dwFacesNum == 0 || !m_pMeshManager)
        return FALSE;

    // 转换 VECTOR3 到 IVERTEX
    // 使用扩展的顶点格式
    D3D12_IMM_VERTEX* pVertices = new D3D12_IMM_VERTEX[dwFacesNum * 3];
    for (DWORD i = 0; i < dwFacesNum * 3; ++i)
    {
        // 创建临时IVERTEX结构
        IVERTEX tempVert;
        tempVert.x = pv3Tri[i].x;
        tempVert.y = pv3Tri[i].y;
        tempVert.z = pv3Tri[i].z;
        tempVert.u1 = 0.0f;
        tempVert.v1 = 0.0f;

        pVertices[i] = ConvertIVertexToD3D12(tempVert, pv3Tri[i]);
    }

    // 创建立即网格对象
    D3D12ImmMeshObject* pImmMesh = m_pMeshManager->CreateImmMeshObject(dwFacesNum * 3, 0);
    if (pImmMesh)
    {
        pImmMesh->UpdateVertices(pVertices, dwFacesNum * 3);
        pImmMesh->RenderImm(m_commandList.Get(), nullptr);
        pImmMesh->Release();
    }

    delete[] pVertices;

    return TRUE;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 三角形缓冲区（存根）
// ============================================

void* __stdcall CoD3D12Device::AllocRenderTriBuffer(IVERTEX** ppIVList, DWORD dwFacesNum, DWORD dwRenderFlag)
{
    // TODO: 实现三角形缓冲区分配
    return nullptr;
}

void __stdcall CoD3D12Device::EnableRenderTriBuffer(void* pTriBufferHandle, void* pMtlHandle, DWORD dwRenderFacesNum)
{
    // TODO: 实现渲染缓冲区启用
}

void __stdcall CoD3D12Device::DisableRenderTriBuffer(void* pTriBufferHandle)
{
    // TODO: 实现渲染缓冲区禁用
}

void __stdcall CoD3D12Device::FreeRenderTriBuffer(void* pTriBufferHandle)
{
    // TODO: 实现渲染缓冲区释放
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 光照（第 3 阶段实现）
// ============================================

BOOL __stdcall CoD3D12Device::SetRTLight(LIGHT_DESC* pLightDesc, DWORD dwLightIndex, DWORD dwFlag)
{
    if (m_pLightManager == nullptr || pLightDesc == nullptr)
        return FALSE;

    return m_pLightManager->SetRTLight(pLightDesc, dwLightIndex, dwFlag);
}

void __stdcall CoD3D12Device::EnableDirectionalLight(DIRECTIONAL_LIGHT_DESC* pLightDesc, DWORD dwFlag)
{
    if (m_pLightManager == nullptr)
        return;

    if (pLightDesc != nullptr)
        m_pLightManager->EnableDirectionalLight(pLightDesc, dwFlag);
}

void __stdcall CoD3D12Device::DisableDirectionalLight()
{
    if (m_pLightManager == nullptr)
        return;

    m_pLightManager->DisableDirectionalLight();
}

void __stdcall CoD3D12Device::SetSpotLightDesc(VECTOR3* pv3From, VECTOR3* pv3To, VECTOR3* pv3Up, float fFov, float fNear, float fFar, float fWidth, BOOL bOrtho, void* pMtlHandle, DWORD dwColorOP, DWORD dwLightIndex, SPOT_LIGHT_TYPE type)
{
    if (m_pLightManager == nullptr)
        return;

    m_pLightManager->SetSpotLightDesc(pv3From, pv3To, pv3Up, fFov, fNear, fFar, fWidth, bOrtho, pMtlHandle, dwColorOP, dwLightIndex, type);
}

void __stdcall CoD3D12Device::SetShadowLightSenderPosition(BOUNDING_SPHERE* pSphere, DWORD dwLightIndex)
{
    if (m_pLightManager == nullptr || pSphere == nullptr)
        return;

    m_pLightManager->SetShadowLightSenderPosition(pSphere, dwLightIndex);
}

void __stdcall CoD3D12Device::SetViewFrusturm(VIEW_VOLUME* pViewVolume, CAMERA_DESC* camera, MATRIX4* pMatView, MATRIX4* pMatProj, MATRIX4* pMatForBilboard)
{
    // TODO: 设置视图截锥体
    // 这是相机相关的功能，需要在相机系统中实现
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 系统信息
// ============================================

void __stdcall CoD3D12Device::GetSystemStatus(SYSTEM_STATUS* pStatus)
{
    if (pStatus)
    {
        *pStatus = m_systemStatus;
    }
}

void __stdcall CoD3D12Device::UpdateWindowSize()
{
    // TODO: 实现窗口大小更新
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 颜色
// ============================================

void __stdcall CoD3D12Device::SetAmbientColor(DWORD dwColor)
{
    m_dwAmbientColor = dwColor;

    // 更新光照管理器（第 3 阶段）
    if (m_pLightManager != nullptr)
    {
        m_pLightManager->SetAmbientColor(dwColor);
    }
}

DWORD __stdcall CoD3D12Device::GetAmbientColor()
{
    return m_dwAmbientColor;
}

void __stdcall CoD3D12Device::SetEmissiveColor(DWORD dwColor)
{
    m_dwEmissiveColor = dwColor;
}

DWORD __stdcall CoD3D12Device::GetEmissiveColor()
{
    return m_dwEmissiveColor;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 性能分析
// ============================================

void __stdcall CoD3D12Device::BeginPerformanceAnalyze()
{
    // TODO: 开始性能分析
}

void __stdcall CoD3D12Device::EndPerformanceAnalyze()
{
    // TODO: 结束性能分析
}

BOOL __stdcall CoD3D12Device::CaptureScreen(char* szFileName)
{
    // TODO: 实现屏幕截图
    return TRUE;
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 材质（第 2 阶段实现）
// ============================================

DWORD __stdcall CoD3D12Device::CreateMaterialSet(MATERIAL_TABLE* pMtlEntry, DWORD dwNum)
{
    if (m_pMaterialManager == nullptr)
        return 0xFFFFFFFF;

    return m_pMaterialManager->CreateMaterialSet(pMtlEntry, dwNum);
}

void __stdcall CoD3D12Device::DeleteMaterialSet(DWORD dwMtlSetIndex)
{
    if (m_pMaterialManager == nullptr)
        return;

    m_pMaterialManager->DeleteMaterialSet(dwMtlSetIndex);
}

void* __stdcall CoD3D12Device::CreateMaterial(MATERIAL* pMaterial, DWORD* pdwWidth, DWORD* pdwHeight, DWORD dwFlag)
{
    if (m_pMaterialManager == nullptr)
        return nullptr;

    return m_pMaterialManager->CreateMaterial(pMaterial, pdwWidth, pdwHeight, dwFlag);
}

void __stdcall CoD3D12Device::SetMaterialTextureBorder(void* pMtlHandle, DWORD dwColor)
{
    if (m_pMaterialManager == nullptr)
        return;

    m_pMaterialManager->SetMaterialTextureBorder(pMtlHandle, dwColor);
}

void __stdcall CoD3D12Device::DeleteMaterial(void* pMtlHandle)
{
    if (m_pMaterialManager == nullptr)
        return;

    m_pMaterialManager->DeleteMaterial(pMtlHandle);
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 其他设置
// ============================================

void __stdcall CoD3D12Device::SetAttentuation0(float att)
{
    // TODO: 设置衰减参数
}

float __stdcall CoD3D12Device::GetAttentuation0()
{
    // TODO: 获取衰减参数
    return 0.0f;
}

BOOL __stdcall CoD3D12Device::ConvertCompressedTexture(char* szFileName, DWORD dwFlag)
{
    // TODO: 转换压缩纹理
    return TRUE;
}

void __stdcall CoD3D12Device::EnableSpecular(float fVal)
{
    // TODO: 启用高光
}

void __stdcall CoD3D12Device::DisableSpecular()
{
    // TODO: 禁用高光
}

void __stdcall CoD3D12Device::SetVerticalSync(BOOL bSwitch)
{
    m_bVSync = bSwitch;
}

BOOL __stdcall CoD3D12Device::IsSetVerticalSync()
{
    return m_bVSync;
}

void __stdcall CoD3D12Device::ResetDevice(BOOL bTest)
{
    // TODO: 重置设备
}

void __stdcall CoD3D12Device::SetFreeVBCacheRate(float fVal)
{
    // TODO: 设置 VB 缓存释放率
}

float __stdcall CoD3D12Device::GetFreeVBCacheRate()
{
    // TODO: 获取 VB 缓存释放率
    return 0.0f;
}

DWORD __stdcall CoD3D12Device::ClearVBCacheWithIDIMeshObject(IDIMeshObject* pObject)
{
    // TODO: 清除 VB 缓存
    return 0;
}

DWORD __stdcall CoD3D12Device::ClearCacheWithMotionUID(void* pMotionUID)
{
    // TODO: 清除缓存
    return 0;
}

void __stdcall CoD3D12Device::SetTickCount(DWORD dwTickCount, BOOL bGameFrame)
{
    // TODO: 设置刻度计数
}

BOOL __stdcall CoD3D12Device::GetD3DDevice(REFIID refiid, void** ppVoid)
{
    if (ppVoid == nullptr)
        return FALSE;

    // 转换到 DX12 设备
    if (refiid == __uuidof(ID3D12Device))
    {
        *ppVoid = m_device.Get();
        (*reinterpret_cast<ID3D12Device**>(ppVoid))->AddRef();
        return TRUE;
    }
    else if (refiid == __uuidof(ID3D12GraphicsCommandList))
    {
        *ppVoid = m_commandList.Get();
        (*reinterpret_cast<ID3D12GraphicsCommandList**>(ppVoid))->AddRef();
        return TRUE;
    }

    return FALSE;
}

BOOL __stdcall CoD3D12Device::InitializeRenderTarget(DWORD dwTexelSize, DWORD dwMaxTexNum)
{
    // TODO: 初始化渲染目标
    return TRUE;
}

void __stdcall CoD3D12Device::SetRenderTextureMustUpdate(BOOL bMustUpdate)
{
    // TODO: 设置渲染纹理更新标志
}

void __stdcall CoD3D12Device::SetAlphaRefValue(DWORD dwRefVaule)
{
    // TODO: 设置 Alpha 参考值
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 纹理加载失败处理
// ============================================

BOOL __stdcall CoD3D12Device::SetLoadFailedTextureTable(TEXTURE_TABLE* pLoadFailedTextureTable, DWORD dwLoadFailedTextureTableSize)
{
    // TODO: 设置加载失败的纹理表
    return TRUE;
}

VOID __stdcall CoD3D12Device::GetLoadFailedTextureTable(TEXTURE_TABLE** ppoutLoadFailedTextureTable, DWORD* poutdwLoadFailedTextureTableSize, DWORD* poutdwFailedTextureCount)
{
    // TODO: 获取加载失败的纹理表
}

// ============================================
// I4DyuchiGXRenderer 接口实现 - 渲染模式
// ============================================

VOID __stdcall CoD3D12Device::SetRenderWireSolidBothMode(BOOL bMode)
{
    // TODO: 设置渲染模式
}

BOOL __stdcall CoD3D12Device::GetRenderWireSolidBothMode(VOID)
{
    // TODO: 获取渲染模式
    return FALSE;
}

// ============================================
// DX12 辅助方法实现
// ============================================

D3D12_CPU_DESCRIPTOR_HANDLE CoD3D12Device::GetCurrentRenderTargetView()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(m_frameIndex, m_rtvDescriptorSize);
    return rtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE CoD3D12Device::GetDepthStencilView()
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* CoD3D12Device::GetCurrentRenderTargetResource()
{
    return m_renderTargets[m_frameIndex].Get();
}

void CoD3D12Device::WaitForGpu()
{
    WaitForPreviousFrame();
}

// ============================================
// 第6-8阶段：高级管理器访问方法实现
// ============================================

D3D12MeshManager* CoD3D12Device::GetMeshManager() const
{
    return m_pMeshManager;
}

D3D12ShadowManager* CoD3D12Device::GetShadowManager() const
{
    return m_pShadowManager;
}

D3D12PostProcessManager* CoD3D12Device::GetPostProcessManager() const
{
    return m_pPostProcessManager;
}

D3D12TerrainManager* CoD3D12Device::GetTerrainManager() const
{
    return m_pTerrainManager;
}

D3D12RenderScheduler* CoD3D12Device::GetRenderScheduler() const
{
    return m_pRenderScheduler;
}
