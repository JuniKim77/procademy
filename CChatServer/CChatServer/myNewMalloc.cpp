#include "myNewMalloc.h"
#include "memoryDebug.h"
#include <time.h>

#undef malloc
#undef _aligned_malloc

void* _Alloc(const char* File, int Line, size_t size)
{
    tm t;
    time_t newTime;

    time(&newTime);
    localtime_s(&t, &newTime);

    void* p = malloc(size);
    USHORT index = InterlockedIncrement16((short*)&g_memoryIdx);
    g_memoryDebug[index].address = p;
    g_memoryDebug[index].fileName = File;
    g_memoryDebug[index].line = Line;
    g_memoryDebug[index].bArray = false;
    g_memoryDebug[index].bAlignment = false;
    g_memoryDebug[index].hour = t.tm_hour;
    g_memoryDebug[index].minute = t.tm_min;
    g_memoryDebug[index].second = t.tm_sec;

    return p;
}

void* Alloc_Align(const char* File, int Line, size_t size, size_t alignment)
{
    tm t;
    time_t newTime;

    time(&newTime);
    localtime_s(&t, &newTime);

    void* p = _aligned_malloc(size, alignment);
    USHORT index = InterlockedIncrement16((short*)&g_memoryIdx);
    g_memoryDebug[index].address = p;
    g_memoryDebug[index].fileName = File;
    g_memoryDebug[index].line = Line;
    g_memoryDebug[index].bArray = false;
    g_memoryDebug[index].bAlignment = true;
    g_memoryDebug[index].hour = t.tm_hour;
    g_memoryDebug[index].minute = t.tm_min;
    g_memoryDebug[index].second = t.tm_sec;

    return p;
}
