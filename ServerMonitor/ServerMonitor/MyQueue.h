#pragma once

#define QUEUE_SIZE (100)

class MyQueue
{
public:
	friend class MonitorGraphUnit;
	MyQueue(int capacity);
	~MyQueue();
	void enqueue(int value);
	void popqueue();
	int peakqueue(int pos);

private:
	int* mBuffer;
	int mCapacity;
	int mRear = 0;
	int mFront = 0;
};