#ifndef SCENEVICTORY
#define SCENEVICTORY
#include "SceneProcess.h"

class SceneVictory : public SceneProcess
{
public:
	SceneVictory();
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() override;
};

#endif