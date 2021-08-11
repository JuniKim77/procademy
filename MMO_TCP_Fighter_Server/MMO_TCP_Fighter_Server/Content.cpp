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
#include "PacketCreater.h"
#include "NetworkProcs.h"

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
	case dfPACKET_CS_ECHO:
		return CS_Echo(from, packet);
		break;
	default:
		// Log...
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Unexpected Message Type [SessionNo: %d]\n",
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
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);	

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;
#ifdef DEBUG
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Move Start [UserNo: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
#endif
	
	User* user = FindUser(from);
	if (user == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Exist Session & Non-Exist User [UserNo: %d]\n", from);

		return false;
	}

	// 싱크 맞춤
	if (abs(user->x - x) > dfERROR_RANGE || abs(user->y - y) > dfERROR_RANGE)
	{
		// 싱크 메세지 송신
		CPacket Packet;
		cpSC_Synchronize(&Packet, user->userNo, user->x, user->y);
		SendPacket_Around(user->userNo, &Packet, true);

		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync Sent [UserNo: %d][Direction: %d][C_X: %d][C_Y: %d]->[S_X: %d][S_Y: %d] [HP: %d]\n",
			from, direction, x, y, user->x, user->y, user->hp);
		
		x = user->x;
		y = user->y;
	}

	// 이동 방향과 액션의 값이 같음.
	user->action = direction;
	user->moveDirection = direction;
	// 단순 좌우 방향 변경
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

	// 주변 섹터에 메세지 전송
	if (Sector_UpdateUser(user))
	{
		UserSectorUpdatePacket(user);
	}

	CPacket Packet;
	cpSC_MoveStart(&Packet, user->userNo, user->moveDirection, user->x, user->y);
	SendPacket_Around(user->userNo, &Packet);

	return true;
}

bool CS_MoveStop(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;

#ifdef DEBUG
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Move Stop [UserNo: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
#endif

	User* user = FindUser(from);
	if (user == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Exist Session & Non-Exist User [UserNo: %d]\n", from);

		return false;
	}

	// 싱크 맞춤
	if (abs(user->x - x) > dfERROR_RANGE || abs(user->y - y) > dfERROR_RANGE)
	{
		// 싱크 메세지 송신
		CPacket Packet;
		cpSC_Synchronize(&Packet, user->userNo, user->x, user->y);
		SendPacket_Around(user->userNo, &Packet, true);

		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync Sent [UserNo: %d][Direction: %d][C_X: %d][C_Y: %d]->[S_X: %d][S_Y: %d] [HP: %d]\n",
			from, direction, x, y, user->x, user->y, user->hp);

		x = user->x;
		y = user->y;
	}

	user->action = dfAction_STAND;
	user->moveDirection = dfAction_STAND;
	user->direction = direction;

	user->x = x;
	user->y = y;

	// 주변 섹션 메세지 송신
	if (Sector_UpdateUser(user))
	{
		UserSectorUpdatePacket(user);
	}
	
	CPacket Packet;
	cpSC_MoveStop(&Packet, user->userNo, user->direction, user->x, user->y);
	SendPacket_Around(user->userNo, &Packet);

	return true;
}

