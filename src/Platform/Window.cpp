#include<hgl/platform/Window.h>
namespace hgl
{
    void Window::OnKeyDown(KeyboardButton kb)
    {
        if(key_push[kb])
            OnKeyPress(kb);
        else
        {
            OnKeyDown(kb);
            key_push[kb]=true;
        }
    }

    void Window::OnKeyUp(KeyboardButton kb)
    {
        OnKeyUp(kb);
        key_push[kb]=false;
    }

    bool Window::Update()
    {
        while(MessageProc())
        {
        }

        if(!active||width==0||height==0)
            this->WaitMessage();

        return(!is_close);
    }
}//namespace hgl