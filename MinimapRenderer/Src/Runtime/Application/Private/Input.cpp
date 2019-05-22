#include "Input.h"

Input* Input::m_Instance;

Input::Input() :
	m_MouseDelta{0, 0},
	m_Keys(256, false)
{}


Input::~Input()
{}

Input* Input::Get()
{
	if (!m_Instance)
	{
		m_Instance = new Input();
	}
	return m_Instance;
}

void Input::UpdateInput(const InputEvent::Event& currentEvent, WPARAM wparam, std::pair<INT32, INT32> screenSpaceMousePosition)
{
	static std::pair<INT32, INT32> mouseStart;        // static means this persists through function calls, and it also forces initialization
	std::pair<INT32, INT32> mousePosition = m_Cursor.GetPosition();

	m_InputEvent = currentEvent;
	ResetMouseDelta();

	switch (currentEvent)
	{
	case InputEvent::RBUTTONDOWN:
	{
		m_Cursor.SetVisible(false);
		mouseStart = mousePosition;
		break;
	}

	case InputEvent::RBUTTONUP:
	{
		m_Cursor.SetVisible(true);
		m_Cursor.SetPosition(mouseStart.first, mouseStart.second);
		break;
	}

	case InputEvent::MOUSEMOVE:
	{
		if (wparam & MK_RBUTTON)
		{
			INT32 deltaX = mousePosition.first - mouseStart.first;
			INT32 deltaY = mousePosition.second - mouseStart.second;
			m_MouseDelta = { deltaX, deltaY };
			m_Cursor.SetPosition(mouseStart.first, mouseStart.second);
		}
		break;
	}

	case InputEvent::KEYDOWN:
	{
		KeyDown((UINT32)wparam);
		break;
	}

	case InputEvent::KEYUP:
	{
		KeyUp((UINT32)wparam);
		break;
	}
	}
}

void Input::KeyDown(UINT32 input)
{
	m_Keys[input] = true;
}

void Input::KeyUp(UINT32 input)
{
	m_Keys[input] = false;
}

void Input::ResetMouseDelta()
{
	m_MouseDelta.first = 0;
	m_MouseDelta.second = 0;
}


bool Input::IsKeyDown(UINT32 key)
{
	if (m_Instance) {
		return m_Instance->m_Keys[key];
	}
	else {
		DebugLog("s", "Tried to read input key with no instance of Input");
		assert(0);
	}
}

std::pair<INT32, INT32> Input::GetMouseDelta()
{
	if (m_Instance) {
		return m_Instance->m_MouseDelta;
	}
	else {
		DebugLog("s", "Tried to read mouse delta with no instance of Input");
		assert(0);
	}
}

std::pair<INT32, INT32> Input::GetMouseScreenSpacePosition()
{
	if (m_Instance) {
		return m_Instance->m_MouseScreenSpacePosition;
	}
	else {
		DebugLog("s", "Tried to read mouse screen space position with no instance of Input");
		assert(0);
	}
}

