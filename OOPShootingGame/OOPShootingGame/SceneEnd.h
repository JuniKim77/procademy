#ifndef SCENEEND
#define SCENEEND
#include "Scene.h"

class SceneEnd : public Scene
{
public:
	SceneEnd();
	// Scene을(를) 통해 상속됨
	virtual void GetKeyChange() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual ~SceneEnd();

private:
	virtual void LoadCSVFile() override;

private:
};

#endif
