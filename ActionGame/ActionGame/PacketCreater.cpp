#include "PacketCreater.h"

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
