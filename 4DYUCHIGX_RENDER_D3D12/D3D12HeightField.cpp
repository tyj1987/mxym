// ============================================
// D3D12HeightField.cpp
// DirectX 12 高度场地形类实现
// 第 7 阶段 - HeightField 地形系统实现
// 100% 完整实现版本
// ============================================

#include "D3D12PCH.h"
#include "D3D12HeightField.h"
#include "IRenderer_GUID.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstring>

// ========== 构造函数 ==========
#include "D3D12PCH.h"

D3D12HeightField::D3D12HeightField()
    : m_dwRefCount(1)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nVertexCount(0)
    , m_nIndexCount(0)
    , m_nPatchSize(DEFAULT_PATCH_SIZE)
    , m_bTileBlendEnabled(FALSE)
    , m_pDevice(nullptr)
{
    ZeroMemory(&m_vertexBufferView, sizeof(m_vertexBufferView));
    ZeroMemory(&m_indexBufferView, sizeof(m_indexBufferView));
}

// ========== 析构函数 ==========
D3D12HeightField::~D3D12HeightField()
{
    // 释放资源由 ComPtr 自动管理
}

// ========== IUnknown::QueryInterface ==========
STDMETHODIMP D3D12HeightField::QueryInterface(REFIID riid, PPVOID ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDIHeightField)
    {
        *ppv = this;
        AddRef();
        return S_OK;
    }

    *ppv = nullptr;
    return E_NOINTERFACE;
}

// ========== IUnknown::AddRef ==========
STDMETHODIMP_(ULONG) D3D12HeightField::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

// ========== IUnknown::Release ==========
STDMETHODIMP_(ULONG) D3D12HeightField::Release(void)
{
    ULONG newRefCount = InterlockedDecrement(&m_dwRefCount);
    if (newRefCount == 0)
    {
        delete this;
    }
    return newRefCount;
}

// ========== 初始化 ==========
HRESULT D3D12HeightField::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    return S_OK;
}

// ========== 从文件加载高度图 ==========
HRESULT D3D12HeightField::LoadHeightMap(const char* szFileName, const HeightMapConfig& config)
{
    if (!szFileName)
        return E_INVALIDARG;

    m_config = config;

    // 确定文件格式并加载
    std::string fileName(szFileName);
    std::string extension;

    // 提取文件扩展名
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        extension = fileName.substr(dotPos + 1);
        // 转换为小写
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    }

    HRESULT hr = S_OK;

    // 根据扩展名选择加载方式
    if (extension == "raw")
    {
        hr = LoadRawHeightMap(szFileName, config);
    }
    else if (extension == "r16")
    {
        hr = LoadR16HeightMap(szFileName, config);
    }
    else if (extension == "tga")
    {
        hr = LoadTGAHeightMap(szFileName, config);
    }
    else
    {
        // 尝试作为RAW加载
        hr = LoadRawHeightMap(szFileName, config);
    }

    if (FAILED(hr))
        return hr;

    // 生成顶点和索引
    hr = GenerateVertices();
    if (FAILED(hr))
        return hr;

    hr = GenerateIndices();
    if (FAILED(hr))
        return hr;

    hr = CalculateNormals();
    if (FAILED(hr))
        return hr;

    hr = CreateBuffers();
    if (FAILED(hr))
        return hr;

    // 创建地形分块
    hr = CreatePatches(m_nPatchSize);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ========== 加载RAW格式高度图 ==========
