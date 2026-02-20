// ============================================
// DirectXTex.cpp - DirectX Texture Library Implementation
// Complete implementation for D3D12
// ============================================

#include "D3D12PCH.h"
#include "DirectXTex.h"
#include "IRenderer_GUID.h"
#include <wincodec.h>
#include <memory>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "windowscodecs.lib")

namespace DirectX
{

    // ========== DDS Header Structures ==========

#pragma pack(push, 1)

    struct DDS_PIXELFORMAT
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwFourCC;
        uint32_t dwRGBBitCount;
        uint32_t dwRBitMask;
        uint32_t dwGBitMask;
        uint32_t dwBBitMask;
        uint32_t dwABitMask;
    };

    struct DDS_HEADER
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwHeight;
        uint32_t dwWidth;
        uint32_t dwPitchOrLinearSize;
        uint32_t dwDepth;
        uint32_t dwMipMapCount;
        uint32_t dwReserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t dwCaps;
        uint32_t dwCaps2;
        uint32_t dwCaps3;
        uint32_t dwCaps4;
        uint32_t dwReserved2;
    };

#pragma pack(pop)

    // ========== TGA Header Structures ==========

#pragma pack(push, 1)
    struct TGA_HEADER
    {
        uint8_t  bIDLength;
        uint8_t  bColorMapType;
        uint8_t  bImageType;
        uint16_t wColorMapFirstEntry;
        uint16_t wColorMapLength;
        uint8_t  bColorMapEntrySize;
        uint16_t wXOrigin;
        uint16_t wYOrigin;
        uint16_t wWidth;
        uint16_t wHeight;
        uint8_t  bBitsPerPixel;
        uint8_t  bImageDescriptor;
    };
