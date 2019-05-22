#include "D3D11/D3D11ConstantBuffer.h"

D3D11ConstantBuffer::D3D11ConstantBuffer() :
	m_ConstantBuffer(nullptr),
	m_BufferSize(0)
{
}

D3D11ConstantBuffer::~D3D11ConstantBuffer()
{}

HRESULT D3D11ConstantBuffer::Initialize(ID3D11Device* device, UINT32 bufferSize)
{
	m_BufferSize = bufferSize;
	D3D11_BUFFER_DESC cBufferDesc;

	ZeroMemory(&cBufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cBufferDesc.ByteWidth = m_BufferSize;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = 0;

	HRESULT hres = device->CreateBuffer(&cBufferDesc, NULL, m_ConstantBuffer.GetAddressOf());
	if (FAILED(hres))
	{
		DebugLog("s", "Failed to create the buffer");
	}
	else
	{
		DebugLog("s", "Buffer created.");
	}
	return hres;
}