HRESULT D3D12HeightField::LoadRawHeightMap(const char* szFileName, const HeightMapConfig& config)
{
    // 打开文件
    std::ifstream file(szFileName, std::ios::binary);
    if (!file.is_open())
        return E_FAIL;

    m_nWidth = config.nWidth;
    m_nHeight = config.nHeight;

    // 分配高度数据
    m_vHeightData.resize(m_nWidth * m_nHeight);

    // 读取RAW数据（8位或16位）
    file.read(reinterpret_cast<char*>(m_vHeightData.data()), m_nWidth * m_nHeight * sizeof(uint8_t));
    if (file.fail())
    {
        // 尝试16位读取
        file.clear();
        file.seekg(0, std::ios::beg);

        std::vector<uint16_t> heightData16(m_nWidth * m_nHeight);
        file.read(reinterpret_cast<char*>(heightData16.data()), m_nWidth * m_nHeight * sizeof(uint16_t));

        if (file.fail())
            return E_FAIL;

        // 转换16位到浮点
        for (UINT i = 0; i < m_nWidth * m_nHeight; ++i)
        {
            m_vHeightData[i] = static_cast<float>(heightData16[i]) / 65535.0f * config.fHeightScale + config.fHeightOffset;
        }
    }
    else
    {
        // 转换8位到浮点
        for (UINT i = 0; i < m_nWidth * m_nHeight; ++i)
        {
            float value = static_cast<float>(m_vHeightData[i]);
            m_vHeightData[i] = value / 255.0f * config.fHeightScale + config.fHeightOffset;
        }
    }

    file.close();
    return S_OK;
}

// ========== 加载R16格式高度图 ==========
HRESULT D3D12HeightField::LoadR16HeightMap(const char* szFileName, const HeightMapConfig& config)
{
    // 打开文件
    std::ifstream file(szFileName, std::ios::binary);
    if (!file.is_open())
        return E_FAIL;

    m_nWidth = config.nWidth;
    m_nHeight = config.nHeight;

    // 分配高度数据
    m_vHeightData.resize(m_nWidth * m_nHeight);

    // 读取16位数据
    std::vector<uint16_t> heightData16(m_nWidth * m_nHeight);
    file.read(reinterpret_cast<char*>(heightData16.data()), m_nWidth * m_nHeight * sizeof(uint16_t));

    if (file.fail())
    {
        file.close();
        return E_FAIL;
    }

    file.close();

    // 转换16位到浮点
    for (UINT i = 0; i < m_nWidth * m_nHeight; ++i)
    {
        m_vHeightData[i] = static_cast<float>(heightData16[i]) / 65535.0f * config.fHeightScale + config.fHeightOffset;
    }

    return S_OK;
}

// ========== 加载TGA格式高度图 ==========
HRESULT D3D12HeightField::LoadTGAHeightMap(const char* szFileName, const HeightMapConfig& config)
{
    // TGA头部结构
#pragma pack(push, 1)
    struct TGAHeader
    {
        uint8_t idLength;
        uint8_t colorMapType;
        uint8_t imageType;
        uint16_t colorMapFirstEntry;
        uint16_t colorMapLength;
        uint8_t colorMapEntrySize;
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t pixelDepth;
        uint8_t imageDescriptor;
    };
#pragma pack(pop)

    // 打开文件
    std::ifstream file(szFileName, std::ios::binary);
    if (!file.is_open())
        return E_FAIL;

    // 读取头部
    TGAHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(TGAHeader));

    if (file.fail())
    {
        file.close();
        return E_FAIL;
    }

    // 检查是否为灰度图像
    if (header.imageType != 3)  // 3 = 灰度图
    {
        file.close();
        return E_FAIL;
    }

    m_nWidth = config.nWidth > 0 ? config.nWidth : header.width;
    m_nHeight = config.nHeight > 0 ? config.nHeight : header.height;

    // 跳过ID字段和颜色映射
    if (header.idLength > 0)
        file.seekg(header.idLength, std::ios::cur);

    // 分配高度数据
    m_vHeightData.resize(m_nWidth * m_nHeight);

    // 读取像素数据
    if (header.pixelDepth == 8)
    {
        std::vector<uint8_t> pixelData(header.width * header.height);
        file.read(reinterpret_cast<char*>(pixelData.data()), header.width * header.height);

        // 转换到配置的大小
        for (UINT z = 0; z < m_nHeight; ++z)
        {
            for (UINT x = 0; x < m_nWidth; ++x)
            {
                UINT srcX = (x * header.width) / m_nWidth;
                UINT srcZ = (z * header.height) / m_nHeight;
                float value = static_cast<float>(pixelData[srcZ * header.width + srcX]);
                m_vHeightData[z * m_nWidth + x] = value / 255.0f * config.fHeightScale + config.fHeightOffset;
            }
        }
    }
    else if (header.pixelDepth == 16)
    {
        std::vector<uint16_t> pixelData(header.width * header.height);
        file.read(reinterpret_cast<char*>(pixelData.data()), header.width * header.height * 2);

        // 转换到配置的大小
        for (UINT z = 0; z < m_nHeight; ++z)
        {
            for (UINT x = 0; x < m_nWidth; ++x)
            {
                UINT srcX = (x * header.width) / m_nWidth;
                UINT srcZ = (z * header.height) / m_nHeight;
                float value = static_cast<float>(pixelData[srcZ * header.width + srcX]);
                m_vHeightData[z * m_nWidth + x] = value / 65535.0f * config.fHeightScale + config.fHeightOffset;
            }
        }
    }
    else
    {
        file.close();
        return E_FAIL;
    }

    file.close();
    return S_OK;
}

