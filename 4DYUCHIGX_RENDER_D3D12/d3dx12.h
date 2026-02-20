// ============================================
// d3dx12.h
// DirectX 12 辅助工具类和函数
// 简化版本，仅包含本项目所需功能
// ============================================

#pragma once

#ifndef D3DX12_H
#define D3DX12_H

#include <d3d12.h>
#include <string.h>

// --------------------------------------------------------------------------
// 辅助宏
// --------------------------------------------------------------------------

// CD3DX12_DEFAULT 标记类型
struct CD3DX12_DEFAULT {};
extern const CD3DX12_DEFAULT D3D12_DEFAULT;
#ifndef D3D12_DEFAULT_BARRIER_API
#define D3D12_DEFAULT_BARRIER_API
#endif

// --------------------------------------------------------------------------
// CD3DX12_SHADER_BYTECODE - 着色器字节码辅助结构
// --------------------------------------------------------------------------
struct CD3DX12_SHADER_BYTECODE : public D3D12_SHADER_BYTECODE
{
    CD3DX12_SHADER_BYTECODE() = default;
    explicit CD3DX12_SHADER_BYTECODE(const D3D12_SHADER_BYTECODE& o) :
        D3D12_SHADER_BYTECODE(o)
    {}

    CD3DX12_SHADER_BYTECODE(ID3DBlob* pShaderBytecode)
    {
        pShaderBytecode = pShaderBytecode;
        BytecodeLength = pShaderBytecode ? pShaderBytecode->GetBufferSize() : 0;
    }

    CD3DX12_SHADER_BYTECODE(void* _pShaderBytecode, SIZE_T _BytecodeLength)
    {
        pShaderBytecode = _pShaderBytecode;
        BytecodeLength = _BytecodeLength;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_HEAP_PROPERTIES - 堆属性辅助类
// --------------------------------------------------------------------------
struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
    CD3DX12_HEAP_PROPERTIES()
    {}

    explicit CD3DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES& o) :
        D3D12_HEAP_PROPERTIES(o)
    {}

    CD3DX12_HEAP_PROPERTIES(
        D3D12_HEAP_TYPE type,
        D3D12_CPU_PAGE_PROPERTY cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL memoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        UINT creationNodeMask = 1,
        UINT visibleNodeMask = 1)
    {
        Type = type;
        CPUPageProperty = cpuPageProperty;
        MemoryPoolPreference = memoryPoolPreference;
        CreationNodeMask = creationNodeMask;
        VisibleNodeMask = visibleNodeMask;
    }

    inline bool IsCPUAccessible() const
    {
        return Type == D3D12_HEAP_TYPE_UPLOAD || Type == D3D12_HEAP_TYPE_READBACK;
    }

    static inline CD3DX12_HEAP_PROPERTIES CreateUploadHeap(UINT creationNodeMask = 1, UINT visibleNodeMask = 1)
    {
        return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, creationNodeMask, visibleNodeMask);
    }

    static inline CD3DX12_HEAP_PROPERTIES CreateDefaultHeap(UINT creationNodeMask = 1, UINT visibleNodeMask = 1)
    {
        return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, creationNodeMask, visibleNodeMask);
    }

    static inline CD3DX12_HEAP_PROPERTIES CreateReadbackHeap(UINT creationNodeMask = 1, UINT visibleNodeMask = 1)
    {
        return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, creationNodeMask, visibleNodeMask);
    }
};

