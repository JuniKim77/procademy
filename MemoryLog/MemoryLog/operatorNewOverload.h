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
			pMemory->mIsArray = false;

			return ret;
		}
	}

	return nullptr;
}

void* operator new[] (size_t size, const char* File, int Line)
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
			pMemory->mIsArray = true;

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
			if (pMemory->mIsArray == false)
			{
				pMemory->mAddr = nullptr;

				return;
			}
			else
			{
				writeLog(LOG_TYPE_ARRAY, pMemory);

				return;
			}
		}
	}

	// �迭�� delete�� ������ ��� ��� ��Ī
	void* correctedAddr = (char*)p - sizeof(void*);
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == correctedAddr && pMemory->mIsArray == true)
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
	printf("dfdf");
	// �ش� �ּ� ��Ī
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == p && pMemory->mIsArray == true)
		{
			pMemory->mAddr = nullptr;

			return;
		}
	}

	// delete �迭�� �߸� ������ ��� ��� ��Ī
	void* correctedAddr = (char*)p - sizeof(void*);
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		myNew* pMemory = &gMemoryPool[i];

		if (pMemory->mAddr == correctedAddr && pMemory->mIsArray == false)
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