#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

class CLFQueue
{
public:
    CLFQueue();
    ~CLFQueue();
    void Enqueue(ULONG64 data);
    bool Dequeue(ULONG64* data);
    bool IsEmpty() { return mSize == 0; }
    DWORD GetSize() { return mSize; }
    DWORD GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
    DWORD GetPoolSize() { return mMemoryPool.GetSize(); }

private:
    int mSize = 0;

    struct Node
    {
        ULONG64 data;
        Node* next;
    };

    Node* mHead = nullptr;        // ���۳�带 ����Ʈ�Ѵ�.
    Node* mTail = nullptr;        // ��������带 ����Ʈ�Ѵ�.
    procademy::TC_LFObjectPool<Node> mMemoryPool;
};
