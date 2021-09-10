#include "PacketCreater.h"
#include "PacketDefine.h"
#include "CPacket.h"

void cpSC_CreateMyUser(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y, BYTE hp)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_CREATE_MY_CHARACTER, 10);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y << hp;
}

void cpSC_CreateOtherUser(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y, BYTE hp)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_CREATE_OTHER_CHARACTER, 10);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y << hp;
}

void cpSC_DeleteUser(CPacket* packet, DWORD id)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_DELETE_CHARACTER, 4);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id;
}

void cpSC_MoveStart(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_MOVE_START, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y;
}

void cpSC_MoveStop(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_MOVE_STOP, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y;
}

void cpSC_Attack1(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_ATTACK1, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y;
}

void cpSC_Attack2(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_ATTACK2, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y;
}

void cpSC_Attack3(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_ATTACK3, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << direction << x << y;
}

void cpSC_Damage(CPacket* packet, DWORD attackID, DWORD damageID, BYTE damageHP)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_DAMAGE, 9);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << attackID << damageID << damageHP;
}

void cpSC_Synchronize(CPacket* packet, DWORD id, WORD x, WORD y)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_SYNC, 8);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << id << x << y;
}

void cpSC_Echo(CPacket* packet, DWORD time)
{
	stHeader header;

	FillHeader(&header, dfPACKET_SC_ECHO, 4);

	packet->PutData((char*)&header, sizeof(stHeader));

	*packet << time;
}

void FillHeader(stHeader* header, BYTE msgType, BYTE msgSize)
{
	header->byCode = dfPACKET_CODE;
	header->byType = msgType;
	header->bySize = msgSize;
}
