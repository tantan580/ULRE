#include<hgl/graph/TextureLoader.h>
#include<hgl/graph/Bitmap.h>
#include<hgl/io/FileInputStream.h>
#include<hgl/log/LogInfo.h>

namespace hgl
{
    namespace graph
    {
        uint32 Texture2DLoader::ComputeTotalBytes()
        {
            uint32 pixel_bits;
            uint32 bytes;

            if(file_header.channels==0)
            {
                if(compress_format<0||compress_format>=CompressFormatCount)
                    return(0);

                pixel_bits=CompressFormatBits[compress_format];
            }
            else
            {
                pixel_bits=pixel_format.pixel_bits();
            }

            bytes=(pixel_bits*file_header.pixel_count())>>3;

            mipmap_zero_total_bytes=bytes;

            if(file_header.mipmaps<=1)
                return bytes;

            uint32 total=0;

            uint32 w=file_header.width;
            uint32 h=file_header.height;

            while(w>=1&&h>=1)
            {
                if(bytes<8)
                    total+=8;
                else
                    total+=bytes;

                if(w==1&&h==1)break;

                if(w>1){w>>=1;bytes>>=1;}
                if(h>1){h>>=1;bytes>>=1;}
            }

            return total;
        }

        bool Texture2DLoader::Load(io::InputStream *is)
        {
            if(!is)return(false);

            if(is->Read(&file_header,sizeof(Tex2DFileHeader))!=sizeof(Tex2DFileHeader))
                return(false);

            if(file_header.version!=3)
                return(false);

            if(file_header.channels==0)     //压缩格式
            {
                if(is->Read(&compress_format,sizeof(uint16))!=sizeof(uint16))
                    return(false);
            }
            else
            {
                if(is->Read(&pixel_format,sizeof(TexPixelFormat))!=sizeof(TexPixelFormat))
                    return(false);
            }

            const uint32 total_bytes=ComputeTotalBytes();

            if(is->Available()<total_bytes)
                return(false);

            void *ptr=OnBegin(total_bytes);

            if(!ptr)
                return(false);
            
            if(is->Read(ptr,total_bytes)!=total_bytes)
                OnError();
            
            OnEnd();

            return(true);
        }

        bool Texture2DLoader::Load(const OSString &filename)
        {
            io::OpenFileInputStream fis(filename);

            if(!fis)
            {
                LOG_ERROR(OS_TEXT("[ERROR] open texture file<")+filename+OS_TEXT("> failed."));
                return(false);
            }

            return this->Load(&fis);
        }
    }//namespace graph

    namespace graph
    {
        Bitmap2DLoader::~Bitmap2DLoader()
        {
            SAFE_CLEAR(bmp);
        }

        void *Bitmap2DLoader::OnBegin(uint32 total_bytes)
        {
            SAFE_CLEAR(bmp);

            bmp=new BitmapData;

            bmp->width      =file_header.width;
            bmp->height     =file_header.height;
            bmp->total_bytes=total_bytes;

            bmp->data=new char[total_bytes];

            return bmp->data;
        }

        BitmapData *Bitmap2DLoader::GetBitmap()
        {
            BitmapData *result=bmp;
            bmp=nullptr;
            return result;
        }
    
        BitmapData *LoadBitmapFromFile(const OSString &filename)
        {
            Bitmap2DLoader loader;

            if(!loader.Load(filename))
                return(nullptr);

            return loader.GetBitmap();
        }
    }//namespace graph
}//namespace hgl
