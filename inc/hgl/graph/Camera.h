﻿#ifndef HGL_GRAPH_CAMERA_INCLUDE
#define HGL_GRAPH_CAMERA_INCLUDE

#include<hgl/math/Math.h>
namespace hgl
{
    namespace graph
    {
        enum class CameraType
        {
            Perspective,
            Ortho
        };//

        /**
        * 摄像机数据结构
        */
        struct Camera
        {
            CameraType type=CameraType::Perspective;

            float width;                ///<视图宽
            float height;               ///<视图高

            float fov=60;               ///<水平FOV
            float znear=0,zfar=1000;    ///<Z轴上离眼睛的距离

            Vector4f eye;               ///<眼睛坐标
            Vector4f center;            ///<视点坐标
            Vector4f up_vector      =Vector4f(0,0,1,0); ///<向上量(默认0,0,1)
            Vector4f forward_vector =Vector4f(0,-1,0,0); ///<向前量(默认0,1,0)
            Vector4f right_vector   =Vector4f(1,0,0,0); ///<向右量(默认0,0,1)

        public:

            WorldMatrix matrix;

            Frustum frustum;

        public:

            void Refresh();
        };//struct Camera

        /**
        * 简单可控像机
        */
        struct ControlCamera:public Camera
        {
        protected:

            /**
            * 向指定向量移动
            * @param move_dist 移动距离
            */
            void Move(const Vector4f &move_dist)
            {
                eye+=move_dist;
                center+=move_dist;
            }

            /**
             * 以自身为中心旋转
             * @param ang 角度
             * @param axis 旋转轴
             */
            void Rotate(double ang,Vector4f axis)
            {
                normalize(axis);
                center=eye+(center-eye)*rotate(hgl_ang2rad(ang),axis);
            }

            /**
             * 以目标为中心旋转
             * @param ang 角度
             * @param axis 旋转轴
             */
             void WrapRotate(double ang,Vector4f axis)
             {
                normalize(axis);
                eye=center+(eye-center)*rotate(hgl_ang2rad(ang),axis);
             }

        public: //方法

            virtual void Backward(float step=0.01){Move((eye-center)*step/length(eye,center));}     ///<向后
            virtual void Forward(float step=0.01){Backward(-step);}                                 ///<向前

            virtual void Up(float step=0.01){Move(step*up_vector);}                                 ///<向上
            virtual void Down(float step=0.01){Up(-step);}                                          ///<向下

            virtual void Right(float step=0.01){Move(normalized(cross(center-eye,up_vector))*step);}///<向右
            virtual void Left(float step=0.01){Right(-step);}                                       ///<向左

        public: //以自身为中心旋转

            virtual void VertRotate(float ang=5){Rotate(ang,cross(center-eye,up_vector));}          ///<垂直方向前后旋转
            virtual void HorzRotate(float ang=5){Rotate(ang,up_vector);}                            ///<水平方向左右旋转

        public: //以目标点为中心旋转

            virtual void WrapVertRotate(float ang=5){WrapRotate(ang,cross(center-eye,up_vector));}  ///<以目标点为中心上下旋转
            virtual void WrapHorzRotate(float ang=5){WrapRotate(ang,up_vector);}                    ///<以目标点为中心左右旋转

        public: //距离

            virtual void Distance(float pos)                                                        ///<调整距离
            {
                if(pos==1.0)return;

                eye=center+(eye-center)*pos;
            }
        };//struct ControlCamera
    }//namespace graph
}//namespace hgl
#endif//HGL_GRAPH_CAMERA_INCLUDE
