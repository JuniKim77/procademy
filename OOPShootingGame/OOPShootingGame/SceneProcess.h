#ifndef SCENEPROCESS
#define SCENEPROCESS
#include "Scene.h"

class SceneProcess : public Scene
{
public:
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() = 0;
	virtual void Update() override;
	virtual void Render() override;

protected:
	SceneProcess(SceneType type, const char* message);

protected:
	virtual void LoadCSVFile() override;

protected:
	int mPrevTime;
	bool mbMessageOn;
	const char* mProcessMessage;
};

#endif
