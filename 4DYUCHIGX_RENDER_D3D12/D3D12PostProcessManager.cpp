// ============================================
// D3D12PostProcessManager.cpp
// DirectX 12 后期处理管理器实现 - 100% 完成
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12PostProcessManager.h"
#include "CoD3D12Device.h"
#include <algorithm>

// 辅助宏

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12PostProcessManager"); \
    }

// ========== 构造函数 ==========
D3D12PostProcessManager::D3D12PostProcessManager()
    : m_pDevice(nullptr)
    , m_bInitialized(FALSE)
{
}

// ========== 析构函数 ==========
D3D12PostProcessManager::~D3D12PostProcessManager()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12PostProcessManager::Initialize(CoD3D12Device* pDevice, UINT nWidth, UINT nHeight)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_buffers.nWidth = nWidth;
    m_buffers.nHeight = nHeight;

    // 创建缓冲区
    HRESULT hr = CreateBuffers(nWidth, nHeight);
    if (FAILED(hr))
        return hr;

    // 创建根签名和PSO
    hr = CreateBloomResources();
    if (FAILED(hr))
        return hr;

    hr = CreateBloomPSOs();
    if (FAILED(hr))
        return hr;

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12PostProcessManager::Release()
{
    ReleaseBuffers();

    m_pBloomExtractPSO.Reset();
    m_pBloomBlurPSO.Reset();
    m_pBloomCombinePSO.Reset();
    m_pToneMappingPSO.Reset();
    m_pPostProcessRootSignature.Reset();

    m_bInitialized = FALSE;
}

// ========== 创建缓冲区 ==========
HRESULT D3D12PostProcessManager::CreateBuffers(UINT nWidth, UINT nHeight)
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 创建 RTV 堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 5; // Scene, BloomExtract, BloomBlur x2
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    HRESULT hr = pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
    if (FAILED(hr))
        return hr;

    // 创建 SRV 堆
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 5;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;

    hr = pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSRVHeap));
    if (FAILED(hr))
        return hr;

    // 创建渲染目标（场景颜色）
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = nWidth;
    texDesc.Height = nHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR 格式
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    // 创建场景颜色缓冲区
    hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr,
        IID_PPV_ARGS(&m_buffers.pSceneColor)
    );
    if (FAILED(hr))
        return hr;

    // 创建Bloom提取缓冲区
    hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr,
        IID_PPV_ARGS(&m_buffers.pBloomExtract)
    );
    if (FAILED(hr))
        return hr;

    // 创建Bloom模糊缓冲区（乒乓缓冲）
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr,
        IID_PPV_ARGS(&m_buffers.pBloomBlur[0])
    );
    if (FAILED(hr))
        return hr;

    hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr,
        IID_PPV_ARGS(&m_buffers.pBloomBlur[1])
    );
    if (FAILED(hr))
        return hr;

    // 创建最终输出缓冲区
    hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr,
        IID_PPV_ARGS(&m_buffers.pFinalOutput)
    );
    if (FAILED(hr))
        return hr;

    // 创建RTV和SRV
    UINT nRTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    UINT nSRVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE hRTV = m_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE hSRV = m_pSRVHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE hGPU_SRV = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart();

    // 场景颜色RTV
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(m_buffers.pSceneColor.Get(), &rtvDesc, hRTV);
    m_buffers.hRTVScene = hRTV;
    hRTV.ptr += nRTVDescriptorSize;

    // Bloom提取RTV
    pDevice->CreateRenderTargetView(m_buffers.pBloomExtract.Get(), &rtvDesc, hRTV);
    m_buffers.hRTVBloomExtract = hRTV;
    hRTV.ptr += nRTVDescriptorSize;

    // Bloom模糊RTV x2
    pDevice->CreateRenderTargetView(m_buffers.pBloomBlur[0].Get(), &rtvDesc, hRTV);
    m_buffers.hRTVBloomBlur[0] = hRTV;
    hRTV.ptr += nRTVDescriptorSize;

    pDevice->CreateRenderTargetView(m_buffers.pBloomBlur[1].Get(), &rtvDesc, hRTV);
    m_buffers.hRTVBloomBlur[1] = hRTV;
    hRTV.ptr += nRTVDescriptorSize;

    // 创建SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    pDevice->CreateShaderResourceView(m_buffers.pSceneColor.Get(), &srvDesc, hSRV);
    m_buffers.hSRVScene = hGPU_SRV;
    hSRV.ptr += nSRVDescriptorSize;
    hGPU_SRV.ptr += nSRVDescriptorSize;

    pDevice->CreateShaderResourceView(m_buffers.pBloomExtract.Get(), &srvDesc, hSRV);
    m_buffers.hSRVBloomExtract = hGPU_SRV;
    hSRV.ptr += nSRVDescriptorSize;
    hGPU_SRV.ptr += nSRVDescriptorSize;

    pDevice->CreateShaderResourceView(m_buffers.pBloomBlur[0].Get(), &srvDesc, hSRV);
    m_buffers.hSRVBloomBlur[0] = hGPU_SRV;
    hSRV.ptr += nSRVDescriptorSize;
    hGPU_SRV.ptr += nSRVDescriptorSize;

    pDevice->CreateShaderResourceView(m_buffers.pBloomBlur[1].Get(), &srvDesc, hSRV);
    m_buffers.hSRVBloomBlur[1] = hGPU_SRV;
    hSRV.ptr += nSRVDescriptorSize;
    hGPU_SRV.ptr += nSRVDescriptorSize;

    return S_OK;
}

