#ifndef SCENEOVER
#define SCENEOVER
#include "SceneProcess.h"

class SceneOver : public SceneProcess
{
public:
	SceneOver();
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() override;
};

#endif