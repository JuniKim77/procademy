#pragma once

#define BUFFER_SIZE (150)

class Node;

class RingBuffer 
{
	enum {
		DEFAULT_SIZE = 1000
	};

public:
	RingBuffer();
	~RingBuffer();
	RingBuffer(int iBufferSize);

	void Resize(int size);

	int GetBufferSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	int GetUseSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기.
	//
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
	int DirectEnqueueSize(void);
	int DirectDequeueSize(void);


	/////////////////////////////////////////////////////////////////////////
	// WritePos 에 데이타 넣음.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)넣은 크기.
	/////////////////////////////////////////////////////////////////////////
	void Enqueue(Node* node);

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	void Dequeue(Node** dest);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 모든 데이타 삭제.
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void);

	void printInfo();

private:
	int mFront;
	int mRear;
	Node** mBuffer;
	int mCapacity;
};