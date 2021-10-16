#include "MyHashMap.h"
#include <climits>
#include <string.h>

MyHashMap::MyHashMap()
{
	mBucket = new bool[0x1ffffff + 1];
	memset(mBucket, 0, 0x1ffffff + 1);
}

MyHashMap::~MyHashMap()
{
	if (mBucket != nullptr)
		delete[] mBucket;
}

void MyHashMap::InsertNode(unsigned int data)
{
	mBucket[data] = true;
}

bool MyHashMap::DeleteNode(unsigned int data)
{
	if (mBucket[data] == false)
		return false;

	mBucket[data] = false;

	return true;
}

bool MyHashMap::SearchData(unsigned int data)
{
	return mBucket[data] == true;
}
