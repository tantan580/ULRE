﻿#ifndef HGL_GRAPH_VULKAN_MEMORY_INCLUDE
#define HGL_GRAPH_VULKAN_MEMORY_INCLUDE

#include<hgl/graph/vulkan/VK.h>
VK_NAMESPACE_BEGIN
class Memory
{
    VkDevice                device;
    VkDeviceMemory          memory;
    VkMemoryRequirements    req;
    uint32_t                index;
    uint32_t                properties;

private:

    friend class Device;

    Memory(VkDevice dev,VkDeviceMemory dm,const VkMemoryRequirements &mr,const uint32 i,const uint32_t p)
    {
        device=dev;
        memory=dm;
        req=mr;
        index=i;
        properties=p;
    }

public:

    virtual ~Memory();

    operator VkDeviceMemory(){return memory;}

    const VkMemoryRequirements &GetRequirements ()const{return req;}

    const uint32_t              GetType         ()const{return req.memoryTypeBits;}
    const VkDeviceSize          GetSize         ()const{return req.size;}
    const VkDeviceSize          GetAligment     ()const{return req.alignment;}
    const uint32_t              GetTypeIndex    ()const{return index;}
    const uint32_t              GetProperties   ()const{return properties;}
    
    void *Map();
    void *Map(VkDeviceSize offset,VkDeviceSize map_size);
    void Unmap();

    bool Bind(VkImage image);
};//class Memory
VK_NAMESPACE_END
#endif//HGL_GRAPH_VULKAN_MEMORY_INCLUDE
