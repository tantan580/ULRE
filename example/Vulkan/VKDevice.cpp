﻿#include"VKDevice.h"
#include<hgl/type/Pair.h>
#include"VKBuffer.h"
#include"VKCommandBuffer.h"
//#include"VKDescriptorSet.h"
#include"VKRenderPass.h"
#include"VKFence.h"
#include"VKSemaphore.h"

VK_NAMESPACE_BEGIN

namespace
{
    bool CreateVulkanBuffer(VulkanBuffer &vb,const DeviceAttribute *rsa,VkBufferUsageFlags buf_usage,VkDeviceSize size,VkSharingMode sharing_mode)
    {
        VkBufferCreateInfo buf_info={};
        buf_info.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.pNext=nullptr;
        buf_info.usage=buf_usage;
        buf_info.size=size;
        buf_info.queueFamilyIndexCount=0;
        buf_info.pQueueFamilyIndices=nullptr;
        buf_info.sharingMode=sharing_mode;
        buf_info.flags=0;

        if(vkCreateBuffer(rsa->device,&buf_info,nullptr,&vb.buffer)!=VK_SUCCESS)
            return(false);

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(rsa->device,vb.buffer,&mem_reqs);

        VkMemoryAllocateInfo alloc_info={};
        alloc_info.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext=nullptr;
        alloc_info.memoryTypeIndex=0;
        alloc_info.allocationSize=mem_reqs.size;

        if(rsa->CheckMemoryType(mem_reqs.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&alloc_info.memoryTypeIndex))
        {
            if(vkAllocateMemory(rsa->device,&alloc_info,nullptr,&vb.memory)==VK_SUCCESS)
            {
                if(vkBindBufferMemory(rsa->device,vb.buffer,vb.memory,0)==VK_SUCCESS)
                {
                    vb.info.buffer=vb.buffer;
                    vb.info.offset=0;
                    vb.info.range=size;

                    return(true);
                }

                vkFreeMemory(rsa->device,vb.memory,nullptr);
            }
        }

        vkDestroyBuffer(rsa->device,vb.buffer,nullptr);
        return(false);
    }
}//namespace

Device::Device(DeviceAttribute *da)
{
    attr=da;
    current_framebuffer=0;
    image_acquired_semaphore=this->CreateSem();

    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &attr->swap_chain;
    present.pWaitSemaphores = nullptr;
    present.waitSemaphoreCount = 0;
    present.pResults = nullptr;
}
Device::~Device()
{
    delete image_acquired_semaphore;
    delete attr;
}

VertexBuffer *Device::CreateBuffer(VkBufferUsageFlags buf_usage,VkFormat format,uint32_t count,VkSharingMode sharing_mode)
{
    const uint32_t stride=GetStrideByFormat(format);

    if(stride==0)
    {
        std::cerr<<"format["<<format<<"] stride length is 0,please use \"Device::CreateBuffer(VkBufferUsageFlags,VkDeviceSize,VkSharingMode)\" function.";
        return(nullptr);
    }

    const VkDeviceSize size=stride*count;

    VulkanBuffer vb;

    if(!CreateVulkanBuffer(vb,attr,buf_usage,size,sharing_mode))
        return(nullptr);

    return(new VertexBuffer(attr->device,vb,format,stride,count));
}

Buffer *Device::CreateBuffer(VkBufferUsageFlags buf_usage,VkDeviceSize size,VkSharingMode sharing_mode)
{
    VulkanBuffer vb;

    if(!CreateVulkanBuffer(vb,attr,buf_usage,size,sharing_mode))
        return(nullptr);

    return(new Buffer(attr->device,vb));
}

