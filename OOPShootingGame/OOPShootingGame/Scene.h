#ifndef SCENE
#define SCENE
#include "myList.h"
#include "Console.h"

enum class SceneType;
class CSVFile;
class ObjectBase;

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern myList<ObjectBase*> gObjectList;

class Scene
{
public:
	friend class SceneManager;
	virtual ~Scene();
	virtual void GetKeyChange() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	Scene(SceneType type);
	virtual void LoadCSVFile() = 0;
	virtual void RenderCSVFileData(int line, const char** pBuf);
	CSVFile* mCSVReader = nullptr;
	SceneType mSceneType;
};

#endif