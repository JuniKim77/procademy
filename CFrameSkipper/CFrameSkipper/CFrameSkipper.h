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
		DWORD			GetTotalTick() { return mTotalTick; }
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
		DWORD		mTotalTick = 0; // 
		DWORD		mOldFrameCounter = 0;
		DWORD		mTimeRemain = 0; // 
		DWORD		mOldTick = 0; // ���� ������ �ð�
		DWORD		mPrevTime = 0;
		DWORD		mMaxFrameTime = 0;
		DWORD		mMinFrameTime = ULONG_MAX;
		int			mFrameCounter = 0; // ������ �� ī����
		int			mLoopCounter = 0; // ������ �󸶳� ���Ҵ��� �ľ�
		int			mStandardTime = 20;
	};
}
