#include "ObjectPool_TLS.h"
#include <iostream>

using namespace std;

class TEST
{
public:
	TEST() 
	{
		cout << "TEST Construct" << endl;
	}

	~TEST()
	{
		cout << "TEST Distructor" << endl;
	}

	int mNum = 0;
};

struct packetDebug
{
	int			logicId;
	DWORD		threadId;
	void* pChunk;
	void* pPacket;
	int			allocCount;
	LONG		freeCount;
};

USHORT g_debugIdx = 0;
packetDebug g_packetDebugs[USHRT_MAX + 1] = { 0, };

void packetLog(
	int			logicId = -9999,
	DWORD		threadId = 0,
	void* pChunk = nullptr,
	void* pPacket = nullptr,
	int			allocCount = -9999,
	LONG		freeCount = 9999
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debugIdx);

	g_packetDebugs[index].logicId = logicId;
	g_packetDebugs[index].threadId = threadId;
	g_packetDebugs[index].pChunk = pChunk;
	g_packetDebugs[index].pPacket = pPacket;
	g_packetDebugs[index].allocCount = allocCount;
	g_packetDebugs[index].freeCount = freeCount;
}

int main()
{
	procademy::ObjectPool_TLS<TEST> tt;

	for (int i = 0; i < 199; ++i)
	{
		TEST* test1 = tt.Alloc();

		test1->mNum = 10;

		tt.Free(test1);
	}

	TEST* test1 = tt.Alloc();

	test1->mNum = 10;

	tt.Free(test1);
	
	test1 = tt.Alloc();

	test1->mNum = 10;

	return 0;
}