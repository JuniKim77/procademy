#ifndef OBJECTBASE
#define BOJECTBASE

class ObjectBase
{
public:
	enum class ObjectType {
		PLAYER,
		ENEMY,
		BULLET
	};
	virtual ~ObjectBase();
	virtual bool Update() = 0;
	virtual void Render() = 0;

protected:
	int mX;
	int mY;
	ObjectType mType;
};

#endif