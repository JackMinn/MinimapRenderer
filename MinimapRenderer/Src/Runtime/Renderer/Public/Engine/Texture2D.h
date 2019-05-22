#pragma once

#include "D3D11/D3D11Util.h"
#include "Misc/RefCounting.h"

class Texture2D : public RefCountedObject {
public:
	Texture2D() :
		m_Resource(nullptr),
		m_SRV(nullptr)
	{
		//if (m_Texture.Get()) {
		//	INT32 test = m_Texture.Get()->AddRef();
		//	DebugLog("si", "Constructor says current texture reference count", test - 1);
		//	m_Texture.Get()->Release();
		//}
	}

	~Texture2D()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(Texture2D);

public:
	ComPtr<ID3D11Resource> m_Resource;
	ComPtr<ID3D11ShaderResourceView> m_SRV;
};