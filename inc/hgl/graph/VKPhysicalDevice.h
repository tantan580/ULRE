﻿#pragma once

#include<hgl/graph/VK.h>
#include<hgl/type/String.h>
#include<hgl/type/Sets.h>

VK_NAMESPACE_BEGIN
class GPUPhysicalDevice
{
    VkInstance                          instance=nullptr;
    VkPhysicalDevice                    physical_device=nullptr;
    VkPhysicalDeviceFeatures            features;
    VkPhysicalDeviceProperties          properties;
    VkPhysicalDeviceMemoryProperties    memory_properties;
    List<VkLayerProperties>             layer_properties;
    List<VkExtensionProperties>         extension_properties;

public:

    GPUPhysicalDevice(VkInstance,VkPhysicalDevice);
    ~GPUPhysicalDevice()=default;

    operator VkPhysicalDevice(){return physical_device;}
    operator const VkPhysicalDevice()const{return physical_device;}

    const bool              CheckMemoryType(uint32_t,VkMemoryPropertyFlags,uint32_t *)const;

    VkPhysicalDeviceType    GetDeviceType()const{return properties.deviceType;}
    const char *            GetDeviceName()const{return properties.deviceName;}

    const VkPhysicalDeviceFeatures &        GetFeatures         ()const{return features;}
    const VkPhysicalDeviceProperties &      GetProperties       ()const{return properties;}
    const VkPhysicalDeviceMemoryProperties &GetMemoryProperties ()const{return memory_properties;}
    
    const bool              GetLayerVersion(const AnsiString &,uint32_t &spec,uint32_t &impl)const;
    const uint32_t          GetExtensionVersion(const AnsiString &name)const;

public:

    const uint32_t          GetConstantSize()const{return properties.limits.maxPushConstantsSize;}

public:

    /**
    * 获取该设备是否是显卡
    */
    const bool isGPU()const
    {
        if(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)return(true);
        if(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)return(true);
        if(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)return(true);

        return(false);
    }

    const bool isDiscreteGPU    ()const{return(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);}           ///<是否是独立显卡
    const bool isIntegratedGPU  ()const{return(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);}         ///<是否是集成显卡
    const bool isVirtualGPU     ()const{return(properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);}            ///<是否是虚拟显卡

public:

    VkFormatProperties GetFormatProperties(const VkFormat format)const
    {
        VkFormatProperties fp;

        hgl_zero(fp);

        vkGetPhysicalDeviceFormatProperties(physical_device,format,&fp);

        return fp;
    }

    bool OptimalSupport (const VkFormat format,const VkFormatFeatureFlags flag)const{VkFormatProperties fp;vkGetPhysicalDeviceFormatProperties(physical_device,format,&fp);return fp.optimalTilingFeatures&flag;}
    bool LinearSupport  (const VkFormat format,const VkFormatFeatureFlags flag)const{VkFormatProperties fp;vkGetPhysicalDeviceFormatProperties(physical_device,format,&fp);return fp.linearTilingFeatures&flag;}
    bool BufferSupport  (const VkFormat format,const VkFormatFeatureFlags flag)const{VkFormatProperties fp;vkGetPhysicalDeviceFormatProperties(physical_device,format,&fp);return fp.bufferFeatures&flag;}
    
    bool IsColorAttachmentOptimal(const VkFormat format)const{return OptimalSupport(format,VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);}
    bool IsDepthAttachmentOptimal(const VkFormat format)const{return OptimalSupport(format,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);}

    bool IsColorAttachmentLinear(const VkFormat format)const{return LinearSupport(format,VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);}
    bool IsDepthAttachmentLinear(const VkFormat format)const{return LinearSupport(format,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);}

    bool IsColorAttachmentBuffer(const VkFormat format)const{return BufferSupport(format,VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);}
    bool IsDepthAttachmentBuffer(const VkFormat format)const{return BufferSupport(format,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);}
    
    VkFormat GetDepthFormat(bool lower_to_high=true)const;
    VkFormat GetDepthStencilFormat(bool lower_to_high=true)const;

public:

    const uint32_t  GetMaxImage1D            ()const{return properties.limits.maxImageDimension1D;}
    const uint32_t  GetMaxImage2D            ()const{return properties.limits.maxImageDimension2D;}
    const uint32_t  GetMaxImage3D            ()const{return properties.limits.maxImageDimension3D;}
    const uint32_t  GetMaxImageCube          ()const{return properties.limits.maxImageDimensionCube;}
    const uint32_t  GetMaxImageArrayLayers   ()const{return properties.limits.maxImageArrayLayers;}
    const uint32_t  GetMaxUBORange           ()const{return properties.limits.maxUniformBufferRange;}
    const uint32_t  GetMaxSSBORange          ()const{return properties.limits.maxStorageBufferRange;}
    const uint32_t  GetMaxBoundDescriptorSets()const{return properties.limits.maxBoundDescriptorSets;}

    const uint32_t  GetMaxVertexInputAttributes  ()const{return properties.limits.maxVertexInputAttributes;}
    const uint32_t  GetMaxVertexInputBindings    ()const{return properties.limits.maxVertexInputBindings;}

    const uint32_t  GetMaxColorAttachments       ()const{return properties.limits.maxColorAttachments;}

    const float     GetMaxSamplerAnisotropy      ()const{return properties.limits.maxSamplerAnisotropy;}
    const float     GetMaxSamplerLodBias         ()const{return properties.limits.maxSamplerLodBias;}

    const void      GetPointSize(float &granularity,float &min_size,float &max_size)
    {
        granularity =properties.limits.pointSizeGranularity;
        min_size    =properties.limits.pointSizeRange[0];
        max_size    =properties.limits.pointSizeRange[1];
    }

    const void      GetLineWidth(float &granularity,float &min_width,float &max_width)
    {
        granularity =properties.limits.lineWidthGranularity;
        min_width   =properties.limits.lineWidthRange[0];
        max_width   =properties.limits.lineWidthRange[1];
    }
};//class GPUPhysicalDevice
VK_NAMESPACE_END
