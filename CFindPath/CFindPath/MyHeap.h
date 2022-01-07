#pragma once
#include "FilePathDTO.h"
#include "CAStar.h"

namespace procademy
{
	class MyHeap
	{
	public:
		enum {
			DEFAULT_SIZE = 10000
		};

		MyHeap();
		MyHeap(int size);
		~MyHeap();
		void InsertData(Node* data);
		Node* GetMin();
		int GetSize() { return mSize; }
		bool UpdateNode(Node* other);
		void ClearHeap() { mSize = 0; }

	private:
		void Swap(int left, int right);
		void Heapify(int index);
		void DeHeapify(int index);
		void resize();

	private:
		int mCapacity;
		int mSize;
		Node** mBuffer;
	};
}
