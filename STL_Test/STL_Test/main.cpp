#include <vector>
#include <list>
#include <unordered_map>
#include <map>
#include <iostream>
#include <WinSock2.h>

using namespace std;

class TEST
{
public:
	TEST()
	{
		cout << "Test Constructor" << endl;
	}

	TEST(const TEST& other)
	{
		cout << "Test Copy Constructor" << endl;
	}

	void operator=(const TEST& other)
	{
		mNum = other.mNum;
	}

	~TEST()
	{
		cout << "Test Destructor" << endl;
	}

	int mNum;
};

int main()
{
	{
		vector<TEST> test;
		//test.reserve(60);

		TEST t1;

		test.push_back(t1);

		TEST t2;

		test.push_back(t2);

		TEST t3;

		test.push_back(t3);

		TEST t4;

		test.push_back(t4);

		TEST t5;

		test.push_back(t5);

		for (int i = 0; i < 5; ++i)
		{
			cout << test[i].mNum << " ";
		}
		cout << endl << endl;
	}

	return 0;
}