// ========== 从函数生成地形 ==========
HRESULT D3D12HeightField::GenerateFromFunction(UINT nWidth, UINT nHeight, float (*HeightFunc)(float x, float z), const HeightMapConfig& config)
{
    if (!HeightFunc)
        return E_INVALIDARG;

    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_config = config;

    // 分配高度数据
    m_vHeightData.resize(nWidth * nHeight);

    // 生成高度数据
    for (UINT z = 0; z < nHeight; ++z)
    {
        for (UINT x = 0; x < nWidth; ++x)
        {
            float fx = static_cast<float>(x) / nWidth;
            float fz = static_cast<float>(z) / nHeight;
            m_vHeightData[z * nWidth + x] = HeightFunc(fx, fz) * config.fHeightScale + config.fHeightOffset;
        }
    }

    // 生成顶点和索引
    HRESULT hr = GenerateVertices();
    if (FAILED(hr))
        return hr;

    hr = GenerateIndices();
    if (FAILED(hr))
        return hr;

    hr = CalculateNormals();
    if (FAILED(hr))
        return hr;

    hr = CreateBuffers();
    if (FAILED(hr))
        return hr;

    // 创建地形分块
    hr = CreatePatches(m_nPatchSize);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ========== 生成顶点 ==========
HRESULT D3D12HeightField::GenerateVertices()
{
    if (m_vHeightData.empty())
        return E_FAIL;

    m_vVertices.clear();
    m_vVertices.reserve(m_nWidth * m_nHeight);

    for (UINT z = 0; z < m_nHeight; ++z)
    {
        for (UINT x = 0; x < m_nWidth; ++x)
        {
            TerrainVertex vertex;

            // 位置
            float fHeight = m_vHeightData[z * m_nWidth + x];
            vertex.position = XMFLOAT3(
                static_cast<float>(x) * m_config.fWorldScale,
                fHeight,
                static_cast<float>(z) * m_config.fWorldScale
            );

            // 纹理坐标
            vertex.texCoord = XMFLOAT2(
                static_cast<float>(x) / (m_nWidth - 1) * m_config.fTextureScale,
                static_cast<float>(z) / (m_nHeight - 1) * m_config.fTextureScale
            );

            // 细节纹理坐标（更高平铺）
            vertex.texCoordDetail = XMFLOAT2(
                static_cast<float>(x) * 0.1f,
                static_cast<float>(z) * 0.1f
            );

            // 法线（稍后计算）
            vertex.normal = XMFLOAT3(0, 1, 0);

            m_vVertices.push_back(vertex);
        }
    }

    m_nVertexCount = static_cast<UINT>(m_vVertices.size());

    return S_OK;
}

// ========== 生成索引 ==========
HRESULT D3D12HeightField::GenerateIndices()
{
    if (m_vVertices.empty())
        return E_FAIL;

    m_vIndices.clear();

    // 每个网格（两个三角形）
    UINT nNumQuads = (m_nWidth - 1) * (m_nHeight - 1);
    m_vIndices.reserve(nNumQuads * 6);

    for (UINT z = 0; z < m_nHeight - 1; ++z)
    {
        for (UINT x = 0; x < m_nWidth - 1; ++x)
        {
            UINT nIndex0 = z * m_nWidth + x;
            UINT nIndex1 = z * m_nWidth + (x + 1);
            UINT nIndex2 = (z + 1) * m_nWidth + (x + 1);
            UINT nIndex3 = (z + 1) * m_nWidth + x;

            // 第一个三角形
            m_vIndices.push_back(nIndex0);
            m_vIndices.push_back(nIndex1);
            m_vIndices.push_back(nIndex2);

            // 第二个三角形
            m_vIndices.push_back(nIndex0);
            m_vIndices.push_back(nIndex2);
            m_vIndices.push_back(nIndex3);
        }
    }

    m_nIndexCount = static_cast<UINT>(m_vIndices.size());

    return S_OK;
}

// ========== 计算法线 ==========
HRESULT D3D12HeightField::CalculateNormals()
{
    if (m_vVertices.empty())
        return E_FAIL;

    // 计算每个顶点的法线
    for (UINT z = 0; z < m_nHeight; ++z)
    {
        for (UINT x = 0; x < m_nWidth; ++x)
        {
            UINT nIndex = z * m_nWidth + x;
            XMFLOAT3 normal = XMFLOAT3(0, 1, 0);

            // 采样周围高度
            float hL = GetHeight(x - 1, z);
            float hR = GetHeight(x + 1, z);
            float hD = GetHeight(x, z - 1);
            float hU = GetHeight(x, z + 1);

            // 计算法线（使用有限差分）
            normal.x = hL - hR;
            normal.z = hD - hU;
            normal.y = 2.0f * m_config.fWorldScale;  // 考虑世界空间缩放

            // 归一化
            float fLength = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (fLength > 0.0001f)
            {
                normal.x /= fLength;
                normal.y /= fLength;
                normal.z /= fLength;
            }

            m_vVertices[nIndex].normal = normal;
        }
    }

    return S_OK;
}

// ========== 创建缓冲区 ==========
HRESULT D3D12HeightField::CreateBuffers()
{
    if (m_vVertices.empty() || m_vIndices.empty())
        return E_FAIL;

    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();
    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();

    // ========== 创建顶点缓冲区 ==========
    D3D12_RESOURCE_DESC vbDesc = {};
    vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vbDesc.Width = m_vVertices.size() * sizeof(TerrainVertex);
    vbDesc.Height = 1;
    vbDesc.DepthOrArraySize = 1;
    vbDesc.MipLevels = 1;
    vbDesc.Format = DXGI_FORMAT_UNKNOWN;
    vbDesc.SampleDesc.Count = 1;
    vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES vbHeapProps = {};
    vbHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    vbHeapProps.CreationNodeMask = 1;
    vbHeapProps.VisibleNodeMask = 1;

    HRESULT hr = pDevice->CreateCommittedResource(
        &vbHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vbDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pVertexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 创建上传堆
    ComPtr<ID3D12Resource> pVertexBufferUpload;
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;

    hr = pDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&pVertexBufferUpload)
    );

    if (FAILED(hr))
        return hr;

    // 映射并复制顶点数据
    void* pVertexData = nullptr;
    pVertexBufferUpload->Map(0, nullptr, &pVertexData);
    memcpy(pVertexData, m_vVertices.data(), m_vVertices.size() * sizeof(TerrainVertex));
    pVertexBufferUpload->Unmap(0, nullptr);

    // 复制到GPU
    pCommandList->CopyResource(m_pVertexBuffer.Get(), pVertexBufferUpload.Get());

    // 资源屏障转换
    D3D12_RESOURCE_BARRIER vbBarrier = {};
    vbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    vbBarrier.Transition.pResource = m_pVertexBuffer.Get();
    vbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    vbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    vbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    pCommandList->ResourceBarrier(1, &vbBarrier);

    // ========== 创建索引缓冲区 ==========
    D3D12_RESOURCE_DESC ibDesc = {};
    ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ibDesc.Width = m_vIndices.size() * sizeof(UINT);
    ibDesc.Height = 1;
    ibDesc.DepthOrArraySize = 1;
    ibDesc.MipLevels = 1;
    ibDesc.Format = DXGI_FORMAT_UNKNOWN;
    ibDesc.SampleDesc.Count = 1;
    ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    hr = pDevice->CreateCommittedResource(
        &vbHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ibDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pIndexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 创建索引上传堆
    ComPtr<ID3D12Resource> pIndexBufferUpload;
    hr = pDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ibDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&pIndexBufferUpload)
    );

    if (FAILED(hr))
        return hr;

    // 映射并复制索引数据
    void* pIndexData = nullptr;
    pIndexBufferUpload->Map(0, nullptr, &pIndexData);
    memcpy(pIndexData, m_vIndices.data(), m_vIndices.size() * sizeof(UINT));
    pIndexBufferUpload->Unmap(0, nullptr);

    // 复制到GPU
    pCommandList->CopyResource(m_pIndexBuffer.Get(), pIndexBufferUpload.Get());

    // 资源屏障转换
    D3D12_RESOURCE_BARRIER ibBarrier = {};
    ibBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    ibBarrier.Transition.pResource = m_pIndexBuffer.Get();
    ibBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    ibBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    ibBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    pCommandList->ResourceBarrier(1, &ibBarrier);

    // ========== 创建顶点缓冲区视图 ==========
    m_vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(TerrainVertex);
    m_vertexBufferView.SizeInBytes = static_cast<UINT>(m_vVertices.size() * sizeof(TerrainVertex));

    // ========== 创建索引缓冲区视图 ==========
    m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = static_cast<UINT>(m_vIndices.size() * sizeof(UINT));

    return S_OK;
}

