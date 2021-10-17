#include "ObjectPool_TLS.h"
#include <iostream>

using namespace std;

class TEST
{
public:
	TEST() 
	{
		cout << "TEST Construct" << endl;
	}

	~TEST()
	{
		cout << "TEST Distructor" << endl;
	}

	int mNum = 0;
};

int main()
{
	procademy::ObjectPool_TLS<TEST> tt;

	for (int i = 0; i < 199; ++i)
	{
		TEST* test1 = tt.Alloc();

		test1->mNum = 10;

		tt.Free(test1);
	}

	TEST* test1 = tt.Alloc();

	test1->mNum = 10;

	tt.Free(test1);
	
	test1 = tt.Alloc();

	test1->mNum = 10;

	return 0;
}