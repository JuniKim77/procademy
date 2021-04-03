#ifndef GAMEGLOBALDATA
#define GAMEGLOBALDATA

#define NAME_SIZE (64)

enum class SceneType;

struct ObjectStat
{
	char name[NAME_SIZE];
	int hp;
	int damage;
	char image;
};


class GameGlobalData
{
public:
	friend class SceneManager;
	friend class ObjectManager;
	bool GetChangeScene();
	void SetChangeScene(bool flag);
	bool GetExit();
	void SetExit(bool exit);
	SceneType GetNextSceneType();
	void SetNextSceneType(SceneType type);
	char** GetProcessFileNameArray();
	char** GetStageFileNameArray();
	int GetCurrentStage();
	void SetCurrentStage(int stage);
	bool GetNextStage();
	ObjectStat* GetObjectStats();
	static void Destroy();
	static GameGlobalData* GetInstance();
	void LoadFileNameList();
	void Init();
	void LoadCSVFile(const char* fileName);

private:
	GameGlobalData();
	~GameGlobalData();

private:
	static GameGlobalData* mObject;
	bool mbChangeScene = true;
	bool mbExit = false;
	SceneType mNextSceneType;
	char** mProcessFileNameArray = nullptr;
	int mProcessFileListSize = 0;
	char** mStageFileNameArray = nullptr;
	int mStageFileListSize = 0;
	int mCurrentStage = 1;
	bool mbNextStage = false;
	ObjectStat* mObjectStats = nullptr;
	const char* mFileList = "file_list.txt";
};

#endif
