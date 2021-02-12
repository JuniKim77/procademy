#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define _MemoryAlloc(type, size) MemoryAlloc<type>(size, __FILE__, __LINE__)
#define MEMORY_MAX (100)

using namespace std;

struct MemoryTrack
{
	void* addr;
	unsigned int size;
	char fileName[128];
	unsigned int lineNum;

	MemoryTrack()
		: addr(nullptr)
	{	}
};

MemoryTrack gMemoryTable[MEMORY_MAX];
unsigned int gMemoryTotalSize = 0;
unsigned int gMemoryTotalCount = 0;

template <typename T>
T* MemoryAlloc(int num, const char* fileName, unsigned int lineNum)
{
	for (int i = 0; i < MEMORY_MAX; ++i)
	{
		if (gMemoryTable[i].addr == nullptr)
		{
			T* ret = new T[num];
			gMemoryTable[i].addr = ret;
			strcpy_s(gMemoryTable[i].fileName, fileName);
			gMemoryTable[i].lineNum = lineNum;
			gMemoryTable[i].size = num * sizeof(T);

			gMemoryTotalCount++;
			gMemoryTotalSize += gMemoryTable[i].size;

			return ret;
		}
	}
	
	return nullptr;
}

template <typename T>
void MemoryRelease(T* memory)
{
	for (int i = 0; i < MEMORY_MAX; ++i)
	{
		if (gMemoryTable[i].addr == memory)
		{
			delete[] memory;
			gMemoryTable[i].addr = nullptr;

			return;
		}
	}
}

void PrintAlloc()
{
	cout << "Total Alloc Size : " << gMemoryTotalSize << endl;
	cout << "Total Alloc Count : " << gMemoryTotalCount << endl;
	cout << showbase << nouppercase;

	for (int i = 0; i < MEMORY_MAX; ++i)
	{
		if (gMemoryTable[i].addr != nullptr)
		{
			cout << "Not Release Memory : [0x" << gMemoryTable[i].addr << "] " << gMemoryTable[i].size << " Bytes" << endl;
			cout << "File : " << gMemoryTable[i].addr << " : " << gMemoryTable[i].lineNum << endl << endl;
		}
	}
}

int main()
{
	int* p4 = _MemoryAlloc(int, 1);
	int* p400 = _MemoryAlloc(int, 100);
	char* pZ1 = _MemoryAlloc(char, 50);
	char* pZ2 = _MemoryAlloc(char, 150);
	char* pZ3 = _MemoryAlloc(char, 60);
	char* pZ4 = _MemoryAlloc(char, 70);

	MemoryRelease(p4);
	//MemoryRelease(p400);
	MemoryRelease(pZ1);
	//MemoryRelease(pZ2);
	MemoryRelease(pZ3);
	MemoryRelease(pZ3);
	MemoryRelease(pZ4);

	PrintAlloc();

	return 0;
}