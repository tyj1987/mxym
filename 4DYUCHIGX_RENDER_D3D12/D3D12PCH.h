// ============================================
// D3D12PCH.h
// DirectX 12 预编译头文件
// 统一管理所有必要的头文件包含
// ============================================

#pragma once

// ========== Windows平台 ==========
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

// ========== DirectX 12 核心配置 ==========
// DirectX Math 配置（必须在包含DirectXMath.h之前定义）
#define XM_NO_XMVECTOR_OVERLOADS 1
#define XM_NO_ALIGNMENT

// ========== DirectX 12 核心 ==========
#include <d3d12.h>
#include <dxgi1_6.h>

// D3DCompiler版本定义（必须在包含d3dcompiler.h之前）
#ifndef D3D_COMPILER_VERSION
#define D3D_COMPILER_VERSION 47
#endif
#include <d3dcompiler.h>

#include "d3dx12.h"

// ========== DirectX Math ==========
#include <DirectXMath.h>
#include <DirectXColors.h>

// ========== Windows Runtime ==========
#include <wrl/client.h>

// ========== 标准库 ==========
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <array>
#include <queue>

// DirectX命名空间（简化类型访问）
namespace DX = DirectX;

// ========== 类型别名 ==========
using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMFLOAT4X4 = DirectX::XMFLOAT4X4;
using XMMATRIX = DirectX::XMMATRIX;
using XMVECTOR = DirectX::XMVECTOR;

// ========== 辅助宏 ==========

// MAKEFOURCC - 创建四字符代码
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
    ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif

// D3DCompile 标志
#ifndef D3DCOMPILE_OPTIMIZE
#define D3DCOMPILE_OPTIMIZE                   0x0001
#define D3DCOMPILE_OPTIMIZE_IEEE_IEEE         0x0002
#define D3DCOMPILE_OPTIMIZE_IEEE_DENORMAL     0x0004
#endif

#define SAFE_RELEASE(p) \
    if ((p) != nullptr) { \
        (p)->Release(); \
        (p) = nullptr; \
    }

#define SAFE_DELETE(p) \
    if ((p) != nullptr) { \
        delete (p); \
        (p) = nullptr; \
    }

#define SAFE_DELETE_ARRAY(p) \
    if ((p) != nullptr) { \
        delete[] (p); \
        (p) = nullptr; \
    }

// ========== 常量 ==========
const UINT D3D12_DEFAULT_VERTEX_BUFFER_SIZE = 1024 * 1024;  // 1MB
const UINT D3D12_DEFAULT_INDEX_BUFFER_SIZE = 256 * 1024;    // 256KB
const UINT D3D12_MAX_TEXTURE_SIZE = 8192;                   // 8K
const UINT D3D12_DEFAULT_MIP_LEVELS = 12;                   // 支持8K纹理

// ========== 调试宏 ==========
#ifdef _DEBUG
    #define DX12_DEBUG_BREAK() __debugbreak()
#else
    #define DX12_DEBUG_BREAK()
#endif

// ========== HRESULT检查宏 ==========
#define DX12_THROW_IF_FAILED(hr) \
    do { \
        HRESULT hr_##__LINE__ = (hr); \
        if (FAILED(hr_##__LINE__)) { \
            DX12_DEBUG_BREAK(); \
            throw std::exception("DirectX 12 error"); \
        } \
    } while (0)

#define DX12_RETURN_IF_FAILED(hr) \
    do { \
        HRESULT hr_##__LINE__ = (hr); \
        if (FAILED(hr_##__LINE__)) { \
            return hr_##__LINE__; \
        } \
    } while (0)

// ========== COM智能指针别名 ==========
// 只在未定义时才定义,避免重复定义
#ifndef D3D12_PCH_COM_PTR_DEFINED
#define D3D12_PCH_COM_PTR_DEFINED
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif

// ========== 前向声明 ==========
class CoD3D12Device;
class D3D12RenderScheduler;
class D3D12MaterialManager;
class D3D12LightManager;
class D3D12SpriteManager;
class D3D12FontManager;
class D3D12MeshManager;
class D3D12ShadowManager;
class D3D12PostProcessManager;
class D3D12TerrainManager;

// ============================================
// 使用说明：
// 在所有 .cpp 文件的第一行包含此文件：
// #include "D3D12PCH.h"
// ============================================
