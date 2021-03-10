#ifndef OBJECTPLAYER
#define OBJECTPLAYER

class ObjectBase;

class ObjectPlayer : public ObjectBase
{
public:
	ObjectPlayer(int x, int y);
	// ObjectBase을(를) 통해 상속됨
	virtual bool Update() override;
	virtual void Render() override;
	virtual void Move() override;

private:
	int mHp;
	int mAttack;
};

#endif