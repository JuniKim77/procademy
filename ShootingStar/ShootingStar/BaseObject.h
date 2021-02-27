class BaseObject
{
public:
	virtual bool Run() = 0;
	virtual void Render() = 0;

protected:
	int mX;
};