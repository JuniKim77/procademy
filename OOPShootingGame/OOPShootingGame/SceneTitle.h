#ifndef SCENETITLE
#define SCENETITLE
#include "SceneProcess.h"

class SceneTitle : public SceneProcess
{
public:
	SceneTitle();
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() override;
};

#endif