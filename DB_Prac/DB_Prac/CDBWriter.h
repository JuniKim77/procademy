#pragma once
#pragma (lib, "mysqlclient.lib")

#include <Windows.h>
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"
#include "TC_LFQueue.h"
#include "DB_protocol.h"

namespace procademy
{
	class CDBWriter
	{
	public:
		CDBWriter();
		virtual		~CDBWriter();
		void		Stop();
		void		WaitEnd();
		void		DBPost(DB_chunk* chunk);

	private:
		static unsigned int WINAPI DBWriteThread(LPVOID arg);
		bool		connect();
		void		beginThread();
		void		Init();
		void		UpdateProc();
		void		FreeChunk(DB_chunk* chunk);

	private:
		MYSQL*							mConnection = nullptr;
		HANDLE							mWriteThread = INVALID_HANDLE_VALUE;
		HANDLE							mUpdateEvent = INVALID_HANDLE_VALUE;
		bool							mbEnd = false;
		TC_LFObjectPool<DB_chunk>		mChunkPool;
		TC_LFQueue<DB_chunk*>			mJobQ;
	};
}
