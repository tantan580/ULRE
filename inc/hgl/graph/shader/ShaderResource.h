﻿#pragma once
#include<hgl/type/String.h>
#include<hgl/type/List.h>
#include<hgl/type/StringList.h>
#include<hgl/graph/VKFormat.h>
#include<hgl/graph/VKStruct.h>

VK_NAMESPACE_BEGIN

/**
 * Shader数据来源
 */
enum class ShaderDataSource
{
    Global=0,       ///<由全局系统提供的数据(如CameraMatrix)
    Private=1,      ///<每种材质的私有数据
};//

/**
 * Shader数据来源类型
 */
enum class ShaderDataSourceType
{
    Static=0,
    Dynamic,
    Instance
};

struct ShaderStage
{
    AnsiString          name;
    uint                location;

    VertexAttribType    type;       ///<成份数量(如vec4中的4)
    VkFormat            format;     ///<对应的Vulkan格式(如vec4对应的FMT_RGBA32F)

    uint                binding;

    bool                global;     ///<是否全局数据
    bool                dynamic;    ///<是否动态数量
};//struct ShaderStage

using ShaderStageList       =ObjectList<ShaderStage>;
using DescriptorSetList     =List<uint32_t>;
using DescriptorBindingList =List<uint32_t>;

struct ShaderDescriptorList
{
    AnsiStringList          name_list;
    DescriptorSetList       set_list;
    DescriptorBindingList   binding_list;
};

#ifndef VK_DESCRIPTOR_TYPE_BEGIN_RANGE
constexpr size_t VK_DESCRIPTOR_TYPE_BEGIN_RANGE=VK_DESCRIPTOR_TYPE_SAMPLER;
#endif//VK_DESCRIPTOR_TYPE_BEGIN_RANGE

#ifndef VK_DESCRIPTOR_TYPE_END_RANGE
constexpr size_t VK_DESCRIPTOR_TYPE_END_RANGE=VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
#endif//VK_DESCRIPTOR_TYPE_END_RANGE
    
#ifndef VK_DESCRIPTOR_TYPE_RANGE_SIZE
constexpr size_t VK_DESCRIPTOR_TYPE_RANGE_SIZE=VK_DESCRIPTOR_TYPE_END_RANGE-VK_DESCRIPTOR_TYPE_BEGIN_RANGE+1;
#endif//VK_DESCRIPTOR_TYPE_RANGE_SIZE

class ShaderResource
{
    VkShaderStageFlagBits stage_flag;

    const void *spv_data;
    uint32 spv_size;

    ShaderStageList stage_inputs;
    ShaderStageList stage_outputs;

    ShaderDescriptorList descriptor_list[VK_DESCRIPTOR_TYPE_RANGE_SIZE];

public:

    ShaderResource(const VkShaderStageFlagBits &,const void *,const uint32);
    virtual ~ShaderResource()=default;

    const   VkShaderStageFlagBits   GetStage            ()const {return stage_flag;}
    const   os_char *               GetStageName        ()const;

    const   uint32_t *              GetCode             ()const {return (uint32_t *)spv_data;}
    const   uint32_t                GetCodeSize         ()const {return spv_size;}

            ShaderStageList &       GetStageInputs      ()      {return stage_inputs;}
            ShaderStageList &       GetStageOutputs     ()      {return stage_outputs;}

    const   uint                    GetStageInputCount  ()const {return stage_inputs.GetCount();}
    const   uint                    GetStageOutputCount ()const {return stage_outputs.GetCount();}

    const   ShaderStage *           GetStageInput       (const AnsiString &)const;
    const   int                     GetStageInputBinding(const AnsiString &)const;

    const   ShaderDescriptorList *  GetDescriptorList   ()const {return descriptor_list;}
            ShaderDescriptorList *  GetDescriptorList   (VkDescriptorType desc_type)
    {
        if(desc_type<VK_DESCRIPTOR_TYPE_BEGIN_RANGE
            ||desc_type>VK_DESCRIPTOR_TYPE_END_RANGE)return nullptr;

        return descriptor_list+desc_type;
    }

    ShaderDescriptorList &GetUBO    (){return descriptor_list[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER];}
    ShaderDescriptorList &GetSSBO   (){return descriptor_list[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER];}
    ShaderDescriptorList &GetSampler(){return descriptor_list[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];}

    const int                       GetBinding      (VkDescriptorType desc_type,const AnsiString &name)const;
    const DescriptorBindingList *   GetBindingList  (VkDescriptorType desc_type)const
    {
        if(desc_type<VK_DESCRIPTOR_TYPE_BEGIN_RANGE
            ||desc_type>VK_DESCRIPTOR_TYPE_END_RANGE)return nullptr;

        return &(descriptor_list[desc_type].binding_list);
    }
};//class ShaderResource

ShaderResource *LoadShaderResource(const uint8 *origin_filedata,const int64 filesize,bool include_file_header);
ShaderResource *LoadShaderResoruce(const OSString &filename);

struct ShaderModuleCreateInfo:public vkstruct_flag<VkShaderModuleCreateInfo,VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO>
{
public:

    ShaderModuleCreateInfo(ShaderResource *sr)
    {
        codeSize=sr->GetCodeSize();
        pCode   =sr->GetCode();
    }
};
VK_NAMESPACE_END