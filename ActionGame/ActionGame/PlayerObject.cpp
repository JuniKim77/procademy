#include "PlayerObject.h"
#include "ActionDefine.h"
#include "ESprite.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include <stdio.h>
#include "EffectObject.h"
#include "Session.h"
#include "PacketCreater.h"

#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

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

void PlayerObject::Render(BYTE* pDest, int destWidth, int destHeight, int destPitch)
{
	gSpriteDib.DrawSprite50(eSHADOW, mCurX, mCurY, pDest,
		destWidth, destHeight, destPitch);
	if (mbPlayerCharacter)
	{
		gSpriteDib.DrawSprite(GetSprite(), mCurX, mCurY, pDest, destWidth,
			destHeight, destPitch);
	}
	else
	{
		gSpriteDib.DrawSpriteRed(GetSprite(), mCurX, mCurY, pDest, destWidth,
			destHeight, destPitch);
	}

	gSpriteDib.DrawSprite(eGUAGE_HP, mCurX - 35, mCurY + 9, pDest, destWidth,
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

	if (mActionCur == mActionInput && this == gPlayerObject)
		return;

	//wprintf_s(L"ID: %d, ActionCur: %d\nActionInput: %d\n", mObjectID, mActionCur, mActionInput);

	mActionOld = mActionCur;
	mActionCur = mActionInput;

	switch (mActionCur)
	{
	case dfACTION_ATTACK1:
		if (this == gPlayerObject)
			SetActionAttack1();
		else
			SetActionAttack1(false);
		break;
	case dfACTION_ATTACK2:
		if (this == gPlayerObject)
			SetActionAttack2();
		else
			SetActionAttack2(false);
		break;
	case dfACTION_ATTACK3:
		if (this == gPlayerObject)
			SetActionAttack3();
		else
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
		if (this == gPlayerObject)
			SetActionMove();
		else
			SetActionMove(false);
		break;
	case dfAction_STAND:
		if (this == gPlayerObject)
			SetActionStand();
		else
			SetActionStand(false);
		break;
	default:
		break;
	}
}

void PlayerObject::InputActionProc()
{
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
	stHeader header;
	csAttack1 packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04, dfDELAY_ATTACK1);
		CreateAttack1Packet(&header, &packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04, dfDELAY_ATTACK1);
		CreateAttack1Packet(&header, &packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND) 
	{
		stHeader stopHeader;
		csMoveStop stopPacket;

		CreateMoveStopPacket(&stopHeader, &stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket((char*)&stopHeader, sizeof(stopHeader));
		g_session.SendPacket((char*)&stopPacket, sizeof(stopPacket));
	}

	g_session.SendPacket((char*)&header, sizeof(header));
	g_session.SendPacket((char*)&packet, sizeof(packet));

	g_session.writeProc();

	LogPacket(&header, (char*)&packet, mObjectID);
}

void PlayerObject::SetActionAttack2(bool sendMsg)
{
	stHeader header;
	csAttack2 packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04, dfDELAY_ATTACK2);
		CreateAttack2Packet(&header, &packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04, dfDELAY_ATTACK2);
		CreateAttack2Packet(&header, &packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND)
	{
		stHeader stopHeader;
		csMoveStop stopPacket;

		CreateMoveStopPacket(&stopHeader, &stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket((char*)&stopHeader, sizeof(stopHeader));
		g_session.SendPacket((char*)&stopPacket, sizeof(stopPacket));
	}

	g_session.SendPacket((char*)&header, sizeof(header));
	g_session.SendPacket((char*)&packet, sizeof(packet));

	g_session.writeProc();

	LogPacket(&header, (char*)&packet, mObjectID);
}

void PlayerObject::SetActionAttack3(bool sendMsg)
{
	stHeader header;
	csAttack3 packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L04, dfDELAY_ATTACK3);
		CreateAttack3Packet(&header, &packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R04, dfDELAY_ATTACK3);
		CreateAttack3Packet(&header, &packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	if (mActionOld != dfAction_STAND)
	{
		stHeader stopHeader;
		csMoveStop stopPacket;

		CreateMoveStopPacket(&stopHeader, &stopPacket, mDirCur, mCurX, mCurY);

		g_session.SendPacket((char*)&stopHeader, sizeof(stopHeader));
		g_session.SendPacket((char*)&stopPacket, sizeof(stopPacket));
	}

	g_session.SendPacket((char*)&header, sizeof(header));
	g_session.SendPacket((char*)&packet, sizeof(packet));

	g_session.writeProc();

	LogPacket(&header, (char*)&packet, mObjectID);
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

	stHeader header;
	csMoveStart packet;

	CreateMoveStartPacket(&header, &packet, mActionCur, mCurX, mCurY);

	g_session.SendPacket((char*)&header, sizeof(header));
	g_session.SendPacket((char*)&packet, sizeof(packet));

	g_session.writeProc();

	LogPacket(&header, (char*)&packet, mObjectID);
}

void PlayerObject::SetActionStand(bool sendMsg)
{
	stHeader header;
	csMoveStop packet;

	if (mbIsLeft)
	{
		SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
		CreateMoveStopPacket(&header, &packet, dfPACKET_MOVE_DIR_LL, mCurX, mCurY);
	}
	else
	{
		SetSprite(ePLAYER_STAND_R01, ePLAYER_STAND_R03, dfDELAY_STAND);
		CreateMoveStopPacket(&header, &packet, dfPACKET_MOVE_DIR_RR, mCurX, mCurY);
	}

	if (!sendMsg)
		return;

	g_session.SendPacket((char*)&header, sizeof(header));
	g_session.SendPacket((char*)&packet, sizeof(packet));

	g_session.writeProc();

	LogPacket(&header, (char*)&packet, mObjectID);
}

void PlayerObject::Move()
{
	int oldX = mCurX;
	int oldY = mCurY;

	switch (mActionCur)
	{
	case dfACTION_MOVE_LL:
		mCurX = max(dfRANGE_MOVE_LEFT, mCurX - 3);
		break;
	case dfACTION_MOVE_LU:
		mCurX = max(dfRANGE_MOVE_LEFT, mCurX - 3);
		mCurY = max(dfRANGE_MOVE_TOP, mCurY - 2);
		break;
	case dfACTION_MOVE_UU:
		mCurY = max(dfRANGE_MOVE_TOP, mCurY - 2);
		break;
	case dfACTION_MOVE_RU:
		mCurX = min(dfRANGE_MOVE_RIGHT, mCurX + 3);
		mCurY = max(dfRANGE_MOVE_TOP, mCurY - 2);
		break;
	case dfACTION_MOVE_RR:
		mCurX = min(dfRANGE_MOVE_RIGHT, mCurX + 3);
		break;
	case dfACTION_MOVE_RD:
		mCurX = min(dfRANGE_MOVE_RIGHT, mCurX + 3);
		mCurY = min(dfRANGE_MOVE_BOTTOM, mCurY + 2);
		break;
	case dfACTION_MOVE_DD:
		mCurY = min(dfRANGE_MOVE_BOTTOM, mCurY + 2);
		break;
	case dfACTION_MOVE_LD:
		mCurX = max(dfRANGE_MOVE_LEFT, mCurX - 3);
		mCurY = min(dfRANGE_MOVE_BOTTOM, mCurY + 2);
		break;
	default:
		break;
	}

	// 경계 처리
	if (mCurX == dfRANGE_MOVE_LEFT || mCurX == dfRANGE_MOVE_RIGHT)
		mCurY = oldY;

	if (mCurY == dfRANGE_MOVE_TOP || mCurY == dfRANGE_MOVE_BOTTOM)
		mCurX = oldX;
}

bool PlayerObject::IsAttackAction(DWORD action) const
{
	return action == dfACTION_ATTACK1 ||
		action == dfACTION_ATTACK2 ||
		action == dfACTION_ATTACK3;
}
