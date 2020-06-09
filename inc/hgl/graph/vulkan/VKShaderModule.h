﻿#ifndef HGL_GRAPH_VULKAN_SHADER_MODULE_INCLUDE
#define HGL_GRAPH_VULKAN_SHADER_MODULE_INCLUDE

#include<hgl/graph/shader/ShaderResource.h>
#include<hgl/type/Set.h>

VK_NAMESPACE_BEGIN

/**
 * Shader模块<br>
 * 该模块提供的是原始的shader数据和信息，不可被修改，只能通过ShaderModuleManage创建和删除
 */
class ShaderModule
{
    VkDevice device;
    int shader_id;
    int ref_count;

private:

    VkPipelineShaderStageCreateInfo *stage_create_info;

protected:

    ShaderResource *shader_resource;

public:

    ShaderModule(VkDevice dev,int id,VkPipelineShaderStageCreateInfo *pssci,ShaderResource *);
    virtual ~ShaderModule();

    const int GetID()const{return shader_id;}

    const int IncRef(){return ++ref_count;}
    const int DecRef(){return --ref_count;}

public:

    const VkShaderStageFlagBits             GetStage        ()const{return stage_create_info->stage;}
    const VkPipelineShaderStageCreateInfo * GetCreateInfo   ()const{return stage_create_info;}

    const int                               GetBinding      (VkDescriptorType desc_type,const UTF8String &name)const
    {
        return shader_resource->GetBinding(desc_type,name);
    }

    const ShaderDescriptorList *            GetDescriptorList()const{return shader_resource->GetDescriptorList();}
};//class ShaderModule

class VertexAttributeBinding;

/**
 * 顶点Shader模块<br>
 * 由于顶点shader在最前方执行，所以它比其它shader多了VertexInput的数据
 */
class VertexShaderModule:public ShaderModule
{
    uint32_t attr_count;
    VkVertexInputBindingDescription *binding_list;
    VkVertexInputAttributeDescription *attribute_list;

private:

    Set<VertexAttributeBinding *> vab_sets;

public:

    VertexShaderModule(VkDevice dev,int id,VkPipelineShaderStageCreateInfo *pssci,ShaderResource *sr);
    virtual ~VertexShaderModule();

    /**
     * 获取输入流绑定点，需要注意的时，这里获取的binding并非是shader中的binding/location，而是绑定顺序的序列号。对应vkCmdBindVertexBuffer的缓冲区序列号
     */
    const int                                   GetStageInputBinding(const UTF8String &name)const{return shader_resource->GetStageInputBinding(name);}

    const uint32_t                              GetAttrCount()const{return attr_count;}

    const VkVertexInputBindingDescription *     GetDescList ()const{return binding_list;}
    const VkVertexInputAttributeDescription *   GetAttrList ()const{return attribute_list;}

    const VkVertexInputBindingDescription *     GetDesc     (const uint32_t index)const{return (index>=attr_count?nullptr:binding_list+index);}
    const VkVertexInputAttributeDescription *   GetAttr     (const uint32_t index)const{return (index>=attr_count?nullptr:attribute_list+index);}

public:

    VertexAttributeBinding *                    CreateVertexAttributeBinding();
    bool                                        Release(VertexAttributeBinding *);
    const uint32_t                              GetInstanceCount()const{return vab_sets.GetCount();}
};//class VertexShaderModule:public ShaderModule
VK_NAMESPACE_END
#endif//HGL_GRAPH_VULKAN_SHADER_MODULE_INCLUDE
