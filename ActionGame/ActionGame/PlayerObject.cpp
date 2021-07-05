#include "PlayerObject.h"
#include "ActionDefine.h"
#include "ESprite.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include <stdio.h>
#include "EffectObject.h"
#include "Session.h"
#include "PacketCreater.h"
#include "CPacket.h"
#include "PacketDefine.h"

//#define dfRANGE_MOVE_TOP	50
//#define dfRANGE_MOVE_LEFT	10
//#define dfRANGE_MOVE_RIGHT	630
//#define dfRANGE_MOVE_BOTTOM	470

extern Session g_session;

PlayerObject::PlayerObject()
	: mDirCur(dfACTION_MOVE_LL)
	, mDirOld(dfACTION_MOVE_LL)
	, mActionCur(dfAction_STAND)
	, mActionOld(dfAction_STAND)
	, mHP(100)
	, mbIsLeft(true)
{
	SetActionStand(false);
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::Render(BYTE* pDest, int destWidth, int destHeight, int destPitch, COORD* camera)
{
	int x = mCurX - camera->X;
	int y = mCurY - camera->Y;

	gSpriteDib.DrawSprite50(eSHADOW, x, y, pDest,
		destWidth, destHeight, destPitch);
	if (mbPlayerCharacter)
	{
		gSpriteDib.DrawSprite(GetSprite(), x, y, pDest, destWidth,
			destHeight, destPitch);
	}
	else
	{
		gSpriteDib.DrawSpriteRed(GetSprite(), x, y, pDest, destWidth,
			destHeight, destPitch);
	}

	gSpriteDib.DrawSprite(eGUAGE_HP, x - 35, y + 9, pDest, destWidth,
		destHeight, destPitch, GetHP());
}

void PlayerObject::Run()
{
	Action(); // 업데이트
	ActionProc();  // Action input 처리 : 이동 방향과 액션을 정리
}

void PlayerObject::Action()
{
	Move(); // 이동
	NextFrame();
}

void PlayerObject::ActionProc()
{
	if (mActionInput == dfAction_NONE)
		return;

	if (this == gPlayerObject)
		ActionPlayerProc();
	else
		ActionEnemyProc();
}

void PlayerObject::ActionPlayerProc()
{
	if (IsAttackAction(mActionCur))
	{
		if (!mbEndFrame)
			return;

		SetActionStand(false);
		mActionInput = dfAction_STAND;
		mActionOld = mActionCur;
		mActionCur = mActionInput;
		return;
	}

	if (mActionCur == mActionInput)
		return;

	//wprintf_s(L"ID: %d, ActionCur: %d\nActionInput: %d\n", mObjectID, mActionCur, mActionInput);

	mActionOld = mActionCur;
	mActionCur = mActionInput;

	switch (mActionCur)
	{
	case dfACTION_ATTACK1:
		SetActionAttack1();
		break;
	case dfACTION_ATTACK2:
		SetActionAttack2();
		break;
	case dfACTION_ATTACK3:
		SetActionAttack3();
		break;
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_DD:
		SetActionMove();
		break;
	case dfAction_STAND:
		SetActionStand();
		break;
	default:
		break;
	}
}

void PlayerObject::ActionEnemyProc()
{
	if (IsAttackAction(mActionCur))
	{
		if (!mbEndFrame)
			return;

		SetActionStand(false);
		mActionInput = dfAction_STAND;
		mActionOld = mActionCur;
		mActionCur = mActionInput;
		return;
	}
	else
	{
		if (mActionCur == mActionInput)
		{
			return;
		}
	}

	//wprintf_s(L"ID: %d, ActionCur: %d\nActionInput: %d\n", mObjectID, mActionCur, mActionInput);

	mActionOld = mActionCur;
	mActionCur = mActionInput;

	switch (mActionCur)
	{
	case dfACTION_ATTACK1:
		SetActionAttack1(false);
		break;
	case dfACTION_ATTACK2:
		SetActionAttack2(false);
		break;
	case dfACTION_ATTACK3:
		SetActionAttack3(false);
		break;
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_DD:
		SetActionMove(false);
		break;
	case dfAction_STAND:
		SetActionStand(false);
		break;
	default:
		break;
	}
}

void PlayerObject::SetDirection(DWORD dir)
{
	mDirCur = dir;
	if (mDirCur == dfAction_NONE) {
		SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
	}
}

void PlayerObject::SetActionAttack1(bool sendMsg)
{
	CPacket packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04, dfDELAY_ATTACK1);
		CreateAttack1Packet(&packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04, dfDELAY_ATTACK1);
		CreateAttack1Packet(&packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND)
	{
		CPacket stopPacket;

		CreateMoveStopPacket(&stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket(stopPacket.GetBufferPtr(), stopPacket.GetSize());
	}

	g_session.SendPacket(packet.GetBufferPtr(), packet.GetSize());

	g_session.writeProc();

	LogPacket(&packet, mObjectID);
}

void PlayerObject::SetActionAttack2(bool sendMsg)
{
	CPacket packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04, dfDELAY_ATTACK2);
		CreateAttack2Packet(&packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04, dfDELAY_ATTACK2);
		CreateAttack2Packet(&packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND)
	{
		CPacket stopPacket;

		CreateMoveStopPacket(&stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket(stopPacket.GetBufferPtr(), stopPacket.GetSize());
	}

	g_session.SendPacket(packet.GetBufferPtr(), packet.GetSize());

	g_session.writeProc();

	LogPacket(&packet, mObjectID);
}

void PlayerObject::SetActionAttack3(bool sendMsg)
{
	CPacket packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L04, dfDELAY_ATTACK3);
		CreateAttack3Packet(&packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R04, dfDELAY_ATTACK3);
		CreateAttack3Packet(&packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND)
	{
		CPacket stopPacket;

		CreateMoveStopPacket(&stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket(stopPacket.GetBufferPtr(), stopPacket.GetSize());
	}

	g_session.SendPacket(packet.GetBufferPtr(), packet.GetSize());

	g_session.writeProc();

	LogPacket(&packet, mObjectID);
}

void PlayerObject::CreateEffect()
{
	if (mbIsLeft)
	{
		BaseObject* effect = new EffectObject;
		if (!IsPlayer()) {
			effect->SetEnemy();
		}
		effect->SetPosition(mCurX - 80, mCurY - 60);
		gObjectList.push_back(effect);
	}
	else
	{
		BaseObject* effect = new EffectObject;
		if (!IsPlayer()) {
			effect->SetEnemy();
		}
		effect->SetPosition(mCurX + 80, mCurY - 60);
		gObjectList.push_back(effect);
	}
}

void PlayerObject::SetActionMove(bool sendMsg)
{
	switch (mActionCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		mbIsLeft = true;
		break;
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		mbIsLeft = false;
		break;
	default:
		break;
	}

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_MOVE_L01, ePLAYER_MOVE_L12, dfDELAY_MOVE);
	}
	else
	{
		SetSprite(ePLAYER_MOVE_R01, ePLAYER_MOVE_R12, dfDELAY_MOVE);
	}

	mDirOld = mDirCur;
	mDirCur = mActionCur;

	if (!sendMsg)
		return;

	CPacket packet;

	CreateMoveStartPacket(&packet, mActionCur, mCurX, mCurY);

	g_session.SendPacket(packet.GetBufferPtr(), packet.GetSize());

	g_session.writeProc();

	LogPacket(&packet, mObjectID);
}

