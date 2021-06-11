#include "MyHeap.h"
#include <iostream>

using namespace std;

MyHeap::MyHeap(int size)
    : mCapacity(size)
    , mSize(0)
{
    mBuffer = new Node*[mCapacity + 1];
}

MyHeap::~MyHeap()
{
    if (mBuffer != nullptr)
        delete[] mBuffer;
}

void MyHeap::InsertData(Node* data)
{
    if (mSize == mCapacity)
    {
        resize();
    }

    mBuffer[mSize + 1] = data;

    mSize++;

    Heapify(mSize);
}

Node* MyHeap::GetMin()
{
    if (mSize == 0)
        return nullptr;

    int retIndex = mSize;

    Swap(1, mSize);
    mSize--;

    DeHeapify(1);

    return mBuffer[retIndex];
}

bool MyHeap::UpdateNode(Node* other)
{
    for (int i = 1; i <= mSize; ++i)
    {
        if (mBuffer[i]->position == other->position)
        {
            if (mBuffer[i]->f > other->f)
            {
                mBuffer[i]->g = other->g;
                mBuffer[i]->f = other->f;
                mBuffer[i]->pParent = other->pParent;

                Heapify(i);

                return true;
            }

            return false;
        }
    }

    return false;
}

void MyHeap::Swap(int left, int right)
{
    Node* temp = mBuffer[left];
    mBuffer[left] = mBuffer[right];
    mBuffer[right] = temp;
}

void MyHeap::Heapify(int index)
{
    if (index <= 1)
        return;

    int parent = index / 2;

    if (mBuffer[parent]->f > mBuffer[index]->f)
    {
        Swap(parent, index);

        Heapify(parent);
    }
}

void MyHeap::DeHeapify(int index)
{
    int left = index * 2;
    int right = index * 2 + 1;

    if (mSize >= right)
    {
        int next = mBuffer[left]->f < mBuffer[right]->f ? left : right;

        if (mBuffer[index]->f > mBuffer[next]->f)
        {
            Swap(index, next);

            DeHeapify(next);
        }
    }
    else if (mSize >= left)
    {
        if (mBuffer[left]->f < mBuffer[index]->f)
        {
            Swap(left, index);

            DeHeapify(left);
        }
    }
}

void MyHeap::resize()
{
    Node** temp = new Node*[mCapacity * 2 + 1];
    for (int i = 0; i < mSize; ++i)
    {
        temp[i] = mBuffer[i];
    }

    mCapacity = mCapacity * 2;
    delete[] mBuffer;
    mBuffer = temp;
}
