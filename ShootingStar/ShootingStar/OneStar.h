#pragma once
#include "BaseObject.h"

class OneStar : public BaseObject
{
	// BaseObject��(��) ���� ��ӵ�
	virtual bool Update() override;
	virtual void Render() override;
};