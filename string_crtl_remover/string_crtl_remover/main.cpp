#include <string>
#include "CProfiler.h"
#include <time.h>
#include <vector>

#define TEST_SIZE (1000)

using namespace std;

string remove_ctrl(string s, int len);
string remove_ctrl_add(string s, int len);
string remove_ctrl_add_reserve(string s, int len);
string remove_ctrl_add_reserve_const(const string& s, int len);
string remove_ctrl_add_reserve_const_it(const string& s, int len);

int main()
{
	string str = "1234567890_+|=`abcdefghijklmnqrstuvwxyz_`/*1234567890asdfghjklqwertyuiopzxcvbnm,>";
	srand(time(NULL));
	vector<int> nums;
	nums.reserve(TEST_SIZE);

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		int num = rand() % 82;

		nums.push_back(num);
	}

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		CProfiler::Begin(L"remove_ctrl");

		for (auto each : nums)
		{
			remove_ctrl(str, each);
		}

		CProfiler::End(L"remove_ctrl");
	}

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		CProfiler::Begin(L"remove_ctrl_2");

		for (auto each : nums)
		{
			remove_ctrl_add(str, each);
		}

		CProfiler::End(L"remove_ctrl_2");
	}

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		CProfiler::Begin(L"remove_ctrl_3");

		for (auto each : nums)
		{
			remove_ctrl_add_reserve(str, each);
		}

		CProfiler::End(L"remove_ctrl_3");
	}

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		CProfiler::Begin(L"remove_ctrl_4");

		for (auto each : nums)
		{
			remove_ctrl_add_reserve_const(str, each);
		}

		CProfiler::End(L"remove_ctrl_4");
	}

	for (int i = 0; i < TEST_SIZE; ++i)
	{
		CProfiler::Begin(L"remove_ctrl_5");

		for (auto each : nums)
		{
			remove_ctrl_add_reserve_const_it(str, each);
		}

		CProfiler::End(L"remove_ctrl_5");
	}

	CProfiler::Print();

	return 0;
}

string remove_ctrl(string s, int len)
{
	string ret;

	for (int i = 0; i < len; ++i)
	{
		if (s[i] >= 20)
		{
			ret = ret + s[i];
		}
	}

	return ret;
}

string remove_ctrl_add(string s, int len)
{
	string ret;

	for (int i = 0; i < len; ++i)
	{
		if (s[i] >= 20)
		{
			ret += s[i];
		}
	}

	return ret;
}

string remove_ctrl_add_reserve(string s, int len)
{
	string ret;
	ret.reserve(len);

	for (int i = 0; i < len; ++i)
	{
		if (s[i] >= 20)
		{
			ret += s[i];
		}
	}

	return ret;
}

string remove_ctrl_add_reserve_const(const string& s, int len)
{
	string ret;
	ret.reserve(len);

	for (int i = 0; i < len; ++i)
	{
		if (s[i] >= 20)
		{
			ret += s[i];
		}
	}

	return ret;
}

string remove_ctrl_add_reserve_const_it(const string& s, int len)
{
	string ret;
	ret.reserve(len);

	auto end = s.begin() + len;

	for (auto iter = s.begin(); iter != end; ++iter)
	{
		if (*iter >= 20)
		{
			ret += *iter;
		}
	}

	return ret;
}
