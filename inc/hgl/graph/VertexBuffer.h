﻿#ifndef HGL_VERTEX_BUFFER_OBJECT_INCLUDE
#define HGL_VERTEX_BUFFER_OBJECT_INCLUDE

#include<hgl/type/Color3f.h>
#include<hgl/type/Color4f.h>
#include<hgl/type/RectScope.h>
// #include<hgl/type/BaseString.h>
#include<hgl/graph/VertexBufferBase.h>
//#include<hgl/LogInfo.h>
#include<GLEWCore/glew.h>
namespace hgl
{
    namespace graph
    {
        class VertexBufferControl;

        /**
         * 顶点属性数据实际模板
         */
        template<typename T,int C> class VertexBuffer:public VertexBufferBase
        {
        protected:

            T *mem_type;                                                                            ///<符合当前类型的地址
            T *access;                                                                              ///<当前访问地址

            T *start;                                                                               ///<访问起始地址

        public:

            VertexBuffer(const uint type,const uint dt,const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBufferBase(type,dt,sizeof(T),C,_size,level)
            {
                count=_size;

                mem_type=(T *)GetData();
                access=0;
                start=0;

                if(_data)
                {
                    memcpy(mem_type,_data,this->total_bytes);
                    this->Update();
                }
            }

            virtual ~VertexBuffer()=default;

            void SetCount(int _count)
            {
                count=_count;

                SetDataSize(_count*C*sizeof(T));

                mem_type=(T *)GetData();
                access=0;
                start=0;
            }

            /**
             * 取得数据区地址
             * @param offset 从第几个数据开始访问
             * @return 访问地址
             */
            T *Get(int offset=0)
            {
                if(!mem_type||offset>=count)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer::Get() out,offset:")+OSString(offset));
                    return(nullptr);
                }

                return mem_type+offset*C;
            }

            /**
             * 开始访问数据区
             * @param offset 从第几个数据开始访问
             * @return 访问地址
             */
            void *Begin(int offset=0)
            {
                if(access)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer::Begin() access!=0,offset:")+OSString(offset));
                    return(nullptr);
                }

                access=Get(offset);

                if(access)
                    start=access;

                return access;
            }

            /**
             * 结束访问
             */
            void End()
            {
                this->Change(((char *)start )-((char *)mem_type),
                             ((char *)access)-((char *)start),
                             start);

                access=nullptr;
                start=nullptr;
            }

            /**
             * 写入指定数量的数据
             * @param vp 数据指针
             * @param number 数据数量
             */
            bool WriteData(const T *vp,const int number)
            {
                if(!this->access||this->access+C*number>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer::Write(const T *,number) out,number:")+OSString(number));
                    return(false);
                }

                memcpy(access,vp,C*number*sizeof(T));

                access+=C*number;

                return(true);
            }
        };//class VertexBuffer

