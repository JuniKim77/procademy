#pragma once

class MyHeap
{
public:
	MyHeap(int size);
	~MyHeap();
	bool InsertData(int data);
	int GetMax();
	void printHeap();
	int GetSize() { return mSize; }

private:
	void Swap(int left, int right);
	void Heapify(int index);
	void DeHeapify(int index);
	void printHelper(int index);

private:
	int mCapacity;
	int mSize;
	int* mBuffer;
};