// --------------------------------------------------------------------------
// CD3DX12_RESOURCE_DESC - 资源描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
    CD3DX12_RESOURCE_DESC()
    {}

    explicit CD3DX12_RESOURCE_DESC(const D3D12_RESOURCE_DESC& o) :
        D3D12_RESOURCE_DESC(o)
    {}

    CD3DX12_RESOURCE_DESC(
        D3D12_RESOURCE_DIMENSION dimension,
        UINT64 alignment,
        UINT64 width,
        UINT height,
        UINT16 depthOrArraySize,
        UINT16 mipLevels,
        DXGI_FORMAT format,
        DXGI_SAMPLE_DESC sampleDesc,
        D3D12_TEXTURE_LAYOUT layout,
        D3D12_RESOURCE_FLAGS flags)
    {
        Dimension = dimension;
        Alignment = alignment;
        Width = width;
        Height = height;
        DepthOrArraySize = depthOrArraySize;
        MipLevels = mipLevels;
        Format = format;
        SampleDesc = sampleDesc;
        Layout = layout;
        Flags = flags;
    }

    static inline CD3DX12_RESOURCE_DESC Buffer(
        UINT64 width,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        UINT64 alignment = 0)
    {
        return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
    }

    static inline CD3DX12_RESOURCE_DESC Tex2D(
        DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        UINT16 sampleCount = 1,
        UINT16 sampleQuality = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0)
    {
        return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, arraySize, mipLevels, format, {sampleCount, sampleQuality}, layout, flags);
    }
};

// --------------------------------------------------------------------------
// CD3DX12_DESCRIPTOR_RANGE - 描述符范围辅助类
// --------------------------------------------------------------------------
struct CD3DX12_DESCRIPTOR_RANGE : public D3D12_DESCRIPTOR_RANGE
{
    CD3DX12_DESCRIPTOR_RANGE()
    {}

    explicit CD3DX12_DESCRIPTOR_RANGE(const D3D12_DESCRIPTOR_RANGE& o) :
        D3D12_DESCRIPTOR_RANGE(o)
    {}

