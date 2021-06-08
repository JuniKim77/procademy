#include "MyHeap.h"
#include <iostream>

using namespace std;

MyHeap::MyHeap(int size)
    : mCapacity(size)
    , mSize(0)
{
    mBuffer = new int[mCapacity + 1];
}

MyHeap::~MyHeap()
{
    if (mBuffer != nullptr)
        delete[] mBuffer;
}

bool MyHeap::InsertData(int data)
{
    if (mSize == mCapacity)
        return false;

    mBuffer[mSize + 1] = data;
    mSize++;

    Heapify(mSize);

    return true;
}

int MyHeap::GetMax()
{
    if (mSize == 0)
        return -1;

    int ret = mBuffer[1];

    Swap(1, mSize);
    mSize--;

    DeHeapify(1);    

    return ret;
}

void MyHeap::printHeap()
{
    printHelper(1);
}

void MyHeap::Swap(int left, int right)
{
    int temp = mBuffer[left];
    mBuffer[left] = mBuffer[right];
    mBuffer[right] = temp;
}

void MyHeap::Heapify(int index)
{
    if (index <= 1)
        return;

    int parent = index / 2;

    if (mBuffer[parent] < mBuffer[index])
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
        int next = mBuffer[left] > mBuffer[right] ? left : right;

        if (mBuffer[index] < mBuffer[next])
        {
            Swap(index, next);

            DeHeapify(next);
        }
    }
    else if (mSize >= left)
    {
        if (mBuffer[left] > mBuffer[index])
        {
            Swap(left, index);

            DeHeapify(left);
        }
    }
}

void MyHeap::printHelper(int index)
{
    if (index > mSize)
        return;

    int left = index * 2;
    int right = index * 2 + 1;

    printHelper(left);

    int depth = index;
    int count = 0;

    while (depth > 1)
    {
        cout << "\t";
        depth /= 2;
        count++;
    }
    cout << mBuffer[index] << "(" << count << ")" << endl;

    printHelper(right);
}
