#pragma once
#include "myNew.h"
#include <memory.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

myNew::myNew(const char* fileName)
{
	tm newTime;
	time_t t;

	memset(mMemory, 0, sizeof(mMemory));
	memset(mLogFileName, 0, sizeof(mLogFileName));

	time(&t);
	localtime_s(&newTime, &t);

	sprintf_s(mLogFileName, sizeof(mLogFileName), "%s_%04d%02d%02d_%02d%02d%02d.txt",
		fileName,
		newTime.tm_year + 1900,
		newTime.tm_mon + 1,
		newTime.tm_mday,
		newTime.tm_hour,
		newTime.tm_min,
		newTime.tm_sec);
}

myNew::~myNew()
{
	for (int i = 0; i < ALLOC_MAX; ++i)
	{
		if (mMemory[i].mAddr != nullptr)
		{
			writeLog(LOG_TYPE_LEAK, &mMemory[i]);
		}
	}
}

void myNew::writeLog(logType type, void* pMemory)
{
	FILE* fout;
	char buffer[128];
	stMemory_Info* pInfo;

	fopen_s(&fout, mLogFileName, "a");

	switch (type)
	{
	case LOG_TYPE_NOALLOC:
		sprintf_s(buffer, sizeof(buffer), "NOALLOC [0x%p]\n", pMemory);

		fwrite(buffer, strlen(buffer), 1, fout);
		break;
	case LOG_TYPE_ARRAY:
		pInfo = (stMemory_Info*)pMemory;
		sprintf_s(buffer, sizeof(buffer), "ARRAY [0x%p][%4d] %s : %d\n", pInfo->mAddr, pInfo->mSize, pInfo->mFileName, pInfo->mLine);

		fwrite(buffer, strlen(buffer), 1, fout);
		break;
	case LOG_TYPE_LEAK:
		pInfo = (stMemory_Info*)pMemory;
		sprintf_s(buffer, sizeof(buffer), "LEAK [0x%p][%4d] %s : %d\n", pInfo->mAddr, pInfo->mSize, pInfo->mFileName, pInfo->mLine);

		fwrite(buffer, strlen(buffer), 1, fout);
		break;
	default:
		break;
	}

	fclose(fout);
}

bool myNew::newAlloc(void** pPtr, const char* fileName, int line, size_t size, bool bArray)
{
	*pPtr = nullptr;

	for (int i = 0; i < ALLOC_MAX; ++i)
	{
		if (mMemory[i].mAddr == nullptr)
		{
			*pPtr = malloc(size);
			mMemory[i].mAddr = *pPtr;
			mMemory[i].mLine = line;
			mMemory[i].mIsArray = bArray;
			mMemory[i].mSize = size;
			strcpy_s(mMemory[i].mFileName, FILE_NAME_LENGTH, fileName);

			return true;
		}
	}
	return false;
}

bool myNew::deleteAlloc(void* pPtr, bool bArray)
{
	// 해당 주소 서칭
	for (int i = 0; i < ALLOC_MAX; ++i)
	{
		if (mMemory[i].mAddr == pPtr)
		{
			if (mMemory[i].mIsArray == bArray)
			{
				mMemory[i].mAddr = nullptr;

				return true;
			}
			else
			{
				writeLog(LOG_TYPE_ARRAY, &mMemory[i]);

				return false;
			}
		}
	}

	// 배열을 delete로 해제한 경우
	void* correctedAddr = (char*)pPtr - sizeof(void*);
	for (int i = 0; i < ALLOC_MAX; ++i)
	{
		if (mMemory[i].mAddr == correctedAddr && mMemory[i].mIsArray == true)
		{
			writeLog(LOG_TYPE_ARRAY, &mMemory[i]);

			return false;
		}
	}

	// 할당하지 않은 메모리를 해제한 경우
	writeLog(LOG_TYPE_NOALLOC, pPtr);
	
	return false;
}
