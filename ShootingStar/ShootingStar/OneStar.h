#pragma once
#include "BaseObject.h"

class OneStar : public BaseObject
{
	// BaseObject을(를) 통해 상속됨
	virtual bool Update() override;
	virtual void Render() override;
};