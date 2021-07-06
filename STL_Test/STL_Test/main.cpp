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
		cout << "Operator = call [Num: " << mNum << "]" << endl;
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

		unordered_map<int, TEST> m1;

		m1[t1.mNum] = t1;
		m1[t2.mNum] = t2;
		m1[t3.mNum] = t3;
		m1.insert(make_pair(t4.mNum, t4));
		m1.insert(make_pair(t5.mNum, t5));
		m1.insert(make_pair(t5.mNum, t6));

		list<TEST> l1;

		l1.push_back(t1);
		l1.push_back(t2);
		l1.push_back(t3);
		l1.push_back(t4);


	}

	return 0;
}