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

int main()
{
	Test t0;

	Test* t1 = new Test;

	Test* t2 = new (t1) (Test);

	Node* node = new Node;

	procademy::ObjectPool<Test> pool1(100, true);

	procademy::ObjectPool<Test2> pool2(100, true);

	Test2* t3 = new Test2;

	Test2* t4 = new (t3) (Test2);

	t4->~Test2();

	return 0;
}