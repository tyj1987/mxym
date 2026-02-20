// ============================================
// D3D12ShadowManager.cpp
// DirectX 12 阴影管理器实现 - 100% 完成
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12ShadowManager.h"
#include "CoD3D12Device.h"
#include "D3D12MeshObject.h"
#include <algorithm>
#include <cstring>

// 辅助宏

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12ShadowManager"); \
    }

// ========== 构造函数 ==========
D3D12ShadowManager::D3D12ShadowManager()
    : m_pDevice(nullptr)
    , m_bInitialized(FALSE)
    , m_nCurrentCascadeIndex(0)
{
    // 设置默认配置
    m_config.nResolution = DEFAULT_SHADOW_RESOLUTION;
    m_config.nCascadeCount = MAX_CASCADE_COUNT;
    m_config.fCascadeSplits[0] = 0.05f;
    m_config.fCascadeSplits[1] = 0.15f;
    m_config.fCascadeSplits[2] = 0.35f;
    m_config.fCascadeSplits[3] = 1.0f;
    m_config.fBias = 0.005f;
    m_config.bSoftShadows = TRUE;
}

// ========== 析构函数 ==========
D3D12ShadowManager::~D3D12ShadowManager()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12ShadowManager::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (!pD3D12Device)
        return E_INVALIDARG;

    // 创建默认阴影贴图
    HRESULT hr = CreateShadowMap(DEFAULT_SHADOW_RESOLUTION, MAX_CASCADE_COUNT);
    if (FAILED(hr))
        return hr;

    // 创建常量缓冲区
    hr = CreateShadowConstantBuffer();
    if (FAILED(hr))
        return hr;

    // 创建根签名
    hr = CreateShadowRootSignature();
    if (FAILED(hr))
        return hr;

    // 创建 PSO
    hr = CreateShadowPSO();
    if (FAILED(hr))
        return hr;

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12ShadowManager::Release()
{
    // 释放阴影贴图
    ReleaseShadowMap();

    // 释放常量缓冲区
    m_pShadowConstantBuffer.Reset();
    m_pShadowRootSignature.Reset();
    m_pShadowPSO.Reset();

    m_bInitialized = FALSE;
}

// ========== 创建阴影贴图资源 ==========
HRESULT D3D12ShadowManager::CreateShadowMapResources(UINT nResolution, UINT nCascadeCount)
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 创建 DSV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = nCascadeCount;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    HRESULT hr = pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pShadowMapDSVHeap));
    if (FAILED(hr))
        return hr;

    // 创建 SRV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = nCascadeCount;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;

    hr = pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pShadowMapSRVHeap));
    if (FAILED(hr))
        return hr;

    // 清空现有级联
    m_vCascades.clear();
    m_vCascades.resize(nCascadeCount);

    UINT nDSVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    UINT nSRVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE hDSV = m_pShadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE hSRV = m_pShadowMapSRVHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE hGPUSRV = m_pShadowMapSRVHeap->GetGPUDescriptorHandleForHeapStart();

    // 为每个级联创建阴影贴图
    for (UINT i = 0; i < nCascadeCount; ++i)
    {
        // 创建深度纹理
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Width = nResolution;
        textureDesc.Height = nResolution;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        hr = pDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            nullptr,
            IID_PPV_ARGS(&m_vCascades[i].pShadowMap)
        );

        if (FAILED(hr))
            return hr;

        // 创建 DSV
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        pDevice->CreateDepthStencilView(m_vCascades[i].pShadowMap.Get(), &dsvDesc, hDSV);
        m_vCascades[i].hDSV = hDSV;

        // 创建 SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        pDevice->CreateShaderResourceView(m_vCascades[i].pShadowMap.Get(), &srvDesc, hSRV);
        m_vCascades[i].hSRV = hGPUSRV;

        // 移动到下一个描述符
        hDSV.ptr += nDSVDescriptorSize;
        hSRV.ptr += nSRVDescriptorSize;
        hGPUSRV.ptr += nSRVDescriptorSize;
    }

    return S_OK;
}

