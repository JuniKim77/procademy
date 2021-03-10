#ifndef SCENE
#define SCENE
#include "myList.h"
#include "ObjectBase.h"
#include "Console.h"
#include "CSVReader.h"
#include "SceneType.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern myList<ObjectBase*> gObjectList;

class Scene
{
public:
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