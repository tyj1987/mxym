// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12SpriteManager.h"
#include <cmath>
#include "D3D12SpriteObject.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <vector>

// ============================================
// 构造函数和析构函数
// ============================================

D3D12SpriteManager::D3D12SpriteManager()
{
    m_pDevice = nullptr;
    m_nVertexBufferSize = 0;
    m_nIndexBufferSize = 0;
    m_nMaxSprites = 1000;  // 默认最大1000个精灵
}

D3D12SpriteManager::~D3D12SpriteManager()
{
    Release();
}

// ============================================
// 初始化和释放
// ============================================

HRESULT D3D12SpriteManager::Initialize(CoD3D12Device* pDevice)
{
    if (pDevice == nullptr)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    try {
        // 创建根签名
        DX12_THROW_IF_FAILED(CreateRootSignature());

        // 创建 PSO
        DX12_THROW_IF_FAILED(CreatePipelineState());

        // 创建顶点缓冲区
        DX12_THROW_IF_FAILED(CreateVertexBuffer());

        // 创建索引缓冲区
        DX12_THROW_IF_FAILED(CreateIndexBuffer());

    }
    catch (const std::exception& e) {
        OutputDebugStringA("D3D12SpriteManager::Initialize failed: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
        return E_FAIL;
    }

    return S_OK;
}

void D3D12SpriteManager::Release()
{
    // 释放所有 Sprite
    for (auto& pair : m_mSpriteMap)
    {
        D3D12SpriteObject* pSprite = pair.second;
        if (pSprite)
        {
            pSprite->Release();
        }
    }
    m_mSpriteMap.clear();

    // 释放资源
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();
    m_pUploadVertexBuffer.Reset();
    m_pUploadIndexBuffer.Reset();
    m_pRootSignature.Reset();
    m_pPSO.Reset();
}

// ============================================
// Sprite 管理
// ============================================

void* D3D12SpriteManager::AddSprite(D3D12SpriteObject* pSprite)
{
    if (pSprite == nullptr)
        return nullptr;

    void* pHandle = static_cast<void*>(pSprite);
    m_mSpriteMap[pHandle] = pSprite;

    pSprite->AddRef();

    return pHandle;
}

void D3D12SpriteManager::RemoveSprite(void* pHandle)
{
    auto it = m_mSpriteMap.find(pHandle);
    if (it != m_mSpriteMap.end())
    {
        D3D12SpriteObject* pSprite = it->second;
        if (pSprite)
        {
            pSprite->Release();
        }
        m_mSpriteMap.erase(it);
    }
}

D3D12SpriteObject* D3D12SpriteManager::GetSprite(void* pHandle)
{
    auto it = m_mSpriteMap.find(pHandle);
    if (it != m_mSpriteMap.end())
    {
        return it->second;
    }
    return nullptr;
}

BOOL D3D12SpriteManager::HasSprite(void* pHandle) const
{
    return m_mSpriteMap.find(pHandle) != m_mSpriteMap.end();
}

// ============================================
// 渲染
// ============================================

BOOL D3D12SpriteManager::RenderSprite(D3D12SpriteObject* pSprite, VECTOR2* pv2Scaling,
                                         float fRot, VECTOR2* pv2Trans, RECT* pRect,
                                         DWORD dwColor, int iZOrder, DWORD dwFlag)
{
    if (pSprite == nullptr)
        return FALSE;

    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();

    if (pD3D12Device == nullptr || pCommandList == nullptr)
        return FALSE;

    // 获取精灵属性
    float fWidth = pSprite->GetWidth();
    float fHeight = pSprite->GetHeight();
    ID3D12Resource* pTexture = pSprite->GetTexture();
    D3D12_GPU_DESCRIPTOR_HANDLE textureSRV = pSprite->GetTextureSRV();

    if (pTexture == nullptr)
        return FALSE;

    // 应用变换
    float fScaleX = pv2Scaling ? pv2Scaling->x : 1.0f;
    float fScaleY = pv2Scaling ? pv2Scaling->y : 1.0f;
    float fTransX = pv2Trans ? pv2Trans->x : 0.0f;
    float fTransY = pv2Trans ? pv2Trans->y : 0.0f;

    // 计算UV坐标
    float fU1 = 0.0f, fV1 = 0.0f, fU2 = 1.0f, fV2 = 1.0f;
    if (pRect)
    {
        fU1 = static_cast<float>(pRect->left) / fWidth;
        fV1 = static_cast<float>(pRect->top) / fHeight;
        fU2 = static_cast<float>(pRect->right) / fWidth;
        fV2 = static_cast<float>(pRect->bottom) / fHeight;
        fWidth = static_cast<float>(pRect->right - pRect->left);
        fHeight = static_cast<float>(pRect->bottom - pRect->top);
    }

    // 转换颜色
    DirectX::XMFLOAT4 color;
    color.x = ((dwColor >> 16) & 0xFF) / 255.0f;
    color.y = ((dwColor >> 8) & 0xFF) / 255.0f;
    color.z = (dwColor & 0xFF) / 255.0f;
    color.w = ((dwColor >> 24) & 0xFF) / 255.0f;

    // 生成精灵顶点（4个顶点，2个三角形）
    SpriteVertex vertices[4];

    // 左上角
    vertices[0].position = DirectX::XMFLOAT3(-fWidth * 0.5f, -fHeight * 0.5f, 0.0f);
    vertices[0].texCoord = DirectX::XMFLOAT2(fU1, fV1);
    vertices[0].color = color;

    // 右上角
    vertices[1].position = DirectX::XMFLOAT3(fWidth * 0.5f, -fHeight * 0.5f, 0.0f);
    vertices[1].texCoord = DirectX::XMFLOAT2(fU2, fV1);
    vertices[1].color = color;

    // 左下角
    vertices[2].position = DirectX::XMFLOAT3(-fWidth * 0.5f, fHeight * 0.5f, 0.0f);
    vertices[2].texCoord = DirectX::XMFLOAT2(fU1, fV2);
    vertices[2].color = color;

    // 右下角
    vertices[3].position = DirectX::XMFLOAT3(fWidth * 0.5f, fHeight * 0.5f, 0.0f);
    vertices[3].texCoord = DirectX::XMFLOAT2(fU2, fV2);
    vertices[3].color = color;

    // 应用变换（缩放、旋转、平移）
    for (int i = 0; i < 4; ++i)
    {
        // 缩放
        vertices[i].position.x *= fScaleX;
        vertices[i].position.y *= fScaleY;

        // 旋转
        if (fRot != 0.0f)
        {
            float fCos = cosf(fRot);
            float fSin = sinf(fRot);
            float x = vertices[i].position.x;
            float y = vertices[i].position.y;
            vertices[i].position.x = x * fCos - y * fSin;
            vertices[i].position.y = x * fSin + y * fCos;
        }

        // 平移
        vertices[i].position.x += fTransX;
        vertices[i].position.y += fTransY;
    }

    // 索引（两个三角形）
    UINT indices[] = { 0, 1, 2, 2, 1, 3 };

    // 上传顶点数据
    void* pVertexData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    m_pUploadVertexBuffer->Map(0, &readRange, &pVertexData);
    memcpy(pVertexData, vertices, sizeof(vertices));
    m_pUploadVertexBuffer->Unmap(0, nullptr);

    // 上传索引数据
    void* pIndexData = nullptr;
    m_pUploadIndexBuffer->Map(0, &readRange, &pIndexData);
    memcpy(pIndexData, indices, sizeof(indices));
    m_pUploadIndexBuffer->Unmap(0, nullptr);

    // 设置渲染状态
    pCommandList->SetPipelineState(m_pPSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

    // 设置纹理
    pCommandList->SetGraphicsRootDescriptorTable(0, textureSRV);

    // 设置顶点缓冲区
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
    vbv.StrideInBytes = sizeof(SpriteVertex);
    vbv.SizeInBytes = sizeof(vertices);
    pCommandList->IASetVertexBuffers(0, 1, &vbv);

    // 设置索引缓冲区
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
    ibv.Format = DXGI_FORMAT_R32_UINT;
    ibv.SizeInBytes = sizeof(indices);
    pCommandList->IASetIndexBuffer(&ibv);

    // 设置图元拓扑
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 绘制
    pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    return TRUE;
}

void D3D12SpriteManager::RenderAllSprites(ID3D12GraphicsCommandList* pCommandList)
{
    if (pCommandList == nullptr)
        return;

    // 按 Z 顺序排序精灵
    std::vector<std::pair<int, D3D12SpriteObject*>> sortedSprites;
    for (auto& pair : m_mSpriteMap)
    {
        D3D12SpriteObject* pSprite = pair.second;
        if (pSprite && pSprite->IsVisible())
        {
            sortedSprites.push_back({ pSprite->GetZOrder(), pSprite });
        }
    }

    std::sort(sortedSprites.begin(), sortedSprites.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    // 批量渲染
    for (auto& pair : sortedSprites)
    {
        D3D12SpriteObject* pSprite = pair.second;

        // 获取精灵变换参数
        VECTOR2 scaling = { pSprite->GetScaleX(), pSprite->GetScaleY() };
        float rotation = pSprite->GetRotation();
        VECTOR2 translation = { pSprite->GetX(), pSprite->GetY() };
        DWORD color = pSprite->GetColor();

        RenderSprite(pSprite, &scaling, rotation, &translation, nullptr, color, pair.first, 0);
    }
}

// ============================================
// 私有辅助方法
// ============================================

HRESULT D3D12SpriteManager::CreateVertexBuffer()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 计算顶点缓冲区大小（每个精灵4个顶点）
    UINT vertexCount = m_nMaxSprites * 4;
    UINT vertexBufferSize = vertexCount * sizeof(SpriteVertex);

    // 创建默认堆顶点缓冲区
    D3D12_RESOURCE_DESC vbDesc = {};
    vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vbDesc.Width = vertexBufferSize;
    vbDesc.Height = 1;
    vbDesc.DepthOrArraySize = 1;
    vbDesc.MipLevels = 1;
    vbDesc.Format = DXGI_FORMAT_UNKNOWN;
    vbDesc.SampleDesc.Count = 1;
    vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES defaultHeapProps = {};
    defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    defaultHeapProps.CreationNodeMask = 1;
    defaultHeapProps.VisibleNodeMask = 1;

    HRESULT hr = pD3D12Device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vbDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pVertexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 创建上传堆
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;

    hr = pD3D12Device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pUploadVertexBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_nVertexBufferSize = vertexBufferSize;

    return S_OK;
}

HRESULT D3D12SpriteManager::CreateIndexBuffer()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 计算索引缓冲区大小（每个精灵6个索引）
    UINT indexCount = m_nMaxSprites * 6;
    UINT indexBufferSize = indexCount * sizeof(UINT);

    // 创建默认堆索引缓冲区
    D3D12_RESOURCE_DESC ibDesc = {};
    ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ibDesc.Width = indexBufferSize;
    ibDesc.Height = 1;
    ibDesc.DepthOrArraySize = 1;
    ibDesc.MipLevels = 1;
    ibDesc.Format = DXGI_FORMAT_UNKNOWN;
    ibDesc.SampleDesc.Count = 1;
    ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES defaultHeapProps = {};
    defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    defaultHeapProps.CreationNodeMask = 1;
    defaultHeapProps.VisibleNodeMask = 1;

    HRESULT hr = pD3D12Device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ibDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pIndexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 创建上传堆
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;

    hr = pD3D12Device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ibDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pUploadIndexBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_nIndexBufferSize = indexBufferSize;

    return S_OK;
}

HRESULT D3D12SpriteManager::CreateRootSignature()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 创建描述符范围（纹理）
    D3D12_DESCRIPTOR_RANGE descriptorRanges[1] = {};
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].RegisterSpace = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 创建根参数
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRanges;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 创建静态采样器
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 创建根签名描述
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                              D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                              D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                              D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // 序列化根签名
    Microsoft::WRL::ComPtr<ID3DBlob> pSignatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                            &pSignatureBlob, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((const char*)pErrorBlob->GetBufferPointer());
        }
        return hr;
    }

    // 创建根签名
    hr = pD3D12Device->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(),
                                          pSignatureBlob->GetBufferSize(),
                                          IID_PPV_ARGS(&m_pRootSignature));

    return hr;
}

