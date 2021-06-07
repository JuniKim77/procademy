#pragma once

class MyHashMap
{
public:
	MyHashMap();
	~MyHashMap();

	void InsertNode(unsigned int data);
	bool DeleteNode(unsigned int data);
	bool SearchData(unsigned int data);

private:
	bool* mBucket;
};