// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12FontManager.h"
#include <cmath>
#include "D3D12FontObject.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <vector>
#include <cwchar>

// 文本顶点结构

struct TextVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texcoord;
    DirectX::XMFLOAT4 color;
};

// ============================================
// 构造函数和析构函数
// ============================================

D3D12FontManager::D3D12FontManager()
{
    m_pDevice = nullptr;
    m_nTextVertexBufferSize = 0;
    m_nTextIndexBufferSize = 0;
    m_nMaxCharacters = 4096;  // 默认最大4096个字符
}

D3D12FontManager::~D3D12FontManager()
{
    Release();
}

// ============================================
// 初始化和释放
// ============================================

HRESULT D3D12FontManager::Initialize(CoD3D12Device* pDevice)
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
        DX12_THROW_IF_FAILED(CreateTextVertexBuffer());

        // 创建索引缓冲区
        DX12_THROW_IF_FAILED(CreateTextIndexBuffer());

    }
    catch (const std::exception& e) {
        OutputDebugStringA("D3D12FontManager::Initialize failed: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
        return E_FAIL;
    }

    return S_OK;
}

void D3D12FontManager::Release()
{
    // 释放所有 Font
    for (auto& pair : m_mFontMap)
    {
        D3D12FontObject* pFont = pair.second;
        if (pFont)
        {
            pFont->Release();
        }
    }
    m_mFontMap.clear();

    // 释放资源
    m_pTextVertexBuffer.Reset();
    m_pUploadTextVertexBuffer.Reset();
    m_pTextIndexBuffer.Reset();
    m_pUploadTextIndexBuffer.Reset();
    m_pRootSignature.Reset();
    m_pTextPSO.Reset();
}

// ============================================
// Font 管理
// ============================================

void* D3D12FontManager::AddFont(D3D12FontObject* pFont)
{
    if (pFont == nullptr)
        return nullptr;

    void* pHandle = static_cast<void*>(pFont);
    m_mFontMap[pHandle] = pFont;

    pFont->AddRef();

    return pHandle;
}

void D3D12FontManager::RemoveFont(void* pHandle)
{
    auto it = m_mFontMap.find(pHandle);
    if (it != m_mFontMap.end())
    {
        D3D12FontObject* pFont = it->second;
        if (pFont)
        {
            pFont->Release();
        }
        m_mFontMap.erase(it);
    }
}

D3D12FontObject* D3D12FontManager::GetFont(void* pHandle)
{
    auto it = m_mFontMap.find(pHandle);
    if (it != m_mFontMap.end())
    {
        return it->second;
    }
    return nullptr;
}

BOOL D3D12FontManager::HasFont(void* pHandle) const
{
    return m_mFontMap.find(pHandle) != m_mFontMap.end();
}

// ============================================
// 文本渲染
// ============================================

