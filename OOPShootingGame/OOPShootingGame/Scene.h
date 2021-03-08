#ifndef SCENE
#define SCENE
#include "myList.h"
#include "ObjectBase.h"
#include "Console.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern myList<ObjectBase*> gObjectList;

class Scene
{
public:
	virtual void GetKeyChange() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	void RenderCSVFileData(int line, const char** pBuf);
};

#endif