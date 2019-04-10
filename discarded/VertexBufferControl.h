﻿#ifndef HGL_GRAPH_VERTEX_BUFFER_OBJECT_CONTROL_INCLUDE
#define HGL_GRAPH_VERTEX_BUFFER_OBJECT_CONTROL_INCLUDE

#include<hgl/type/DataType.h>
#include<GLEWCore/glew.h>
namespace hgl
{
    namespace graph
    {
        class VertexBufferControl
        {
        protected:

            uint type;
            uint index;

        public:

            uint GetIndex()const { return index; }

        public:

            VertexBufferControl(uint t, uint i) { type = t; index = i; }
            virtual ~VertexBufferControl()
            {
                Clear();
            }

            virtual void Set(GLsizei, void *,GLenum)=0;
            virtual void Change(GLintptr,GLsizei, void *)=0;
            void Clear()
            {
                if(!type||!index)return;

                glDeleteBuffers(1, &index);
                type = index = 0;
            }
        };//class VertexBufferControl
    }//namespace graph
}//namespace hgl
#endif//HGL_GRAPH_VERTEX_BUFFER_OBJECT_CONTROL_INCLUDE