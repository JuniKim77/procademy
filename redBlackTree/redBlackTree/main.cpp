#include "RedBlackTree.h"
#include <time.h>
#include "MyProfiler.h"
#include <unordered_set>

void getRandNum(unsigned int* num);
void insertNum(RedBlackTree& rbTree, std::unordered_set<unsigned int>& setNum, bool checkPerformance);
bool deleteNum(RedBlackTree& rbTree, std::unordered_set<unsigned int>& setNum, bool checkPerformance);

int main()
{
	ProfileReset();
	RedBlackTree rbTree;
	srand(time(NULL));

	std::unordered_set<unsigned int> setNums;

	for (int i = 0; i < 100000; i++)
	{
		insertNum(rbTree, setNums, false);
	}

	for (int i = 0; i < 100000; ++i)
	{
		unsigned int num;
		getRandNum(&num);

		if ((num & 1) == 1)
		{
			insertNum(rbTree, setNums, true);
		}
		else
		{
			if (!deleteNum(rbTree, setNums, true))
			{
				printf("Delete error\n");
			}
		}

		if (!rbTree.CheckBalance())
		{
			printf("Balance error\n");
		}

		if (i % 10000 == 0)
		{
			for (auto iter = setNums.begin(); iter != setNums.end(); ++iter)
			{
				PRO_BEGIN(L"RedBlackSearch");
				bool found = rbTree.SearchData(*iter);
				PRO_END(L"RedBlackSearch");

				if (!found)
				{
					printf("Search error\n");
				}
			}
		}
	}

	ProfileDataOutText(TEXT("Profile"));

	return 0;
}

void getRandNum(unsigned int* number)
{
	unsigned int num = rand();

	num <<= 16;

	num |= rand();

	*number = num;
}

void insertNum(RedBlackTree& rbTree, std::unordered_set<unsigned int>& setNum, bool checkPerformance)
{
	unsigned int num;

	getRandNum(&num);

	while (1)
	{
		auto ret = setNum.insert(num);

		if (ret.second == true)
			break;

		getRandNum(&num);
	}

	if (checkPerformance)
	{
		PRO_BEGIN(L"RedBlackInsert");
		rbTree.InsertNode(num);
		PRO_END(L"RedBlackInsert");
	}
	else
	{
		rbTree.InsertNode(num);
	}
}

bool deleteNum(RedBlackTree& rbTree, std::unordered_set<unsigned int>& setNum, bool checkPerformance)
{
	unsigned int num;
	bool ret = true;

	getRandNum(&num);

	while (1)
	{
		auto iter = setNum.find(num);

		if (iter == setNum.end())
		{
			getRandNum(&num);
		}
		else
		{
			if (checkPerformance)
			{
				PRO_BEGIN(L"RedBlackDelete");
				ret = rbTree.DeleteNode(*iter);
				PRO_END(L"RedBlackDelete");
			}
			else
			{
				ret = rbTree.DeleteNode(*iter);
			}

			setNum.erase(iter);
			break;
		}
	}

	return ret;
}
