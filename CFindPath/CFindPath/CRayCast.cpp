#include "CRayCast.h"

#define abs(a) ((a) > 0 ? (a) : (-a))

void procademy::CRayCast::SetBegin(Coordi& begin)
{
    mBegin = begin;
    mCur = begin;
}

void procademy::CRayCast::SetEnd(Coordi& end)
{
	mEnd = end;

	mdx = mEnd.x - mBegin.x;
	mdy = mEnd.y - mBegin.y;

	// �̵��� ���� ���ϱ�
	mXMove = mdx < 0 ? -1 : 1;
	mYMove = mdy < 0 ? -1 : 1;

	mdx = abs(mdx);
	mdy = abs(mdy);
	mNum = 0;

	// ���� ���� ���
	mXAxis = mdx >= mdy;
	if (mXAxis)
	{
		mCount = mdx / 2;
	}
	else
	{
		mCount = mdy / 2;
	}
}

bool procademy::CRayCast::GetNext(Coordi& next)
{
	if (mXAxis)
	{
		next = mCur;

		mCur.x += mXMove;
		mCount += mdy;

		if (mCount >= mdx)
		{
			mCur.y += mYMove;
			mCount -= mdx;
		}

		return mNum++ == (mdx + 1);
	}
	else
	{
		next = mCur;

		mCur.y += mYMove;
		mCount += mdx;

		if (mCount >= mdy)
		{
			mCur.x += mXMove;
			mCount -= mdy;
		}

		return mNum++ == (mdy + 1);
	}
}

void procademy::CRayCast::Reset()
{
	mCur = mBegin;
	mNum = 0;
	if (mXAxis)
	{
		mCount = mdx / 2;
	}
	else
	{
		mCount = mdy / 2;
	}
}
