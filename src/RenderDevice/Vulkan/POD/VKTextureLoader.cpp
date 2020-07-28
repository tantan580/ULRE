﻿#include<hgl/graph/vulkan/VK.h>
#include<hgl/graph/vulkan/VKDevice.h>
#include<hgl/graph/vulkan/VKBuffer.h>
#include<hgl/graph/TextureLoader.h>
#include<hgl/io/FileInputStream.h>
#include<hgl/log/LogInfo.h>

VK_NAMESPACE_BEGIN
namespace
{
    struct PixelFormat
    {
        VkFormat        format;

        uint8           channels;   //颜色通道数
        char            colors[4];
        uint8           bits[4];
        VulkanDataType  type;

    public:

        const uint GetPixelBytes()const{return (bits[0]+bits[1]+bits[2]+bits[3])>>3;}                   ///<获取单个象素所需字节数
    };//

    constexpr PixelFormat pf_list[]=
    {
        {UFMT_BGRA4,      4,{'B','G','R','A'},{ 4, 4, 4, 4},VulkanDataType::UNORM},
        {UFMT_RGB565,     3,{'R','G','B', 0 },{ 5, 6, 5, 0},VulkanDataType::UNORM},
        {UFMT_A1RGB5,     4,{'A','R','G','B'},{ 1, 5, 5, 5},VulkanDataType::UNORM},
        {UFMT_R8,         1,{'R', 0 , 0 , 0 },{ 8, 0, 0, 0},VulkanDataType::UNORM},
        {UFMT_RG8,        2,{'R','G', 0 , 0 },{ 8, 8, 0, 0},VulkanDataType::UNORM},
        {UFMT_RGBA8,      4,{'R','G','B','A'},{ 8, 8, 8, 8},VulkanDataType::UNORM},
        {UFMT_A2BGR10,    4,{'A','B','G','R'},{ 2,10,10,10},VulkanDataType::UNORM},
        {UFMT_R16,        1,{'R', 0 , 0 , 0 },{16, 0, 0, 0},VulkanDataType::UNORM},
        {UFMT_R16F,       1,{'R', 0 , 0 , 0 },{16, 0, 0, 0},VulkanDataType::SFLOAT},
        {UFMT_RG16,       2,{'R','G', 0 , 0 },{16,16, 0, 0},VulkanDataType::UNORM},
        {UFMT_RG16F,      2,{'R','G', 0 , 0 },{16,16, 0, 0},VulkanDataType::SFLOAT},
        {UFMT_RGBA16,     4,{'R','G','B','A'},{16,16,16,16},VulkanDataType::UNORM},
        {UFMT_RGBA16F,    4,{'R','G','B','A'},{16,16,16,16},VulkanDataType::SFLOAT},
        {UFMT_R32U,       1,{'R', 0 , 0 , 0 },{32, 0, 0, 0},VulkanDataType::UINT},
        {UFMT_R32I,       1,{'R', 0 , 0 , 0 },{32, 0, 0, 0},VulkanDataType::SINT},
        {UFMT_R32F,       1,{'R', 0 , 0 , 0 },{32, 0, 0, 0},VulkanDataType::SFLOAT},
        {UFMT_RG32U,      2,{'R','G', 0 , 0 },{32,32, 0, 0},VulkanDataType::UINT},
        {UFMT_RG32I,      2,{'R','G', 0 , 0 },{32,32, 0, 0},VulkanDataType::SINT},
        {UFMT_RG32F,      2,{'R','G', 0 , 0 },{32,32, 0, 0},VulkanDataType::SFLOAT},
        {UFMT_RGB32U,     3,{'R','G','B', 0 },{32,32,32, 0},VulkanDataType::UINT},
        {UFMT_RGB32I,     3,{'R','G','B', 0 },{32,32,32, 0},VulkanDataType::SINT},
        {UFMT_RGB32F,     3,{'R','G','B', 0 },{32,32,32, 0},VulkanDataType::SFLOAT},
        {UFMT_RGBA32U,    4,{'R','G','B','A'},{32,32,32,32},VulkanDataType::UINT},
        {UFMT_RGBA32I,    4,{'R','G','B','A'},{32,32,32,32},VulkanDataType::SINT},
        {UFMT_RGBA32F,    4,{'R','G','B','A'},{32,32,32,32},VulkanDataType::SFLOAT},
        {UFMT_B10GR11UF,  3,{'B','G','R', 0 },{10,11,11, 0},VulkanDataType::UFLOAT}
    };

    constexpr uint PixelFormatCount=sizeof(pf_list)/sizeof(PixelFormat);

    const VkFormat GetVulkanFormat(const Tex2DFileHeader &fh)
    {
        const PixelFormat *pf=pf_list;

        for(uint i=0;i<PixelFormatCount;i++,++pf)
        {
            if(fh.channels!=pf->channels)continue;
            if(fh.datatype!=(uint8)pf->type)continue;

            if(fh.colors[0]!=pf->colors[0])continue;
            if(fh.colors[1]!=pf->colors[1])continue;
            if(fh.colors[2]!=pf->colors[2])continue;
            if(fh.colors[3]!=pf->colors[3])continue;

            if(fh.bits[0]!=pf->bits[0])continue;
            if(fh.bits[1]!=pf->bits[1])continue;
            if(fh.bits[2]!=pf->bits[2])continue;
            if(fh.bits[3]!=pf->bits[3])continue;

            return pf->format;
        }

        return VK_FORMAT_UNDEFINED;
    }

    class VkTexture2DLoader:public Texture2DLoader
    {
    protected:

        Device *device;

        VkFormat format;
        vulkan::Buffer *buf;

        Texture2D *tex;

    public:

        VkTexture2DLoader(Device *dev):device(dev)
        {
            buf=nullptr;
            format=VK_FORMAT_UNDEFINED;
            tex=nullptr;
        }

        virtual ~VkTexture2DLoader()
        {
            SAFE_CLEAR(buf);
            SAFE_CLEAR(tex);
        }

        void *OnBegin(uint32 total_bytes) override
        {
            format=GetVulkanFormat(file_header);

            if(!CheckVulkanFormat(format))
                return(nullptr);

            buf=device->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,total_bytes);

            if(!buf)
                return(nullptr);

            return buf->Map();
        }

        void OnEnd() override
        {
            buf->Unmap();

            tex=device->CreateTexture2D(format,buf,file_header.width,file_header.height);
        }

        Texture2D *GetTexture()
        {
            Texture2D *result=tex;
            tex=nullptr;
            return result;
        }
    };//class VkTexture2DLoader
}//namespace

Texture2D *CreateTextureFromFile(Device *device,const OSString &filename)
{
    VkTexture2DLoader loader(device);

    if(!loader.Load(filename))
        return(false);

    return loader.GetTexture();
}
VK_NAMESPACE_END
