#pragma once
#include <process.h>
#include <Windows.h>

#define BUFFER_SIZE (150)

class RingBuffer 
{
	enum {
		DEFAULT_SIZE = 10000
	};

public:
	RingBuffer();
	~RingBuffer();
	RingBuffer(int iBufferSize);

	void Resize(int size);

	int GetBufferSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	// 체크 완료
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	int GetUseSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기.
	// 체크 완료
	// Parameters: 없음.
	// Return: (int)남은용량.
	/////////////////////////////////////////////////////////////////////////
	int GetFreeSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
	// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
	//
	// Parameters: 없음.
	// Return: (int)사용가능 용량.
	////////////////////////////////////////////////////////////////////////
	int DirectEnqueueSize(void); // 체크 완료
	int DirectDequeueSize(void); // 체크 완료


	/////////////////////////////////////////////////////////////////////////
	// WritePos 에 데이타 넣음.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)넣은 크기.
	/////////////////////////////////////////////////////////////////////////
	int Enqueue(char* chpData, int iSize); // 완료

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int Dequeue(char* chpDest, int iSize); // 완료

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int Peek(char* chpDest, int iSize); // 완료


	/////////////////////////////////////////////////////////////////////////
	// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	bool MoveRear(int iSize);
	bool MoveFront(int iSize);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 모든 데이타 삭제.
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void);


	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 Front 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char* GetFrontBufferPtr(void);


	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 RearPos 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char* GetRearBufferPtr(void);

	void printInfo();

	bool IsFrontZero() { return mFront == 0; }

	void Lock(bool readonly);
	void Unlock(bool readonly);

	int GetRear() { return mRear; }
	int GetFront() { return mFront; }

private:
	int mFront;
	int mRear;
	char* mBuffer;
	int mCapacity;
	SRWLOCK mSrwLock;
};