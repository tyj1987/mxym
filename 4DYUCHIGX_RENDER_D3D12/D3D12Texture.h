#pragma once

// 必须首先包含预编译头
#include "D3D12PCH.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <DirectXMath.h>
#include <vector>
#include <map>



// 纹理类
class D3D12Texture
{
public:
    D3D12Texture();
    virtual ~D3D12Texture();

    // 从文件加载纹理
    HRESULT LoadFromFile(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const wchar_t* filename
    );

    // 从内存创建纹理
    HRESULT CreateFromMemory(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const void* data,
        UINT width,
        UINT height,
        DXGI_FORMAT format,
        UINT mipLevels = 0
    );

    // 创建渲染目标纹理
    HRESULT CreateRenderTarget(
        ID3D12Device* device,
        UINT width,
        UINT height,
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM
    );

    // 创建深度模板纹理
    HRESULT CreateDepthStencil(
        ID3D12Device* device,
        UINT width,
        UINT height,
        DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT
    );

    // 创建纹理资源视图
    HRESULT CreateShaderResourceView(ID3D12Device* device);
    HRESULT CreateRenderTargetView(ID3D12Device* device);
    HRESULT CreateDepthStencilView(ID3D12Device* device);

    // 释放资源
    void Release();

    // 获取资源
    ID3D12Resource* GetResource() const { return m_textureResource.Get(); }
    ID3D12Resource* GetUploadResource() const { return m_uploadHeap.Get(); }

    // 获取描述符句柄
    D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const { return m_srvHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const { return m_rtvHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_dsvHandle; }

    // 获取纹理信息
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    DXGI_FORMAT GetFormat() const { return m_format; }
    UINT GetMipLevels() const { return m_mipLevels; }

private:
    HRESULT LoadDDS(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const wchar_t* filename
    );

    HRESULT LoadBMP(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const wchar_t* filename
    );

    HRESULT LoadTGA(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const wchar_t* filename
    );

    HRESULT GenerateMipmaps(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList
    );

private:
    ComPtr<ID3D12Resource> m_textureResource;
    ComPtr<ID3D12Resource> m_uploadHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_srvHandle = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle = {};

    UINT m_width = 0;
    UINT m_height = 0;
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
    UINT m_mipLevels = 0;

    BOOL m_isRenderTarget = FALSE;
    BOOL m_isDepthStencil = FALSE;
};

// 纹理管理器
class D3D12TextureManager
{
public:
    D3D12TextureManager(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);
    ~D3D12TextureManager();

    // 加载纹理（带缓存）
    D3D12Texture* LoadTexture(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const wchar_t* filename
    );

    // 释放所有纹理
    void ReleaseAll();

    // 获取纹理
    D3D12Texture* GetTexture(const std::wstring& filename);

private:
    ID3D12Device* m_device;
    ID3D12DescriptorHeap* m_srvHeap;
    UINT m_srvDescriptorSize = 0;

    std::map<std::wstring, D3D12Texture*> m_textureCache;
};
