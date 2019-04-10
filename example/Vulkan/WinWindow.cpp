﻿#include"Window.h"
#include<Windows.h>
#include<vulkan/vk_sdk_platform.h>
#include<vulkan/vulkan.h>
#include<vulkan/vulkan_win32.h>

namespace hgl
{
	namespace graph
	{
		namespace
		{
			constexpr wchar_t WIN_CLASS_NAME[] = L"CMGameEngine/ULRE Window Class";

			LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
			{
				Window *win=(Window *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

				switch (uMsg) 
				{
					case WM_CLOSE:
						PostQuitMessage(0);
						break;
					default:
						break;
				}

				return (DefWindowProcW(hWnd, uMsg, wParam, lParam));
			}
		}

		/**
		 * Windows平台窗口实现
		 */
		class WinWindow :public Window
		{
			HWND win_hwnd=nullptr;
			HDC win_dc = nullptr;

		protected:

			bool Registry()
			{
				WNDCLASSEXW win_class;

				hgl_zero(win_class);

				win_class.cbSize = sizeof(WNDCLASSEXW);
				win_class.style = CS_HREDRAW | CS_VREDRAW;
				win_class.lpfnWndProc = WndProc;
				win_class.cbClsExtra = 0;
				win_class.cbWndExtra = 0;
				win_class.hInstance = GetModuleHandle(nullptr);
				win_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
				win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
				win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
				win_class.lpszMenuName = nullptr;
				win_class.lpszClassName = WIN_CLASS_NAME;
				win_class.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

				return RegisterClassExW(&win_class);
			}

			bool Create()
			{
				constexpr DWORD win_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU ;

				int win_left, win_top;
				int win_width, win_height;

				{
					RECT win_rect;

					win_rect.left = 0;
					win_rect.right = width;
					win_rect.top = 0;
					win_rect.bottom = height;

					AdjustWindowRectEx(&win_rect, win_style, false, 0); //计算窗口坐标

					win_width = win_rect.right - win_rect.left;
					win_height = win_rect.bottom - win_rect.top;
				}

				if (width && height)
				{
					win_left = (GetSystemMetrics(SM_CXSCREEN) - win_width) / 2;
					win_top = (GetSystemMetrics(SM_CYSCREEN) - win_height) / 2;
				}
				else
				{
					win_left = CW_USEDEFAULT;
					win_top = CW_USEDEFAULT;
				}

				win_hwnd = CreateWindowExW(0,
					WIN_CLASS_NAME,             // class name
					win_name.c_str(),           // app name
					win_style,					// window style
					win_left,win_top,			// x/y coords
					win_width,					// width
					win_height,					// height
					nullptr,					// handle to parent
					nullptr,					// handle to menu
					GetModuleHandleW(nullptr),  // hInstance
					nullptr);					// no extra parameters

				if (!win_hwnd)
				{
					UnregisterClassW(WIN_CLASS_NAME, GetModuleHandleW(nullptr));
					return(false);
				}

				win_dc = GetDC(win_hwnd);
				SetWindowLongPtrW(win_hwnd, GWLP_USERDATA, (LONG_PTR)this);
				return(true);
			}

		public:

			using Window::Window;
			~WinWindow()
			{
				Close();
			}

			const char* GetVulkanSurfaceExtname()const
			{
				return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}

			bool Create(uint w, uint h) override
			{
				width = w;
				height = h;

				if (!Registry())
					return(false);

				return Create();
			}

			bool Create(uint, uint, uint)override 
			{
				return(false);
			}

			void Close()override
			{
				ReleaseDC(win_hwnd, win_dc);
				DestroyWindow(win_hwnd);
				UnregisterClassW(WIN_CLASS_NAME, GetModuleHandleW(nullptr));

				win_dc = nullptr;
				win_hwnd = nullptr;
			}

			void Show()override 
			{
				ShowWindow(win_hwnd, SW_SHOW);
				SetForegroundWindow(win_hwnd);
				SetFocus(win_hwnd);

				UpdateWindow(win_hwnd);
			}

			void Hide()override 
			{
				ShowWindow(win_hwnd, SW_HIDE);
				UpdateWindow(win_hwnd);
			}
		};//class WinWindow :public Window

		Window* CreateRenderWindow(const WideString& win_name)
		{
			return(new WinWindow(win_name));
		}
	}//namespace graph
}//namespace hgl
