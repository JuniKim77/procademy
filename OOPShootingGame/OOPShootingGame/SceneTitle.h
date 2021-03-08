#ifndef SCENETITLE
#define SCENETITLE
#include "Scene.h"
#include "CSVReader.h"

class SceneTitle : public Scene
{
public:
	SceneTitle();
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
