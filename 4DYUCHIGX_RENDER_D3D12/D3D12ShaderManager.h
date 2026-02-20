#pragma once

// 必须首先包含预编译头
#include "D3D12PCH.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <d3dcompiler.h>
#include <vector>
#include "D3D12Fixes.h"
#include "d3dx12.h"
#include "D3D12RootSignature.h"
#include "D3D12PipelineState.h"

// 前向声明
class D3D12PipelineState;

// 着色器编译器
class D3D12ShaderCompiler
{
public:
    // 编译 HLSL 着色器文件
    static HRESULT CompileFromFile(
        const wchar_t* filename,
        const char* entryPoint,
        const char* target,
        ID3DBlob** blob
    );

    static HRESULT CompileFromString(
        const char* source,
        SIZE_T sourceSize,
        const char* entryPoint,
        const char* target,
        ID3DBlob** blob
    );

    // 常用着色器编译配置
    static HRESULT CompileVertexShader(
        const wchar_t* filename,
        ID3DBlob** blob
    );

    static HRESULT CompilePixelShader(
        const wchar_t* filename,
        ID3DBlob** blob
    );

private:
    static HRESULT CompileInternal(
        const void* sourceCode,
        SIZE_T sourceSize,
        const char* entryPoint,
        const char* target,
        ID3DBlob** blob
    );
};

// D3D12PipelineState 类已移至独立的 D3D12PipelineState.h/cpp 文件
// 此处保留前向声明

// 输入布局管理
class D3D12InputLayout
{
public:
    // 获取标准顶点格式输入布局
    static D3D12_INPUT_LAYOUT_DESC GetPositionLayout();

    static D3D12_INPUT_LAYOUT_DESC GetPositionNormalLayout();

    static D3D12_INPUT_LAYOUT_DESC GetPositionNormalTextureLayout();

    static D3D12_INPUT_LAYOUT_DESC GetPositionColorLayout();

    static D3D12_INPUT_LAYOUT_DESC GetPositionNormalTextureColorLayout();

private:
    static const D3D12_INPUT_ELEMENT_DESC s_positionLayout[];
    static const D3D12_INPUT_ELEMENT_DESC s_positionNormalLayout[];
    static const D3D12_INPUT_ELEMENT_DESC s_positionNormalTextureLayout[];
    static const D3D12_INPUT_ELEMENT_DESC s_positionColorLayout[];
    static const D3D12_INPUT_ELEMENT_DESC s_positionNormalTextureColorLayout[];
};

// 常用的辅助函数
namespace D3D12Utils
{
    // 上传数据到 GPU
    inline HRESULT UploadDataToGPU(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* destination,
        const void* data,
        UINT64 size
    );

    // 创建资源屏障
    inline void TransitionResource(
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
    );

    // 设置资源屏障数组
    inline void ResourceBarrier(
        ID3D12GraphicsCommandList* commandList,
        D3D12_RESOURCE_BARRIER* barriers,
        UINT numBarriers
    );
}
