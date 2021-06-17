#pragma once

class CRayCast
{
	struct Coordi
	{
		int x;
		int y;

		bool operator == (Coordi& other)
		{
			return x == other.x && y == other.y;
		}
	};

public:
	CRayCast();
	void SetBegin(int x, int y);


private:
	Coordi mBegin;
	Coordi mEnd;
	Coordi mCur;
};