        /**
         * 一元数据缓冲区
         */
        template<typename T,uint DT> class VertexBuffer1:public VertexBuffer<T,1>
        {
        public:

            VertexBuffer1(const uint type,const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer<T,1>(type,DT,_size,_data,level){}
            VertexBuffer1(const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer<T,1>(GL_ARRAY_BUFFER,DT,_size,_data,level){}
            virtual ~VertexBuffer1()=default;

            bool Write(const T v1)
            {
                if(!this->access||this->access+1>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer1::Write(const T) out"));
                    return(false);
                }

                *this->access++=v1;
                return(true);
            }

            /**
             * 将一个值重复多次写入缓冲区
             * @param v 值
             * @param count 写入数量
             */
            bool WriteRepeat(const T v,const int count)
            {
                if(!this->access||this->access+count>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer1::Write(const T,")+OSString(count)+OS_TEXT(") out"));
                    return(false);
                }

                hgl_set(this->access,v,count);
                this->access+=count;
                return(true);
            }
        };//class VertexBuffer1

        /**
         * 索引数据缓冲区
         */
        template<typename T,uint DT> class ElementBuffer:public VertexBuffer1<T,DT>
        {
        public:
            ElementBuffer(const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer1<T,DT>(GL_ELEMENT_ARRAY_BUFFER,DT,_size,_data,level){}
            virtual ~ElementBuffer()=default;
        };//

        /**
         * 二元数据缓冲区
         */
        template<typename T,uint DT> class VertexBuffer2:public VertexBuffer<T,2>
        {
        public:

            VertexBuffer2(const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer<T,2>(GL_ARRAY_BUFFER,DT,_size,_data,level){}
            virtual ~VertexBuffer2()=default;

            bool Write(const T v1,const T v2)
            {
                if(!this->access||this->access+2>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::Write(const T ,const T) out"));
                    return(false);
                }

                *this->access++=v1;
                *this->access++=v2;

                return(true);
            }

            bool Write(const T *v)
            {
                if(!this->access||this->access+2>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::Write(T *) out"));
                    return(false);
                }

                *this->access++=*v++;
                *this->access++=*v;

                return(true);
            }

            bool Write(const Vector2f &v)
            {
                if(!this->access||this->access+2>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::Write(vec2 &) out"));
                    return(false);
                }

                *this->access++=v.x;
                *this->access++=v.y;

                return(true);
            }

            /**
             * 将一个值重复多次写入缓冲区
             * @param v 值
             * @param count 写入数量
             */
            bool Write(const Vector2f &v,const int count)
            {
                if(!this->access||this->access+(count<<1)>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer1::Write(const Vector2f &,")+OSString(count)+OS_TEXT(") out"));
                    return(false);
                }

                for(int i=0;i<count;i++)
                {
                    *this->access++=v.x;
                    *this->access++=v.y;
                }

                return(true);
            }

            bool WriteLine(const T start_x,const T start_y,const T end_x,const T end_y)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::WriteLine(T,T,T,T) out"));
                    return(false);
                }

                *this->access++=start_x;
                *this->access++=start_y;
                *this->access++=end_x;
                *this->access++=end_y;

                return(true);
            }

            bool WriteLine(const Vector2f &start,const Vector2f &end)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::WriteLine(vec2,vec2) out"));
                    return(false);
                }

                *this->access++=start.x;
                *this->access++=start.y;
                *this->access++=end.x;
                *this->access++=end.y;

                return(true);
            }

            /**
             * 写入2D三角形
             */
            bool WriteTriangle(const Vector2f &v1,const Vector2f &v2,const Vector2f &v3)
            {
                if(!this->access||this->access+6>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::WriteTriangle(vec2,vec2,vec2) out"));
                    return(false);
                }

                *this->access++=v1.x;
                *this->access++=v1.y;

                *this->access++=v2.x;
                *this->access++=v2.y;

                *this->access++=v3.x;
                *this->access++=v3.y;

                return(true);
            }

            /**
             * 写入2D三角形
             */
            bool WriteTriangle(const Vector2f *v)
            {
                if(!this->access||this->access+6>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer2::WriteTriangle(vec2 *) out"));
                    return(false);
                }

                *this->access++=v->x;
                *this->access++=v->y;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;

                return(true);
            }

            /**
             * 写入2D四边形坐标数据
             */
            bool WriteQuad(const Vector2f &lt,const Vector2f &rt,const Vector2f &rb,const Vector2f &lb)
            {
                if(WriteTriangle(lt,lb,rb))
                    if(WriteTriangle(lt,rb,rt))
                        return(true);

                    //                 LOG_HINT(OS_TEXT("VertexBuffer2::WriteQuad(vec2 &,vec2 &,vec2 &,vec2 &) error"));
                    return(false);
            }

            /**
             * 写入2D矩形（两个三角形）坐标数据
             */
            bool WriteRect(const T left,const T top,const T width,const T height)
            {
                const Vector2f lt(left		,top);
                const Vector2f rt(left+width,top);
                const Vector2f rb(left+width,top+height);
                const Vector2f lb(left		,top+height);

                return WriteQuad(lt,rt,rb,lb);
            }
        };//class VertexBuffer2

