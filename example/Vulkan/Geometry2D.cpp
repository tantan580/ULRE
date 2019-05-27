﻿// 3.Geometry2D
// 该范例有两个作用：
//                  一、测试绘制2D几何体
//                  二、试验动态合并材质渲染机制、包括普通合并与Instance

#include"VulkanAppFramework.h"
#include<hgl/filesystem/FileSystem.h>
#include<hgl/graph/InlineGeometry.h>
#include<hgl/graph/SceneDB.h>
#include<hgl/graph/RenderableInstance.h>
#include<hgl/graph/RenderList.h>

using namespace hgl;
using namespace hgl::graph;

bool SaveToFile(const OSString &filename,VK_NAMESPACE::PipelineCreater *pc);
bool LoadFromFile(const OSString &filename,VK_NAMESPACE::PipelineCreater *pc);

constexpr uint32_t SCREEN_WIDTH=128;
constexpr uint32_t SCREEN_HEIGHT=128;

struct WorldConfig
{
    Matrix4f mvp;
}world;

class TestApp:public VulkanApplicationFramework
{
private:

    uint swap_chain_count=0;

            SceneDB *           db                  =nullptr;
            SceneNode *         render_root         =nullptr;
            RenderList *        render_list         =nullptr;

    vulkan::Material *          material            =nullptr;
    vulkan::DescriptorSets *    descriptor_sets     =nullptr;
    
    vulkan::Renderable          *ro_rectangle       =nullptr,
                                *ro_circle          =nullptr,
                                *ro_round_rectangle =nullptr;

    vulkan::Buffer *            ubo_mvp             =nullptr;

    vulkan::Pipeline *          pipeline            =nullptr;
    vulkan::CommandBuffer **    cmd_buf             =nullptr;

public:

    ~TestApp()
    {
        SAFE_CLEAR(render_list);
        SAFE_CLEAR(render_root);
        SAFE_CLEAR(db);

        SAFE_CLEAR_OBJECT_ARRAY(cmd_buf,swap_chain_count);
    }

private:

    bool InitMaterial()
    {
        material=shader_manage->CreateMaterial(OS_TEXT("OnlyPosition.vert.spv"),
                                               OS_TEXT("FlatColor.frag.spv"));
        if(!material)
            return(false);

        descriptor_sets=material->CreateDescriptorSets();

        db->Add(material);
        db->Add(descriptor_sets);
        return(true);
    }

    void CreateRenderObject()
    {
        {
            struct RectangleCreateInfo rci;

            rci.scope.Set(10,10,20,20);

            ro_rectangle=CreateRectangle(db,material,&rci);
        }

        {
            struct RoundRectangleCreateInfo rrci;

            rrci.scope.Set(SCREEN_WIDTH-30,10,20,20);
            rrci.radius=5;
            rrci.round_per=5;

            ro_round_rectangle=CreateRoundRectangle(db,material,&rrci);
        }

        {
            struct CircleCreateInfo cci;

            cci.center.x=SCREEN_WIDTH/2;
            cci.center.y=SCREEN_HEIGHT/2;

            cci.radius.x=SCREEN_WIDTH*0.35;
            cci.radius.y=SCREEN_HEIGHT*0.35;

            cci.field_count=8;

            ro_circle=CreateCircle(db,material,&cci);
        }
    }

    bool InitUBO()
    {
        const VkExtent2D extent=device->GetExtent();

        world.mvp=ortho(extent.width,extent.height);

        ubo_mvp=db->CreateUBO(sizeof(WorldConfig),&world);

        if(!ubo_mvp)
            return(false);

        if(!descriptor_sets->BindUBO(material->GetUBO("world"),*ubo_mvp))
            return(false);

        descriptor_sets->Update();
        return(true);
    }
    
    bool InitPipeline()
    {
        constexpr os_char PIPELINE_FILENAME[]=OS_TEXT("2DSolid.pipeline");

        {
            vulkan::PipelineCreater *pipeline_creater=new vulkan::PipelineCreater(device,material,device->GetRenderPass(),device->GetExtent());
            pipeline_creater->SetDepthTest(false);
            pipeline_creater->SetDepthWrite(false);
            pipeline_creater->CloseCullFace();
            pipeline_creater->Set(PRIM_TRIANGLE_FAN);

            pipeline=pipeline_creater->Create();
            db->Add(pipeline);
            delete pipeline_creater;
        }

        return pipeline;
    }

    bool InitScene()
    {

        render_root=new SceneNode();
        render_list=new RenderList();

        render_root->Add(db->CreateRenderableInstance(pipeline,descriptor_sets,ro_rectangle));
        render_root->Add(db->CreateRenderableInstance(pipeline,descriptor_sets,ro_round_rectangle));
        render_root->Add(db->CreateRenderableInstance(pipeline,descriptor_sets,ro_circle));

        render_root->ExpendToList(render_list);

        return(true);
    }

    bool InitCommandBuffer()
    {
        cmd_buf=hgl_zero_new<vulkan::CommandBuffer *>(swap_chain_count);

        for(uint i=0;i<swap_chain_count;i++)
        {
            cmd_buf[i]=device->CreateCommandBuffer();

            if(!cmd_buf[i])
                return(false);

            cmd_buf[i]->Begin();
                cmd_buf[i]->BeginRenderPass(device->GetRenderPass(),device->GetFramebuffer(i));
                    render_list->Render(cmd_buf[i]);
                cmd_buf[i]->EndRenderPass();
            cmd_buf[i]->End();
        }

        return(true);
    }

public:

    bool Init()
    {
        if(!VulkanApplicationFramework::Init(SCREEN_WIDTH,SCREEN_HEIGHT))
            return(false);

        swap_chain_count=device->GetSwapChainImageCount();

        db=new SceneDB(device);

        if(!InitMaterial())
            return(false);

        CreateRenderObject();

        if(!InitUBO())
            return(false);
            
        if(!InitPipeline())
            return(false);

        if(!InitScene())
            return(false);

        if(!InitCommandBuffer())
            return(false);

        return(true);
    }

    void Draw() override
    {
        const uint32_t frame_index=device->GetCurrentFrameIndices();

        const vulkan::CommandBuffer *cb=cmd_buf[frame_index];

        Submit(*cb);
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
