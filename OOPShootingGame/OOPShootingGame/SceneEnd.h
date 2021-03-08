#ifndef SCENEEND
#define SCENEEND
#include "Scene.h"
#include "CSVReader.h"

class SceneEnd : public Scene
{
public:
	SceneEnd();
	// Scene��(��) ���� ��ӵ�
	virtual void GetKeyChange() override;
	virtual void Update() override;
	virtual void Render() override;

private:
	void LoadCSVFile();

private:
	int mPrevTime;
	bool mbMessageOn;
	CSVFile* mCSVReader;
};

#endif
