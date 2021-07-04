#include "Content.h"
#include <stdio.h>
#include "PacketDefine.h"
#include "CLogger.h"
#include "Session.h"
#include "Sector.h"
#include "User.h"
#include "Container.h"
#include "CPacket.h"
#include "ActionDefine.h"

extern CLogger g_Logger;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet)
{
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
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"�������� �ʴ� �޼��� ���� [SessionNo: %d]\n",
			from);
		break;
	}

	return true;
}

bool CS_MoveStart(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"�������� �ʴ� ���� [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Move Start [UserNo: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);

	User* user = FindUser(from);
	if (user == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"���� ���� & �������� �ʴ� ���� [UserNo: %d]\n", from);

		return false;
	}

	// ��ũ ����
	if (abs(user->x - x) > dfERROR_RANGE || abs(user->y - y) > dfERROR_RANGE)
	{
		// ��ũ �޼��� �۽�

		x = user->x;
		y = user->y;
	}

	// ����� �׼��� ���� ����.
	user->action = direction;
	user->moveDirection = direction;
	// �ܼ� �¿� ���� ����
	switch (direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		user->direction = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		user->direction = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		break;
	}

	user->x = x;
	user->y = y;

	// �ֺ� ���׿� �޼��� ����

	return true;
}

bool CS_MoveStop(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"�������� �ʴ� ���� [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Move Stop [UserNo: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);

	User* user = FindUser(from);
	if (user == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"���� ���� & �������� �ʴ� ���� [UserNo: %d]\n", from);

		return false;
	}

	// ��ũ ����
	if (abs(user->x - x) > dfERROR_RANGE || abs(user->y - y) > dfERROR_RANGE)
	{
		// ��ũ �޼��� �۽�

		x = user->x;
		y = user->y;
	}

	user->action = dfAction_STAND;
	user->moveDirection = dfAction_STAND;
	user->direction = direction;

	user->x = x;
	user->y = y;

	// ���� �޼��� �۽�

	return true;
}

bool CS_Attack1(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"�������� �ʴ� ���� [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);



	return true;
}

bool CS_Attack2(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"�������� �ʴ� ���� [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
	return true;
}

bool CS_Attack3(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"�������� �ʴ� ���� [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
	return true;
}