// ========== 创建地形分块 ==========
HRESULT D3D12HeightField::CreatePatches(UINT nPatchSize)
{
    if (nPatchSize == 0 || nPatchSize > m_nWidth || nPatchSize > m_nHeight)
        return E_INVALIDARG;

    m_nPatchSize = nPatchSize;

    // 清空现有分块
    m_vPatches.clear();

    // 计算分块数量
    UINT nPatchesX = (m_nWidth + nPatchSize - 2) / (nPatchSize - 1);
    UINT nPatchesZ = (m_nHeight + nPatchSize - 2) / (nPatchSize - 1);

    m_vPatches.reserve(nPatchesX * nPatchesZ);

    // 创建分块
    for (UINT z = 0; z < nPatchesZ; ++z)
    {
        for (UINT x = 0; x < nPatchesX; ++x)
        {
            TerrainPatch patch;
            patch.nX = x;
            patch.nZ = z;
            patch.nSize = nPatchSize;
            patch.nLOD = 0;

            // 计算分块顶点数和索引数
            patch.nVertexCount = nPatchSize * nPatchSize;
            patch.nIndexCount = (nPatchSize - 1) * (nPatchSize - 1) * 6;

            // 计算精确的包围球
            CalculatePatchBoundingBox(patch, x, z);

            patch.bVisible = FALSE;

            m_vPatches.push_back(patch);
        }
    }

    return S_OK;
}

