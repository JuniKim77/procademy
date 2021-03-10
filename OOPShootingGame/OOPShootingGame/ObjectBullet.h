#ifndef OBJECTBULLET
#define OBJECTBULLET

class ObjectBase;

class ObjectBullet : public ObjectBase
{
public:
	ObjectBullet(int x, int y, int damage, bool isEnemy);
	// ObjectBase을(를) 통해 상속됨
	virtual bool Update() override;
	virtual void Render() override;
	virtual void Move() override;
	void Collision();

private:
	int mDamage;
	bool mbEnemy;
};

#endif