    CD3DX12_DESCRIPTOR_RANGE(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
    {
        RangeType = rangeType;
        NumDescriptors = numDescriptors;
        BaseShaderRegister = baseShaderRegister;
        RegisterSpace = registerSpace;
        OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_DESCRIPTOR_RANGE1 - 描述符范围辅助类（版本1）
// --------------------------------------------------------------------------
struct CD3DX12_DESCRIPTOR_RANGE1 : public D3D12_DESCRIPTOR_RANGE1
{
    CD3DX12_DESCRIPTOR_RANGE1()
    {}

    explicit CD3DX12_DESCRIPTOR_RANGE1(const D3D12_DESCRIPTOR_RANGE1& o) :
        D3D12_DESCRIPTOR_RANGE1(o)
    {}

    CD3DX12_DESCRIPTOR_RANGE1(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
    {
        RangeType = rangeType;
        NumDescriptors = numDescriptors;
        BaseShaderRegister = baseShaderRegister;
        RegisterSpace = registerSpace;
        Flags = flags;
        OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_ROOT_PARAMETER - 根参数辅助类
// --------------------------------------------------------------------------
struct CD3DX12_ROOT_PARAMETER : public D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER()
    {}

    explicit CD3DX12_ROOT_PARAMETER(const D3D12_ROOT_PARAMETER& o) :
        D3D12_ROOT_PARAMETER(o)
    {}

    static inline void InitAsDescriptorTable(
        _Out_ CD3DX12_ROOT_PARAMETER& rootParam,
        UINT numDescriptorRanges,
        _In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam.ShaderVisibility = visibility;
        rootParam.DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
        rootParam.DescriptorTable.pDescriptorRanges = pDescriptorRanges;
    }

    static inline void InitAsConstants(
        _Out_ CD3DX12_ROOT_PARAMETER& rootParam,
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParam.ShaderVisibility = visibility;
        rootParam.Constants.Num32BitValues = num32BitValues;
        rootParam.Constants.ShaderRegister = shaderRegister;
        rootParam.Constants.RegisterSpace = registerSpace;
    }

    static inline void InitAsCBV(
        _Out_ CD3DX12_ROOT_PARAMETER& rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParam.ShaderVisibility = visibility;
        rootParam.Descriptor.ShaderRegister = shaderRegister;
        rootParam.Descriptor.RegisterSpace = registerSpace;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_ROOT_PARAMETER1 - 根参数辅助类（版本1）
// --------------------------------------------------------------------------
struct CD3DX12_ROOT_PARAMETER1 : public D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1()
    {}

    explicit CD3DX12_ROOT_PARAMETER1(const D3D12_ROOT_PARAMETER1& o) :
        D3D12_ROOT_PARAMETER1(o)
    {}

    static inline void InitAsDescriptorTable(
        _Out_ CD3DX12_ROOT_PARAMETER1& rootParam,
        UINT numDescriptorRanges,
        _In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam.ShaderVisibility = visibility;
        rootParam.DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
        rootParam.DescriptorTable.pDescriptorRanges = pDescriptorRanges;
    }

    static inline void InitAsConstants(
        _Out_ CD3DX12_ROOT_PARAMETER1& rootParam,
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParam.ShaderVisibility = visibility;
        rootParam.Constants.Num32BitValues = num32BitValues;
        rootParam.Constants.ShaderRegister = shaderRegister;
        rootParam.Constants.RegisterSpace = registerSpace;
    }

    static inline void InitAsCBV(
        _Out_ CD3DX12_ROOT_PARAMETER1& rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParam.ShaderVisibility = visibility;
        rootParam.Descriptor.ShaderRegister = shaderRegister;
        rootParam.Descriptor.RegisterSpace = registerSpace;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_ROOT_SIGNATURE_DESC - 根签名描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
{
    CD3DX12_ROOT_SIGNATURE_DESC()
    {}

    explicit CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o) :
        D3D12_ROOT_SIGNATURE_DESC(o)
    {}

    CD3DX12_ROOT_SIGNATURE_DESC(
        UINT numParameters,
        _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
        UINT numStaticSamplers = 0,
        _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
    {
        NumParameters = numParameters;
        pParameters = _pParameters;
        NumStaticSamplers = numStaticSamplers;
        pStaticSamplers = _pStaticSamplers;
        Flags = flags;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_ROOT_SIGNATURE_DESC1 - 根签名描述辅助类（版本1）
// --------------------------------------------------------------------------
struct CD3DX12_ROOT_SIGNATURE_DESC1 : public D3D12_ROOT_SIGNATURE_DESC1
{
    CD3DX12_ROOT_SIGNATURE_DESC1()
    {}

    explicit CD3DX12_ROOT_SIGNATURE_DESC1(const D3D12_ROOT_SIGNATURE_DESC1& o) :
        D3D12_ROOT_SIGNATURE_DESC1(o)
    {}

    CD3DX12_ROOT_SIGNATURE_DESC1(
        UINT numParameters,
        _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
        UINT numStaticSamplers = 0,
        _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
    {
        NumParameters = numParameters;
        pParameters = _pParameters;
        NumStaticSamplers = numStaticSamplers;
        pStaticSamplers = _pStaticSamplers;
        Flags = flags;
    }
};

// --------------------------------------------------------------------------
// 辅助函数
// --------------------------------------------------------------------------
inline HRESULT D3DX12SerializeVersionedRootSignature(
    _In_ const D3D12_ROOT_SIGNATURE_DESC1* pRootSignatureDesc,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
    // 将 D3D12_ROOT_SIGNATURE_DESC1 转换为 D3D12_VERSIONED_ROOT_SIGNATURE_DESC
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc = {};
    versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedDesc.Desc_1_1 = *pRootSignatureDesc;

    return D3D12SerializeVersionedRootSignature(
        &versionedDesc,
        ppBlob,
        ppErrorBlob);
}

// --------------------------------------------------------------------------
// D3D12_ROOT_SIGNATURE_VERSION 适配
// --------------------------------------------------------------------------
#ifndef D3D12_ROOT_SIGNATURE_VERSION_1_0
#define D3D12_ROOT_SIGNATURE_VERSION_1_0 D3D_ROOT_SIGNATURE_VERSION_1_0
#define D3D12_ROOT_SIGNATURE_VERSION_1 D3D_ROOT_SIGNATURE_VERSION_1_0
#endif

#ifndef D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_DESCRIPTOR_LAYOUT
#define D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_DESCRIPTOR_LAYOUT D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
#endif

#ifndef D3D12_ROOT_SIGNATURE_REGISTER_SPACE
#define D3D12_ROOT_SIGNATURE_REGISTER_SPACE 0
#endif

// --------------------------------------------------------------------------
// CD3DX12_RESOURCE_BARRIER - 资源屏障辅助类
// --------------------------------------------------------------------------
struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
    CD3DX12_RESOURCE_BARRIER()
    {}

    explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER& o) :
        D3D12_RESOURCE_BARRIER(o)
    {}

    static inline CD3DX12_RESOURCE_BARRIER Transition(
        ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
    {
        D3D12_RESOURCE_BARRIER bar;
        bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        bar.Transition.pResource = pResource;
        bar.Transition.StateBefore = stateBefore;
        bar.Transition.StateAfter = stateAfter;
        bar.Transition.Subresource = subresource;
        bar.Flags = flags;
        return CD3DX12_RESOURCE_BARRIER(bar);
    }

    static inline CD3DX12_RESOURCE_BARRIER Aliasing(
        ID3D12Resource* pResourceBefore,
        ID3D12Resource* pResourceAfter)
    {
        D3D12_RESOURCE_BARRIER bar;
        bar.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        bar.Aliasing.pResourceBefore = pResourceBefore;
        bar.Aliasing.pResourceAfter = pResourceAfter;
        return CD3DX12_RESOURCE_BARRIER(bar);
    }

    static inline CD3DX12_RESOURCE_BARRIER UAV(
        ID3D12Resource* pResource)
    {
        D3D12_RESOURCE_BARRIER bar;
        bar.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        bar.UAV.pResource = pResource;
        return CD3DX12_RESOURCE_BARRIER(bar);
    }
};

// --------------------------------------------------------------------------
// CD3DX12_DEPTH_STENCIL_VIEW_DESC - 深度模板视图描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_DEPTH_STENCIL_VIEW_DESC : public D3D12_DEPTH_STENCIL_VIEW_DESC
{
    CD3DX12_DEPTH_STENCIL_VIEW_DESC()
    {}

    explicit CD3DX12_DEPTH_STENCIL_VIEW_DESC(const D3D12_DEPTH_STENCIL_VIEW_DESC& o) :
        D3D12_DEPTH_STENCIL_VIEW_DESC(o)
    {}

    CD3DX12_DEPTH_STENCIL_VIEW_DESC(
        D3D12_DSV_DIMENSION viewDimension,
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
        D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE)
    {
        Format = format;
        ViewDimension = viewDimension;
        Flags = flags;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_SHADER_RESOURCE_VIEW_DESC - 着色器资源视图描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_SHADER_RESOURCE_VIEW_DESC : public D3D12_SHADER_RESOURCE_VIEW_DESC
{
    CD3DX12_SHADER_RESOURCE_VIEW_DESC()
    {}

    explicit CD3DX12_SHADER_RESOURCE_VIEW_DESC(const D3D12_SHADER_RESOURCE_VIEW_DESC& o) :
        D3D12_SHADER_RESOURCE_VIEW_DESC(o)
    {}

    CD3DX12_SHADER_RESOURCE_VIEW_DESC(
        D3D12_SRV_DIMENSION viewDimension,
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
        UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
    {
        Format = format;
        ViewDimension = viewDimension;
        Shader4ComponentMapping = shader4ComponentMapping;
    }

    static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC Tex2D(
        DXGI_FORMAT format,
        UINT mostDetailedMip = 0,
        UINT mipLevels = -1,
        UINT planeSlice = 0,
        FLOAT resourceMinLODClamp = 0.0f)
    {
        CD3DX12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = format;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Texture2D.MostDetailedMip = mostDetailedMip;
        desc.Texture2D.MipLevels = mipLevels;
        desc.Texture2D.PlaneSlice = planeSlice;
        desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;
        return desc;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_CPU_DESCRIPTOR_HANDLE - CPU描述符句柄辅助类
// --------------------------------------------------------------------------
struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE() = default;
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o) :
        D3D12_CPU_DESCRIPTOR_HANDLE(o)
    {}
    CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }

    inline void Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        ptr += SIZE_T(INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
    }

    inline CD3DX12_CPU_DESCRIPTOR_HANDLE Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) const
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE ret = *this;
        ret.Offset(offsetInDescriptors, descriptorIncrementSize);
        return ret;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_GPU_DESCRIPTOR_HANDLE - GPU描述符句柄辅助类
// --------------------------------------------------------------------------
struct CD3DX12_GPU_DESCRIPTOR_HANDLE : public D3D12_GPU_DESCRIPTOR_HANDLE
{
    CD3DX12_GPU_DESCRIPTOR_HANDLE() = default;
    explicit CD3DX12_GPU_DESCRIPTOR_HANDLE(const D3D12_GPU_DESCRIPTOR_HANDLE& o) :
        D3D12_GPU_DESCRIPTOR_HANDLE(o)
    {}
    CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }

    inline void Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        ptr += UINT64(INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
    }

    inline CD3DX12_GPU_DESCRIPTOR_HANDLE Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) const
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE ret = *this;
        ret.Offset(offsetInDescriptors, descriptorIncrementSize);
        return ret;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_BLEND_DESC - 混合状态描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC
{
    CD3DX12_BLEND_DESC()
    {}

    explicit CD3DX12_BLEND_DESC(const D3D12_BLEND_DESC& o) :
        D3D12_BLEND_DESC(o)
    {}

    inline static CD3DX12_BLEND_DESC DEFAULT()
    {
        CD3DX12_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            TRUE, // BlendEnable
            FALSE, // LogicOpEnable
            D3D12_BLEND_SRC_ALPHA, // SrcBlend
            D3D12_BLEND_INV_SRC_ALPHA, // DestBlend
            D3D12_BLEND_OP_ADD, // BlendOp
            D3D12_BLEND_ONE, // SrcBlendAlpha
            D3D12_BLEND_ZERO, // DestBlendAlpha
            D3D12_BLEND_OP_ADD, // BlendOpAlpha
            D3D12_LOGIC_OP_NOOP, // LogicOp
            D3D12_COLOR_WRITE_ENABLE_ALL, // RenderTargetWriteMask
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            desc.RenderTarget[i] = defaultRenderTargetBlendDesc;
        return desc;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_RASTERIZER_DESC - 光栅化状态描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
{
    CD3DX12_RASTERIZER_DESC()
    {}

    explicit CD3DX12_RASTERIZER_DESC(const D3D12_RASTERIZER_DESC& o) :
        D3D12_RASTERIZER_DESC(o)
    {}

    inline static CD3DX12_RASTERIZER_DESC DEFAULT()
    {
        CD3DX12_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D12_FILL_MODE_SOLID;
        desc.CullMode = D3D12_CULL_MODE_BACK;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.DepthClipEnable = TRUE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;
        desc.ForcedSampleCount = 0;
        desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        return desc;
    }
};

// --------------------------------------------------------------------------
// CD3DX12_STATIC_SAMPLER_DESC - 静态采样器描述辅助类
// --------------------------------------------------------------------------
struct CD3DX12_STATIC_SAMPLER_DESC : public D3D12_STATIC_SAMPLER_DESC
{
    CD3DX12_STATIC_SAMPLER_DESC() = default;
    explicit CD3DX12_STATIC_SAMPLER_DESC(const D3D12_STATIC_SAMPLER_DESC& o) :
        D3D12_STATIC_SAMPLER_DESC(o)
    {}

    CD3DX12_STATIC_SAMPLER_DESC(
        D3D12_FILTER filter,
        D3D12_TEXTURE_ADDRESS_MODE addressU,
        D3D12_TEXTURE_ADDRESS_MODE addressV,
        D3D12_TEXTURE_ADDRESS_MODE addressW,
        FLOAT mipLODBias,
        UINT maxAnisotropy,
        D3D12_COMPARISON_FUNC comparisonFunc,
        D3D12_STATIC_BORDER_COLOR borderColor,
        FLOAT minLOD,
        FLOAT maxLOD,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        Filter = filter;
        AddressU = addressU;
        AddressV = addressV;
        AddressW = addressW;
        MipLODBias = mipLODBias;
        MaxAnisotropy = maxAnisotropy;
        ComparisonFunc = comparisonFunc;
        BorderColor = borderColor;
        MinLOD = minLOD;
        MaxLOD = maxLOD;
        ShaderRegister = shaderRegister;
        RegisterSpace = registerSpace;
        ShaderVisibility = shaderVisibility;
    }
};

#endif // D3DX12_H