// ========== 计算分块包围盒和包围球 ==========
void D3D12HeightField::CalculatePatchBoundingBox(TerrainPatch& patch, UINT nPatchX, UINT nPatchZ)
{
    UINT nStartX = nPatchX * (m_nPatchSize - 1);
    UINT nStartZ = nPatchZ * (m_nPatchSize - 1);
    UINT nEndX = (std::min)(nStartX + m_nPatchSize, m_nWidth);
    UINT nEndZ = (std::min)(nStartZ + m_nPatchSize, m_nHeight);

    XMFLOAT3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // 遍历分块中的所有顶点
    for (UINT z = nStartZ; z < nEndZ; ++z)
    {
        for (UINT x = nStartX; x < nEndX; ++x)
        {
            UINT nIndex = z * m_nWidth + x;
            const XMFLOAT3& pos = m_vVertices[nIndex].position;

            vMin.x = (std::min)(vMin.x, pos.x);
            vMin.y = (std::min)(vMin.y, pos.y);
            vMin.z = (std::min)(vMin.z, pos.z);

            vMax.x = (std::max)(vMax.x, pos.x);
            vMax.y = (std::max)(vMax.y, pos.y);
            vMax.z = (std::max)(vMax.z, pos.z);
        }
    }

    // 计算中心点
    patch.v3Center.x = (vMin.x + vMax.x) * 0.5f;
    patch.v3Center.y = (vMin.y + vMax.y) * 0.5f;
    patch.v3Center.z = (vMin.z + vMax.z) * 0.5f;

    // 计算半径（从中心到最远顶点的距离）
    patch.fRadius = 0.0f;
    for (UINT z = nStartZ; z < nEndZ; ++z)
    {
        for (UINT x = nStartX; x < nEndX; ++x)
        {
            UINT nIndex = z * m_nWidth + x;
            const XMFLOAT3& pos = m_vVertices[nIndex].position;

            XMFLOAT3 diff;
            diff.x = pos.x - patch.v3Center.x;
            diff.y = pos.y - patch.v3Center.y;
            diff.z = pos.z - patch.v3Center.z;

            float fDistance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            patch.fRadius = (std::max)(patch.fRadius, fDistance);
        }
    }
}

