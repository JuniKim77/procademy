#include "EffectObject.h"
#include "SpriteDib.h"
#include "ESprite.h" 
#include "ActionDefine.h"
#include <stdio.h>

EffectObject::EffectObject()
	: mbEffectBegin(false)
	, mAttackID(0)
{
	mObjectType = EObjectType::OBJECT_TYPE_EFFECT;
	SetSprite(eEFFECT_SPARK_01, eEFFECT_SPARK_04, dfDELAY_EFFECT);
}

EffectObject::~EffectObject()
{
}

void EffectObject::Render(BYTE* pDest, int destWidth, int destHeight, int destPitch)
{
	if (mbEffectBegin && !mbEndFrame)
	{
		if (IsPlayer())
		{
			gSpriteDib.DrawSprite(GetSprite(), mCurX, mCurY, pDest, destWidth,
				destHeight, destPitch);
		}
		else
		{
			gSpriteDib.DrawSpriteRed(GetSprite(), mCurX, mCurY, pDest, destWidth,
				destHeight, destPitch);
		}
	}
}

void EffectObject::Run()
{
	if (mbEffectBegin)
	{
		NextFrame();
	}
	else
	{
		CheckDistance();
	}
}

void EffectObject::CheckDistance()
{
	for (auto iter = gObjectList.begin(); iter != gObjectList.end(); ++iter)
	{
		if ((*iter)->GetObectID() == mAttackID)
		{
			mbEffectBegin = true;
			return;
		}
	}
}
