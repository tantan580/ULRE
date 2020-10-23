﻿#include<hgl/graph/VKTexture.h>
#include<hgl/graph/VKSampler.h>
#include<hgl/graph/VKDevice.h>
#include<hgl/graph/VKPhysicalDevice.h>
#include<hgl/graph/VKFence.h>
#include<hgl/graph/VKBuffer.h>
#include<hgl/graph/VKCommandBuffer.h>
#include<hgl/graph/VKMemory.h>
#include<hgl/graph/VKImageCreateInfo.h>
VK_NAMESPACE_BEGIN
namespace
{
    const uint32_t GetMipLevels(const uint32_t size)
    {
        return static_cast<uint32_t>(std::floor(std::log2(size)))+1;
    }
}//namespace

bool GPUDevice::CheckFormatSupport(const VkFormat format,const uint32_t bits,ImageTiling tiling) const
{
    const VkFormatProperties fp=attr->physical_device->GetFormatProperties(format);
    
    if(tiling==ImageTiling::Optimal)
        return(fp.optimalTilingFeatures&bits);
    else
        return(fp.linearTilingFeatures&bits);
}

Texture2D *GPUDevice::CreateTexture2D(TextureData *tex_data)
{
    if(!tex_data)
        return(nullptr);

    return(new Texture2D(attr->device,tex_data));
}

Texture2D *GPUDevice::CreateTexture2D(GPUMemory *mem,VkImage image,ImageView *image_view,VkImageLayout image_layout,ImageTiling tiling)
{
    TextureData *tex_data=new TextureData;

    tex_data->memory        = mem;
    tex_data->image_layout  = image_layout;
    tex_data->image         = image;
    tex_data->image_view    = image_view;

    tex_data->mip_levels    = 0;
    tex_data->tiling        = VkImageTiling(tiling);

    return CreateTexture2D(tex_data);
}

void GPUDevice::Clear(TextureCreateInfo *tci)
{
    if(!tci)return;

    if(tci->image)DestoryImage(tci->image);
    if(tci->image_view)delete tci->image_view;
    if(tci->memory)delete tci->memory;

    delete tci;
}

Texture2D *GPUDevice::CreateTexture2D(TextureCreateInfo *tci)
{
    if(!tci)return(nullptr);

    if(!tci->image)
    {        
        Image2DCreateInfo ici(tci->usage,tci->tiling,tci->format,tci->extent.width,tci->extent.height,tci->mipmap);
        tci->image=CreateImage(&ici);

        if(!tci->image)
        {
            Clear(tci);
            return(nullptr);
        }

        tci->memory=CreateMemory(tci->image);
    }

    if(!tci->image_view)
        tci->image_view=CreateImageView2D(attr->device,tci->format,tci->extent,tci->aspect,tci->image);

    Texture2D *tex=CreateTexture2D(tci->memory,tci->image,tci->image_view,tci->image_layout,tci->tiling);

    if(!tex)
    {
        Clear(tci);
        return(nullptr);
    }

    delete tci;
    return tex;
}

Texture2D *GPUDevice::CreateTexture2D(VkFormat format,uint32_t width,uint32_t height,VkImageAspectFlags aspectMask,VkImage image,VkImageLayout image_layout,ImageTiling tiling)
{
    if(!CheckTextureFormatSupport(format,tiling))return(nullptr);
 
    TextureCreateInfo *tci=new TextureCreateInfo;
    
    tci->extent.width   =width;
    tci->extent.height  =height;
    tci->extent.depth   =1;

    tci->format         =format;
    tci->aspect         =aspectMask;
    tci->image          =image;
    tci->image_layout   =image_layout;
    tci->tiling         =tiling;

    return CreateTexture2D(tci);
}

