#include<hgl/graph/vulkan/VKSemaphore.h>
VK_NAMESPACE_BEGIN
Semaphore::~Semaphore()
{
    vkDestroySemaphore(device,sem,nullptr);
}
VK_NAMESPACE_END