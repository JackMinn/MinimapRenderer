#pragma once

class WindowsApplication
{
public:
	WindowsApplication() {}

	~WindowsApplication();

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(WindowsApplication);

	void CreateApplication(HINSTANCE const instanceHandle, LPCWSTR applicationName, void* gameEngine);
	void ShutdownApplication();
	void ProcessMessagesLoop();
	bool GetClientDimensions(LONG& width, LONG& height);
	HWND GetWindowHandle() const { return m_WindowHandle; }

private:
	void RegisterWindow(HINSTANCE const instanceHandle, HICON const iconHandle, LPCWSTR windowClassName);
	void CreateAndShowWindow(void* const gameEngine);

	static LRESULT CALLBACK EngineMessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	LPCWSTR m_ApplicationName;
	HINSTANCE m_InstanceHandle;
	HWND m_WindowHandle;
};

