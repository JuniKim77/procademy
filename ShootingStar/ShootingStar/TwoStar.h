#pragma once
#include "BaseObject.h"

class TwoStar : public BaseObject
{
	// BaseObject��(��) ���� ��ӵ�
	virtual bool Update() override;
	virtual void Render() override;
};