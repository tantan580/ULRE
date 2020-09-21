﻿// 该范例主要用于测试gl_FragCoord值

#include"VulkanAppFramework.h"
#include<hgl/math/Math.h>
#include<hgl/filesystem/FileSystem.h>

using namespace hgl;
using namespace hgl::graph;

constexpr uint32_t SCREEN_WIDTH=128;
constexpr uint32_t SCREEN_HEIGHT=128;

constexpr uint32_t VERTEX_COUNT=4;

constexpr float vertex_data[VERTEX_COUNT][2]=
{
    {0,0},
    {SCREEN_WIDTH,0},
    {0,SCREEN_HEIGHT},
    {SCREEN_WIDTH,SCREEN_HEIGHT}
};

class TestApp:public VulkanApplicationFramework
{
private:

    Camera cam;

    vulkan::MaterialInstance *  material_instance   =nullptr;
    vulkan::Renderable *        render_obj          =nullptr;
    vulkan::Buffer *            ubo_world_matrix    =nullptr;

    vulkan::Pipeline *          pipeline            =nullptr;

    vulkan::VAB *               vertex_buffer       =nullptr;

public:

    ~TestApp()
    {
        SAFE_CLEAR(vertex_buffer);
    }

private:

    bool InitMaterial()
    {
        material_instance=db->CreateMaterialInstance(OS_TEXT("res/material/FragColor"));

        if(!material_instance)
            return(false);
            
        pipeline=CreatePipeline(material_instance,OS_TEXT("res/pipeline/solid2d"),Prim::TriangleStrip);

        if(!pipeline)
            return(false);

        return(true);
    }

    bool InitUBO()
    {
        const VkExtent2D extent=sc_render_target->GetExtent();

        cam.vp_width=cam.width=extent.width;
        cam.vp_height=cam.height=extent.height;        

        cam.Refresh();

        ubo_world_matrix=db->CreateUBO(sizeof(WorldMatrix),&cam.matrix);

        if(!ubo_world_matrix)
            return(false);

        if(!material_instance->BindUBO("world",ubo_world_matrix))return(false);
        if(!material_instance->BindUBO("frag_world",ubo_world_matrix))return(false);

        material_instance->Update();
        return(true);
    }

    bool InitVBO()
    {
        render_obj=db->CreateRenderable(material_instance,VERTEX_COUNT);
        if(!render_obj)return(false);

        vertex_buffer=device->CreateVAB(VAF_VEC2,VERTEX_COUNT,vertex_data);
        if(!vertex_buffer)return(false);

        if(!render_obj->Set(VAN::Position,vertex_buffer))return(false);

        return(true);
    }

public:

    bool Init()
    {
        if(!VulkanApplicationFramework::Init(SCREEN_WIDTH,SCREEN_HEIGHT))
            return(false);

        if(!InitMaterial())
            return(false);

        if(!InitUBO())
            return(false);

        if(!InitVBO())
            return(false);

        BuildCommandBuffer(pipeline,material_instance,render_obj);

        return(true);
    }

    void Resize(int w,int h)override
    {
        cam.vp_width=w;
        cam.vp_height=h;

        cam.Refresh();

        ubo_world_matrix->Write(&cam.matrix);

        BuildCommandBuffer(pipeline,material_instance,render_obj);
    }
};//class TestApp:public VulkanApplicationFramework

int main(int,char **)
{
    TestApp app;

    if(!app.Init())
        return(-1);

    while(app.Run());

    return 0;
}