        /**
         * 三元数据缓冲区
         */
        template<typename T,uint DT> class VertexBuffer3:public VertexBuffer<T,3>
        {
        public:

            VertexBuffer3(const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer<T,3>(GL_ARRAY_BUFFER,DT,_size,_data,level){}
            virtual ~VertexBuffer3()=default;

            /**
             * 计算绑定盒
             * @param min_vertex 最小值坐标
             * @param max_vertex 最大值坐标
             */
            template<typename V>
            void GetBoundingBox(V &min_vertex,V &max_vertex)
            {
                T *p=this->mem_type;

                //先以corner为最小值,length为最大值，求取最小最大值
                min_vertex.x=*p++;
                min_vertex.y=*p++;
                min_vertex.z=*p++;

                max_vertex=min_vertex;

                for(int i=1;i<this->count;i++)
                {
                    if(*p<min_vertex.x)min_vertex.x=*p;
                    if(*p>max_vertex.x)max_vertex.x=*p;
                    ++p;

                    if(*p<min_vertex.y)min_vertex.y=*p;
                    if(*p>max_vertex.y)max_vertex.y=*p;
                    ++p;

                    if(*p<min_vertex.z)min_vertex.z=*p;
                    if(*p>max_vertex.z)max_vertex.z=*p;
                    ++p;
                }
            }

            bool Write(const T v1,const T v2,const T v3)
            {
                if(!this->access||this->access+3>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(T,T,T) out"));
                    return(false);
                }

                *this->access++=v1;
                *this->access++=v2;
                *this->access++=v3;

                return(true);
            }

            bool Write(const T *v)
            {
                if(!this->access||this->access+3>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(T *) out"));
                    return(false);
                }

                *this->access++=*v++;
                *this->access++=*v++;
                *this->access++=*v;

                return(true);
            }

            bool Write(const Vector3f &v)
            {
                if(!this->access||this->access+3>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(vec3 &) out"));
                    return(false);
                }

                *this->access++=v.x;
                *this->access++=v.y;
                *this->access++=v.z;

                return(true);
            }

            bool Write(const Vector4f &v)
            {
                if(!this->access||this->access+3>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(vec4 &) out"));
                    return(false);
                }

                *this->access++=v.x;
                *this->access++=v.y;
                *this->access++=v.z;

                return(true);
            }

            /**
             * 将一个值重复多次写入缓冲区
             * @param v 值
             * @param count 写入数量
             */
            bool Write(const Vector3f &v,const int count)
            {
                if(!this->access||this->access+(count*3)>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(const Vector3f,")+OSString(count)+OS_TEXT(") out"));
                    return(false);
                }

                for(int i=0;i<count;i++)
                {
                    *this->access++=v.x;
                    *this->access++=v.y;
                    *this->access++=v.z;
                }

                return(true);
            }

            bool Write(const Color3f &v)
            {
                if(!this->access||this->access+3>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::Write(color3f &) out"));
                    return(false);
                }

                *this->access++=v.r;
                *this->access++=v.g;
                *this->access++=v.b;

                return(true);
            }

            bool WriteLine(const T start_x,const T start_y,const T start_z,const T end_x,const T end_y,const T end_z)
            {
                if(!this->access||this->access+6>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::WriteLine(T,T,T,T,T,T) out"));
                    return(false);
                }

                *this->access++=start_x;
                *this->access++=start_y;
                *this->access++=start_z;
                *this->access++=end_x;
                *this->access++=end_y;
                *this->access++=end_z;

                return(true);
            }

            bool WriteLine(const Vector3f &start,const Vector3f &end)
            {
                if(!this->access||this->access+6>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::WriteLine(vec3,vec3) out"));
                    return(false);
                }

                *this->access++=start.x;
                *this->access++=start.y;
                *this->access++=start.z;
                *this->access++=end.x;
                *this->access++=end.y;
                *this->access++=end.z;

                return(true);
            }

            /**
             * 写入3D三角形
             */
            bool WriteTriangle(const Vector3f &v1,const Vector3f &v2,const Vector3f &v3)
            {
                if(!this->access||this->access+9>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::WriteTriangle(vec3,vec3,vec3) out"));
                    return(false);
                }

                *this->access++=v1.x;
                *this->access++=v1.y;
                *this->access++=v1.z;

                *this->access++=v2.x;
                *this->access++=v2.y;
                *this->access++=v2.z;

                *this->access++=v3.x;
                *this->access++=v3.y;
                *this->access++=v3.z;

                return(true);
            }

            /**
             * 写入3D三角形
             */
            bool WriteTriangle(const Vector3f *v)
            {
                if(!this->access||this->access+9>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer3::WriteTriangle(vec3 *) out"));
                    return(false);
                }

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;

                return(true);
            }

            /**
             * 写入3D四边形坐标数据
             */
            bool WriteQuad(const Vector3f &lt,const Vector3f &rt,const Vector3f &rb,const Vector3f &lb)
            {
                if(WriteTriangle(lt,lb,rb))
                    if(WriteTriangle(lt,rb,rt))
                        return(true);

                    //                 LOG_HINT(OS_TEXT("VertexBuffer3::WriteQuad(vec3 &,vec3 &,vec3 &,vec3 &) error"));
                    return(false);
            }
        };//class VertexBuffer3

        /**
         * 四元数据缓冲区
         */
        template<typename T,uint DT> class VertexBuffer4:public VertexBuffer<T,4>
        {
        public:

            VertexBuffer4(const int _size,const T *_data=nullptr,uint level=GL_STATIC_DRAW):VertexBuffer<T,4>(GL_ARRAY_BUFFER,DT,_size,_data,level){}
            virtual ~VertexBuffer4()=default;

            /**
             * 计算绑定盒
             * @param min_vertex 最小值坐标
             * @param max_vertex 最大值坐标
             */
            template<typename V>
            void GetBoundingBox(V &min_vertex,V &max_vertex)
            {
                T *p=this->mem_type;

                //先以corner为最小值,length为最大值，求取最小最大值
                min_vertex.x=*p++;
                min_vertex.y=*p++;
                min_vertex.z=*p++;

                max_vertex=min_vertex;

                for(int i=1;i<this->count;i++)
                {
                    if(*p<min_vertex.x)min_vertex.x=*p;
                    if(*p>max_vertex.x)max_vertex.x=*p;
                    ++p;

                    if(*p<min_vertex.y)min_vertex.y=*p;
                    if(*p>max_vertex.y)max_vertex.y=*p;
                    ++p;

                    if(*p<min_vertex.z)min_vertex.z=*p;
                    if(*p>max_vertex.z)max_vertex.z=*p;
                    ++p;
                }
            }

            bool Write(const T v1,const T v2,const T v3,const T v4)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::Write(T,T,T,T) out"));
                    return(false);
                }

                *this->access++=v1;
                *this->access++=v2;
                *this->access++=v3;
                *this->access++=v4;

                return(true);
            }