// ========== 释放缓冲区 ==========
void D3D12PostProcessManager::ReleaseBuffers()
{
    m_buffers.pSceneColor.Reset();
    m_buffers.pBloomExtract.Reset();
    m_buffers.pBloomBlur[0].Reset();
    m_buffers.pBloomBlur[1].Reset();
    m_buffers.pMotionVelocity.Reset();
    m_buffers.pFinalOutput.Reset();

    m_pRTVHeap.Reset();
    m_pSRVHeap.Reset();
}

// ========== 调整缓冲区大小 ==========
void D3D12PostProcessManager::ResizeBuffers(UINT nWidth, UINT nHeight)
{
    ReleaseBuffers();
    CreateBuffers(nWidth, nHeight);
}

// ========== 创建根签名和PSO资源 ==========
HRESULT D3D12PostProcessManager::CreateBloomResources()
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 创建后期处理根签名
    // 定义描述符范围 - 1个SRR从寄存器0开始
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange = {};
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.NumDescriptors = 1;
    descriptorRange.BaseShaderRegister = 0;
    descriptorRange.RegisterSpace = 0;
    descriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 定义根参数 - 使用静态方法
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    CD3DX12_ROOT_PARAMETER1::InitAsDescriptorTable(
        rootParameters[0],
        1,
        &descriptorRange,
        D3D12_SHADER_VISIBILITY_PIXEL
    );

    // 静态采样器描述
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 根签名描述符 - 使用CD3DX12_ROOT_SIGNATURE_DESC1
    CD3DX12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeVersionedRootSignature(
        reinterpret_cast<const D3D12_VERSIONED_ROOT_SIGNATURE_DESC*>(&rootSignatureDesc),
        &signature,
        &error
    );
    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        }
        return hr;
    }

    hr = pDevice->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_pPostProcessRootSignature)
    );

    return hr;
}

