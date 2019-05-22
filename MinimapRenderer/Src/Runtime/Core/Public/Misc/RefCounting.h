#pragma once

class RefCountedObject
{
public:
	RefCountedObject() : m_RefCount(0) {}
	virtual ~RefCountedObject() { assert(!m_RefCount); }
	UINT32 AddRef() const
	{
		return UINT32(++m_RefCount);
	}
	UINT32 Release() const
	{
		UINT32 refs = UINT32(--m_RefCount);
		if (refs == 0)
		{
			delete this;
		}
		return refs;
	}
	UINT32 GetRefCount() const
	{
		return UINT32(m_RefCount);
	}
private:
	mutable INT32 m_RefCount;
};