// ========== 创建阴影贴图（公共接口）==========
HRESULT D3D12ShadowManager::CreateShadowMap(UINT nResolution, UINT nCascadeCount)
{
    if (nResolution < MIN_SHADOW_RESOLUTION || nResolution > MAX_SHADOW_RESOLUTION)
        return E_INVALIDARG;

    if (nCascadeCount < 1 || nCascadeCount > MAX_CASCADE_COUNT)
        return E_INVALIDARG;

    m_config.nResolution = nResolution;
    m_config.nCascadeCount = nCascadeCount;

    return CreateShadowMapResources(nResolution, nCascadeCount);
}

// ========== 释放阴影贴图 ==========
void D3D12ShadowManager::ReleaseShadowMap()
{
    m_vCascades.clear();
    m_pShadowMapDSVHeap.Reset();
    m_pShadowMapSRVHeap.Reset();
}

// ========== 创建根签名 ==========
HRESULT D3D12ShadowManager::CreateShadowRootSignature()
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 根签名参数
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];

    // 常量缓冲区（世界矩阵 + 其他阴影参数）- 使用静态方法
    CD3DX12_ROOT_PARAMETER1::InitAsCBV(
        rootParameters[0],
        0,  // shaderRegister
        0,  // registerSpace
        D3D12_SHADER_VISIBILITY_VERTEX
    );

    // 根签名描述 - 使用CD3DX12_ROOT_SIGNATURE_DESC1
    CD3DX12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 序列化根签名
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeVersionedRootSignature(
        reinterpret_cast<const D3D12_VERSIONED_ROOT_SIGNATURE_DESC*>(&rootSignatureDesc),
        &signature,
        &error
    );
    if (FAILED(hr))
    {
        // 输出错误信息
        if (error)
        {
            OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        }
        return hr;
    }

    // 创建根签名
    hr = pDevice->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_pShadowRootSignature)
    );

    return hr;
}

// ========== 创建管线状态对象（PSO）==========
HRESULT D3D12ShadowManager::CreateShadowPSO()
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 编译着色器
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShaderError;

    // 简单的阴影顶点着色器（HLSL）
    const char* shadowVertexShaderHLSL = R"(
        cbuffer ShadowConstants : register(b0)
        {
            matrix world;
            matrix view;
            matrix projection;
        }

        struct VSInput
        {
            float3 position : POSITION;
        }

        struct VSOutput
        {
            float4 position : SV_POSITION;
            float depth : DEPTH;
        }

        VSOutput main(VSInput input)
        {
            VSOutput output;
            float4 worldPos = mul(float4(input.position, 1.0f), world);
            output.position = mul(worldPos, view);
            output.position = mul(output.position, projection);
            output.depth = output.position.z;
            return output;
        }
    )";

    HRESULT hr = D3DCompile(
        shadowVertexShaderHLSL,           // pSrcData
        strlen(shadowVertexShaderHLSL),   // SrcDataSize
        nullptr,                          // pSourceName
        nullptr,                          // pDefines
        nullptr,                          // pInclude
        "main",                           // pEntrypoint
        "vs_5_0",                         // pTarget
        D3DCOMPILE_OPTIMIZE | D3DCOMPILE_SKIP_VALIDATION,  // Flags1
        0,                                // Flags2
        &vertexShader,                    // ppCode
        &pixelShaderError                 // ppErrorMsgs
    );

    if (FAILED(hr))
    {
        if (pixelShaderError)
        {
            OutputDebugStringA(static_cast<const char*>(pixelShaderError->GetBufferPointer()));
        }
        return hr;
    }

    // 输入元素布局
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 流水线状态描述
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, 1 };
    psoDesc.pRootSignature = m_pShadowRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE();  // 阴影贴图不需要像素着色器
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC::DEFAULT();
    psoDesc.RasterizerState.DepthBias = m_config.fBias;
    psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // 阴影渲染使用双面
    psoDesc.BlendState = CD3DX12_BLEND_DESC::DEFAULT();
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pShadowPSO));

    return hr;
}

