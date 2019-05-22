#pragma once

#include "D3D11Util.h"

#include "SimpleMath.h"
#include "Misc/RefCounting.h"

#define CB_DRAW_SIZE 128
#define CB_FRAME_SIZE 80
#define CB_APPLICATION_SIZE 128

enum ConstanBuffer
{
	CB_GLOBALS,
	CB_DRAW,
	CB_FRAME,
	CB_APPLICATION,
	CB_COUNT
};

class D3D11ConstantBuffer : public RefCountedObject
{
public:
	D3D11ConstantBuffer();
	~D3D11ConstantBuffer();
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(D3D11ConstantBuffer);

	HRESULT Initialize(ID3D11Device* device, UINT32 bufferSize);
	ID3D11Buffer* Get() const { return m_ConstantBuffer.Get(); }
	ID3D11Buffer** GetAddressOf() { return m_ConstantBuffer.GetAddressOf(); }

private:
	ComPtr<ID3D11Buffer> m_ConstantBuffer;
	UINT32 m_BufferSize;

};
