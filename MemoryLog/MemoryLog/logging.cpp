#pragma once
#include "logging.h"
#include "myNew.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

void writeLog(logType type, void* pMemory)
{
	tm newTime;
	time_t t;
	FILE* fout;
	char fileName[128];
	char buffer[128];
	myNew* pMyNew;

	time(&t);
	localtime_s(&newTime, &t);
	sprintf_s(fileName, sizeof(fileName), "Alloc_%d%02d%02d_%02d%02d%02d.txt",
		newTime.tm_year + 1900, newTime.tm_mon + 1, newTime.tm_mday,
		newTime.tm_hour, newTime.tm_min, newTime.tm_sec);
	fopen_s(&fout, fileName, "ba");

	switch (type)
	{
	case LOG_TYPE_NOALLOC:
		sprintf_s(buffer, sizeof(buffer), "NOALLOC [0x%p]", pMemory);

		fwrite(buffer, strlen(buffer) + 1, 1, fout);
		break;
	case LOG_TYPE_ARRAY:
		pMyNew = (myNew*)pMemory;
		sprintf_s(buffer, sizeof(buffer), "ARRAY [0x%p][%4d]%s : %d", pMemory, pMyNew->mSize, pMyNew->mFileName, pMyNew->mLine);

		fwrite(buffer, strlen(buffer) + 1, 1, fout);
		break;
	case LOG_TYPE_LEAK:
		pMyNew = (myNew*)pMemory;
		sprintf_s(buffer, sizeof(buffer), "LEAK [0x%p][%4d]%s : %d", pMemory, pMyNew->mSize, pMyNew->mFileName, pMyNew->mLine);

		fwrite(buffer, strlen(buffer) + 1, 1, fout);
		break;
	default:
		break;
	}

	fclose(fout);
}
