#include "ObjectPool.h"
#include <iostream>

class Test
{
public:
	Test()
		: mData(0)
	{
		std::cout << "Test1 Constructor\n" << std::endl;
	}

private: 
	int mData;
};

class Test2
{
public:
	Test2()
		: mData(0)
	{
		std::cout << "Test2 Constructor" << std::endl;
	}

	~Test2()
	{
		std::cout << "Test2 Destructor" << std::endl;
	}

	void SetData(int data) { mData = data; }

private:
	int mData;
};


struct Node
{
	Test data;
	int num;
};

struct stTest
{
	int test[4];
};

struct stTest2
{
	unsigned short a;
	stTest b;
};

int main()
{
	procademy::ObjectPool<Test2> pool2(10, true);

	Test2* t1[13];

	for (int i = 0; i < 13; ++i)
	{
		t1[i] = pool2.Alloc();
		t1[i]->SetData(i + 1);
	}

	for (int i = 0; i < 13; ++i)
	{
		pool2.Free(t1[i]);
	}

	return 0;
}