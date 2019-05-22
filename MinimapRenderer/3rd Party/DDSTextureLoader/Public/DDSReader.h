#pragma once

#include "DDSTextureLoader.h"

namespace DDSReader {
	HRESULT LoadDDSDataFromFile(const wchar_t* fileName, std::unique_ptr<uint8_t[]>& ddsData, const uint8_t** imageBitData);
}

