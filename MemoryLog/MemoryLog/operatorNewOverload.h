#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myNew.h"

myNew gMemoryPool;

void* operator new (size_t size, const char* File, int Line)
{
	void* p;
	gMemoryPool.newAlloc(&p, File, Line, size);

	return p;
}

void* operator new[] (size_t size, const char* File, int Line)
{
	void* p;
	gMemoryPool.newAlloc(&p, File, Line, size, true);

	return p;
}

void operator delete (void* p)
{
	if (gMemoryPool.deleteAlloc(p))
	{
		free(p);
	}
}

void operator delete[] (void* p)
{
	if (gMemoryPool.deleteAlloc(p, true))
	{
		free(p);
	}
}

void operator delete (void* p, const char* File, int Line)
{
}
void operator delete[](void* p, const char* File, int Line)
{
}

#define new new(__FILE__, __LINE__)