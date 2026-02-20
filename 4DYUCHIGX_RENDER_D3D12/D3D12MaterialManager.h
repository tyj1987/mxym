#pragma once

#include "D3D12PCH.h"
#include <map>
#include <vector>
#include <string>

#include "../4DyuchiGRX_common/typedef.h"



// 前向声明
class CoD3D12Device;
class D3D12Material;
class D3D12TextureLoader;

// DirectX 12 材质管理器
class D3D12MaterialManager
{
public:
    D3D12MaterialManager();
    virtual ~D3D12MaterialManager();

    // ========== 初始化 ==========

    // 初始化材质管理器
    HRESULT Initialize(CoD3D12Device* pDevice, UINT nMaxSRVCount = 1000);

    // 释放所有资源
    void Release();

    // ========== 材质创建和删除 ==========

    // 创建材质
    void* CreateMaterial(const MATERIAL* pMaterial, DWORD* pdwWidth, DWORD* pdwHeight, DWORD dwFlag);

    // 删除材质
    void DeleteMaterial(void* pMtlHandle);

    // 创建材质集合
    DWORD CreateMaterialSet(MATERIAL_TABLE* pMtlEntry, DWORD dwNum);

    // 删除材质集合
    void DeleteMaterialSet(DWORD dwMtlSetIndex);

    // ========== 材质设置 ==========

    // 设置材质纹理边框
    void SetMaterialTextureBorder(void* pMtlHandle, DWORD dwColor);

    // ========== 材质查询 ==========

    // 获取材质
    D3D12Material* GetMaterial(void* pMtlHandle);

    // 获取材质集合
    std::vector<D3D12Material*>* GetMaterialSet(DWORD dwMtlSetIndex);

    // 检查材质是否存在
    BOOL HasMaterial(void* pMtlHandle) const;

    // ========== 纹理管理 ==========

    // 加载纹理（带缓存）
    HRESULT LoadTexture(const char* szFileName,
                        ID3D12Resource** ppTexture,
                        D3D12_GPU_DESCRIPTOR_HANDLE* pSRV);

    // 创建着色器资源视图
    HRESULT CreateShaderResourceView(ID3D12Resource* pResource,
                                     D3D12_GPU_DESCRIPTOR_HANDLE* pSRV);

    // 清理纹理缓存
    void ClearTextureCache();

    // 获取纹理缓存统计
    size_t GetTextureCacheSize() const { return m_mTextureCache.size(); }

    // ========== 描述符堆管理 ==========

    // 获取 SRV 堆
    ID3D12DescriptorHeap* GetSRVHeap() const { return m_pSRVHeap.Get(); }

    // 获取 SRV 描述符大小
    UINT GetSRVDescriptorSize() const { return m_nSRVDescriptorSize; }

    // 获取 SRV 堆起始句柄
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHeapStart() const;

    // ========== 统计信息 ==========

    // 获取材质数量
    size_t GetMaterialCount() const { return m_mMaterialMap.size(); }

    // 获取材质集合数量
    size_t GetMaterialSetCount() const { return m_mMaterialSetMap.size(); }

    // 获取已使用的 SRV 数量
    UINT GetUsedSRVCount() const { return m_nCurrentSRVOffset; }

    // 获取最大 SRV 数量
    UINT GetMaxSRVCount() const { return m_nMaxSRVCount; }

private:
    // ========== 辅助方法 ==========

    // 生成材质 ID
    DWORD GenerateMaterialID();

    // 生成材质句柄
    void* GenerateMaterialHandle(D3D12Material* pMaterial);

    // 从句柄提取材质指针
    D3D12Material* HandleToMaterial(void* pHandle) const;

    // 从文件加载纹理
    HRESULT LoadTextureFromFile(const char* szFileName, ID3D12Resource** ppResource);

    // 创建 D3D12 纹理资源
    HRESULT CreateD3D12Texture(const void* pData, UINT width, UINT height,
                               DXGI_FORMAT format, ID3D12Resource** ppResource);

    // 上传纹理数据到 GPU
    HRESULT UploadTextureData(ID3D12Resource* pTexture, const void* pData,
                              UINT rowPitch, UINT slicePitch);

    // 创建系统纹理（白色、黑色、法线等）
    HRESULT CreateSystemTextures();

    // 获取系统纹理（白色）
    D3D12_GPU_DESCRIPTOR_HANDLE GetWhiteTexture() const;

    // 获取系统纹理（黑色）
    D3D12_GPU_DESCRIPTOR_HANDLE GetBlackTexture() const;

    // 获取系统纹理（法线）
    D3D12_GPU_DESCRIPTOR_HANDLE GetNormalTexture() const;

    // ========== 材质管理 ==========

    // 材质映射（句柄 -> 材质）
    std::map<void*, D3D12Material*> m_mMaterialMap;

    // 材质集合映射（索引 -> 材质集合）
    std::map<DWORD, std::vector<D3D12Material*>> m_mMaterialSetMap;

    // 材质 ID 生成器
    DWORD m_dwNextMaterialID;

    // ========== 纹理缓存 ==========

    // 纹理映射（文件名 -> 资源）
    std::map<std::string, ComPtr<ID3D12Resource>> m_mTextureCache;

    // SRV 映射（文件名 -> SRV）
    std::map<std::string, D3D12_GPU_DESCRIPTOR_HANDLE> m_mSRVCache;

    // ========== 描述符堆 ==========

    // SRV 描述符堆
    ComPtr<ID3D12DescriptorHeap> m_pSRVHeap;

    // SRV 描述符大小
    UINT m_nSRVDescriptorSize;

    // 当前 SRV 偏移
    UINT m_nCurrentSRVOffset;

    // 最大 SRV 数量
    UINT m_nMaxSRVCount;

    // ========== 系统纹理 ==========

    // 白色纹理（默认纹理）
    ComPtr<ID3D12Resource> m_pWhiteTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hWhiteTextureSRV;

    // 黑色纹理
    ComPtr<ID3D12Resource> m_pBlackTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hBlackTextureSRV;

    // 法线贴图（平坦法线）
    ComPtr<ID3D12Resource> m_pNormalTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hNormalTextureSRV;

    // ========== 设备指针 ==========

    CoD3D12Device* m_pDevice;

    // ========== 纹理加载器 ==========

    D3D12TextureLoader* m_pTextureLoader;
};