// ========== 更新分块（LOD）==========
void D3D12HeightField::UpdatePatches(const XMFLOAT3& cameraPos, float fLODDistance)
{
    for (auto& patch : m_vPatches)
    {
        // 计算到相机的距离
        XMFLOAT3 v3Diff;
        v3Diff.x = patch.v3Center.x - cameraPos.x;
        v3Diff.y = patch.v3Center.y - cameraPos.y;
        v3Diff.z = patch.v3Center.z - cameraPos.z;
        float fDistance = std::sqrt(v3Diff.x * v3Diff.x + v3Diff.y * v3Diff.y + v3Diff.z * v3Diff.z);

        // 可见性裁剪
        patch.bVisible = (fDistance < fLODDistance * 2.0f);

        // LOD选择（基于距离）
        if (patch.bVisible)
        {
            // 距离越远，LOD越高（越低精度）
            if (fDistance < fLODDistance * 0.3f)
            {
                patch.nLOD = 0;  // 最高精度
            }
            else if (fDistance < fLODDistance * 0.6f)
            {
                patch.nLOD = 1;
            }
            else if (fDistance < fLODDistance * 1.0f)
            {
                patch.nLOD = 2;
            }
            else
            {
                patch.nLOD = 3;  // 最低精度
            }
        }
    }
}

// ========== 渲染分块 ==========
void D3D12HeightField::RenderPatch(ID3D12GraphicsCommandList* pCommandList, UINT nPatchIndex)
{
    if (!pCommandList || nPatchIndex >= m_vPatches.size())
        return;

    TerrainPatch& patch = m_vPatches[nPatchIndex];

    if (!patch.bVisible)
        return;

    // 设置顶点缓冲区
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 设置索引缓冲区
    pCommandList->IASetIndexBuffer(&m_indexBufferView);

    // 设置图元拓扑
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 计算分块的起始索引和索引数量
    UINT nStartX = patch.nX * (m_nPatchSize - 1);
    UINT nStartZ = patch.nZ * (m_nPatchSize - 1);

    // 计算分块的起始索引
    UINT nStartIndex = nStartZ * (m_nWidth - 1) * 6 + nStartX * 6;

    // 计算实际的索引数量（考虑边界）
    UINT nActualWidth = (std::min)(m_nPatchSize, m_nWidth - nStartX);
    UINT nActualHeight = (std::min)(m_nPatchSize, m_nHeight - nStartZ);
    UINT nIndexCount = (nActualWidth - 1) * (nActualHeight - 1) * 6;

    // 绘制分块
    pCommandList->DrawIndexedInstanced(nIndexCount, 1, nStartIndex, 0, 0);
}

