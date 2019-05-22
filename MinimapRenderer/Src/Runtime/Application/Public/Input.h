#pragma once
#include "Cursor.h"

namespace InputEvent {
	enum Event {
		RBUTTONDOWN,
		LBUTTONDOWN,
		RBUTTONUP,
		LBUTTONUP,
		MOUSEMOVE,
		KEYDOWN,
		KEYUP
	};
}

class Input
{
public:
	static bool IsKeyDown(uint32_t);
	static std::pair<INT32, INT32> GetMouseDelta();
	static std::pair<INT32, INT32> GetMouseScreenSpacePosition();

private:
	friend class WindowsApplication;

	Input();
	~Input();
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(Input);

	static Input* Get();
	void UpdateInput(const InputEvent::Event&, WPARAM, std::pair<INT32, INT32> = { 0, 0 });
	void KeyDown(uint32_t);
	void KeyUp(uint32_t);
	void ResetMouseDelta();

	static Input* m_Instance; //must be defined in .cpp file as it is static
	Cursor m_Cursor;
	InputEvent::Event m_InputEvent;
	std::pair<INT32, INT32> m_MouseDelta;
	std::pair<INT32, INT32> m_MouseScreenSpacePosition;
	std::vector<bool> m_Keys;
};

