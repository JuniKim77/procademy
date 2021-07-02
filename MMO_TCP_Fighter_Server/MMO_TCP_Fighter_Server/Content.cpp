#include "Content.h"
#include <stdio.h>
#include "PacketDefine.h"

bool PacketProc(DWORD from, WORD msgType, CPacket* packet)
{
	wprintf_s(L"패킷 수신 [UserNo: %d][msgType: %d]\n", from, msgType);

	switch (msgType)
	{
	case dfPACKET_CS_MOVE_START:
		return CS_MoveStart(from, packet);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return CS_MoveStop(from, packet);
		break;
	case dfPACKET_CS_ATTACK1:
		return CS_Attack1(from, packet);
		break;
	case dfPACKET_CS_ATTACK2:
		return CS_Attack2(from, packet);
		break;
	case dfPACKET_CS_ATTACK3:
		return CS_Attack3(from, packet);
		break;
	default:
		// Log...
		break;
	}

	return true;
}

bool CS_MoveStart(DWORD from, CPacket* packet)
{
	return false;
}

bool CS_MoveStop(DWORD from, CPacket* packet)
{
	return false;
}

bool CS_Attack1(DWORD from, CPacket* packet)
{
	return false;
}

bool CS_Attack2(DWORD from, CPacket* packet)
{
	return false;
}

bool CS_Attack3(DWORD from, CPacket* packet)
{
	return false;
}
