// ============================================
// DirectXTex.h - DirectX Texture Library
// Complete implementation for D3D12
// ============================================

#pragma once

#ifndef DIRECTX_TEX_H
#define DIRECTX_TEX_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <vector>
#include <memory>

namespace DirectX
{

    // ========== Texture Loading Flags ==========

    enum DDS_FLAGS : uint32_t
    {
        DDS_FLAGS_NONE = 0x0,
        DDS_FLAGS_FORCE_RGB = 0x1,
        DDS_FLAGS_NO_16BPP = 0x2,
        DDS_FLAGS_ALLOW_LARGE_FILES = 0x4,
        DDS_FLAGS_IGNORE_MIPS = 0x8,
        DDS_FLAGS_FORCE_WIC_RESERVED = 0x10000,
    };

    enum TGA_FLAGS : uint32_t
    {
        TGA_FLAGS_NONE = 0x0,
        TGA_FLAGS_FORCE_RGB = 0x1,
        TGA_FLAGS_NO_16BPP = 0x2,
    };

    enum WIC_FLAGS : uint32_t
    {
        WIC_FLAGS_NONE = 0x0,
        WIC_FLAGS_FORCE_RGB = 0x1,
        WIC_FLAGS_NO_16BPP = 0x2,
        WIC_FLAGS_ALLOW_LARGE_FILES = 0x4,
        WIC_FLAGS_IGNORE_SRGB = 0x8,
        WIC_FLAGS_FORCE_SRGB = 0x10,
        WIC_FLAGS_ALL_FRAMES = 0x20,
        WIC_FLAGS_DITHER = 0x10000,
        WIC_FLAGS_FILTER_POINT = 0x100000,
        WIC_FLAGS_FILTER_LINEAR = 0x200000,
        WIC_FLAGS_FILTER_CUBIC = 0x300000,
        WIC_FLAGS_FILTER_FANT = 0x400000,
    };

    // ========== Texture Metadata ==========

    enum CPLM : uint32_t
    {
        CPLM_NONE = 0,
        CPLM_USAGE_DEFAULT = 0x1,
        CPLM_USAGE_GPU_READ = 0x2,
    };

    enum TEX_MISC_FLAGS : uint32_t
    {
        TEX_MISC_MISCFLAG_UNUSED = 0x1,
        TEX_MISC_TEXTURECUBE = 0x4,
        TEX_MISC_ALLOW_COMPLEX = 0x8,
    };

    enum TEX_MISC_FLAG2 : uint32_t
    {
        TEX_MISC_FLAG2_UNUSED = 0x1,
        TEX_MISC_FLAG2_ALPHA_MODE_MASK = 0x6,
        TEX_MISC_FLAG2_UNKNOWN_MODE = 0x0,
        TEX_MISC_FLAG2_PREMULTIPLIED = 0x2,
        TEX_MISC_FLAG2_STRAIGHT = 0x4,
    };

    enum TEX_ALPHA_MODE : uint32_t
    {
        TEX_ALPHA_MODE_UNKNOWN = 0,
        TEX_ALPHA_MODE_STRAIGHT = 1,
        TEX_ALPHA_MODE_PREMULTIPLIED = 2,
        TEX_ALPHA_MODE_OPAQUE = 3,
        TEX_ALPHA_MODE_CUSTOM = 4,
    };

    struct TexMetadata
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;           // Depth for 3D textures, or array size for texture arrays
        uint32_t arraySize;       // Array size for texture arrays/cube maps
        uint32_t mipLevels;
        uint32_t miscFlags;
        uint32_t miscFlags2;
        DXGI_FORMAT format;
        DXGI_FORMAT deviceFormat; // Format to use for device operations
        CPLM cpmFlags;
        TEX_ALPHA_MODE alphaMode;
        bool IsCubeMap;
        bool IsVolumeMap;
        bool IsPMAlpha;           // Is the image using premultiplied alpha

