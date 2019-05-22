#include "Windows/WindowsApplication.h"
#include "Misc/Utilities.h"

#include "Input.h"
#include "Engine/RenderEngine.h"
extern RenderEngine g_RenderEngine;

WindowsApplication::~WindowsApplication()
{
}

//[https://stackoverflow.com/questions/11783086/get-exact-window-region-size-createwindow-window-size-isnt-correct-size-of-wi]
//CreateWindowEx in Windows 10 is not giving me the desired window dimensions
//Probably best to just have small dimensions and start maximized like most applications, starting with the working area is a good idea
void WindowsApplication::CreateApplication(HINSTANCE const instanceHandle, LPCWSTR applicationName, void* const gameEngine)
{
	m_InstanceHandle = instanceHandle;
	m_ApplicationName = applicationName;
	RegisterWindow(instanceHandle, LoadIcon(NULL, IDI_WINLOGO), m_ApplicationName);
	CreateAndShowWindow(gameEngine);
}

void WindowsApplication::ShutdownApplication()
{
	Input* input = Input::Get();
	if (input)
	{
		delete input;
	}

	::DestroyWindow(m_WindowHandle);
	m_WindowHandle = NULL;

	::UnregisterClass(m_ApplicationName, m_InstanceHandle);
	m_InstanceHandle = NULL;
}

void WindowsApplication::RegisterWindow(HINSTANCE const instanceHandle, HICON const iconHandle, LPCWSTR windowClassName)
{
	WNDCLASSEX wc = { 0 };

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instanceHandle;
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);
}

void WindowsApplication::CreateAndShowWindow(void* const gameEngine)
{
	HMONITOR Monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(Monitor, &MonitorInfo);

	const LONG workingWidth = MonitorInfo.rcWork.right - MonitorInfo.rcWork.left;
	const LONG workingHeight = MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top;

	auto windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	auto windowStyleEx = WS_EX_APPWINDOW;

	LONG windowXPos, windowYPos, windowWidth, windowHeight;
	windowWidth = workingWidth;
	windowHeight = workingHeight;
	windowXPos = 0;
	windowYPos = 0;

	m_WindowHandle = ::CreateWindowEx(windowStyleEx,
		m_ApplicationName,
		m_ApplicationName,
		windowStyle,
		windowXPos,
		windowYPos,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		m_InstanceHandle,
		gameEngine);

	// We will start the window maximized upon initial show.
	::ShowWindow(m_WindowHandle, SW_SHOWMAXIMIZED);
	//::ShowWindow(m_WindowHandle, SW_SHOW);
	::SetForegroundWindow(m_WindowHandle);
	::SetFocus(m_WindowHandle);
}

void WindowsApplication::ProcessMessagesLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	bool done = false, result = true;

	done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			result = g_RenderEngine.RenderFrame();
			if (!result)
			{
				done = true;
			}
		}
	}
}

bool WindowsApplication::GetClientDimensions(LONG& width, LONG& height)
{
	RECT clientRect = { 0, 0, 0, 0 };
	bool result = ::GetClientRect(m_WindowHandle, &clientRect);
	if (result) {
		width = clientRect.right - clientRect.left;
		height = clientRect.bottom - clientRect.top;
	}
	
	return result;
}

LRESULT CALLBACK WindowsApplication::EngineMessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	POINT mouse;
	mouse.x = GET_X_LPARAM(lparam);
	mouse.y = GET_Y_LPARAM(lparam);

	std::pair<INT32, INT32> temp = { mouse.x, mouse.y };

	Input* input = Input::Get();
	switch (umsg)
	{
	case WM_RBUTTONDOWN:
	{
		SetCapture(hwnd);
		input->UpdateInput(InputEvent::RBUTTONDOWN, wparam, temp);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		input->UpdateInput(InputEvent::MOUSEMOVE, wparam, temp);
		return 0;
	}

	case WM_RBUTTONUP:
	{
		input->UpdateInput(InputEvent::RBUTTONUP, wparam, temp);
		ReleaseCapture();
		return 0;
	}

	case WM_KEYDOWN:
	{
		input->KeyDown((UINT32)wparam);
		return 0;
	}

	case WM_KEYUP:
	{
		input->KeyUp((UINT32)wparam);
		return 0;
	}

	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

LRESULT CALLBACK WindowsApplication::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_CREATE:
	{
		return 0;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	default:
	{
		return EngineMessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
