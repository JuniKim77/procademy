#pragma once
#include <climits>
#include <stdlib.h>
#include <windows.h>

struct memDebug;

extern USHORT g_memoryIdx;
extern memDebug g_memoryDebug[USHRT_MAX + 1];

void* _Alloc(const char* File, int Line, size_t size);
void* Alloc_Align(const char* File, int Line, size_t size, size_t alignment);

#define malloc(size) _Alloc(__FILE__, __LINE__, size)
#define _aligned_malloc(size, alignment) Alloc_Align(__FILE__, __LINE__, size, alignment)