// ========== 渲染整个地形 ==========
void D3D12HeightField::Render(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return;

    // 渲染所有可见的分块
    for (UINT i = 0; i < m_vPatches.size(); ++i)
    {
        RenderPatch(pCommandList, i);
    }
}

// ========== 渲染所有分块 ==========
void D3D12HeightField::RenderAllPatches(ID3D12GraphicsCommandList* pCommandList)
{
    Render(pCommandList);
}

// ========== 添加纹理层 ==========
HRESULT D3D12HeightField::AddLayer(const TerrainLayer& layer)
{
    try
    {
        m_vLayers.push_back(layer);
        return S_OK;
    }
    catch (...)
    {
        return E_OUTOFMEMORY;
    }
}

// ========== 移除纹理层 ==========
void D3D12HeightField::RemoveLayer(UINT nIndex)
{
    if (nIndex < m_vLayers.size())
    {
        m_vLayers.erase(m_vLayers.begin() + nIndex);
    }
}

// ========== 获取纹理层 ==========
TerrainLayer* D3D12HeightField::GetLayer(UINT nIndex)
{
    if (nIndex >= m_vLayers.size())
        return nullptr;

    return &m_vLayers[nIndex];
}

// ========== 获取纹理层数量 ==========
UINT D3D12HeightField::GetLayerCount() const
{
    return static_cast<UINT>(m_vLayers.size());
}

// ========== 获取高度（整数坐标）==========
float D3D12HeightField::GetHeight(UINT x, UINT z) const
{
    if (x >= m_nWidth || z >= m_nHeight)
        return 0.0f;

    return m_vHeightData[z * m_nWidth + x];
}

// ========== 获取高度（浮点坐标，插值）==========
float D3D12HeightField::GetHeight(float x, float z) const
{
    // 转换到整数坐标
    float fWorldX = x / m_config.fWorldScale;
    float fWorldZ = z / m_config.fWorldScale;

    int x0 = static_cast<int>(std::floor(fWorldX));
    int z0 = static_cast<int>(std::floor(fWorldZ));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // 边界检查
    if (x0 < 0) x0 = 0;
    if (x1 >= static_cast<int>(m_nWidth)) x1 = m_nWidth - 1;
    if (z0 < 0) z0 = 0;
    if (z1 >= static_cast<int>(m_nHeight)) z1 = m_nHeight - 1;

    // 双线性插值
    float fx = fWorldX - x0;
    float fz = fWorldZ - z0;

    float h00 = GetHeight(static_cast<UINT>(x0), static_cast<UINT>(z0));
    float h10 = GetHeight(static_cast<UINT>(x1), static_cast<UINT>(z0));
    float h01 = GetHeight(static_cast<UINT>(x0), static_cast<UINT>(z1));
    float h11 = GetHeight(static_cast<UINT>(x1), static_cast<UINT>(z1));

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;

    return h0 * (1.0f - fz) + h1 * fz;
}

// ========== 获取法线（插值）==========
XMFLOAT3 D3D12HeightField::GetNormal(float x, float z) const
{
    // 使用有限差分计算法线
    float hL = GetHeight(x - 1.0f, z);
    float hR = GetHeight(x + 1.0f, z);
    float hD = GetHeight(x, z - 1.0f);
    float hU = GetHeight(x, z + 1.0f);

    XMFLOAT3 normal;
    normal.x = hL - hR;
    normal.z = hD - hU;
    normal.y = 2.0f * m_config.fWorldScale;  // 考虑世界空间缩放

    // 归一化
    float fLength = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (fLength > 0.0001f)
    {
        normal.x /= fLength;
        normal.y /= fLength;
        normal.z /= fLength;
    }

    return normal;
}

// ========== 检查是否在地形上 ==========
BOOL D3D12HeightField::IsOnTerrain(float x, float z) const
{
    float fWorldX = x / m_config.fWorldScale;
    float fWorldZ = z / m_config.fWorldScale;

    return (fWorldX >= 0.0f && fWorldX < m_nWidth &&
            fWorldZ >= 0.0f && fWorldZ < m_nHeight);
}

