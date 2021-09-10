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

	TEST(int num)
	{
		cout << "Test Constructor with Num" << endl;
		mNum = num;
	}

	TEST(const TEST& other)
	{
		mNum = other.mNum;
		cout << "Test Copy Constructor [Num: " << mNum << "]" << endl;
	}

	void operator=(const TEST& other)
	{
		mNum = other.mNum;
	}

	~TEST()
	{
		cout << "Test Destructor [Num: " << mNum << "]" << endl;
	}

	int mNum;
};

int main()
{
	{
		unordered_map<int, TEST*> test;

		TEST samples[10];

		for (int i = 0; i < 10; ++i)
		{
			samples[i].mNum = i + 1;
			test[i + 1] = &samples[i];
		}

		cout << "Sample Size: " << test.size() << endl;

		auto iter = test.find(5);

		cout << "Delete sample: " << iter->second->mNum << endl;

		test.erase(iter);

		cout << "Sample Size: " << test.size() << endl;

		iter = test.find(5);

		if (iter == test.end())
		{
			cout << "Not Found" << endl;
		}
	}

	{
		vector<TEST> test;
		//test.reserve(60);

		TEST t1(1);

		test.push_back(t1);

		TEST t2(2);

		test.push_back(t2);

		TEST t3(3);

		test.push_back(t3);

		TEST t4(4);

		test.push_back(t4);

		TEST t5(5);

		test.push_back(t5);

		TEST t6(6);

		test.push_back(t6);

		TEST t7(7);

		test.push_back(t7);

		TEST t8(8);

		test.push_back(t8);

		TEST t9(9);

		test.push_back(t9);

		TEST t10(10);

		test.push_back(t10);

		TEST t11(11);

		test.push_back(t11);

		TEST t12(12);

		test.push_back(t12);

		TEST t13(13);

		test.push_back(t13);

		for (int i = 0; i < 13; ++i)
		{
			cout << test[i].mNum << " ";
		}
		cout << endl << endl;
	}

	return 0;
}