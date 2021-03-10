#ifndef OBJECTBULLET
#define OBJECTBULLET

class ObjectBase;

class ObjectBullet : public ObjectBase
{
public:
	ObjectBullet(int x, int y, int damage, bool isEnemy);
	// ObjectBase��(��) ���� ��ӵ�
	virtual bool Update() override;
	virtual void Render() override;
	virtual void Move() override;
	void Collision();

private:
	int mDamage;
	bool mbEnemy;
};

#endif