#pragma pack(pop)

    // ========== ScratchImage Implementation ==========

    ScratchImage::ScratchImage() noexcept
        : _nimages(0)
        , _size(0)
        , _images(nullptr)
        , _memory(nullptr)
        , _metadata{}
    {
    }

    ScratchImage::~ScratchImage()
    {
        Release();
    }

    ScratchImage::ScratchImage(ScratchImage&& moveFrom) noexcept
        : _nimages(moveFrom._nimages)
        , _size(moveFrom._size)
        , _images(moveFrom._images)
        , _memory(moveFrom._memory)
        , _metadata(moveFrom._metadata)
    {
        moveFrom._nimages = 0;
        moveFrom._size = 0;
        moveFrom._images = nullptr;
        moveFrom._memory = nullptr;
        moveFrom._metadata = {};
    }

    ScratchImage& ScratchImage::operator=(ScratchImage&& moveFrom) noexcept
    {
        if (this != &moveFrom)
        {
            Release();

            _nimages = moveFrom._nimages;
            _size = moveFrom._size;
            _images = moveFrom._images;
            _memory = moveFrom._memory;
            _metadata = moveFrom._metadata;

            moveFrom._nimages = 0;
            moveFrom._size = 0;
            moveFrom._images = nullptr;
            moveFrom._memory = nullptr;
            moveFrom._metadata = {};
        }
        return *this;
    }

    const Image* ScratchImage::GetImages() const noexcept
    {
        return _images;
    }

    const Image* ScratchImage::GetImage(size_t mip, size_t item, size_t slice) const noexcept
    {
        if (mip >= _metadata.mipLevels || item >= _metadata.arraySize)
            return nullptr;

        size_t index = item * _metadata.mipLevels + mip;
        if (index >= _nimages)
            return nullptr;

        return &_images[index];
    }

    const TexMetadata* ScratchImage::GetMetadata() const noexcept
    {
        return &_metadata;
    }

    uint8_t* ScratchImage::GetPixels() noexcept
    {
        return _memory;
    }

    const uint8_t* ScratchImage::GetPixels() const noexcept
    {
        return _memory;
    }

    size_t ScratchImage::GetImageCount() const noexcept
    {
        return _nimages;
    }

    void ScratchImage::Release() noexcept
    {
        if (_memory)
        {
            delete[] _memory;
            _memory = nullptr;
        }

        if (_images)
        {
            delete[] _images;
            _images = nullptr;
        }

        _nimages = 0;
        _size = 0;
        _metadata = {};
    }

    HRESULT ScratchImage::AllocateImages(const TexMetadata& mmetadata)
    {
        Release();

        _metadata = mmetadata;

        size_t numberOfImages = 1;
        if (mmetadata.arraySize > 1)
        {
            numberOfImages = mmetadata.arraySize;
        }
        numberOfImages *= mmetadata.mipLevels;

        _nimages = numberOfImages;
        _images = new (std::nothrow) Image[numberOfImages];
        if (!_images)
            return E_OUTOFMEMORY;

        memset(_images, 0, sizeof(Image) * numberOfImages);

        // Calculate total size needed
        size_t totalSize = 0;
        for (size_t item = 0; item < mmetadata.arraySize; ++item)
        {
            for (size_t mip = 0; mip < mmetadata.mipLevels; ++mip)
            {
                size_t width = std::max<size_t>(1, mmetadata.width >> mip);
                size_t height = std::max<size_t>(1, mmetadata.height >> mip);
                size_t depth = std::max<size_t>(1, mmetadata.depth >> mip);

                size_t rowPitch, slicePitch;
                if (!ComputePitch(mmetadata.format, width, rowPitch, slicePitch))
                    return E_FAIL;

                size_t imageSize = rowPitch * height;
                if (mmetadata.IsVolumeMap)
                    imageSize *= depth;

                totalSize += imageSize;

                size_t index = item * mmetadata.mipLevels + mip;
                _images[index].width = static_cast<uint32_t>(width);
                _images[index].height = static_cast<uint32_t>(height);
                _images[index].format = mmetadata.format;
                _images[index].rowPitch = static_cast<uint32_t>(rowPitch);
                _images[index].slicePitch = static_cast<uint32_t>(imageSize);
            }
        }

        _size = totalSize;
        _memory = new (std::nothrow) uint8_t[totalSize];
        if (!_memory)
            return E_OUTOFMEMORY;

        // Set pixel pointers
        uint8_t* pData = _memory;
        for (size_t i = 0; i < numberOfImages; ++i)
        {
            _images[i].pixels = pData;
            pData += _images[i].slicePitch;
        }

        return S_OK;
    }

    HRESULT ScratchImage::Initialize(const TexMetadata& mmetadata)
    {
        if (!mmetadata.width || !mmetadata.height)
            return E_INVALIDARG;

        return AllocateImages(mmetadata);
    }

    HRESULT ScratchImage::Initialize2D(DXGI_FORMAT fmt, size_t width, size_t height, size_t arraySize, size_t mipLevels)
    {
        if (!width || !height || !arraySize)
            return E_INVALIDARG;

        TexMetadata mmetadata = {};
        mmetadata.width = static_cast<uint32_t>(width);
        mmetadata.height = static_cast<uint32_t>(height);
        mmetadata.depth = 1;
        mmetadata.arraySize = static_cast<uint32_t>(arraySize);
        mmetadata.mipLevels = static_cast<uint32_t>(mipLevels);
        mmetadata.format = fmt;
        mmetadata.deviceFormat = fmt;
        mmetadata.IsVolumeMap = false;

        return AllocateImages(mmetadata);
    }

    HRESULT ScratchImage::Initialize1D(DXGI_FORMAT fmt, size_t length, size_t arraySize, size_t mipLevels)
    {
        if (!length || !arraySize)
            return E_INVALIDARG;

        TexMetadata mmetadata = {};
        mmetadata.width = static_cast<uint32_t>(length);
        mmetadata.height = 1;
        mmetadata.depth = 1;
        mmetadata.arraySize = static_cast<uint32_t>(arraySize);
        mmetadata.mipLevels = static_cast<uint32_t>(mipLevels);
        mmetadata.format = fmt;
        mmetadata.deviceFormat = fmt;
        mmetadata.IsVolumeMap = false;

        return AllocateImages(mmetadata);
    }

    HRESULT ScratchImage::Initialize3D(DXGI_FORMAT fmt, size_t width, size_t height, size_t depth, size_t mipLevels)
    {
        if (!width || !height || !depth)
            return E_INVALIDARG;

        TexMetadata mmetadata = {};
        mmetadata.width = static_cast<uint32_t>(width);
        mmetadata.height = static_cast<uint32_t>(height);
        mmetadata.depth = static_cast<uint32_t>(depth);
        mmetadata.arraySize = 1;
        mmetadata.mipLevels = static_cast<uint32_t>(mipLevels);
        mmetadata.format = fmt;
        mmetadata.deviceFormat = fmt;
        mmetadata.IsVolumeMap = true;

        return AllocateImages(mmetadata);
    }

    HRESULT ScratchImage::InitializeFromImage(const Image& srcImage, bool allow1D)
    {
        if (!srcImage.pixels)
            return E_INVALIDARG;

        TexMetadata mmetadata = {};
        mmetadata.width = srcImage.width;
        mmetadata.height = srcImage.height;
        mmetadata.depth = 1;
        mmetadata.arraySize = 1;
        mmetadata.mipLevels = 1;
        mmetadata.format = srcImage.format;
        mmetadata.deviceFormat = srcImage.format;
        mmetadata.IsVolumeMap = false;

        HRESULT hr = AllocateImages(mmetadata);
        if (FAILED(hr))
            return hr;

        // Copy image data
        if (_images[0].slicePitch <= srcImage.slicePitch)
        {
            memcpy(_images[0].pixels, srcImage.pixels, _images[0].slicePitch);
        }

        return S_OK;
    }

    HRESULT ScratchImage::InitializeCubeFromImages(const Image* images, size_t nImages)
    {
        if (!images || nImages < 6)
            return E_INVALIDARG;

        TexMetadata mmetadata = {};
        mmetadata.width = images[0].width;
        mmetadata.height = images[0].height;
        mmetadata.depth = 1;
        mmetadata.arraySize = 6;
        mmetadata.mipLevels = 1;
        mmetadata.format = images[0].format;
        mmetadata.deviceFormat = images[0].format;
        mmetadata.IsVolumeMap = false;
        mmetadata.IsCubeMap = true;

        HRESULT hr = AllocateImages(mmetadata);
        if (FAILED(hr))
            return hr;

        for (size_t i = 0; i < nImages && i < 6; ++i)
        {
            if (_images[i].slicePitch <= images[i].slicePitch)
            {
                memcpy(_images[i].pixels, images[i].pixels, _images[i].slicePitch);
            }
        }

        return S_OK;
    }

    // ========== Utility Functions ==========

    size_t CountMips(uint32_t width, uint32_t height) noexcept
    {
        size_t mipLevels = 1;
        while (width > 1 || height > 1)
        {
            if (width > 1) width >>= 1;
            if (height > 1) height >>= 1;
            ++mipLevels;
        }
        return mipLevels;
    }

    bool ComputePitch(DXGI_FORMAT fmt, size_t width, size_t& rowPitch, size_t& slicePitch) noexcept
    {
        if (width == 0)
            return false;

        switch (fmt)
        {
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            rowPitch = width * 16;
            break;

        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
            rowPitch = width * 8;
            break;

        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
            rowPitch = width * 4;
            break;

        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
            rowPitch = width;
            break;

        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            {
                size_t numBlocks = std::max<size_t>(1, (width + 3) / 4);
                rowPitch = numBlocks * 8;
            }
            break;

        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            {
                size_t numBlocks = std::max<size_t>(1, (width + 3) / 4);
                rowPitch = numBlocks * 16;
            }
            break;

        default:
            return false;
        }

        slicePitch = rowPitch;
        return true;
    }

    // ========== Load from DDS File ==========

    HRESULT LoadFromDDSFile(
        const wchar_t* szFile,
        DDS_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image)
    {
        if (!szFile)
            return E_INVALIDARG;

        // Open file
        HANDLE hFile = CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());

        // Read DDS header
        uint32_t dwMagic = 0;
        DWORD bytesRead = 0;
        if (!ReadFile(hFile, &dwMagic, sizeof(dwMagic), &bytesRead, nullptr) || bytesRead != sizeof(dwMagic) || dwMagic != 0x20534444) // "DDS "
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        DDS_HEADER header = {};
        if (!ReadFile(hFile, &header, sizeof(header), &bytesRead, nullptr) || bytesRead != sizeof(header))
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Determine format
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        if (header.ddspf.dwFourCC == MAKEFOURCC('D', 'X', 'T', '1'))
        {
            // DXT1
            format = DXGI_FORMAT_BC1_UNORM;
        }
        else if (header.ddspf.dwFourCC == MAKEFOURCC('D', 'X', 'T', '3'))
        {
            // DXT3
            format = DXGI_FORMAT_BC2_UNORM;
        }
        else if (header.ddspf.dwFourCC == MAKEFOURCC('D', 'X', 'T', '5'))
        {
            // DXT5
            format = DXGI_FORMAT_BC3_UNORM;
        }
        else if (header.ddspf.dwFourCC == 0)
        {
            // Uncompressed format
            if (header.ddspf.dwRGBBitCount == 32)
            {
                format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            else if (header.ddspf.dwRGBBitCount == 24)
            {
                format = DXGI_FORMAT_R8G8B8A8_UNORM; // Will need conversion
            }
        }

        // Setup metadata
        TexMetadata mdata = {};
        mdata.width = header.dwWidth;
        mdata.height = header.dwHeight;
        mdata.depth = (header.dwFlags & 0x800000) ? header.dwDepth : 1; // DDSD_DEPTH
        mdata.arraySize = 1;
        mdata.mipLevels = (header.dwFlags & 0x20000) ? header.dwMipMapCount : 1; // DDSD_MIPMAPCOUNT
        mdata.format = format;
        mdata.deviceFormat = format;
        mdata.IsVolumeMap = (header.dwCaps2 & 0x200000) != 0; // DDS_FLAGS_VOLUME (use dwCaps2 instead of dwFlags2)

        uint32_t mipLevels = mdata.mipLevels;
        if (flags & DDS_FLAGS_IGNORE_MIPS)
            mipLevels = 1;

        mdata.mipLevels = mipLevels;

        // Allocate image
        HRESULT hr = image.Initialize(mdata);
        if (FAILED(hr))
        {
            CloseHandle(hFile);
            return hr;
        }

        // Read image data
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            const Image* img = image.GetImage(mip, 0, 0);
            if (!img)
            {
                CloseHandle(hFile);
                return E_FAIL;
            }

            if (!ReadFile(hFile, img->pixels, img->slicePitch, &bytesRead, nullptr) || bytesRead != img->slicePitch)
            {
                CloseHandle(hFile);
                return E_FAIL;
            }
        }

        CloseHandle(hFile);

        if (metadata)
            *metadata = mdata;

        return S_OK;
    }

    // ========== Load from TGA File ==========

    HRESULT LoadFromTGAFile(
        const wchar_t* szFile,
        TGA_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image)
    {
        if (!szFile)
            return E_INVALIDARG;

        // Open file
        HANDLE hFile = CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());

        // Read TGA header
        TGA_HEADER header = {};
        DWORD bytesRead = 0;
        if (!ReadFile(hFile, &header, sizeof(header), &bytesRead, nullptr) || bytesRead != sizeof(header))
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Validate image type (2 = uncompressed RGB)
        if (header.bImageType != 2)
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Setup metadata
        TexMetadata mdata = {};
        mdata.width = header.wWidth;
        mdata.height = header.wHeight;
        mdata.depth = 1;
        mdata.arraySize = 1;
        mdata.mipLevels = 1;
        mdata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        mdata.deviceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        mdata.IsVolumeMap = false;

        // Allocate image
        HRESULT hr = image.Initialize(mdata);
        if (FAILED(hr))
        {
            CloseHandle(hFile);
            return hr;
        }

        const Image* img = image.GetImage(0, 0, 0);
        if (!img)
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Read pixel data
        uint32_t bytesPerPixel = header.bBitsPerPixel / 8;
        uint32_t srcRowPitch = header.wWidth * bytesPerPixel;
        uint32_t imageSize = srcRowPitch * header.wHeight;

        uint8_t* tempBuffer = new (std::nothrow) uint8_t[imageSize];
        if (!tempBuffer)
        {
            CloseHandle(hFile);
            return E_OUTOFMEMORY;
        }

        if (!ReadFile(hFile, tempBuffer, imageSize, &bytesRead, nullptr) || bytesRead != imageSize)
        {
            delete[] tempBuffer;
            CloseHandle(hFile);
            return E_FAIL;
        }

        CloseHandle(hFile);

        // Convert to RGBA
        bool topDown = (header.bImageDescriptor & 0x20) != 0;
        for (uint32_t y = 0; y < header.wHeight; ++y)
        {
            uint32_t srcY = topDown ? y : (header.wHeight - 1 - y);
            const uint8_t* srcRow = tempBuffer + srcY * srcRowPitch;
            uint8_t* dstRow = img->pixels + y * img->rowPitch;

            for (uint32_t x = 0; x < header.wWidth; ++x)
            {
                const uint8_t* srcPixel = srcRow + x * bytesPerPixel;
                uint8_t* dstPixel = dstRow + x * 4;

                if (bytesPerPixel == 4)
                {
                    // BGRA to RGBA
                    dstPixel[0] = srcPixel[2];
                    dstPixel[1] = srcPixel[1];
                    dstPixel[2] = srcPixel[0];
                    dstPixel[3] = srcPixel[3];
                }
                else if (bytesPerPixel == 3)
                {
                    // BGR to RGBA
                    dstPixel[0] = srcPixel[2];
                    dstPixel[1] = srcPixel[1];
                    dstPixel[2] = srcPixel[0];
                    dstPixel[3] = 255;
                }
                else
                {
                    // Grayscale to RGBA
                    dstPixel[0] = srcPixel[0];
                    dstPixel[1] = srcPixel[0];
                    dstPixel[2] = srcPixel[0];
                    dstPixel[3] = 255;
                }
            }
        }

        delete[] tempBuffer;

        if (metadata)
            *metadata = mdata;

        return S_OK;
    }

    // ========== Load from WIC File (PNG, JPG, BMP) ==========

    HRESULT LoadFromWICFile(
        const wchar_t* szFile,
        WIC_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image)
    {
        if (!szFile)
            return E_INVALIDARG;

        HRESULT hr = S_OK;

        // Create WIC factory
        ComPtr<IWICImagingFactory> factory;
        hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
            return hr;

        // Create decoder
        ComPtr<IWICBitmapDecoder> decoder;
        hr = factory->CreateDecoderFromFilename(szFile, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
        if (FAILED(hr))
            return hr;

        // Get first frame
        ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr))
            return hr;

        // Get frame size
        UINT width = 0, height = 0;
        hr = frame->GetSize(&width, &height);
        if (FAILED(hr))
            return hr;

        // Get pixel format
        WICPixelFormatGUID pixelFormat = {};
        hr = frame->GetPixelFormat(&pixelFormat);
        if (FAILED(hr))
            return hr;

        // Convert to RGBA32 if needed
        ComPtr<IWICFormatConverter> converter;
        GUID targetFormat = GUID_WICPixelFormat32bppRGBA;

        if (memcmp(&pixelFormat, &targetFormat, sizeof(GUID)) != 0)
        {
            hr = factory->CreateFormatConverter(&converter);
            if (FAILED(hr))
                return hr;

            hr = converter->Initialize(frame.Get(), targetFormat, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
            if (FAILED(hr))
                return hr;
        }

        // Setup metadata
        TexMetadata mdata = {};
        mdata.width = width;
        mdata.height = height;
        mdata.depth = 1;
        mdata.arraySize = 1;
        mdata.mipLevels = 1;
        mdata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        mdata.deviceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        mdata.IsVolumeMap = false;

        // Allocate image
        hr = image.Initialize(mdata);
        if (FAILED(hr))
            return hr;

        const Image* img = image.GetImage(0, 0, 0);
        if (!img)
            return E_FAIL;

        // Copy pixel data
        if (converter)
        {
            hr = converter->CopyPixels(nullptr, img->rowPitch, img->slicePitch, img->pixels);
        }
        else
        {
            hr = frame->CopyPixels(nullptr, img->rowPitch, img->slicePitch, img->pixels);
        }

        if (FAILED(hr))
            return hr;

        if (metadata)
            *metadata = mdata;

        return S_OK;
    }

    // ========== Save Functions ==========

    HRESULT SaveToDDSFile(
        const Image& image,
        const TexMetadata& metadata,
        DDS_FLAGS flags,
        const wchar_t* szFile)
    {
        if (!szFile || !image.pixels)
            return E_INVALIDARG;

        // Create DDS header
        DDS_HEADER header = {};
        header.dwSize = sizeof(DDS_HEADER);
        header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
        header.dwWidth = metadata.width;
        header.dwHeight = metadata.height;
        header.dwPitchOrLinearSize = image.rowPitch;
        header.dwDepth = metadata.depth;
        header.dwMipMapCount = metadata.mipLevels;
        header.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
        header.ddspf.dwFlags = 0x40; // DDPF_FOURCC

        // Set FourCC based on format
        switch (metadata.format)
        {
        case DXGI_FORMAT_BC1_UNORM:
            header.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '1');
            break;
        case DXGI_FORMAT_BC2_UNORM:
            header.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '3');
            break;
        case DXGI_FORMAT_BC3_UNORM:
            header.ddspf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');
            break;
        default:
            header.ddspf.dwFlags = 0x1 | 0x40; // DDPF_RGB | DDPF_ALPHAPIXELS
            header.ddspf.dwRGBBitCount = 32;
            header.ddspf.dwRBitMask = 0xFF000000;
            header.ddspf.dwGBitMask = 0x00FF0000;
            header.ddspf.dwBBitMask = 0x0000FF00;
            header.ddspf.dwABitMask = 0x000000FF;
            break;
        }

        header.dwCaps = 0x1000; // DDSCAPS_TEXTURE

        // Open file for writing
        HANDLE hFile = CreateFileW(szFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());

        // Write DDS magic
        uint32_t dwMagic = 0x20534444; // "DDS "
        DWORD bytesWritten = 0;
        if (!WriteFile(hFile, &dwMagic, sizeof(dwMagic), &bytesWritten, nullptr) || bytesWritten != sizeof(dwMagic))
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Write header
        if (!WriteFile(hFile, &header, sizeof(header), &bytesWritten, nullptr) || bytesWritten != sizeof(header))
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Write pixel data
        if (!WriteFile(hFile, image.pixels, image.slicePitch, &bytesWritten, nullptr) || bytesWritten != image.slicePitch)
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        CloseHandle(hFile);
        return S_OK;
    }

    HRESULT SaveToWICFile(
        const Image& image,
        WIC_FLAGS flags,
        REFGUID containerFormat,
        const wchar_t* szFile)
    {
        if (!szFile || !image.pixels)
            return E_INVALIDARG;

        HRESULT hr = S_OK;

        // Create WIC factory
        ComPtr<IWICImagingFactory> factory;
        hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
            return hr;

        // Create encoder
        ComPtr<IWICBitmapEncoder> encoder;
        hr = factory->CreateEncoder(containerFormat, nullptr, &encoder);
        if (FAILED(hr))
            return hr;

        // Create stream
        ComPtr<IWICStream> stream;
        hr = factory->CreateStream(&stream);
        if (FAILED(hr))
            return hr;

        hr = stream->InitializeFromFilename(szFile, GENERIC_WRITE);
        if (FAILED(hr))
            return hr;

        hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
        if (FAILED(hr))
            return hr;

        // Create frame
        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> props;
        hr = encoder->CreateNewFrame(&frame, &props);
        if (FAILED(hr))
            return hr;

        hr = frame->Initialize(props.Get());
        if (FAILED(hr))
            return hr;

        // Set frame size
        hr = frame->SetSize(image.width, image.height);
        if (FAILED(hr))
            return hr;

        // Set pixel format
        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        hr = frame->SetPixelFormat(&format);
        if (FAILED(hr))
            return hr;

        // Write pixels
        hr = frame->WritePixels(image.height, image.rowPitch, image.slicePitch, const_cast<uint8_t*>(image.pixels));
        if (FAILED(hr))
            return hr;

        hr = frame->Commit();
        if (FAILED(hr))
            return hr;

        hr = encoder->Commit();
        if (FAILED(hr))
            return hr;

        return S_OK;
    }

    // ========== Image Processing Functions ==========

    HRESULT Convert(
        const Image* srcImages,
        size_t nimages,
        const TexMetadata& metadata,
        DXGI_FORMAT format,
        ScratchImage& result,
        float threshold)
    {
        if (!srcImages || nimages == 0)
            return E_INVALIDARG;

        TexMetadata mdata = metadata;
        mdata.format = format;
        mdata.deviceFormat = format;

        HRESULT hr = result.Initialize(mdata);
        if (FAILED(hr))
            return hr;

        // Simple copy for now - proper conversion would require format-specific logic
        for (size_t i = 0; i < nimages; ++i)
        {
            const Image* src = &srcImages[i];
            const Image* dst = result.GetImage(static_cast<size_t>(i), 0, 0);

            if (!dst)
                return E_FAIL;

            size_t copySize = std::min<size_t>(src->slicePitch, dst->slicePitch);
            memcpy(dst->pixels, src->pixels, copySize);
        }

        return S_OK;
    }

    HRESULT Resize(
        const Image& srcImage,
        size_t width,
        size_t height,
        DXGI_FORMAT format,
        ScratchImage& result)
    {
        if (!srcImage.pixels || width == 0 || height == 0)
            return E_INVALIDARG;

        HRESULT hr = result.Initialize2D(format, width, height, 1, 1);
        if (FAILED(hr))
            return hr;

        const Image* dst = result.GetImage(0, 0, 0);
        if (!dst)
            return E_FAIL;

        // Simple nearest-neighbor resize
        float xScale = static_cast<float>(srcImage.width) / width;
        float yScale = static_cast<float>(srcImage.height) / height;

        uint32_t bytesPerPixel = 4; // Assuming RGBA8

        for (uint32_t y = 0; y < height; ++y)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                uint32_t srcX = static_cast<uint32_t>(x * xScale);
                uint32_t srcY = static_cast<uint32_t>(y * yScale);

                srcX = std::min(srcX, srcImage.width - 1);
                srcY = std::min(srcY, srcImage.height - 1);

                const uint8_t* srcPixel = srcImage.pixels + (srcY * srcImage.rowPitch + srcX * bytesPerPixel);
                uint8_t* dstPixel = dst->pixels + (y * dst->rowPitch + x * bytesPerPixel);

                memcpy(dstPixel, srcPixel, bytesPerPixel);
            }
        }

        return S_OK;
    }

    HRESULT GenerateMipMaps(
        const Image* srcImages,
        size_t nimages,
        const TexMetadata& metadata,
        DXGI_FORMAT format,
        size_t maxMipLevels,
        ScratchImage& mipChain)
    {
        if (!srcImages || nimages == 0)
            return E_INVALIDARG;

        size_t mipLevels = CountMips(metadata.width, metadata.height);
        if (maxMipLevels > 0 && maxMipLevels < mipLevels)
            mipLevels = maxMipLevels;

        TexMetadata mdata = metadata;
        mdata.mipLevels = static_cast<uint32_t>(mipLevels);
        mdata.format = format;
        mdata.deviceFormat = format;

        HRESULT hr = mipChain.Initialize(mdata);
        if (FAILED(hr))
            return hr;

        // Copy base mip
        const Image* dstBase = mipChain.GetImage(0, 0, 0);
        if (!dstBase)
            return E_FAIL;

        size_t copySize = std::min<size_t>(srcImages[0].slicePitch, dstBase->slicePitch);
        memcpy(dstBase->pixels, srcImages[0].pixels, copySize);

        // Generate mips using simple box filter
        for (size_t mip = 1; mip < mipLevels; ++mip)
        {
            const Image* src = mipChain.GetImage(mip - 1, 0, 0);
            Image* dst = const_cast<Image*>(mipChain.GetImage(mip, 0, 0));

            if (!src || !dst)
                return E_FAIL;

            uint32_t bytesPerPixel = 4;

            for (uint32_t y = 0; y < dst->height; ++y)
            {
                for (uint32_t x = 0; x < dst->width; ++x)
                {
                    // Sample 2x2 region from source
                    uint32_t srcX = x * 2;
                    uint32_t srcY = y * 2;

                    uint8_t accum[4] = {0, 0, 0, 0};
                    uint32_t count = 0;

                    for (uint32_t sy = 0; sy < 2 && srcY + sy < src->height; ++sy)
                    {
                        for (uint32_t sx = 0; sx < 2 && srcX + sx < src->width; ++sx)
                        {
                            const uint8_t* srcPixel = src->pixels + ((srcY + sy) * src->rowPitch + (srcX + sx) * bytesPerPixel);
                            accum[0] += srcPixel[0];
                            accum[1] += srcPixel[1];
                            accum[2] += srcPixel[2];
                            accum[3] += srcPixel[3];
                            ++count;
                        }
                    }

                    uint8_t* dstPixel = dst->pixels + (y * dst->rowPitch + x * bytesPerPixel);
                    dstPixel[0] = static_cast<uint8_t>(accum[0] / count);
                    dstPixel[1] = static_cast<uint8_t>(accum[1] / count);
                    dstPixel[2] = static_cast<uint8_t>(accum[2] / count);
                    dstPixel[3] = static_cast<uint8_t>(accum[3] / count);
                }
            }
        }

        return S_OK;
    }

} // namespace DirectX
