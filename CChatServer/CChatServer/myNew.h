#pragma once
#include <climits>
#include <stdlib.h>
#include <windows.h>

struct memDebug;

extern USHORT g_memoryIdx;
extern memDebug g_memoryDebug[USHRT_MAX + 1];

void* operator new (size_t size);
void* operator new (size_t size, const char* File, int Line);
void* operator new[](size_t size);
void* operator new[](size_t size, const char* File, int Line);
