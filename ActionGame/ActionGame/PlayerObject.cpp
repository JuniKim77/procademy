#include "PlayerObject.h"
#include "ActionDefine.h"
#include "ESprite.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include <stdio.h>

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
	ActionInputProc();
	Action();
}

void PlayerObject::Action()
{
	Move();
	NextFrame();
	ActionProc();
}

void PlayerObject::ActionInputProc()
{
	if (mInAction == mActionInput)
		return;

	if (mInAction == dfACTION_ATTACK1 || mInAction == dfACTION_ATTACK2 || mInAction == dfACTION_ATTACK3)
	{
		if (!mbEndFrame)
			return;
	}

	mInAction = mActionInput;

	switch (mActionInput)
	{
	case dfACTION_ATTACK1:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04, dfDELAY_ATTACK1);
		}
		else
		{
			SetSprite(ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04, dfDELAY_ATTACK1);
		}
		mDir = dfAction_STAND;
		break;
	case dfACTION_ATTACK2:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04, dfDELAY_ATTACK2);
		}
		else
		{
			SetSprite(ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04, dfDELAY_ATTACK2);
		}
		mDir = dfAction_STAND;
		break;
	case dfACTION_ATTACK3:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L04, dfDELAY_ATTACK3);
		}
		else
		{
			SetSprite(ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R04, dfDELAY_ATTACK3);
		}
		mDir = dfAction_STAND;
		break;
	case dfAction_STAND:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
		}
		else
		{
			SetSprite(ePLAYER_STAND_R01, ePLAYER_STAND_R03, dfDELAY_STAND);
		}
		mDir = dfAction_STAND;
		break;
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetSprite(ePLAYER_MOVE_L01, ePLAYER_MOVE_L12, dfDELAY_MOVE);
		mDir = mActionInput;
		mbIsLeft = true;
		break;
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetSprite(ePLAYER_MOVE_R01, ePLAYER_MOVE_R12, dfDELAY_MOVE);
		mbIsLeft = false;
		mDir = mActionInput;
		break;
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_DD:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_MOVE_L01, ePLAYER_MOVE_L12, dfDELAY_MOVE);
		}
		else
		{
			SetSprite(ePLAYER_MOVE_R01, ePLAYER_MOVE_R12, dfDELAY_MOVE);
		}
		mDir = mActionInput;
		break;
	default:
		if (mbIsLeft)
		{
			SetSprite(ePLAYER_STAND_L01, ePLAYER_STAND_L03, dfDELAY_STAND);
		}
		else
		{
			SetSprite(ePLAYER_STAND_R01, ePLAYER_STAND_R03, dfDELAY_STAND);
		}
		break;
	}
}

void PlayerObject::ActionProc()
{
	switch (mActionInput)
	{
	case dfACTION_ATTACK1:
	case dfACTION_ATTACK2:
	case dfACTION_ATTACK3:
		if (mbEndFrame)
		{
			SetActionStand();

			mActionInput = dfAction_STAND;
		}
		break;
	default:
		break;
	}
}

void PlayerObject::SetActionStand()
{
}

void PlayerObject::SetActionAttack1()
{
}

void PlayerObject::SetActionAttack2()
{
}

void PlayerObject::SetActionAttack3()
{
}

void PlayerObject::SetActionMove()
{
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
