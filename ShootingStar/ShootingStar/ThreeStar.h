#pragma once
#include "BaseObject.h"

class ThreeStar : public BaseObject
{
	// BaseObject��(��) ���� ��ӵ�
	virtual bool Update() override;
	virtual void Render() override;
};