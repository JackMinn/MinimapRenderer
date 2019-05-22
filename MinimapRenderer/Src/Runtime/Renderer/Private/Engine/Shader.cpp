#include "Engine/Shader.h"
#include "D3D11/D3D11Util.h"
#include <d3dcompiler.h>

std::string g_VertexEntry = "VSMain";
std::string g_PixelEntry = "PSMain";

bool Shader::Initialize(ID3D11Device* device, const std::string& vsFileName, const std::string& psFileName)
{
	std::wstring vsFileNameW(vsFileName.begin(), vsFileName.end());
	std::wstring psFileNameW(psFileName.begin(), psFileName.end());

	// Initialize the vertex and pixel shaders.
	HRESULT result;
	ComPtr<ID3DBlob> errorMessage;
	ComPtr<ID3DBlob> vertexShaderBlob;
	ComPtr<ID3DBlob> pixelShaderBlob;

	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&cBufferDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));


	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	vertexShaderBlob = nullptr;
	pixelShaderBlob = nullptr;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFileNameW.c_str(), NULL, NULL, g_VertexEntry.c_str(), "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		vertexShaderBlob.GetAddressOf(), errorMessage.GetAddressOf());
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			DebugLog("ssss", "Error compiling ", vsFileName.c_str(), ". ", reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			DebugLog("ss", "Missing shader file: ", vsFileName.c_str());
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFileNameW.c_str(), NULL, NULL, g_PixelEntry.c_str(), "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBlob, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			DebugLog("ssss", "Error compiling ", psFileName.c_str(), ". ", reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			DebugLog("ss", "Missing shader file: ", psFileName.c_str()); //doesnt currently print the full name
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, m_VertexShader.GetAddressOf());
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create vertex shader from blob.");
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, m_PixelShader.GetAddressOf());
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create pixel shader from blob.");
		return false;
	}

	result = GetInputLayoutFromVertexShaderSignature(vertexShaderBlob.Get(), device, m_InputLayout.GetAddressOf());
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create input layout from vertex blob.");
		return false;
	}

	//result = ShaderUtils::GetConstantBuffersFromShader(vertexShaderBlob, m_cbSize, m_cbVariables);
	//if (FAILED(result))
	//{
	//	DebugLog("s", "Unable to create globals constant buffer from vertex blob.");
	//	return false;
	//}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, m_SamplerState.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


HRESULT Shader::GetInputLayoutFromVertexShaderSignature(ID3DBlob* shaderBlob, ID3D11Device* device, ID3D11InputLayout** inputLayout)
{
	HRESULT result;

	// Reflect shader info
	ID3D11ShaderReflection* vertexShaderReflection = nullptr;
	result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&vertexShaderReflection);
	if (FAILED(result))
	{
		return result;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	vertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlot = 0;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		DebugLog("ss", "Semantic Name is: ", paramDesc.SemanticName);

		// determine DXGI format (UINT/SINT/FLOAT R/RG/RGB/RGB
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = device->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), inputLayout);

	//Free allocation shader reflection memory
	vertexShaderReflection->Release();
	return hr;
}
