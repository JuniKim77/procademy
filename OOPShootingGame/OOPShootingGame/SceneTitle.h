#ifndef SCENETITLE
#define SCENETITLE
#include "SceneProcess.h"

class SceneTitle : public SceneProcess
{
public:
	SceneTitle();
	// Scene을(를) 통해 상속됨
	virtual void GetKeyChange() override;
};

#endif
