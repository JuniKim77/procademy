#include "CChatServerSingle.h"

unsigned int __stdcall procademy::CChatServerSingle::UpdateFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    while (1)
    {
        if (!chatServer->CompleteMessage())
        {
            break;
        }
    }

    return 0;
}

bool procademy::CChatServerSingle::CompleteMessage()
{
    DWORD transferredSize = 0;
    CNetPacket* pPacket = nullptr;
    WSAOVERLAPPED* pOverlapped = nullptr;
    
    BOOL gqcsRet = GetQueuedCompletionStatus(mIOCP, &transferredSize, (PULONG_PTR)&pPacket, &pOverlapped, INFINITE);



    return false;
}