            bool Write(const T *v)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::Write(T *) out"));
                    return(false);
                }

                *this->access++=*v++;
                *this->access++=*v++;
                *this->access++=*v++;
                *this->access++=*v;

                return(true);
            }

            bool Write(const Vector4f &v)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::Write(color4 &) out"));
                    return(false);
                }

                *this->access++=v.x;
                *this->access++=v.y;
                *this->access++=v.z;
                *this->access++=v.w;

                return(true);
            }

            bool Write(const Color4f &v)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::Write(color4 &) out"));
                    return(false);
                }

                *this->access++=v.r;
                *this->access++=v.g;
                *this->access++=v.b;
                *this->access++=v.a;

                return(true);
            }

            /**
             * 将一个值重复多次写入缓冲区
             * @param v 值
             * @param count 写入数量
             */
            bool Write(const Vector4f &v,const int count)
            {
                if(!this->access||this->access+(count<<2)>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::Write(const Vector4f,")+OSString(count)+OS_TEXT(") out"));
                    return(false);
                }

                for(int i=0;i<count;i++)
                {
                    *this->access++=v.x;
                    *this->access++=v.y;
                    *this->access++=v.z;
                    *this->access++=v.w;
                }

                return(true);
            }

            bool WriteLine(const T start_x,const T start_y,const T start_z,const T end_x,const T end_y,const T end_z)
            {
                if(!this->access||this->access+8>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteLine(T,T,T,T,T,T) out"));
                    return(false);
                }

                *this->access++=start_x;
                *this->access++=start_y;
                *this->access++=start_z;
                *this->access++=1.0f;
                *this->access++=end_x;
                *this->access++=end_y;
                *this->access++=end_z;
                *this->access++=1.0f;

                return(true);
            }

            bool WriteLine(const Vector3f &start,const Vector3f &end)
            {
                if(!this->access||this->access+8>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteLine(vec3,vec3) out"));
                    return(false);
                }

                *this->access++=start.x;
                *this->access++=start.y;
                *this->access++=start.z;
                *this->access++=1.0f;
                *this->access++=end.x;
                *this->access++=end.y;
                *this->access++=end.z;
                *this->access++=1.0f;

                return(true);
            }

            /**
             * 写入3D三角形
             */
            bool WriteTriangle(const Vector3f &v1,const Vector3f &v2,const Vector3f &v3)
            {
                if(!this->access||this->access+12>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteTriangle(vec3,vec3,vec3) out"));
                    return(false);
                }

                *this->access++=v1.x;
                *this->access++=v1.y;
                *this->access++=v1.z;
                *this->access++=1.0f;

                *this->access++=v2.x;
                *this->access++=v2.y;
                *this->access++=v2.z;
                *this->access++=1.0f;

                *this->access++=v3.x;
                *this->access++=v3.y;
                *this->access++=v3.z;
                *this->access++=1.0f;

                return(true);
            }

            /**
             * 写入3D三角形
             */
            bool WriteTriangle(const Vector3f *v)
            {
                if(!this->access||this->access+12>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteTriangle(vec3 *) out"));
                    return(false);
                }

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;
                *this->access++=1.0f;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;
                *this->access++=1.0f;
                ++v;

                *this->access++=v->x;
                *this->access++=v->y;
                *this->access++=v->z;
                *this->access++=1.0f;

                return(true);
            }

            /**
             * 写入2D矩形,注:这个函数会依次写入Left,Top,Width,Height四个值
             */
            template<typename V>
            bool WriteRectangle2D(const RectScope2<V> &rect)
            {
                if(!this->access||this->access+4>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteRectangle2D(RectScope2 ) out"));
                    return(false);
                }

                *this->access++=rect.Left;
                *this->access++=rect.Top;
                *this->access++=rect.Width;
                *this->access++=rect.Height;

                return(true);
            }

            /**
             * 写入2D矩形,注:这个函数会依次写入Left,Top,Width,Height四个值
             */
            template<typename V>
            bool WriteRectangle2D(const RectScope2<V> *rect,const int count)
            {
                if(!this->access||this->access+(4*count)>this->mem_end)
                {
                    //                     LOG_HINT(OS_TEXT("VertexBuffer4::WriteRectangle2D(RectScope2 *,count) out"));
                    return(false);
                }

                for(int i=0;i<count;i++)
                {
                    *this->access++=rect->Left;
                    *this->access++=rect->Top;
                    *this->access++=rect->Width;
                    *this->access++=rect->Height;

                    ++rect;
                }

                return(true);
            }
        };//class VertexBuffer4

        //缓冲区具体数据类型定义
        using EB16=ElementBuffer<uint16,GL_UNSIGNED_SHORT   >;
        using EB32=ElementBuffer<uint32,GL_UNSIGNED_INT     >;

