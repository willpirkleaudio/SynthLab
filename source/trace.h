#pragma once
// TRACE macro for win32
#ifndef __TRACE_H__850CE873
#define __TRACE_H__850CE873

#include <crtdbg.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _DEBUG
#define TRACEMAXSTRING	1024

static char szBuffer[TRACEMAXSTRING];
inline void TRACE(const char* format,...)
{
	va_list args;
	va_start(args,format);
	int nBuf;
	nBuf = _vsnprintf(szBuffer,
				   TRACEMAXSTRING,
				   format,
				   args);
	va_end(args);

	_RPT0(_CRT_WARN,szBuffer);
}
#define TRACEF _snprintf(szBuffer,TRACEMAXSTRING,"%s(%d): ", \
				&strrchr(__FILE__,'\\')[1],__LINE__); \
				_RPT0(_CRT_WARN,szBuffer); \
				TRACE
#else
// Remove for release mode
#define TRACE  ((void)0)
#define TRACEF ((void)0)
#endif

#endif // __TRACE_H__850CE873