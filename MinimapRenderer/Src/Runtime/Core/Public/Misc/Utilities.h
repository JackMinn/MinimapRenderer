#pragma once

// System includes
#include <windows.h>
#include <iostream>
#include <sstream> 
#include <string>
#include <stdarg.h>
#include <cstddef>


#if _DEBUG
//#	define DebugLog(fmt, ...) EngineDebugLog(fmt, __VA_ARGS__);
#define DebugLog(fmt, ...)
#else
#	define DebugLog(fmt, ...)
#endif

inline void EngineDebugLog(const char *szTypes, ...)
{
	va_list args;
	va_start(args, szTypes);

	std::stringstream ss;

	// Step through the list.  
	for (int i = 0; szTypes[i] != '\0'; ++i) {
		switch (szTypes[i]) {   // Type to expect.  
		case 'i':
			ss << va_arg(args, INT32);
			break;

		case 'u':
			ss << va_arg(args, UINT32);
			break;

		case 'f':
			ss << va_arg(args, double);
			break;

		case 'c':
			ss << va_arg(args, char);
			break;

		case 's':
			ss << va_arg(args, char*);
			break;

		default:
			break;
		}
	}
	ss << std::endl;
	va_end(args);

	OutputDebugStringA(ss.str().c_str());
}

#define DEBUG_ASSERT(x) assert(x)

#define NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(x) \
	x(const x&) = delete; \
	x& operator=(const x&) = delete;

inline void RemoveWhiteSpaces(std::string& s) {
	std::string::iterator end_pos;
	end_pos = std::remove(s.begin(), s.end(), ' ');
	s.erase(end_pos, s.end());
}