void PlayerObject::SetActionStand(bool sendMsg)
{
	CPacket packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
		CreateMoveStopPacket(&packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_STAND_R01, ePLAYER_STAND_R03, dfDELAY_STAND);
		CreateMoveStopPacket(&packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	g_session.SendPacket(packet.GetBufferPtr(), packet.GetSize());

	g_session.writeProc();

	LogPacket(&packet, mObjectID);
}

void PlayerObject::Move()
{
	int x = mCurX;
	int y = mCurY;

	switch (mActionCur)
	{
	case dfACTION_MOVE_LL:
		x = mCurX - 3;
		break;
	case dfACTION_MOVE_LU:
		x = mCurX - 3;
		y = mCurY - 2;
		break;
	case dfACTION_MOVE_UU:
		y = mCurY - 2;
		break;
	case dfACTION_MOVE_RU:
		x = mCurX + 3;
		y = mCurY - 2;
		break;
	case dfACTION_MOVE_RR:
		x = mCurX + 3;
		break;
	case dfACTION_MOVE_RD:
		x = mCurX + 3;
		y = mCurY + 2;
		break;
	case dfACTION_MOVE_DD:
		y = mCurY + 2;
		break;
	case dfACTION_MOVE_LD:
		x = mCurX - 3;
		y = mCurY + 2;
		break;
	default:
		break;
	}

	// 경계 처리
	if (x < dfRANGE_MOVE_LEFT || x > dfRANGE_MOVE_RIGHT ||
		y < dfRANGE_MOVE_TOP || y > dfRANGE_MOVE_BOTTOM)
		return;

	mCurX = x;
	mCurY = y;
}

bool PlayerObject::IsAttackAction(DWORD action) const
{
	return action == dfACTION_ATTACK1 ||
		action == dfACTION_ATTACK2 ||
		action == dfACTION_ATTACK3;
}