Texture2D *GPUDevice::CreateTexture2D(VkFormat format,uint32_t width,uint32_t height,VkImageAspectFlags aspectMask,uint usage,VkImageLayout image_layout,ImageTiling tiling)
{
    if(!CheckTextureFormatSupport(format,tiling))return(nullptr);
    
    TextureCreateInfo *tci=new TextureCreateInfo;

    tci->extent.width   =width;
    tci->extent.height  =height;
    tci->extent.depth   =1;

    tci->format         =format;
    tci->aspect         =aspectMask;
    tci->usage          =usage;
    tci->image_layout   =image_layout;
    tci->tiling         =tiling;

    return CreateTexture2D(tci);
}

Texture2D *GPUDevice::CreateTexture2D(VkFormat format,GPUBuffer *buf,uint32_t width,uint32_t height,VkImageAspectFlags aspectMask,uint usage,VkImageLayout image_layout,const ImageTiling tiling)
{
    if(!buf)return(nullptr);

    Texture2D *tex=CreateTexture2D(format,width,height,aspectMask,usage,image_layout,tiling);

    if(!tex)return(nullptr);

    ChangeTexture2D(tex,buf,0,0,width,height);

    return(tex);
}

Texture2D *GPUDevice::CreateTexture2D(VkFormat format,void *data,uint32_t width,uint32_t height,uint32_t size,VkImageAspectFlags aspectMask,uint usage,VkImageLayout image_layout,ImageTiling tiling)
{
    Texture2D *tex=CreateTexture2D(format,width,height,aspectMask,usage,image_layout,tiling);

    if(!tex)return(nullptr);

    if(data)
    {
        GPUBuffer *buf=CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,size,data);

        if(buf)
        {    
            ChangeTexture2D(tex,buf,0,0,width,height);
            delete buf;
        }
    }

    return(tex);
}

bool GPUDevice::ChangeTexture2D(Texture2D *tex,GPUBuffer *buf,const VkBufferImageCopy *buffer_image_copy,const int count)
{
    if(!tex||!buf)
        return(false);

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask     = tex->GetAspect();
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.levelCount     = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount     = 1;

    ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.srcAccessMask        = 0;
    imageMemoryBarrier.dstAccessMask        = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout            = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image                = tex->GetImage();
    imageMemoryBarrier.subresourceRange     = subresourceRange;

    texture_cmd_buf->Begin();
    texture_cmd_buf->PipelineBarrier(
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    texture_cmd_buf->CopyBufferToImage(
        buf->GetBuffer(),
        tex->GetImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        count,
        buffer_image_copy);

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    texture_cmd_buf->PipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    texture_cmd_buf->End();

    SubmitTexture(*texture_cmd_buf);
    return(true);
}

bool GPUDevice::ChangeTexture2D(Texture2D *tex,GPUBuffer *buf,const List<ImageRegion> &ir_list)
{
    if(!tex||!buf||ir_list.GetCount()<=0)
        return(false);

    const int ir_count=ir_list.GetCount();
    int count=0;

    AutoDeleteArray<VkBufferImageCopy> buffer_image_copy(ir_count);
    VkBufferImageCopy *tp=buffer_image_copy;
    const ImageRegion *sp=ir_list.GetData();

    VkDeviceSize offset=0;

    for(int i=0;i<ir_list.GetCount();i++)
    {
        tp->bufferOffset      = offset;
        tp->bufferRowLength   = 0;
        tp->bufferImageHeight = 0;
        tp->imageSubresource.aspectMask       = tex->GetAspect();
        tp->imageSubresource.mipLevel         = 0;
        tp->imageSubresource.baseArrayLayer   = 0;
        tp->imageSubresource.layerCount       = 1;
        tp->imageOffset.x     = sp->left;
        tp->imageOffset.y     = sp->top;
        tp->imageOffset.z     = 0;
        tp->imageExtent.width = sp->width;
        tp->imageExtent.height= sp->height;
        tp->imageExtent.depth = 1;

        offset+=sp->bytes;
        ++sp;
        ++tp;
    }

    return ChangeTexture2D(tex,buf,buffer_image_copy,ir_count);
}

bool GPUDevice::ChangeTexture2D(Texture2D *tex,GPUBuffer *buf,uint32_t left,uint32_t top,uint32_t width,uint32_t height)
{
    if(!tex||!buf
     ||left<0||left+width>tex->GetWidth()
     ||top<0||top+height>tex->GetHeight()
     ||width<=0||height<=0)
        return(false);

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset      = 0;
    buffer_image_copy.bufferRowLength   = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource.aspectMask       = tex->GetAspect();
    buffer_image_copy.imageSubresource.mipLevel         = 0;
    buffer_image_copy.imageSubresource.baseArrayLayer   = 0;
    buffer_image_copy.imageSubresource.layerCount       = 1;
    buffer_image_copy.imageOffset.x     = left;
    buffer_image_copy.imageOffset.y     = top;
    buffer_image_copy.imageOffset.z     = 0;
    buffer_image_copy.imageExtent.width = width;
    buffer_image_copy.imageExtent.height= height;
    buffer_image_copy.imageExtent.depth = 1;
    
    return ChangeTexture2D(tex,buf,&buffer_image_copy,1);
}

bool GPUDevice::ChangeTexture2D(Texture2D *tex,void *data,uint32_t left,uint32_t top,uint32_t width,uint32_t height,uint32_t size)
{
    if(!tex||!data
     ||left<0||left+width>tex->GetWidth()
     ||top<0||top+height>tex->GetHeight()
     ||width<=0||height<=0
     ||size<=0)
        return(false);

    GPUBuffer *buf=CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,size,data);
    
    bool result=ChangeTexture2D(tex,buf,left,top,width,height);

    delete buf;

    return(result);
}

