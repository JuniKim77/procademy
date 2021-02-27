#include "BaseObject.h"

class ThreeStar : public BaseObject
{
	// BaseObject을(를) 통해 상속됨
	virtual bool Run() override;
	virtual void Render() override;
};