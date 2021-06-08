#pragma once

#include "AStar.h"

class MyHeap
{
public:
	MyHeap(int size);
	~MyHeap();
	void InsertData(Node* data);
	Node* GetMin();
	void printHeap();
	int GetSize() { return mSize; }

private:
	void Swap(int left, int right);
	void Heapify(int index);
	void DeHeapify(int index);
	void printHelper(int index);
	void resize();

private:
	int mCapacity;
	int mSize;
	Node* mBuffer;
};