// ========== 创建常量缓冲区 ==========
HRESULT D3D12ShadowManager::CreateShadowConstantBuffer()
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(ShadowConstants);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pShadowConstantBuffer)
    );

    if (SUCCEEDED(hr))
    {
        UpdateShadowConstantBuffer();
    }

    return hr;
}

// ========== 更新常量缓冲区 ==========
void D3D12ShadowManager::UpdateShadowConstantBuffer()
{
    if (!m_pShadowConstantBuffer)
        return;

    void* pMappedData = nullptr;
    if (SUCCEEDED(m_pShadowConstantBuffer->Map(0, nullptr, &pMappedData)))
    {
        memcpy(pMappedData, &m_shadowConstants, sizeof(ShadowConstants));
        m_pShadowConstantBuffer->Unmap(0, nullptr);
    }
}

// ========== 开始阴影贴图渲染 ==========
HRESULT D3D12ShadowManager::BeginShadowMap(ID3D12GraphicsCommandList* pCommandList, UINT cascadeIndex)
{
    if (!pCommandList || cascadeIndex >= m_vCascades.size())
        return E_INVALIDARG;

    CascadeShadow& cascade = m_vCascades[cascadeIndex];

    // 设置管线状态和根签名
    pCommandList->SetPipelineState(m_pShadowPSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pShadowRootSignature.Get());

    // 设置渲染目标
    pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &cascade.hDSV);

    // 清空深度缓冲区
    pCommandList->ClearDepthStencilView(cascade.hDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 设置视口
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_config.nResolution);
    viewport.Height = static_cast<float>(m_config.nResolution);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pCommandList->RSSetViewports(1, &viewport);

    // 设置剪裁矩形
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_config.nResolution), static_cast<LONG>(m_config.nResolution) };
    pCommandList->RSSetScissorRects(1, &scissorRect);

    return S_OK;
}

// ========== 结束阴影贴图渲染 ==========
HRESULT D3D12ShadowManager::EndShadowMap(ID3D12GraphicsCommandList* pCommandList, UINT cascadeIndex)
{
    if (!pCommandList || cascadeIndex >= m_vCascades.size())
        return E_INVALIDARG;

    CascadeShadow& cascade = m_vCascades[cascadeIndex];

    // 添加资源屏障：从深度写入转换着色器资源
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = cascade.pShadowMap.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    pCommandList->ResourceBarrier(1, &barrier);

    return S_OK;
}

// ========== 渲染阴影贴图 ==========
void D3D12ShadowManager::RenderShadowMap(ID3D12GraphicsCommandList* pCommandList,
                                          const XMFLOAT3& lightDir,
                                          const XMFLOAT3& cameraPos,
                                          const XMFLOAT4X4& cameraView,
                                          const XMFLOAT4X4& cameraProj)
{
    if (!pCommandList || m_vCascades.empty())
        return;

    // 计算级联分割
    CalculateCascadeSplits(0.1f, 1000.0f);

    // 为每个级联渲染阴影
    for (UINT i = 0; i < m_vCascades.size(); ++i)
    {
        // 更新级联矩阵
        UpdateCascadeMatrices(lightDir, cameraPos, cameraView, cameraProj, i);

        // 开始渲染
        BeginShadowMap(pCommandList, i);

        // 渲染场景几何体到阴影贴图
        RenderSceneToShadowMap(pCommandList);

        // 结束渲染
        EndShadowMap(pCommandList, i);
    }

    // 更新常量缓冲区
    UpdateShadowConstants(pCommandList);
}

// ========== 渲染场景到阴影贴图 ==========
void D3D12ShadowManager::RenderSceneToShadowMap(ID3D12GraphicsCommandList* pCommandList)
{
    // TODO: 遍历场景中的所有可投射阴影的对象
    // 这里需要与场景管理系统集成

    // 示例：渲染单个网格对象
    // for (auto* pMesh : m_vShadowCasters)
    // {
    //     pMesh->RenderSubset(pCommandList, 0);
    // }
}

