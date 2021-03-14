#ifndef SCENEMANAGER
#define SCENEMANAGER

#include "Scene.h"
#include "SceneType.h"

class SceneManager
{
public:
	static SceneManager* GetInstance();
	void Run();
	void LoadScene();
	void Destroy();

private:
	~SceneManager();
	SceneManager();

private:
	enum {
		FILE_NAME_SIZE = 64
	};
	static SceneManager* mManager;
	Scene* mpScene;
};

#endif