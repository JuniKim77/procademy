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
	static SceneType mNextSceneType;
	static char** mFileNameArray;

private:
	~SceneManager();
	SceneManager();
	void LoadFileNameList();
	void GetNextLine(char** pBegin, char** pEnd, char* buffer);

private:
	enum {
		FILE_NAME_SIZE = 64
	};
	static SceneManager* mManager;
	Scene* mpScene;	
	const char* mFileList = "file_list.txt";
	int mFileListSize;
};

#endif