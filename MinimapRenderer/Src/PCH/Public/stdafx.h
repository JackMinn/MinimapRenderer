// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

//https://www.cygnus-software.com/papers/precompiledheaders.html
//http://www.cplusplus.com/articles/2z86b7Xj/
//https://stackoverflow.com/questions/14714877/mismatch-detected-for-runtimelibrary

#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <utility>
#include <assert.h>

//Standard Template Library Header Files
#include <list>
#include <vector>
#include <queue>
#include <string>
#include <stack>
#include <unordered_map>

// reference additional headers your program requires here
#include "Misc/Utilities.h"

#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;