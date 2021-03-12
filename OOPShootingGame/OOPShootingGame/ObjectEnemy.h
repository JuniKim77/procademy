#ifndef OBJECTENEMY
#define OBJECTENEMY

class ObjectBase;

class ObjectEnemy : public ObjectBase
{
public:
	ObjectEnemy(int x, int y, int type);
	// ObjectBase��(��) ���� ��ӵ�
	virtual bool Update() override;
	virtual void Render() override;
	virtual void Move() override;
	void Attack();

private:
	int mMoveTimer;
	int mAttackTimer;
	int mMoveCounter = 0;
	int mAttackCount = 0;
	int mMoveDir = -1;
};

#endif