#ifndef OPERATORNEWOVERLOAD
#define OPERATORNEWOVERLOAD

#include "myNew.h"

extern myNew gMemoryPool;

void* operator new (size_t size, const char* File, int Line);
void* operator new[](size_t size, const char* File, int Line);
void operator delete (void* p);
void operator delete[](void* p);

void operator delete (void* p, const char* File, int Line);
void operator delete[](void* p, const char* File, int Line);

#define new new(__FILE__, __LINE__)

#endif