        TexMetadata() noexcept
            : width(0)
            , height(0)
            , depth(1)
            , arraySize(1)
            , mipLevels(0)
            , miscFlags(0)
            , miscFlags2(0)
            , format(DXGI_FORMAT_UNKNOWN)
            , deviceFormat(DXGI_FORMAT_UNKNOWN)
            , cpmFlags(CPLM_NONE)
            , alphaMode(TEX_ALPHA_MODE_UNKNOWN)
            , IsCubeMap(false)
            , IsVolumeMap(false)
            , IsPMAlpha(false)
        {}
    };

    // ========== Image Data ==========

    struct Image
    {
        uint8_t* pixels;          // Pointer to image data
        uint32_t width;           // Image width in pixels
        uint32_t height;          // Image height in pixels
        DXGI_FORMAT format;       // Pixel format
        uint32_t rowPitch;        // Row pitch (bytes per row)
        uint32_t slicePitch;      // Slice pitch (total bytes for this image/slice)

        Image() noexcept
            : pixels(nullptr)
            , width(0)
            , height(0)
            , format(DXGI_FORMAT_UNKNOWN)
            , rowPitch(0)
            , slicePitch(0)
        {}
    };

    // ========== Scratch Image ==========

    class ScratchImage
    {
    public:
        ScratchImage() noexcept;
        ~ScratchImage();

        ScratchImage(ScratchImage&& moveFrom) noexcept;
        ScratchImage& operator= (ScratchImage&& moveFrom) noexcept;

        ScratchImage(const ScratchImage&) = delete;
        ScratchImage& operator=(const ScratchImage&) = delete;

        // Get image data
        const Image* GetImages() const noexcept;
        const Image* GetImage(size_t mip = 0, size_t item = 0, size_t slice = 0) const noexcept;
        const TexMetadata* GetMetadata() const noexcept;

        uint8_t* GetPixels() noexcept;
        const uint8_t* GetPixels() const noexcept;

        size_t GetImageCount() const noexcept;

        // Image manipulation
        HRESULT Initialize(const TexMetadata& mmetadata);
        HRESULT Initialize1D(DXGI_FORMAT fmt, size_t length, size_t arraySize, size_t mipLevels);
        HRESULT Initialize2D(DXGI_FORMAT fmt, size_t width, size_t height, size_t arraySize, size_t mipLevels);
        HRESULT Initialize3D(DXGI_FORMAT fmt, size_t width, size_t height, size_t depth, size_t mipLevels);
        HRESULT InitializeFromImage(const Image& srcImage, bool allow1D = false);
        HRESULT InitializeCubeFromImages(const Image* images, size_t nImages);

        void Release() noexcept;

        // Override operators
        bool IsValid() const noexcept { return (_nimages > 0 && _memory != nullptr); }

    private:
        size_t _nimages;           // Number of images in the array
        size_t _size;              // Total size of image data in bytes
        Image* _images;            // Array of image structures
        uint8_t* _memory;          // Allocated image data
        TexMetadata _metadata;     // Metadata for the image

        // Private helper to allocate memory
        HRESULT AllocateImages(const TexMetadata& mmetadata);
    };

    // ========== Load Functions ==========

    // Load from DDS file
    HRESULT LoadFromDDSFile(
        const wchar_t* szFile,
        DDS_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image);

    // Load from TGA file
    HRESULT LoadFromTGAFile(
        const wchar_t* szFile,
        TGA_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image);

    // Load from WIC-supported file (PNG, JPG, BMP, etc.)
    HRESULT LoadFromWICFile(
        const wchar_t* szFile,
        WIC_FLAGS flags,
        TexMetadata* metadata,
        ScratchImage& image);

    // ========== Save Functions ==========

    HRESULT SaveToDDSFile(
        const Image& image,
        const TexMetadata& metadata,
        DDS_FLAGS flags,
        const wchar_t* szFile);

    HRESULT SaveToWICFile(
        const Image& image,
        WIC_FLAGS flags,
        REFGUID containerFormat,
        const wchar_t* szFile);

    // ========== Image Processing Functions ==========

    // Convert image format
    HRESULT Convert(
        const Image* srcImages,
        size_t nimages,
        const TexMetadata& metadata,
        DXGI_FORMAT format,
        ScratchImage& result,
        float threshold = 0.5f);

    // Resize image
    HRESULT Resize(
        const Image& srcImage,
        size_t width,
        size_t height,
        DXGI_FORMAT format,
        ScratchImage& result);

    // Generate mipmaps
    HRESULT GenerateMipMaps(
        const Image* srcImages,
        size_t nimages,
        const TexMetadata& metadata,
        DXGI_FORMAT format,
        size_t maxMipLevels,
        ScratchImage& mipChain);

    // ========== Utilities ==========

    // Compute number of miplevels
    size_t CountMips(uint32_t width, uint32_t height) noexcept;

    // Compute pitch requirements
    bool ComputePitch(DXGI_FORMAT fmt, size_t width, size_t& rowPitch, size_t& slicePitch) noexcept;

} // namespace DirectX

#endif // DIRECTX_TEX_H
