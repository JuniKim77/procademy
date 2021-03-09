#ifndef SCENEPLAY
#define SCENEPLAY
#include "Scene.h"

class ScenePlay : public Scene
{
public:
	ScenePlay();
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() override;
	virtual void Update() override;
	virtual void Render() override;

private:
	virtual void LoadCSVFile() override;

private:
	int mStage = 1;
};

#endif