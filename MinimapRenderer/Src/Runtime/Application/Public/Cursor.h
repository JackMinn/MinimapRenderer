#pragma once

class Cursor
{
public:
	Cursor()
	{}

	~Cursor()
	{}

	inline std::pair<INT32, INT32> GetPosition() const
	{
		POINT CursorPos;
		::GetCursorPos(&CursorPos);

		return { CursorPos.x, CursorPos.y };
	}

	inline void SetPosition(const INT32& x, const INT32& y)
	{
		::SetCursorPos(x, y);
	}

	inline void SetVisible(bool visible)
	{
		if (visible)
		{
			while (::ShowCursor(true) < 0);
		}
		else
		{		
			while (::ShowCursor(false) >= 0);
		}
	}

	inline void LockInPosition()
	{
		std::pair<INT32, INT32> pos = GetPosition();
		RECT bounds = { pos.first, pos.second, pos.first, pos.second };
		::ClipCursor(&bounds);
	}

	inline bool GetCursorBounds(RECT* const pRect)
	{
		return ::GetClipCursor(pRect);
	}

	inline bool SetCursorBounds(const RECT* const pRect)
	{
		return ::ClipCursor(pRect);
	}
};

