#ifndef SCENEOVER
#define SCENEOVER
#include "SceneProcess.h"

class SceneOver : public SceneProcess
{
public:
	SceneOver();
	// Scene을(를) 통해 상속됨
	virtual void GetKeyChange() override;
};

#endif