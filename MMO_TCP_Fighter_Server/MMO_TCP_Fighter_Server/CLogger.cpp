#include "CLogger.h"
#include <stdio.h>
#include <stdarg.h>

void CLogger::_Log(int logLevel, const WCHAR* format, ...)
{
	const WCHAR* pFormat = format;

	if (logLevel >= mLogLevel)
	{
		va_list ap;

		va_start(ap, format);
		{
            while (*pFormat != '\0') {
                int val;
                const WCHAR* str;
                double val_f;

                if (*pFormat == L'%')
                {
                    ++pFormat;
                    switch (*pFormat) {
                    case 's':
                        str = va_arg(ap, const WCHAR*);
                        wprintf_s(L"%s", str);
                        break;
                    case 'c':
                        val = va_arg(ap, unsigned int);
                        putwchar(val);
                        break;
                    case 'd':
                        val = va_arg(ap, int);
                        wprintf_s(L"%d", val);
                        break;
                    case 'f':
                        val_f = va_arg(ap, double);
                        wprintf_s(L"%.3f", val_f);
                        break;
                    default:
                        // 에러?
                        break;
                    }
                }
                else
                {
                    putwchar(*pFormat);
                }

                ++pFormat;
            }
		}
		va_end(ap);
	}

	// 에러 로그는 여기에...
    if (logLevel == dfLOG_LEVEL_ERROR)
    {

    }
}