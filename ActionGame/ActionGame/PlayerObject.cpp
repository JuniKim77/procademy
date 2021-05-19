#include "PlayerObject.h"
#include "ActionDefine.h"
#include "ESprite.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include <stdio.h>
#include "EffectObject.h"

#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

PlayerObject::PlayerObject()
	: mDir(dfAction_STAND)
	, mHP(100)
	, mbIsLeft(true)
{
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
	ActionInputProc();  // Action input 처리 : 이동 방향과 액션을 정리
	Action(); // 업데이트
}

void PlayerObject::Action()
{
	Move(); // 이동
	NextFrame();
}

void PlayerObject::ActionInputProc()
{
	if (!IsInputChanged())
		return;

	if (IsAttackAction())
	{
		if (!mbEndFrame)
			return;
	}

	mInAction = mActionInput;

	switch (mActionInput)
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
	case dfAction_STAND:
		SetActionStand();
		break;
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		mbIsLeft = true;
		mDir = mActionInput;
		SetActionMove();
		break;
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		mbIsLeft = false;
		mDir = mActionInput;
		SetActionMove();
		break;
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_DD:
		mDir = mActionInput;
		SetActionMove();
		break;
	default:
		SetActionStand();
		break;
	}
}

void PlayerObject::SetActionStand()
{
	if (mbIsLeft)
	{
		SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
	}
	else
	{
		SetSprite(ePLAYER_STAND_R01, ePLAYER_STAND_R03, dfDELAY_STAND);
	}
	mDir = dfAction_STAND;
}

void PlayerObject::SetActionAttack1()
{
	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04, dfDELAY_ATTACK1);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX - 100, mCurY - 50);
		gObjectList.push_back(effect);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04, dfDELAY_ATTACK1);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX + 100, mCurY - 50);
		gObjectList.push_back(effect);
	}

	mDir = dfAction_STAND;
}

void PlayerObject::SetActionAttack2()
{
	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04, dfDELAY_ATTACK2);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX - 100, mCurY - 50);
		gObjectList.push_back(effect);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04, dfDELAY_ATTACK2);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX + 100, mCurY - 50);
		gObjectList.push_back(effect);
	}
	mDir = dfAction_STAND;
}

void PlayerObject::SetActionAttack3()
{
	if (mbIsLeft)
	{
		SetSprite(ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L04, dfDELAY_ATTACK3);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX - 100, mCurY - 50);
		gObjectList.push_back(effect);
	}
	else
	{
		SetSprite(ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R04, dfDELAY_ATTACK3);
		BaseObject* effect = new EffectObject;
		effect->SetPosition(mCurX + 100, mCurY - 50);
		gObjectList.push_back(effect);
	}

	mDir = dfAction_STAND;
}

void PlayerObject::SetActionMove()
{
	if (mbIsLeft)
	{
		SetSprite(ePLAYER_MOVE_L01, ePLAYER_MOVE_L12, dfDELAY_MOVE);
	}
	else
	{
		SetSprite(ePLAYER_MOVE_R01, ePLAYER_MOVE_R12, dfDELAY_MOVE);
	}
}

void PlayerObject::Move()
{
	switch (mDir)
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
}
