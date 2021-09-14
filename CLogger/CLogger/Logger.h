/*****************************************************************************
Module   : Logger
Notices   :
   - 스레드 테스트 완료
Todo   :
*****************************************************************************/

#pragma once
#include <direct.h>
#include <strsafe.h>
#include <stdio.h>
#include <locale.h>
#include <windows.h>

enum class en_LOG_LEVEL
{
	LEVEL_DEBUG,
	LEVEL_ERROR,
	LEVEL_SYSTEM,
	OFF
};

extern en_LOG_LEVEL g_LogLevel;

extern FILE* g_File;
extern SRWLOCK      g_File_srw;
#define            LockFlie()         AcquireSRWLockExclusive(&g_File_srw)
#define            UnlockFlie()      ReleaseSRWLockExclusive(&g_File_srw)

extern WCHAR      g_szBuffer[MAX_PATH];
extern SRWLOCK      g_szBuffer_srw;
#define            LockFoler()         AcquireSRWLockExclusive(&g_szBuffer_srw)
#define            UnlockFoler()      ReleaseSRWLockExclusive(&g_szBuffer_srw)

#define SYSLOG_LEVEL(LogLevel) g_LogLevel = LogLevel

#define SYSLOG_DIRECTORY(szFolderName)                        \
do {                                                \
   SetDirectory(szFolderName);                              \
} while (0)                                             \

#define LOG(szType, LogLevel, szStringFormat, ...)               \
do{                                                   \
   Log(szType, LogLevel, szStringFormat, ##__VA_ARGS__);         \
}while(0)                                             \

#define LOG_HEX(szType, LogLevel, szLog, pByte, iByteLen, iRow)      \
do{                                                   \
   LogHex(szType, LogLevel, szLog, pByte, iByteLen, iRow);         \
}while(0)                                             \

#define CONSOLE(LogLevel, szStringFormat, ...)                  \
do{                                                   \
   LogConsole(LogLevel, szStringFormat, ##__VA_ARGS__);         \
}while(0)                                             \

void InitializeLogger(void);
void SetDirectory(const WCHAR* szFolderName);
void Log(const WCHAR* szType, en_LOG_LEVEL LogLevel, const WCHAR* szStringFormat, ...);
void LogConsole(en_LOG_LEVEL LogLevel, const WCHAR* szStringFormat, ...);
void LogHex(const WCHAR* szType, en_LOG_LEVEL LogLevel, const WCHAR* szLog, char* pByte, int iByteLen, int iRow = 4);