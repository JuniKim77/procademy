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
	void Init();

public:
	static bool mbChangeScene;
	static bool mbExit;
	static SceneType mNextSceneType;
	static char** mProcessFileNameArray;
	static char** mStageFileNameArray;
	static int mCurrentStage;
	static bool mbNextStage;

private:
	~SceneManager();
	SceneManager();
	void LoadFileNameList();

private:
	enum {
		FILE_NAME_SIZE = 64
	};
	static SceneManager* mManager;
	Scene* mpScene;	
	const char* mFileList = "file_list.txt";
	int mProcessFileListSize;
	int mStageFileListSize;
};

#endif