void GPUDevice::GenerateMipmaps(Texture2D *tex)
{
//VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    // Check if image format supports linear blitting

    const uint32_t mipLevels=GetMipLevels(std::max(tex->GetWidth(),tex->GetHeight()));

    VkImage image=tex->GetImage();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = tex->GetAspect();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = tex->GetWidth();
    int32_t mipHeight = tex->GetHeight();

    texture_cmd_buf->Begin();

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        texture_cmd_buf->PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = tex->GetAspect();
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = tex->GetAspect();
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        texture_cmd_buf->BlitImage(
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        texture_cmd_buf->PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    texture_cmd_buf->PipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    texture_cmd_buf->End();
}

bool GPUDevice::SubmitTexture(const VkCommandBuffer *cmd_bufs,const uint32_t count)
{
    if(!cmd_bufs||count<=0)
        return(false);

    textureSQ->Submit(cmd_bufs,count,nullptr,nullptr);
//    textureSQ->WaitQueue();
    textureSQ->WaitFence();

    return(true);
}

Sampler *GPUDevice::CreateSampler(VkSamplerCreateInfo *sci)
{
    static VkSamplerCreateInfo default_sampler_create_info=
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        0.0f,
        false,
        0,
        false,
        VK_COMPARE_OP_NEVER,
        0.0f,
        1.0f,
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        false
    };

    if(!sci)
        sci=&default_sampler_create_info;

    VkSampler sampler;

    sci->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    //if(attr->physical_device->features.samplerAnisotropy)         //不知道为什么不准，先全部禁用吧
    //{
    //    sci->maxAnisotropy = attr->physical_device->properties.limits.maxSamplerAnisotropy;
    //    sci->anisotropyEnable = VK_TRUE;
    //} 
    //else 
    {
        sci->maxAnisotropy = 1.0;
        sci->anisotropyEnable = VK_FALSE;
    }

    if(vkCreateSampler(attr->device,sci,nullptr,&sampler)!=VK_SUCCESS)
        return(nullptr);

    return(new Sampler(attr->device,sampler));
}
VK_NAMESPACE_END