BOOL D3D12FontManager::RenderFont(D3D12FontObject* pFont, TCHAR* str, DWORD dwLen,
                                  RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type,
                                  int iZOrder, DWORD dwFlag)
{
    if (pFont == nullptr || str == nullptr)
        return FALSE;

    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();

    if (pD3D12Device == nullptr || pCommandList == nullptr)
        return FALSE;

    // 获取字体纹理
    ID3D12Resource* pFontTexture = pFont->GetTexture();
    D3D12_GPU_DESCRIPTOR_HANDLE fontTextureSRV = pFont->GetTextureSRV();

    if (pFontTexture == nullptr)
        return FALSE;

    // 计算文本长度
    DWORD textLength = dwLen;
    if (textLength == 0 && str != nullptr)
    {
        // 使用_tcslen计算长度
        #ifdef _UNICODE
        textLength = (DWORD)wcslen(str);
        #else
        textLength = (DWORD)strlen(str);
        #endif
    }

    if (textLength == 0)
        return TRUE;

    // 转换颜色
    DirectX::XMFLOAT4 color;
    color.x = ((dwColor >> 16) & 0xFF) / 255.0f;
    color.y = ((dwColor >> 8) & 0xFF) / 255.0f;
    color.z = (dwColor & 0xFF) / 255.0f;
    color.w = ((dwColor >> 24) & 0xFF) / 255.0f;

    // 获取字体信息
    float fCharWidth = pFont->GetCharWidth();
    float fCharHeight = pFont->GetCharHeight();
    float fLineSpacing = pFont->GetLineSpacing();

    // 准备顶点数据
    std::vector<TextVertex> vertices;
    std::vector<UINT> indices;
    vertices.reserve(textLength * 4);
    indices.reserve(textLength * 6);

    // 设置起始位置
    float fX = pRect ? (float)pRect->left : 0.0f;
    float fY = pRect ? (float)pRect->top : 0.0f;
    float fMaxX = pRect ? (float)pRect->right : 1920.0f;
    float fMaxY = pRect ? (float)pRect->bottom : 1080.0f;

    UINT vertexIndex = 0;

    // 遍历文本字符
    for (DWORD i = 0; i < textLength; ++i)
    {
        TCHAR ch = str[i];

        // 处理换行符
        if (ch == _T('\n'))
        {
            fX = pRect ? (float)pRect->left : 0.0f;
            fY += fCharHeight + fLineSpacing;
            continue;
        }

        // 处理制表符
        if (ch == _T('\t'))
        {
            fX += fCharWidth * 4.0f;
            continue;
        }

        // 跳过空格（可选）
        if (ch == _T(' '))
        {
            fX += fCharWidth;
            continue;
        }

        // 计算字符纹理坐标
        float fU1 = 0.0f, fV1 = 0.0f, fU2 = 1.0f, fV2 = 1.0f;
        pFont->GetCharTexCoords(ch, fU1, fV1, fU2, fV2);

        // 生成四边形顶点
        TextVertex quadVertices[4];

        // 左上角
        quadVertices[0].position = DirectX::XMFLOAT3(fX, fY, 0.0f);
        quadVertices[0].texcoord = DirectX::XMFLOAT2(fU1, fV1);
        quadVertices[0].color = color;

        // 右上角
        quadVertices[1].position = DirectX::XMFLOAT3(fX + fCharWidth, fY, 0.0f);
        quadVertices[1].texcoord = DirectX::XMFLOAT2(fU2, fV1);
        quadVertices[1].color = color;

        // 左下角
        quadVertices[2].position = DirectX::XMFLOAT3(fX, fY + fCharHeight, 0.0f);
        quadVertices[2].texcoord = DirectX::XMFLOAT2(fU1, fV2);
        quadVertices[2].color = color;

        // 右下角
        quadVertices[3].position = DirectX::XMFLOAT3(fX + fCharWidth, fY + fCharHeight, 0.0f);
        quadVertices[3].texcoord = DirectX::XMFLOAT2(fU2, fV2);
        quadVertices[3].color = color;

        // 添加顶点
        vertices.insert(vertices.end(), quadVertices, quadVertices + 4);

        // 添加索引（两个三角形）
        indices.push_back(vertexIndex + 0);
        indices.push_back(vertexIndex + 1);
        indices.push_back(vertexIndex + 2);
        indices.push_back(vertexIndex + 2);
        indices.push_back(vertexIndex + 1);
        indices.push_back(vertexIndex + 3);

        vertexIndex += 4;

        // 移动到下一个字符位置
        fX += fCharWidth;

        // 自动换行检查
        if (fX >= fMaxX && pRect != nullptr)
        {
            fX = (float)pRect->left;
            fY += fCharHeight + fLineSpacing;

            // 检查是否超出底部
            if (fY >= fMaxY)
            {
                break;
            }
        }
    }

    if (vertices.empty())
        return TRUE;

    // 上传顶点数据
    void* pVertexData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    m_pUploadTextVertexBuffer->Map(0, &readRange, &pVertexData);
    memcpy(pVertexData, vertices.data(), vertices.size() * sizeof(TextVertex));
    m_pUploadTextVertexBuffer->Unmap(0, nullptr);

    // 上传索引数据
    void* pIndexData = nullptr;
    m_pUploadTextIndexBuffer->Map(0, &readRange, &pIndexData);
    memcpy(pIndexData, indices.data(), indices.size() * sizeof(UINT));
    m_pUploadTextIndexBuffer->Unmap(0, nullptr);

    // 设置渲染状态
    pCommandList->SetPipelineState(m_pTextPSO.Get());
    pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

    // 设置字体纹理
    pCommandList->SetGraphicsRootDescriptorTable(0, fontTextureSRV);

    // 设置顶点缓冲区
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = m_pTextVertexBuffer->GetGPUVirtualAddress();
    vbv.StrideInBytes = sizeof(TextVertex);
    vbv.SizeInBytes = (UINT)vertices.size() * sizeof(TextVertex);
    pCommandList->IASetVertexBuffers(0, 1, &vbv);

    // 设置索引缓冲区
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = m_pTextIndexBuffer->GetGPUVirtualAddress();
    ibv.Format = DXGI_FORMAT_R32_UINT;
    ibv.SizeInBytes = (UINT)indices.size() * sizeof(UINT);
    pCommandList->IASetIndexBuffer(&ibv);

    // 设置图元拓扑
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 绘制
    pCommandList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);

    return TRUE;
}

// ============================================
// 私有辅助方法
// ============================================

HRESULT D3D12FontManager::CreateTextVertexBuffer()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 计算顶点缓冲区大小（每个字符4个顶点）
    UINT vertexCount = m_nMaxCharacters * 4;
    UINT vertexBufferSize = vertexCount * sizeof(TextVertex);

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
        IID_PPV_ARGS(&m_pTextVertexBuffer)
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
        IID_PPV_ARGS(&m_pUploadTextVertexBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_nTextVertexBufferSize = vertexBufferSize;

    return S_OK;
}

HRESULT D3D12FontManager::CreateTextIndexBuffer()
{
    ID3D12Device* pD3D12Device = m_pDevice->GetD3D12Device();
    if (pD3D12Device == nullptr)
        return E_FAIL;

    // 计算索引缓冲区大小（每个字符6个索引）
    UINT indexCount = m_nMaxCharacters * 6;
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
        IID_PPV_ARGS(&m_pTextIndexBuffer)
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
        IID_PPV_ARGS(&m_pUploadTextIndexBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_nTextIndexBufferSize = indexBufferSize;

    return S_OK;
}

HRESULT D3D12FontManager::CreateRootSignature()
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

HRESULT D3D12FontManager::CreatePipelineState()
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

            output.texcoord = input.texcoord;
            output.color = input.color;

            return output;
        }
    )";

    // 嵌入式像素着色器
    const char* pixelShaderHLSL = R"(
        Texture2D texFontTexture : register(t0);
        SamplerState samplerLinear : register(s0);

        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
            float4 color : COLOR;
        };

        float4 main(PSInput input) : SV_TARGET
        {
            float4 fontColor = texFontTexture.Sample(samplerLinear, input.texcoord);
            return fontColor * input.color;
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
            OutputDebugStringA("Font Vertex Shader Error:\n");
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
            OutputDebugStringA("Font Pixel Shader Error:\n");
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
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // 文本不剔除
    psoDesc.DepthStencilState.DepthEnable = FALSE;  // 文本不需要深度测试
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = 3;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = pD3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pTextPSO));

    return hr;
}
