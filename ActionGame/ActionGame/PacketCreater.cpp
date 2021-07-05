#include "PacketCreater.h"
#include "CPacket.h"
#include "PacketDefine.h"
#include <stdio.h>

void CreateMoveStartPacket(CPacket* packet, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	header.byCode = 0x89;
	header.byType = dfPACKET_CS_MOVE_START;
	header.bySize = 5;

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << direction << x << y;
}

void CreateMoveStopPacket(CPacket* packet, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	header.byCode = 0x89;
	header.byType = dfPACKET_CS_MOVE_STOP;
	header.bySize = 5;

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << direction << x << y;
}

void CreateAttack1Packet(CPacket* packet, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	header.byCode = 0x89;
	header.byType = dfPACKET_CS_ATTACK1;
	header.bySize = 5;

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << direction << x << y;
}

void CreateAttack2Packet(CPacket* packet, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	header.byCode = 0x89;
	header.byType = dfPACKET_CS_ATTACK2;
	header.bySize = 5;

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << direction << x << y;
}

void CreateAttack3Packet(CPacket* packet, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	header.byCode = 0x89;
	header.byType = dfPACKET_CS_ATTACK3;
	header.bySize = 5;

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << direction << x << y;
}

void LogPacket(CPacket* packet, DWORD id)
{
	stHeader header;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> header.byCode >> header.bySize >> header.byType;
	*packet >> direction >> x >> y;

	switch (header.byType)
	{
	case dfPACKET_CS_MOVE_START:
	{
		wprintf_s(L"# PACKET_MOVE_START # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, direction, x, y);
		break;
	}
	case dfPACKET_CS_MOVE_STOP:
	{
		wprintf_s(L"# PACKET_MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, direction, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK1:
	{
		wprintf_s(L"# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, direction, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK2:
	{
		wprintf_s(L"# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, direction, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK3:
	{
		wprintf_s(L"# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d / Y:%d\n",
			id, direction, x, y);
		break;
	}
	default:
		break;
	}
}