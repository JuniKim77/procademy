#ifndef OBJECTBASE
#define BOJECTBASE

#define dfSCREEN_WIDTH		81		// ÄÜ¼Ö °¡·Î 80Ä­ + NULL
#define dfSCREEN_HEIGHT		24		// ÄÜ¼Ö ¼¼·Î 24Ä­

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

enum class ObjectType;

class ObjectBase
{
public:
	friend class ObjectManager;
	ObjectBase(int x, int y, int hp, int damage, char image, ObjectType type);
	virtual ~ObjectBase();
	virtual bool Update() = 0;
	virtual void Render() = 0;
	virtual void Move() = 0;
	bool GetIsLive() { return mbLive; }
	void TakeDamage(int damage);

protected:
	int mX;
	int mY;
	int mAttack;
	int mHp;
	ObjectType mType;
	char mImage;
	bool mbLive = true;
};

#endif