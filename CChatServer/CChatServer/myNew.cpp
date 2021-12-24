#include "myNew.h"
#include "memoryDebug.h"

#undef new

void* operator new(size_t size, const char* File, int Line)
{
    void* p = malloc(size);
    USHORT index = InterlockedIncrement16((short*)&g_memoryIdx);
    g_memoryDebug[index].address = p;
    g_memoryDebug[index].fileName = File;
    g_memoryDebug[index].line = Line;
    g_memoryDebug[index].bArray = false;
    g_memoryDebug[index].bAlignment = false;

    return p;
}

void* operator new[](size_t size, const char* File, int Line)
{
    void* p = malloc(size);
    USHORT index = InterlockedIncrement16((short*)&g_memoryIdx);
    g_memoryDebug[index].address = p;
    g_memoryDebug[index].fileName = File;
    g_memoryDebug[index].line = Line;
    g_memoryDebug[index].bArray = true;
    g_memoryDebug[index].bAlignment = false;

    return nullptr;
}