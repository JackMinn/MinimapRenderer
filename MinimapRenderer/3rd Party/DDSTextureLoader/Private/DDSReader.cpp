#include "DDSReader.h"

#include <assert.h>
#include <algorithm>
#include <memory>



HRESULT DDSReader::LoadDDSDataFromFile(const wchar_t* fileName, std::unique_ptr<uint8_t[]>& ddsData, const uint8_t** imageBitData)
{
	HRESULT hr = DirectX::LoadTextureDataFromFilePublic(fileName,
		ddsData,
		imageBitData
	);
	if (FAILED(hr))
	{
		return hr;
	}
}

