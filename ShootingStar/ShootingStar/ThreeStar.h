#include "BaseObject.h"

class ThreeStar : public BaseObject
{
	// BaseObject��(��) ���� ��ӵ�
	virtual bool Run() override;
	virtual void Render() override;
};