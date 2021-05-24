#include "PacketCreater.h"
#include <wchar.h>

void CreateMoveStartPacket(stHeader* header, csMoveStart* packet, BYTE direction, WORD x, WORD y)
{
	header->byCode = 0x89;
	header->byType = dfPACKET_CS_MOVE_START;
	header->bySize = sizeof(csMoveStart);

	packet->Direction = direction;
	packet->X = x;
	packet->Y = y;
}

void CreateMoveStopPacket(stHeader* header, csMoveStop* packet, BYTE direction, WORD x, WORD y)
{
	header->byCode = 0x89;
	header->byType = dfPACKET_CS_MOVE_STOP;
	header->bySize = sizeof(csMoveStop);

	packet->Direction = direction;
	packet->X = x;
	packet->Y = y;
}

void CreateAttack1Packet(stHeader* header, csAttack1* packet, BYTE direction, WORD x, WORD y)
{
	header->byCode = 0x89;
	header->byType = dfPACKET_CS_ATTACK1;
	header->bySize = sizeof(csAttack1);

	packet->Direction = direction;
	packet->X = x;
	packet->Y = y;
}

void CreateAttack2Packet(stHeader* header, csAttack2* packet, BYTE direction, WORD x, WORD y)
{
	header->byCode = 0x89;
	header->byType = dfPACKET_CS_ATTACK2;
	header->bySize = sizeof(csAttack2);

	packet->Direction = direction;
	packet->X = x;
	packet->Y = y;
}

void CreateAttack3Packet(stHeader* header, csAttack3* packet, BYTE direction, WORD x, WORD y)
{
	header->byCode = 0x89;
	header->byType = dfPACKET_CS_ATTACK3;
	header->bySize = sizeof(csAttack3);

	packet->Direction = direction;
	packet->X = x;
	packet->Y = y;
}

void LogPacket(stHeader* header, char* msg, DWORD id)
{
	switch (header->byType)
	{
	case dfPACKET_CS_MOVE_START:
	{
		csMoveStart* packet = (csMoveStart*)msg;
		wprintf_s(L"# PACKET_MOVE_START # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, packet->Direction, packet->X, packet->Y);
		break;
	}
	case dfPACKET_CS_MOVE_STOP:
	{
		csMoveStop* packet = (csMoveStop*)msg;
		wprintf_s(L"# PACKET_MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, packet->Direction, packet->X, packet->Y);
		break;
	}
	case dfPACKET_CS_ATTACK1:
	{
		csAttack1* packet = (csAttack1*)msg;
		wprintf_s(L"# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, packet->Direction, packet->X, packet->Y);
		break;
	}
	case dfPACKET_CS_ATTACK2:
	{
		csAttack2* packet = (csAttack2*)msg;
		wprintf_s(L"# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, packet->Direction, packet->X, packet->Y);
		break;
	}
	case dfPACKET_CS_ATTACK3:
	{
		csAttack3* packet = (csAttack3*)msg;
		wprintf_s(L"# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, packet->Direction, packet->X, packet->Y);
		break;
	}
	default:
		break;
	}
}