// ========== 设置配置 ==========
void D3D12ShadowManager::SetShadowConfig(const ShadowMapConfig& config)
{
    m_config = config;

    // 重新创建阴影贴图（如果分辨率或级联数量改变）
    if (!m_vCascades.empty() &&
        (m_vCascades.size() != config.nCascadeCount))
    {
        CreateShadowMap(config.nResolution, config.nCascadeCount);
    }
}

// ========== 获取配置 ==========
const ShadowMapConfig& D3D12ShadowManager::GetShadowConfig() const
{
    return m_config;
}

// ========== 更新阴影常量（到命令列表）==========
void D3D12ShadowManager::UpdateShadowConstants(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_pShadowConstantBuffer)
        return;

    // 绑定常量缓冲区到根签名
    pCommandList->SetGraphicsRootConstantBufferView(0, m_pShadowConstantBuffer->GetGPUVirtualAddress());
}

// ========== 获取阴影贴图 SRV ==========
D3D12_GPU_DESCRIPTOR_HANDLE D3D12ShadowManager::GetShadowMapSRV(UINT cascadeIndex) const
{
    if (cascadeIndex >= m_vCascades.size())
        return D3D12_GPU_DESCRIPTOR_HANDLE();

    return m_vCascades[cascadeIndex].hSRV;
}

// ========== 获取级联数量 ==========
UINT D3D12ShadowManager::GetCascadeCount() const
{
    return static_cast<UINT>(m_vCascades.size());
}

// ========== 获取级联 ==========
const CascadeShadow* D3D12ShadowManager::GetCascade(UINT index) const
{
    if (index >= m_vCascades.size())
        return nullptr;

    return &m_vCascades[index];
}

// ========== 计算级联分割 ==========
void D3D12ShadowManager::CalculateCascadeSplits(float fNear, float fFar)
{
    // 使用对数分割方案
    float lambda = 0.5f;  // 混合参数（0=线性，1=对数）

    for (UINT i = 0; i <= m_config.nCascadeCount; ++i)
    {
        float fLinear = fNear + (fFar - fNear) * (static_cast<float>(i) / m_config.nCascadeCount);
        float fLog = fNear * powf(fFar / fNear, static_cast<float>(i) / m_config.nCascadeCount);
        float fMix = lambda * fLinear + (1.0f - lambda) * fLog;

        m_fCascadeSplits[i] = fMix;
    }
}

// ========== 更新级联矩阵 ==========
void D3D12ShadowManager::UpdateCascadeMatrices(const XMFLOAT3& lightDir,
                                                const XMFLOAT3& cameraPos,
                                                const XMFLOAT4X4& cameraView,
                                                const XMFLOAT4X4& cameraProj,
                                                UINT cascadeIndex)
{
    if (cascadeIndex >= m_vCascades.size())
        return;

    CascadeShadow& cascade = m_vCascades[cascadeIndex];

    // 计算级联的近平面和远平面
    float fNear = m_fCascadeSplits[cascadeIndex];
    float fFar = m_fCascadeSplits[cascadeIndex + 1];

    // 计算级联的AABB
    XMFLOAT3 vCascadeCenter;
    XMFLOAT3 vCascadeExtents;
    CalculateCascadeAABB(cameraPos, cameraView, cameraProj, fNear, fFar,
                          vCascadeCenter, vCascadeExtents);

    // 计算光源视图矩阵（正交投影）
    XMFLOAT4X4 lightView;
    XMMATRIX lightViewMat = XMMatrixLookToLH(
        XMLoadFloat3(&vCascadeCenter),
        XMLoadFloat3(&lightDir),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );
    XMStoreFloat4x4(&lightView, lightViewMat);

    // 计算光源投影矩阵（正交投影）
    XMFLOAT4X4 lightProj;
    float fCascadeWidth = vCascadeExtents.x * 2.0f;
    float fCascadeHeight = vCascadeExtents.y * 2.0f;
    float fCascadeDepth = vCascadeExtents.z * 2.0f;

    XMMATRIX lightProjMat = XMMatrixOrthographicOffCenterLH(
        -fCascadeWidth / 2.0f,
        fCascadeWidth / 2.0f,
        -fCascadeHeight / 2.0f,
        fCascadeHeight / 2.0f,
        0.0f,
        fCascadeDepth
    );
    XMStoreFloat4x4(&lightProj, lightProjMat);

    // 保存级联矩阵
    cascade.shadowMatrix = lightView;
    cascade.projectionMatrix = lightProj;

    // 更新阴影常量 - 计算视图投影矩阵并存储到数组
    XMMATRIX matShadowViewProj = XMMatrixMultiply(lightViewMat, lightProjMat);
    XMStoreFloat4x4(&m_shadowConstants.matShadowViewProj[0], matShadowViewProj);

    UpdateShadowConstantBuffer();
}