#define USING_VB1234(type,gl_type,short_name)   using VB1##short_name=VertexBuffer1<type,gl_type>;  \
                                                using VB2##short_name=VertexBuffer2<type,gl_type>;  \
                                                using VB3##short_name=VertexBuffer3<type,gl_type>;  \
                                                using VB4##short_name=VertexBuffer4<type,gl_type>;

        USING_VB1234(int8,  GL_BYTE,    i8)
        USING_VB1234(int8,  GL_BYTE,    b)
        USING_VB1234(int16, GL_SHORT,   i16)
        USING_VB1234(int16, GL_SHORT,   s)
        USING_VB1234(int32, GL_INT,     i32)
        USING_VB1234(int32, GL_INT,     i)

        USING_VB1234(int8,  GL_UNSIGNED_BYTE,   u8)
        USING_VB1234(int8,  GL_UNSIGNED_BYTE,   ub)
        USING_VB1234(int16, GL_UNSIGNED_SHORT,  u16)
        USING_VB1234(int16, GL_UNSIGNED_SHORT,  us)
        USING_VB1234(int32, GL_UNSIGNED_INT,    u32)
        USING_VB1234(int32, GL_UNSIGNED_INT,    ui)

        USING_VB1234(float, GL_FLOAT,   f)
        USING_VB1234(double,GL_DOUBLE,  d)

#undef USING_VB1234

    }//namespace graph
}//namespace hgl
#endif//HGL_VERTEX_BUFFER_OBJECT_INCLUDE