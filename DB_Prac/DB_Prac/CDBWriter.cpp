#include "CDBWriter.h"
#include "CLogger.h"
#include "CCrashDump.h"
#include <process.h>
#include "DB_protocol.h"

procademy::CDBWriter::CDBWriter()
{
	if (!connect())
	{
		CRASH();
	}	

	Init();
	beginThread();
}

procademy::CDBWriter::~CDBWriter()
{
	mysql_close(mConnection);
	CloseHandle(mUpdateEvent);
}

void procademy::CDBWriter::Stop()
{
	mbEnd = true;
	SetEvent(mUpdateEvent);
}

void procademy::CDBWriter::WaitEnd()
{
	DWORD retval = WaitForSingleObject(mWriteThread, INFINITE);

	switch (retval)
	{
	case WAIT_FAILED:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"DB Writer Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"DB Writer Thread TimeOut Error\n");
		break;
	case WAIT_OBJECT_0:
		CLogger::_Log(dfLOG_LEVEL_SYSTEM ,L"DB Writer Thread None Error\n");
		break;
	default:
		break;
	}

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"DB Writer Thread End\n");
}

void procademy::CDBWriter::DBPost(DB_chunk* chunk)
{
	mJobQ.Enqueue(chunk);
	SetEvent(mUpdateEvent);
}

unsigned int __stdcall procademy::CDBWriter::DBWriteThread(LPVOID arg)
{
	CDBWriter* writer = (CDBWriter*)arg;

	writer->UpdateProc();

	return 0;
}

bool procademy::CDBWriter::connect()
{
	MYSQL conn;

	mConnection = mysql_real_connect(&conn, "127.0.0.1", "root", "123456789", "test", 3306, nullptr, 0);

	if (mConnection == nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Mysql connection error : %s", mysql_error(&conn));
		return false;
	}

	mysql_set_character_set(mConnection, "utf8");

	return true;
}

void procademy::CDBWriter::beginThread()
{
	mWriteThread = (HANDLE)_beginthreadex(nullptr, 0, DBWriteThread, this, 0, nullptr);
}

void procademy::CDBWriter::Init()
{
	mUpdateEvent = CreateEvent(nullptr, false, false, nullptr);
}

void procademy::CDBWriter::UpdateProc()
{
	while (!mbEnd)
	{
		DWORD retval = WaitForSingleObject(mUpdateEvent, INFINITE);
		DB_chunk* chunk = nullptr;

		while (!mJobQ.IsEmpty())
		{
			mJobQ.Dequeue(&chunk);

			switch (chunk->type)
			{
			case dfCreateUser:
				break;
			case dfSelectUser:
				break;
			case dfCreateItem:
				break;
			case dfSelectItem:
				break;
			default:
				break;
			}

			FreeChunk(chunk);
		}
	}
}

void procademy::CDBWriter::FreeChunk(DB_chunk* chunk)
{
	// √ ±‚»≠...
	mChunkPool.Free(chunk);
}