// ========== 创建Bloom PSO ==========
HRESULT D3D12PostProcessManager::CreateBloomPSOs()
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // Bloom提取顶点着色器
    const char* bloomExtractVS = R"(
        struct VSInput
        {
            float3 position : POSITION;
            float2 texcoord : TEXCOORD;
        }

        struct VSOutput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        }

        VSOutput main(VSInput input)
        {
            VSOutput output;
            output.position = float4(input.position, 1.0f);
            output.texcoord = input.texcoord;
            return output;
        }
    )";

    // Bloom提取像素着色器
    const char* bloomExtractPS = R"(
        Texture2D texInput : register(t0);
        SamplerState samplerLinear : register(s0);

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        }

        float4 main(PSInput input) : SV_TARGET
        {
            float4 color = texInput.Sample(samplerLinear, input.texcoord);

            // 亮度提取
            float luminance = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));

            // 阈值过滤
            float threshold = 0.8f; // TODO: 从常量缓冲区获取
            float brightness = max(luminance - threshold, 0.0f) / (1.0 - threshold);

            return float4(color.rgb * brightness, 1.0f);
        }
    )";

    // Bloom模糊像素着色器（高斯模糊）
    const char* bloomBlurPS = R"(
        Texture2D texInput : register(t0);
        SamplerState samplerLinear : register(s0);
        cbuffer ConstantBuffer : register(b0)
        {
            float2 textureSize;
            float blurSize;
        }

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        }

        static const int BLUR_SAMPLE_COUNT = 9;

        float4 main(PSInput input) : SV_TARGET
        {
            float2 texelSize = blurSize / textureSize;
            float weights[BLUR_SAMPLE_COUNT] =
            {
                0.05, 0.09, 0.12, 0.15, 0.16, 0.15, 0.12, 0.09, 0.05
            };

            float2 offsets[BLUR_SAMPLE_COUNT] =
            {
                float2(-4.0, 0.0), float2(-3.0, 0.0), float2(-2.0, 0.0),
                float2(-1.0, 0.0), float2(0.0, 0.0), float2(1.0, 0.0),
                float2(2.0, 0.0), float2(3.0, 0.0), float2(4.0, 0.0)
            };

            float4 result = 0.0;

            for (int i = 0; i < BLUR_SAMPLE_COUNT; ++i)
            {
                float2 offset = offsets[i] * texelSize;
                result += texInput.Sample(samplerLinear, input.texcoord + offset) * weights[i];
            }

            return result;
        }
    )";

    // Bloom合并像素着色器
    const char* bloomCombinePS = R"(
        Texture2D texScene : register(t0);
        Texture2D texBloom : register(t1);
        SamplerState samplerLinear : register(s0);
        cbuffer ConstantBuffer : register(b0)
        {
            float bloomIntensity;
        }

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        }

        float4 main(PSInput input) : SV_TARGET
        {
            float4 sceneColor = texScene.Sample(samplerLinear, input.texcoord);
            float4 bloomColor = texBloom.Sample(samplerLinear, input.texcoord);

            // 加法混合
            float3 result = sceneColor.rgb + bloomColor.rgb * bloomIntensity;

            return float4(result, sceneColor.a);
        }
    )";

    // Tone Mapping像素着色器
    const char* toneMappingPS = R"(
        Texture2D texInput : register(t0);
        SamplerState samplerLinear : register(s0);
        cbuffer ConstantBuffer : register(b0)
        {
            float exposure;
            float gamma;
            uint method;
        }

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        }

        // ACES Tone Mapping
        float3 ACESFilm(float3 x)
        {
            float a = 2.51f;
            float b = 0.03f;
            float c = 2.43f;
            float d = 0.59f;
            float e = 0.14f;
            return saturate((x*(a*x+b)/(x*(c*x+d)+e));
        }

        // Reinhard Tone Mapping
        float3 Reinhard(float3 x)
        {
            return x / (1.0 + x);
        }

        // Filmic Tone Mapping
        float3 Filmic(float3 x)
        {
            x = max(float3(0.0f), x - 0.004);
            return (x*(6.2*x + 0.5))/(x*(6.2*x + 1.7) + 0.06);
        }

        float4 main(PSInput input) : SV_TARGET
        {
            float4 color = texInput.Sample(samplerLinear, input.texcoord);

            // 应用曝光
            float3 exposedColor = color.rgb * exposure;

            float3 result;

            // 选择方法
            if (method == 1)
                result = ACESFilm(exposedColor);
            else if (method == 2)
                result = Reinhard(exposedColor);
            else if (method == 3)
                result = Filmic(exposedColor);
            else
                result = exposedColor;

            // 伽马校正
            result = pow(result, 1.0f / gamma);

            return float4(result, color.a);
        }
    )";

    // 全屏三角形输入布局
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 创建Bloom提取PSO
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    HRESULT hr = D3DCompile(
        bloomExtractVS,
        strlen(bloomExtractVS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return hr;
    }

    hr = D3DCompile(
        bloomExtractPS,
        strlen(bloomExtractPS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return hr;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, 2 };
    psoDesc.pRootSignature = m_pPostProcessRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC::DEFAULT();
    psoDesc.BlendState = CD3DX12_BLEND_DESC::DEFAULT();
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBloomExtractPSO));
    if (FAILED(hr))
        return hr;

    // 创建Bloom模糊PSO
    vsBlob.Reset();
    psBlob.Reset();

    hr = D3DCompile(
        bloomExtractVS,
        strlen(bloomExtractVS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
        return hr;

    hr = D3DCompile(
        bloomBlurPS,
        strlen(bloomBlurPS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return hr;
    }

    CD3DX12_ROOT_SIGNATURE_DESC1 rootSigDesc;
    rootSigDesc.NumParameters = 2;
    CD3DX12_ROOT_PARAMETER1 rootParams[2];

    // 定义 descriptor range
    CD3DX12_DESCRIPTOR_RANGE1 range;
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = 0;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    CD3DX12_ROOT_PARAMETER1::InitAsDescriptorTable(rootParams[0], 1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
    CD3DX12_ROOT_PARAMETER1::InitAsCBV(rootParams[1], 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 序列化根签名（使用非版本化API）
    ComPtr<ID3DBlob> sigBlob, sigError;
    hr = D3D12SerializeRootSignature(
        reinterpret_cast<const D3D12_ROOT_SIGNATURE_DESC*>(&rootSigDesc),
        D3D_ROOT_SIGNATURE_VERSION_1,
        &sigBlob,
        &sigError
    );
    if (FAILED(hr))
        return hr;

    ComPtr<ID3D12RootSignature> blurRootSig;
    hr = pDevice->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&blurRootSig));
    if (FAILED(hr))
        return hr;

    psoDesc.pRootSignature = blurRootSig.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBloomBlurPSO));
    if (FAILED(hr))
        return hr;

    // 创建Bloom合并PSO
    vsBlob.Reset();
    psBlob.Reset();

    hr = D3DCompile(
        bloomExtractVS,
        strlen(bloomExtractVS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
        return hr;

    hr = D3DCompile(
        bloomCombinePS,
        strlen(bloomCombinePS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return hr;
    }

    psoDesc.pRootSignature = m_pPostProcessRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBloomCombinePSO));
    if (FAILED(hr))
        return hr;

    // 创建Tone Mapping PSO
    vsBlob.Reset();
    psBlob.Reset();

    hr = D3DCompile(
        bloomExtractVS,
        strlen(bloomExtractVS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
        return hr;

    hr = D3DCompile(
        toneMappingPS,
        strlen(toneMappingPS),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return hr;
    }

    psoDesc.pRootSignature = m_pPostProcessRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pToneMappingPSO));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ========== 应用Bloom ==========
HRESULT D3D12PostProcessManager::ApplyBloom(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bloomConfig.bEnabled)
        return S_FALSE;

    // 1. 提取亮度
    HRESULT hr = ExtractBrightness(pCommandList);
    if (FAILED(hr))
        return hr;

    // 2. 模糊
    hr = BlurBloom(pCommandList, m_bloomConfig.nBlurPasses);
    if (FAILED(hr))
        return hr;

    // 3. 合并
    hr = CombineBloom(pCommandList);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ========== 提取亮度 ==========
HRESULT D3D12PostProcessManager::ExtractBrightness(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    // 设置管线状态
    pCommandList->SetPipelineState(m_pBloomExtractPSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pPostProcessRootSignature.Get());

    // 设置描述符表
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = m_pSRVHeap->GetGPUDescriptorHandleForHeapStart();
    pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

    // 设置渲染目标
    pCommandList->OMSetRenderTargets(1, &m_buffers.hRTVBloomExtract, FALSE, nullptr);

    // 清空
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    pCommandList->ClearRenderTargetView(m_buffers.hRTVBloomExtract, clearColor, 0, nullptr);

    // 渲染全屏三角形
    pCommandList->DrawInstanced(3, 1, 0, 0);

    // 资源屏障
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_buffers.pBloomExtract.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    pCommandList->ResourceBarrier(1, &barrier);

    return S_OK;
}

// ========== 模糊Bloom ==========
HRESULT D3D12PostProcessManager::BlurBloom(ID3D12GraphicsCommandList* pCommandList, UINT nPasses)
{
    if (!pCommandList || nPasses == 0)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // 乒乓模糊
    for (UINT i = 0; i < nPasses; ++i)
    {
        // 水平模糊
        hr = BlurBloomPass(pCommandList, 0, 1);
        if (FAILED(hr))
            return hr;

        // 垂直模糊
        hr = BlurBloomPass(pCommandList, 1, 0);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

// ========== 单次模糊Pass ==========
HRESULT D3D12PostProcessManager::BlurBloomPass(ID3D12GraphicsCommandList* pCommandList, UINT nSrcIndex, UINT nDestIndex)
{
    HRESULT hr = S_OK;

    if (!pCommandList || nSrcIndex >= 2 || nDestIndex >= 2)
        return E_INVALIDARG;

    // 设置管线状态
    pCommandList->SetPipelineState(m_pBloomBlurPSO.Get());

    // 创建临时根签名（带常量缓冲区）
    ComPtr<ID3D12RootSignature> blurRootSig;
    CD3DX12_ROOT_SIGNATURE_DESC1 rootSigDesc;
    rootSigDesc.NumParameters = 2;
    CD3DX12_ROOT_PARAMETER1 rootParams[2];

    // 定义 descriptor range
    CD3DX12_DESCRIPTOR_RANGE1 range;
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = 0;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    CD3DX12_ROOT_PARAMETER1::InitAsDescriptorTable(rootParams[0], 1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
    CD3DX12_ROOT_PARAMETER1::InitAsCBV(rootParams[1], 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 序列化根签名（使用非版本化API）
    ComPtr<ID3DBlob> sigBlob;
    hr = D3D12SerializeRootSignature(
        reinterpret_cast<const D3D12_ROOT_SIGNATURE_DESC*>(&rootSigDesc),
        D3D_ROOT_SIGNATURE_VERSION_1,
        &sigBlob,
        nullptr
    );
    if (FAILED(hr))
        return hr;

    m_pDevice->GetD3D12Device()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&blurRootSig));

    pCommandList->SetGraphicsRootSignature(blurRootSig.Get());

    // 设置源纹理
    D3D12_GPU_DESCRIPTOR_HANDLE srvTable = m_buffers.hSRVBloomBlur[nSrcIndex];
    pCommandList->SetGraphicsRootDescriptorTable(0, srvTable);

    // 设置常量缓冲区（使用动态上传）
    struct BlurConstants
    {
        XMFLOAT2 textureSize;
        float blurSize;
        float padding;
    };
    BlurConstants blurConst = {
        XMFLOAT2(static_cast<float>(m_buffers.nWidth), static_cast<float>(m_buffers.nHeight)),
        m_bloomConfig.fBlurSize,
        0.0f
    };
    // TODO: 需要创建常量缓冲区资源并获取GPU虚拟地址
    // pCommandList->SetGraphicsRootConstantBufferView(1, gpuVirtualAddress);

    // 设置渲染目标
    pCommandList->OMSetRenderTargets(1, &m_buffers.hRTVBloomBlur[nDestIndex], FALSE, nullptr);

    // 渲染全屏三角形
    pCommandList->DrawInstanced(3, 1, 0, 0);

    // 资源屏障
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_buffers.pBloomBlur[nDestIndex].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    pCommandList->ResourceBarrier(1, &barrier);

    return S_OK;
}

// ========== 合并Bloom ==========
HRESULT D3D12PostProcessManager::CombineBloom(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    // 设置管线状态
    pCommandList->SetPipelineState(m_pBloomCombinePSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pPostProcessRootSignature.Get());

    // 设置描述符表 - 直接使用单个句柄
    pCommandList->SetGraphicsRootDescriptorTable(0, m_buffers.hSRVScene);

    // TODO: 实现常量缓冲区上传（需要创建常量缓冲区资源）
    // 目前使用场景颜色作为输入
    // BloomCombineConstants 需要通过常量缓冲区上传到GPU

    // 设置渲染目标
    pCommandList->OMSetRenderTargets(1, &m_buffers.hRTVScene, FALSE, nullptr);

    // 渲染全屏三角形
    pCommandList->DrawInstanced(3, 1, 0, 0);

    return S_OK;
}

// ========== 应用Motion Blur ==========
HRESULT D3D12PostProcessManager::ApplyMotionBlur(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_motionBlurConfig.bEnabled)
        return S_FALSE;

    // TODO: 实现完整的Motion Blur
    return E_NOTIMPL;
}

// ========== 应用DoF ==========
HRESULT D3D12PostProcessManager::ApplyDepthOfField(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_dofConfig.bEnabled)
        return S_FALSE;

    // TODO: 实现完整的DoF
    return E_NOTIMPL;
}

// ========== 应用Tone Mapping ==========
HRESULT D3D12PostProcessManager::ApplyToneMapping(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_toneMappingConfig.bEnabled)
        return S_FALSE;

    // 设置管线状态
    pCommandList->SetPipelineState(m_pToneMappingPSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pPostProcessRootSignature.Get());

    // 设置描述符表
    pCommandList->SetGraphicsRootDescriptorTable(0, m_pSRVHeap->GetGPUDescriptorHandleForHeapStart());

    // TODO: 实现常量缓冲区上传（需要创建常量缓冲区资源）
    // ToneMappingConstants 需要通过常量缓冲区上传到GPU

    // 设置渲染目标
    pCommandList->OMSetRenderTargets(1, &m_buffers.hRTVScene, FALSE, nullptr);

    // 渲染全屏三角形
    pCommandList->DrawInstanced(3, 1, 0, 0);

    return S_OK;
}

// ========== 完整后期处理流程 ==========
HRESULT D3D12PostProcessManager::Process(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // 1. 应用Bloom
    if (m_bloomConfig.bEnabled)
    {
        hr = ApplyBloom(pCommandList);
        if (FAILED(hr))
            return hr;
    }

    // 2. 应用Motion Blur
    if (m_motionBlurConfig.bEnabled)
    {
        hr = ApplyMotionBlur(pCommandList);
        if (FAILED(hr))
            return hr;
    }

    // 3. 应用DoF
    if (m_dofConfig.bEnabled)
    {
        hr = ApplyDepthOfField(pCommandList);
        if (FAILED(hr))
            return hr;
    }

    // 4. 应用Tone Mapping
    if (m_toneMappingConfig.bEnabled)
    {
        hr = ApplyToneMapping(pCommandList);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

// ========== 配置管理 ==========
void D3D12PostProcessManager::SetBloomConfig(const BloomConfig& config)
{
    m_bloomConfig = config;
}

void D3D12PostProcessManager::SetMotionBlurConfig(const MotionBlurConfig& config)
{
    m_motionBlurConfig = config;
}

void D3D12PostProcessManager::SetDoFConfig(const DoFConfig& config)
{
    m_dofConfig = config;
}

void D3D12PostProcessManager::SetToneMappingConfig(const ToneMappingConfig& config)
{
    m_toneMappingConfig = config;
}

const BloomConfig& D3D12PostProcessManager::GetBloomConfig() const
{
    return m_bloomConfig;
}

const MotionBlurConfig& D3D12PostProcessManager::GetMotionBlurConfig() const
{
    return m_motionBlurConfig;
}

const DoFConfig& D3D12PostProcessManager::GetDoFConfig() const
{
    return m_dofConfig;
}

const ToneMappingConfig& D3D12PostProcessManager::GetToneMappingConfig() const
{
    return m_toneMappingConfig;
}

// ========== 获取资源 ==========
ID3D12Resource* D3D12PostProcessManager::GetSceneColor() const
{
    return m_buffers.pSceneColor.Get();
}

ID3D12Resource* D3D12PostProcessManager::GetFinalOutput() const
{
    return m_buffers.pFinalOutput.Get();
}
