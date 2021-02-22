#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#define MAX_SIZE (100)

extern myNew gMemoryPool[MAX_SIZE];

void* operator new (size_t size, const char* File, int Line)
{
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == nullptr)
		{
			void* ret = malloc(size);

			pMemory->mAddr = ret;
			memcpy(pMemory->mFileName, File, strlen(File) + 1);
			pMemory->mLine = Line;
			pMemory->mSize = size;

			return ret;
		}
	}

	return nullptr;
}

void operator delete (void* p)
{
	// �ش� �ּ� ��Ī
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == p)
		{
			pMemory->mAddr = nullptr;

			return;
		}
	}

	// �迭�� delete�� ������ ��� ��� ��Ī
	void* correctedAddr = (char*)p - 4;
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == correctedAddr)
		{
			writeLog(LOG_TYPE_ARRAY, pMemory);

			return;
		}
	}

	// �Ҵ����� ���� �޸𸮸� ������ ���
	writeLog(LOG_TYPE_NOALLOC, p);
}

void operator delete[] (void* p)
{
	// �ش� �ּ� ��Ī
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == p)
		{
			pMemory->mAddr = nullptr;

			return;
		}
	}

	// �迭�� delete�� ������ ��� ��� ��Ī
	void* correctedAddr = (char*)p - 4;
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == correctedAddr)
		{
			writeLog(LOG_TYPE_ARRAY, pMemory);

			return;
		}
	}

	// �Ҵ����� ���� �޸𸮸� ������ ���
	writeLog(LOG_TYPE_NOALLOC, p);
}

void operator delete (void* p, const char* File, int Line)
{
}
void operator delete[](void* p, const char* File, int Line)
{
}

#define new new(__FILE__, __LINE__)