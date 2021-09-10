#include "CLogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define dfLOG_SIZE (1024)

void CLogger::_Log(int logLevel, const WCHAR* format, ...)
{
    if (logLevel == dfLOG_LEVEL_DEBUG && mLogLevel > logLevel)
        return;

    WCHAR log[dfLOG_SIZE] = { 0, };
    WCHAR* pLog = log;
    int count = 0;

    va_list ap;

    va_start(ap, format);
    {
        while (*format != '\0') {
            int val;
            const WCHAR* str;
            double val_f;

            if (*format == L'%')
            {
                int len;
                ++format;
                switch (*format) {
                case 's':
                    str = va_arg(ap, const WCHAR*);
                    len = swprintf_s(pLog, dfLOG_SIZE - count, L"%s", str);
                    break;
                case 'c':
                    val = va_arg(ap, unsigned int);
                    *pLog = val;
                    len = 1;
                    break;
                case 'd':
                    val = va_arg(ap, int);
                    len = swprintf_s(pLog, dfLOG_SIZE - count, L"%d", val);
                    break;
                case 'f':
                    val_f = va_arg(ap, double);
                    len = swprintf_s(pLog, dfLOG_SIZE - count, L"%.3f", val_f);
                    break;
                default:
                    // 에러?
                    break;
                }

                count += len;
                pLog += len;
            }
            else
            {
                *pLog = *format;
                ++pLog;
                ++count;
            }
            ++format;
        }
    }
    va_end(ap);

	if (logLevel >= mLogLevel)
	{
        wprintf_s(log);
	}

    tm t;
    time_t curTime;

    time(&curTime);
    localtime_s(&t, &curTime);

    WCHAR fileName[80];

    swprintf_s(fileName, _countof(fileName), L"%04d%02d_Log.txt",
        t.tm_year + 1900, t.tm_mon + 1);

    FILE* fout;

    _wfopen_s(&fout, fileName, L"a");

	// 에러 로그는 여기에...
    if (logLevel == dfLOG_LEVEL_ERROR)
	{
		fwprintf_s(fout, L"[%02d/%02d/%02d %02d:%02d:%02d] #### %s\n\n",
			t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
			t.tm_hour, t.tm_min, t.tm_sec, log);
	}

    // 시스템 로그는 여기에...
    if (logLevel == dfLOG_LEVEL_NOTICE)
    {
        fwprintf_s(fout, L"[%02d/%02d/%02d %02d:%02d:%02d] %s",
            t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
            t.tm_hour, t.tm_min, t.tm_sec, log);
    }

    fclose(fout);
}