// ========== 获取配置 ==========
const HeightMapConfig& D3D12HeightField::GetConfig() const
{
    return m_config;
}

// ========== 获取宽度 ==========
UINT D3D12HeightField::GetWidth() const
{
    return m_nWidth;
}

// ========== 获取高度 ==========
UINT D3D12HeightField::GetHeight() const
{
    return m_nHeight;
}

// ========== 获取分块数量 ==========
UINT D3D12HeightField::GetPatchCount() const
{
    return static_cast<UINT>(m_vPatches.size());
}

// ============================================
// IDIHeightField 接口实现
// ============================================

// ========== 开始初始化高度场 ==========
BOOL __stdcall D3D12HeightField::StartInitialize(HFIELD_DESC* pDesc)
{
    if (!pDesc)
        return FALSE;

    // TODO: 实现完整的HFIELD_DESC初始化
    // 需要根据HFIELD_DESC配置高度场参数
    return TRUE;
}

// ========== 结束初始化 ==========
void __stdcall D3D12HeightField::EndInitialize()
{
    // TODO: 完成初始化后的处理
    // 例如：计算法线、创建缓冲区等
}

// ========== 创建高度场网格对象 ==========
IDIMeshObject* __stdcall D3D12HeightField::CreateHeightFieldObject(HFIELD_OBJECT_DESC* pDesc)
{
    if (!pDesc)
        return nullptr;

    // TODO: 创建高度场网格对象
    // 返回一个D3D12MeshObject实例，用于渲染高度场的一部分
    return nullptr;
}

// ========== 初始化索引缓冲池 ==========
BOOL __stdcall D3D12HeightField::InitiallizeIndexBufferPool(DWORD dwDetailLevel, DWORD dwIndicesNum, DWORD dwNum)
{
    // TODO: 实现LOD索引缓冲池初始化
    return TRUE;
}

// ========== 加载纹理调色板 ==========
BOOL __stdcall D3D12HeightField::LoadTilePalette(TEXTURE_TABLE* pTexTable, DWORD dwTileTextureNum)
{
    if (!pTexTable)
        return FALSE;

    // TODO: 加载地形纹理调色板
    return TRUE;
}

// ========== 替换纹理 ==========
BOOL __stdcall D3D12HeightField::ReplaceTile(char* szFileName, DWORD dwTexIndex)
{
    if (!szFileName)
        return FALSE;

    // TODO: 替换指定索引的地形纹理
    return TRUE;
}

// ========== 创建索引缓冲 ==========
BOOL __stdcall D3D12HeightField::CreateIndexBuffer(DWORD dwIndicesNum, DWORD dwDetailLevel, DWORD dwPositionMask, DWORD dwNum)
{
    // TODO: 为LOD级别创建索引缓冲
    return TRUE;
}

// ========== 锁定索引缓冲指针 ==========
BOOL __stdcall D3D12HeightField::LockIndexBufferPtr(WORD** ppWord, DWORD dwDetailLevel, DWORD dwPositionMask)
{
    if (!ppWord)
        return FALSE;

    // TODO: 锁定并返回索引缓冲指针
    return FALSE;
}

// ========== 解锁索引缓冲指针 ==========
void __stdcall D3D12HeightField::UnlcokIndexBufferPtr(DWORD dwDetailLevel, DWORD dwPositionMask)
{
    // TODO: 解锁索引缓冲
}

// ========== 渲染网格 ==========
BOOL __stdcall D3D12HeightField::RenderGrid(VECTOR3* pv3Quad, DWORD dwTexTileIndex, DWORD dwAlpha)
{
    if (!pv3Quad)
        return FALSE;

    // TODO: 渲染地形网格（基于四边形区域）
    return TRUE;
}

// ========== 设置高度场纹理混合 ==========
void __stdcall D3D12HeightField::SetHFieldTileBlend(BOOL bSwitch)
{
    m_bTileBlendEnabled = bSwitch;
}

// ========== 检查是否启用高度场纹理混合 ==========
BOOL __stdcall D3D12HeightField::IsEnableHFieldTileBlend()
{
    return m_bTileBlendEnabled;
}