bool CS_Attack1(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;
#ifdef DEBUG
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
#endif

	User* attacker = FindUser(from);

	if (attacker == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Exist Session & Non-Exist User [UserNo: %d]\n", from);

		return false;
	}

	// 싱크 맞춤
	if (abs(attacker->x - x) > dfERROR_RANGE || abs(attacker->y - y) > dfERROR_RANGE)
	{
		// 싱크 메세지 송신
		CPacket Packet;
		cpSC_Synchronize(&Packet, attacker->userNo, attacker->x, attacker->y);
		SendPacket_Around(attacker->userNo, &Packet, true);

		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync Sent [UserNo: %d][Direction: %d][C_X: %d][C_Y: %d]->[S_X: %d][S_Y: %d] [HP: %d]\n",
			from, direction, x, y, attacker->x, attacker->y, attacker->hp);

		x = attacker->x;
		y = attacker->y;
	}

	attacker->action = dfACTION_ATTACK1;
	attacker->moveDirection = dfAction_STAND;
	attacker->direction = direction;
	attacker->x = x;
	attacker->y = y;

	if (Sector_UpdateUser(attacker))
	{
		UserSectorUpdatePacket(attacker);
	}

	CPacket Packet;
	cpSC_Attack1(&Packet, attacker->userNo, attacker->direction, attacker->x, attacker->y);
	SendPacket_Around(attacker->userNo, &Packet);

	switch (attacker->direction)
	{
	case dfACTION_MOVE_LL:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((attacker->x - user->x) <= dfATTACK1_RANGE_X &&
				(attacker->x - user->x) > 0 &&
				abs(attacker->y - user->y) <= dfATTACK1_RANGE_Y)
			{
				user->hp -= dfATTACK1_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX -= 1;
		sectorY -= 1;

		if (sectorX >= 0)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((attacker->x - user->x) <= dfATTACK1_RANGE_X &&
						(attacker->x - user->x) > 0 &&
						abs(attacker->y - user->y) <= dfATTACK1_RANGE_Y)
					{
						user->hp -= dfATTACK1_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	case dfACTION_MOVE_RR:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((user->x - attacker->x) <= dfATTACK1_RANGE_X &&
				(user->x - attacker->x) > 0 &&
				abs(user->y - attacker->y) <= dfATTACK1_RANGE_Y)
			{
				user->hp -= dfATTACK1_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX += 1;
		sectorY -= 1;

		if (sectorX < dfSECTOR_MAX_X)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((user->x - attacker->x) <= dfATTACK1_RANGE_X &&
						(user->x - attacker->x) > 0 &&
						abs(user->y - attacker->y) <= dfATTACK1_RANGE_Y)
					{
						user->hp -= dfATTACK1_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_1 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	default:
		break;
	}

	return true;
}

bool CS_Attack2(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;
#ifdef DEBUG
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
#endif	

	User* attacker = FindUser(from);

	if (attacker == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Exist Session & Non-Exist User [UserNo: %d]\n", from);

		return false;
	}

	// 싱크 맞춤
	if (abs(attacker->x - x) > dfERROR_RANGE || abs(attacker->y - y) > dfERROR_RANGE)
	{
		// 싱크 메세지 송신
		CPacket Packet;
		cpSC_Synchronize(&Packet, attacker->userNo, attacker->x, attacker->y);
		SendPacket_Around(attacker->userNo, &Packet, true);

		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync Sent [UserNo: %d][Direction: %d][C_X: %d][C_Y: %d]->[S_X: %d][S_Y: %d] [HP: %d]\n",
			from, direction, x, y, attacker->x, attacker->y, attacker->hp);

		x = attacker->x;
		y = attacker->y;
	}

	attacker->action = dfACTION_ATTACK2;
	attacker->moveDirection = dfAction_STAND;
	attacker->direction = direction;
	attacker->x = x;
	attacker->y = y;

	if (Sector_UpdateUser(attacker))
	{
		UserSectorUpdatePacket(attacker);
	}

	CPacket Packet;
	cpSC_Attack2(&Packet, attacker->userNo, attacker->direction, attacker->x, attacker->y);
	SendPacket_Around(attacker->userNo, &Packet);

	switch (attacker->direction)
	{
	case dfACTION_MOVE_LL:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((attacker->x - user->x) <= dfATTACK2_RANGE_X &&
				(attacker->x - user->x) > 0 &&
				abs(attacker->y - user->y) <= dfATTACK2_RANGE_Y)
			{
				user->hp -= dfATTACK2_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX -= 1;
		sectorY -= 1;

		if (sectorX >= 0)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((attacker->x - user->x) <= dfATTACK2_RANGE_X &&
						(attacker->x - user->x) > 0 &&
						abs(attacker->y - user->y) <= dfATTACK2_RANGE_Y)
					{
						user->hp -= dfATTACK2_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	case dfACTION_MOVE_RR:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((user->x - attacker->x) <= dfATTACK2_RANGE_X &&
				(user->x - attacker->x) > 0 &&
				abs(user->y - attacker->y) <= dfATTACK2_RANGE_Y)
			{
				user->hp -= dfATTACK2_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX += 1;
		sectorY -= 1;

		if (sectorX < dfSECTOR_MAX_X)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((user->x - attacker->x) <= dfATTACK2_RANGE_X &&
						(user->x - attacker->x) > 0 &&
						abs(user->y - attacker->y) <= dfATTACK2_RANGE_Y)
					{
						user->hp -= dfATTACK2_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_2 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	default:
		break;
	}

	return true;
}

bool CS_Attack3(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);

		return false;
	}

	BYTE direction;
	short x;
	short y;

	*packet >> direction >> x >> y;
#ifdef DEBUG
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 [Attacker: %d][Direction: %d][X: %d][Y: %d]\n",
		from, direction, x, y);
#endif

	User* attacker = FindUser(from);

	if (attacker == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Exist Session & Non-Exist User [UserNo: %d]\n", from);

		return false;
	}

	// 싱크 맞춤
	if (abs(attacker->x - x) > dfERROR_RANGE || abs(attacker->y - y) > dfERROR_RANGE)
	{
		// 싱크 메세지 송신
		CPacket Packet;
		cpSC_Synchronize(&Packet, attacker->userNo, attacker->x, attacker->y);
		SendPacket_Around(attacker->userNo, &Packet, true);

		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync Sent [UserNo: %d][Direction: %d][C_X: %d][C_Y: %d]->[S_X: %d][S_Y: %d] [HP: %d]\n",
			from, direction, x, y, attacker->x, attacker->y, attacker->hp);

		x = attacker->x;
		y = attacker->y;

	}

	attacker->action = dfACTION_ATTACK3;
	attacker->moveDirection = dfAction_STAND;
	attacker->direction = direction;
	attacker->x = x;
	attacker->y = y;

	if (Sector_UpdateUser(attacker))
	{
		UserSectorUpdatePacket(attacker);
	}

	CPacket Packet;
	cpSC_Attack3(&Packet, attacker->userNo, attacker->direction, attacker->x, attacker->y);
	SendPacket_Around(attacker->userNo, &Packet);

	switch (attacker->direction)
	{
	case dfACTION_MOVE_LL:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((attacker->x - user->x) <= dfATTACK3_RANGE_X &&
				(attacker->x - user->x) > 0 &&
				abs(attacker->y - user->y) <= dfATTACK3_RANGE_Y)
			{
				user->hp -= dfATTACK3_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX -= 1;
		sectorY -= 1;

		if (sectorX >= 0)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((attacker->x - user->x) <= dfATTACK3_RANGE_X &&
						(attacker->x - user->x) > 0 &&
						abs(attacker->y - user->y) <= dfATTACK3_RANGE_Y)
					{
						user->hp -= dfATTACK3_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	case dfACTION_MOVE_RR:
	{
		// 지금 섹터 조사
		int sectorX = attacker->curSector.x;
		int sectorY = attacker->curSector.y;

		for (auto iter = g_Sector[sectorY][sectorX].begin();
			iter != g_Sector[sectorY][sectorX].end(); ++iter)
		{
			User* user = *iter;

			if ((user->x - attacker->x) <= dfATTACK3_RANGE_X &&
				(user->x - attacker->x) > 0 &&
				abs(user->y - attacker->y) <= dfATTACK3_RANGE_Y)
			{
				user->hp -= dfATTACK3_DAMAGE;
				Packet.Clear();

				cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
				SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
				g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
					from, direction, user->userNo, x, y);
#endif
			}
		}
		// 인접 섹터 조사
		sectorX += 1;
		sectorY -= 1;

		if (sectorX < dfSECTOR_MAX_X)
		{
			for (int i = 0; i < 3; ++i, ++sectorY)
			{
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
				{
					continue;
				}

				for (auto iter = g_Sector[sectorY][sectorX].begin();
					iter != g_Sector[sectorY][sectorX].end(); ++iter)
				{
					User* user = *iter;

					if ((user->x - attacker->x) <= dfATTACK3_RANGE_X &&
						(user->x - attacker->x) > 0 &&
						abs(user->y - attacker->y) <= dfATTACK3_RANGE_Y)
					{
						user->hp -= dfATTACK3_DAMAGE;
						Packet.Clear();

						cpSC_Damage(&Packet, attacker->userNo, user->userNo, user->hp);
						SendPacket_Around(user->userNo, &Packet, true);
#ifdef DEBUG
						g_Logger._Log(dfLOG_LEVEL_DEBUG, L"CS Attack_3 HIT [Attacker: %d][Direction: %d][Dest: %d][X: %d][Y: %d]\n",
							from, direction, user->userNo, x, y);
#endif
					}
				}
			}
		}

		break;
	}
	default:
		break;
	}

	return true;
}

bool CS_Echo(DWORD from, CPacket* packet)
{
	Session* session = FindSession(from);

	if (session == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_ERROR, L"Non-Exist Session [SessionNo: %d]\n", from);

		return false;
	}

	DWORD time;

	*packet >> time;

	ULONGLONG curTime = GetTickCount64();
	CPacket Packet;

	session->SetLastRecvTime(curTime);
	
	cpSC_Echo(&Packet, time);
	SendPacket_Unicast(from, &Packet);	

	return true;
}

bool UserMoveCheck(int x, int y)
{
	return x >= 0 && x < dfRANGE_MOVE_RIGHT && y >= 0 && y < dfRANGE_MOVE_BOTTOM;
}
