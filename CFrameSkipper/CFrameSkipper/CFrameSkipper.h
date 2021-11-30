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
		/// 무거운 로직(렌더링, 메세지 처리)를 쉬고 로직만 돌지 결정하는 함수
		/// </summary>
		bool			IsSkip() { return mTimeRemain >= 20; }
		bool			IsOverSecond() { return mTotalTick >= 1000; }
		ULONGLONG		GetTotalTick() { return mTotalTick; }
		/// <summary>
		/// 시간 상태 Update
		/// </summary>
		void			CheckTime();
		/// <summary>
		///	이번 1초에 카운팅된 프레임 수 반환
		/// </summary>
		int				GetFrameCount() { return mFrameCounter; }
		void			RunSleep();
		void			Reset();
		/// <summary>
		/// 1초가 지난 경우 초기화
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
		ULONGLONG	mOldTick = 0; // 이전 프레임 시간
		ULONGLONG	mPrevTime = 0;
		ULONGLONG	mMaxFrameTime = 0;
		ULONGLONG	mMinFrameTime = LLONG_MAX;
		int			mFrameCounter = 0; // 프레임 수 카운팅
		int			mLoopCounter = 0; // 루프를 얼마나 돌았는지 파악
		int			mStandardTime = 20;
	};
}
