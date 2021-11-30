#pragma once

#include <wtypes.h>

namespace procademy
{
	class CFrameSkipper
	{
	public:
		CFrameSkipper();
		~CFrameSkipper();
		/// <summary>
		/// ���ſ� ����(������, �޼��� ó��)�� ���� ������ ���� �����ϴ� �Լ�
		/// </summary>
		bool			IsSkip() { return mTimeRemain >= 20; }
		bool			IsOverSecond() { return mTotalTick >= 1000; }
		ULONGLONG		GetTotalTick() { return mTotalTick; }
		/// <summary>
		/// �ð� ���� Update
		/// </summary>
		void			CheckTime();
		/// <summary>
		///	�̹� 1�ʿ� ī���õ� ������ �� ��ȯ
		/// </summary>
		int				GetFrameCount() { return mFrameCounter; }
		void			RunSleep();
		void			Reset();
		/// <summary>
		/// 1�ʰ� ���� ��� �ʱ�ȭ
		/// </summary>
		void			Refresh();
		void			UpdateRemain();
		ULONGLONG		GetOldFrameCount() { return mOldFrameCounter; }
		void			AddLoopCounter() { mLoopCounter++; }
		int				GetLoopCounter() { return mLoopCounter; }
		void			PrintStatus();
		void			SetMaxFrame(int frame);

	private:
		ULONGLONG	mTotalTick = 0; // 
		ULONGLONG	mOldFrameCounter = 0;
		ULONGLONG	mTimeRemain = 0; // 
		ULONGLONG	mOldTick = 0; // ���� ������ �ð�
		ULONGLONG	mPrevTime = 0;
		ULONGLONG	mMaxFrameTime = 0;
		ULONGLONG	mMinFrameTime = LLONG_MAX;
		int			mFrameCounter = 0; // ������ �� ī����
		int			mLoopCounter = 0; // ������ �󸶳� ���Ҵ��� �ľ�
		int			mStandardTime = 20;
	};
}