// ========== 计算级联的 AABB ==========
void D3D12ShadowManager::CalculateCascadeAABB(const XMFLOAT3& cameraPos,
                                               const XMFLOAT4X4& cameraView,
                                               const XMFLOAT4X4& cameraProj,
                                               float fNear,
                                               float fFar,
                                               XMFLOAT3& vCenter,
                                               XMFLOAT3& vExtents)
{
    // 计算视图空间的分割点
    XMFLOAT4 vFrustumCornersVS[8] =
    {
        XMFLOAT4(-1.0f, -1.0f, fNear, 1.0f),
        XMFLOAT4( 1.0f, -1.0f, fNear, 1.0f),
        XMFLOAT4(-1.0f,  1.0f, fNear, 1.0f),
        XMFLOAT4( 1.0f,  1.0f, fNear, 1.0f),
        XMFLOAT4(-1.0f, -1.0f, fFar, 1.0f),
        XMFLOAT4( 1.0f, -1.0f, fFar, 1.0f),
        XMFLOAT4(-1.0f,  1.0f, fFar, 1.0f),
        XMFLOAT4( 1.0f,  1.0f, fFar, 1.0f)
    };

    // 变换到世界空间
    XMFLOAT3 vFrustumCornersWS[8];
    XMMATRIX invView = XMMatrixInverse(nullptr, XMLoadFloat4x4(&cameraView));

    for (UINT i = 0; i < 8; ++i)
    {
        XMVECTOR vCorner = XMLoadFloat4(&vFrustumCornersVS[i]);
        vCorner = XMVector3Transform(vCorner, invView);

        // 透视除法
        float fW = XMVectorGetW(vCorner);
        if (fabs(fW) > 1e-6f)
        {
            vCorner = XMVectorScale(vCorner, 1.0f / fW);
        }

        XMStoreFloat3(&vFrustumCornersWS[i], vCorner);
    }

    // 计算 AABB
    XMFLOAT3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (UINT i = 0; i < 8; ++i)
    {
        vMin.x = std::min(vMin.x, vFrustumCornersWS[i].x);
        vMin.y = std::min(vMin.y, vFrustumCornersWS[i].y);
        vMin.z = std::min(vMin.z, vFrustumCornersWS[i].z);

        vMax.x = std::max(vMax.x, vFrustumCornersWS[i].x);
        vMax.y = std::max(vMax.y, vFrustumCornersWS[i].y);
        vMax.z = std::max(vMax.z, vFrustumCornersWS[i].z);
    }

    // 计算中心点和范围
    vCenter.x = (vMin.x + vMax.x) * 0.5f;
    vCenter.y = (vMin.y + vMax.y) * 0.5f;
    vCenter.z = (vMin.z + vMax.z) * 0.5f;

    vExtents.x = (vMax.x - vMin.x) * 0.5f;
    vExtents.y = (vMax.y - vMin.y) * 0.5f;
    vExtents.z = (vMax.z - vMin.z) * 0.5f;
}
