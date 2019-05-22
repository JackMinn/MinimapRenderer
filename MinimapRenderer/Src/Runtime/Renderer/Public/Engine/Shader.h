#pragma once

#include "Misc/RefCounting.h"
#include "D3D11/D3D11Util.h"

class Shader : public RefCountedObject {
public:
	Shader() :
		m_SamplerState(nullptr),
		m_VertexShader(nullptr),
		m_PixelShader(nullptr),
		m_InputLayout(nullptr),
		m_DepthTestEnabled(true),
		m_DepthTestFunc(D3D11_COMPARISON_LESS),
		m_CullMode(D3D11_CULL_BACK)
	{}

	virtual ~Shader()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(Shader);

	virtual bool Initialize(ID3D11Device* device, const std::string& vsFileName, const std::string& psFileName);
	virtual void Shutdown() {};

	ID3D11InputLayout* GetInputLayout() const { return m_InputLayout.Get(); }
	ID3D11VertexShader* GetVertexShader() const { return m_VertexShader.Get(); }
	ID3D11PixelShader* GetPixelShader() const { return m_PixelShader.Get(); }
	ID3D11SamplerState** GetAddressOfSamplerState() { return m_SamplerState.GetAddressOf(); }

private:
	static HRESULT GetInputLayoutFromVertexShaderSignature(ID3DBlob* shaderBlob, ID3D11Device* device, ID3D11InputLayout** inputLayout);

private:
	ComPtr<ID3D11SamplerState> m_SamplerState;
	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11PixelShader> m_PixelShader;
	ComPtr<ID3D11InputLayout> m_InputLayout;

	bool m_DepthTestEnabled;
	D3D11_COMPARISON_FUNC m_DepthTestFunc;
	D3D11_CULL_MODE m_CullMode;
};