HRESULT D3D12SpriteManager::CreatePipelineState()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 嵌入式顶点着色器
    const char* vertexShaderHLSL = R"(
        cbuffer ConstantBuffer : register(b0)
        {
            matrix projection;
            matrix view;
        }

        struct VSInput
        {
            float3 position : POSITION;
            float2 texcoord : TEXCOORD;
            float4 color : COLOR;
        };

        struct VSOutput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
            float4 color : COLOR;
        };

        VSOutput main(VSInput input)
        {
            VSOutput output;

            // 变换到裁剪空间
            float4 worldPos = float4(input.position, 1.0f);
            output.position = mul(worldPos, view);
            output.position = mul(output.position, projection);

            output.texCoord = input.texCoord;
            output.color = input.color;

            return output;
        }
    )";

    // 嵌入式像素着色器
    const char* pixelShaderHLSL = R"(
        Texture2D texTexture : register(t0);
        SamplerState samplerLinear : register(s0);

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
            float4 color : COLOR;
        };

        float4 main(PSInput input) : SV_TARGET
        {
            float4 texColor = texTexture.Sample(samplerLinear, input.texCoord);
            return texColor * input.color;
        }
    )";

    // 编译顶点着色器
    Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderErrorBlob;
    HRESULT hr = D3DCompile(vertexShaderHLSL, strlen(vertexShaderHLSL), nullptr, nullptr, nullptr,
                           "main", "vs_5_0", 0, 0, &pVertexShaderBlob, &pVertexShaderErrorBlob);

    if (FAILED(hr))
    {
        if (pVertexShaderErrorBlob)
        {
            OutputDebugStringA("Sprite Vertex Shader Error:\n");
            OutputDebugStringA((const char*)pVertexShaderErrorBlob->GetBufferPointer());
        }
        return hr;
    }

    // 编译像素着色器
    Microsoft::WRL::ComPtr<ID3DBlob> pPixelShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pPixelShaderErrorBlob;
    hr = D3DCompile(pixelShaderHLSL, strlen(pixelShaderHLSL), nullptr, nullptr, nullptr,
                   "main", "ps_5_0", 0, 0, &pPixelShaderBlob, &pPixelShaderErrorBlob);

    if (FAILED(hr))
    {
        if (pPixelShaderErrorBlob)
        {
            OutputDebugStringA("Sprite Pixel Shader Error:\n");
            OutputDebugStringA((const char*)pPixelShaderErrorBlob->GetBufferPointer());
        }
        return hr;
    }

    // 定义输入布局
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 创建PSO描述
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_pRootSignature.Get();
    psoDesc.VS = { pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC::DEFAULT();
    psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC::DEFAULT();
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // 精灵不剔除
    psoDesc.DepthStencilState.DepthEnable = FALSE;  // 精灵不需要深度测试
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = 3;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = pD3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO));

    return hr;
}
