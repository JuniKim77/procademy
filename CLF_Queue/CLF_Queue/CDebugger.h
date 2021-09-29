#pragma once
#include <wtypes.h>

class CDebugger
{
public:
	enum DEBUG_CONST
	{
		DEBUG_CONST_MAX_LEN = 128
	};

	CDebugger() = delete;
	static void _Log(const WCHAR* format, ...);
	static void Initialize();
	static void Destroy();
	static void SetDirectory(const WCHAR* path);
	static void PrintLogOut(const WCHAR* szFileName);

private:
	static WORD sIndex;
	static WCHAR sFolderPath[MAX_PATH];
	static WCHAR* sLogData[USHRT_MAX + 1];
	static bool sIsOver;
};