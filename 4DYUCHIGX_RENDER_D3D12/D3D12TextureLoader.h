// ============================================
// D3D12TextureLoader.h
// DirectX 12 纹理加载器
// 100% 完成度实施
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <string>
#include <vector>
#include <cstdint>
// 前向声明
class CoD3D12Device;

// DDS 像素格式
struct DDS_PIXELFORMAT
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

// DDS 头部
struct DDS_HEADER
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    DWORD dwPitchOrLinearSize;
    DWORD dwDepth;
    DWORD dwMipMapCount;
    DWORD dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
    DWORD dwReserved2;
};

// DirectX 12 纹理加载器
class D3D12TextureLoader
{
public:
    D3D12TextureLoader();
    ~D3D12TextureLoader();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);
    void Release();

    // ========== 从文件加载纹理 ==========

    // 从 DDS 文件加载纹理（内部使用）
    HRESULT LoadTextureFromDDS(const char* szFileName,
                               ID3D12Resource** ppTexture,
                               D3D12_SUBRESOURCE_DATA* pSubresourceData);

    // 从 DDS 文件加载纹理
    HRESULT LoadTextureFromFile(const char* szFileName,
                                ID3D12Resource** ppTexture,
                                D3D12_SUBRESOURCE_DATA* pSubresourceData);

    // 从 TGA 文件加载纹理
    HRESULT LoadTextureFromTGA(const char* szFileName,
                               ID3D12Resource** ppTexture,
                               D3D12_SUBRESOURCE_DATA* pSubresourceData);

    // 从内存数据创建纹理
    HRESULT CreateTextureFromMemory(const void* pData,
                                    UINT width,
                                    UINT height,
                                    DXGI_FORMAT format,
                                    ID3D12Resource** ppTexture,
                                    D3D12_SUBRESOURCE_DATA* pSubresourceData);

    // ========== 创建纹理资源 ==========

    // 创建 2D 纹理资源
    HRESULT CreateTexture2D(UINT width,
                            UINT height,
                            DXGI_FORMAT format,
                            UINT mipLevels,
                            ID3D12Resource** ppTexture);

    // 创建上传堆纹理
    HRESULT CreateUploadTexture(UINT width,
                                UINT height,
                                DXGI_FORMAT format,
                                const void* pData,
                                UINT rowPitch,
                                UINT slicePitch,
                                ID3D12Resource** ppUploadTexture);

    // ========== 上传纹理数据 ==========

    // 上传纹理数据到 GPU
    HRESULT UploadTextureData(ID3D12Resource* pDestination,
                               ID3D12Resource* pSource,
                               UINT64 uploadBufferSize,
                               ID3D12GraphicsCommandList* pCommandList);

    // 生成 MipMaps
    HRESULT GenerateMipMaps(ID3D12Resource* pTexture,
                            ID3D12GraphicsCommandList* pCommandList);

    // ========== 辅助功能 ==========

    // 获取 DXGI 格式的像素大小
    static UINT GetFormatSize(DXGI_FORMAT format);

    // 判断是否为压缩格式
    static BOOL IsCompressedFormat(DXGI_FORMAT format);

    // 获取文件扩展名
    static std::string GetFileExtension(const char* szFileName);

    // 从 DDS 文件读取头部信息
    HRESULT ReadDDSHeader(const uint8_t* pData, size_t dataSize,
                          DDS_HEADER* pHeader, DDS_PIXELFORMAT* pPixelFormat);

    // 解析 DDS 格式到 DXGI 格式
    DXGI_FORMAT ParseDDSFormat(const DDS_PIXELFORMAT& ddpf);

private:
    // TGA 文件头部
    struct TGA_HEADER
    {
        uint8_t  bIDLength;
        uint8_t  bColorMapType;
        uint8_t  bImageType;
        uint16_t wColorMapEntry;
        uint16_t wColorMapLength;
        uint8_t  bColorMapEntrySize;
        uint16_t wXOrigin;
        uint16_t wYOrigin;
        uint16_t wWidth;
        uint16_t wHeight;
        uint8_t  bBitsPerPixel;
        uint8_t  bImageDescriptor;
    };

    // ========== 内部辅助方法 ==========

    // 读取整个文件到内存
    HRESULT ReadFileToMemory(const char* szFileName,
                             std::vector<uint8_t>& fileData);

    // 解压缩 BC1-BC7 纹理数据
    HRESULT DecompressBC(const void* pCompressedData,
                         UINT width,
                         UINT height,
                         DXGI_FORMAT format,
                         std::vector<uint8_t>& decompressedData);

    // 转换 BGRA 到 RGBA
    void ConvertBGRAtoRGBA(uint8_t* pData, UINT pixelCount);

    // ========== 成员变量 ==========

    CoD3D12Device* m_pDevice;
    ID3D12Device* m_pD3D12Device;

    // 上传命令队列（用于纹理上传）
    ComPtr<ID3D12CommandQueue> m_pUploadQueue;
    ComPtr<ID3D12CommandAllocator> m_pUploadAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_pUploadCommandList;

    bool m_bInitialized;
};
