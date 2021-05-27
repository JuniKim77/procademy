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
	procademy::ObjectPool<Test> pool1(100, true);

	procademy::ObjectPool<Test2> pool2(100, false);
	return 0;
}