#ifndef SCENEVICTORY
#define SCENEVICTORY
#include "SceneProcess.h"

class SceneVictory : public SceneProcess
{
public:
	SceneVictory();
	// Scene을(를) 통해 상속됨
	virtual void GetKeyChange() override;
};

#endif