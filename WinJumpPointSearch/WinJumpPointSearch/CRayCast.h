#pragma once

class CRayCast
{
public:
	struct Coordi
	{
		int x;
		int y;

		bool operator == (Coordi& other)
		{
			return x == other.x && y == other.y;
		}
	};
	CRayCast() {}
	void SetBegin(Coordi begin);
	void SetEnd(Coordi end);
	bool GetNext(Coordi& next);
	void Reset();

private:
	Coordi mBegin;
	Coordi mEnd;
	Coordi mCur;
	int mCount;
	int mdx;
	int mdy;
	int mNum;
	bool mXAxis;
	char mXMove;
	char mYMove;
};