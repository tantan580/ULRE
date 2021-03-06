﻿#ifndef HGL_GRAPH_FONT_SOURCE_WINDOWS_INCLUDE
#define HGL_GRAPH_FONT_SOURCE_WINDOWS_INCLUDE

#include<hgl/graph/font/FontSource.h>
#include<hgl/type/MemBlock.h>
#include<windows.h>

namespace hgl
{
    namespace graph
    {
        class WinBitmapFont:public FontSourceSingle
        {
            HDC hdc;
            HFONT hfont;    		
            
            GLYPHMETRICS gm;
            MAT2 mat;

            uint ggo;

            MemBlock<uint8> buffer;

        public:

            WinBitmapFont(const Font &);
            ~WinBitmapFont();

            bool MakeCharBitmap(FontBitmap *,u32char) override;					///<产生字体数据
        };//class WinBitmapFont
    }//namespace graph
}//namespace hgl
#endif//HGL_GRAPH_FONT_SOURCE_WINDOWS_INCLUDE