CommandBuffer *Device::CreateCommandBuffer()
{
    if(!attr->cmd_pool)
        return(nullptr);

    VkCommandBufferAllocateInfo cmd={};
    cmd.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext=nullptr;
    cmd.commandPool=attr->cmd_pool;
    cmd.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount=1;

    VkCommandBuffer cmd_buf;

    VkResult res=vkAllocateCommandBuffers(attr->device,&cmd,&cmd_buf);

    if(res!=VK_SUCCESS)
        return(nullptr);

    return(new CommandBuffer(attr->device,attr->cmd_pool,cmd_buf));
}

RenderPass *Device::CreateRenderPass()
{
    VkAttachmentDescription attachments[2];
    attachments[0].format=attr->format;
    attachments[0].samples=VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags=0;

    attachments[1].format=attr->depth.format;
    attachments[1].samples=VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags=0;

    VkAttachmentReference color_reference={0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_reference={1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass={};
    subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags=0;
    subpass.inputAttachmentCount=0;
    subpass.pInputAttachments=nullptr;
    subpass.colorAttachmentCount=1;
    subpass.pColorAttachments=&color_reference;
    subpass.pResolveAttachments=nullptr;
    subpass.pDepthStencilAttachment=&depth_reference;
    subpass.preserveAttachmentCount=0;
    subpass.pPreserveAttachments=nullptr;

    VkRenderPassCreateInfo rp_info={};
    rp_info.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext=nullptr;
    rp_info.attachmentCount=2;
    rp_info.pAttachments=attachments;
    rp_info.subpassCount=1;
    rp_info.pSubpasses=&subpass;
    rp_info.dependencyCount=0;
    rp_info.pDependencies=nullptr;

    VkRenderPass render_pass;

    if(vkCreateRenderPass(attr->device,&rp_info,nullptr,&render_pass)!=VK_SUCCESS)
        return(nullptr);

    return(new RenderPass(attr->device,render_pass));
}

Fence *Device::CreateFence()
{
    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0;

    VkFence drawFence;

    if(vkCreateFence(attr->device, &fenceInfo, nullptr, &drawFence)!=VK_SUCCESS)
        return(nullptr);

    return(new Fence(attr->device,drawFence));
}

Semaphore *Device::CreateSem()
{
    VkSemaphoreCreateInfo SemaphoreCreateInfo;
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SemaphoreCreateInfo.pNext = nullptr;
    SemaphoreCreateInfo.flags = 0;

    VkSemaphore sem;
    if(vkCreateSemaphore(attr->device, &SemaphoreCreateInfo, nullptr, &sem)!=VK_SUCCESS)
        return(nullptr);

    return(new Semaphore(attr->device,sem));
}

bool Device::AcquireNextImage()
{
    return(vkAcquireNextImageKHR(attr->device,attr->swap_chain,UINT64_MAX,*image_acquired_semaphore,VK_NULL_HANDLE,&current_framebuffer)==VK_SUCCESS);
}

bool Device::QueueSubmit(CommandBuffer *buf,Fence *fence)
{
    if(!buf||!fence)
        return(false);

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1] = {};

    VkSemaphore sem=*image_acquired_semaphore;
    VkCommandBuffer cmd_bufs=*buf;

    submit_info[0].pNext = nullptr;
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount = 1;
    submit_info[0].pWaitSemaphores = &sem;
    submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cmd_bufs;
    submit_info[0].signalSemaphoreCount = 0;
    submit_info[0].pSignalSemaphores = nullptr;

    return(vkQueueSubmit(attr->graphics_queue, 1, submit_info, *fence)==VK_SUCCESS);
}

bool Device::Wait(Fence *f,bool wait_all,uint64_t time_out)
{
    VkResult res;
    VkFence fence=*f;

    do {
        res = vkWaitForFences(attr->device, 1, &fence, wait_all, time_out);
    } while (res == VK_TIMEOUT);

    return(true);
}

bool Device::QueuePresent()
{
    present.pImageIndices = &current_framebuffer;

    return(vkQueuePresentKHR(attr->present_queue, &present)==VK_SUCCESS);
}
VK_NAMESPACE_END