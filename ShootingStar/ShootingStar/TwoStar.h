#include "BaseObject.h"

class TwoStar : public BaseObject
{
	// BaseObject��(��) ���� ��ӵ�
	virtual bool Run() override;
	virtual void Render() override;
};