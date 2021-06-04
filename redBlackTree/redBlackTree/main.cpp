#include "RedBlackTree.h"
#include <time.h>
#include "MyProfiler.h"
#include <unordered_set>

void getRandNum(unsigned int* num);

int main()
{
	ProfileReset();
	RedBlackTree rbTree;
	srand(time(NULL));

	std::unordered_set<unsigned int> setNums;

	for (int i = 0; i < 100000; i++)
	{
		unsigned int num;

		getRandNum(&num);

		while (1)
		{
			auto ret = setNums.insert(num);

			if (ret.second == true)
				break;
			
			getRandNum(&num);
		}

		PRO_BEGIN(L"RedBlackInsert");
		rbTree.InsertNode(num);
		PRO_END(L"RedBlackInsert");
	}

	for (auto iter = setNums.begin(); iter != setNums.end(); ++iter)
	{
		PRO_BEGIN(L"RedBlackSearch");
		bool found = rbTree.SearchData(*iter);
		PRO_END(L"RedBlackSearch");

		if (!found)
		{
			printf("error\n");
		}
	}

	int count = 1;

	while (!setNums.empty())
	{
		auto iter = setNums.begin();

		PRO_BEGIN(L"RedBlackDelete");
		rbTree.DeleteNode(*iter);
		PRO_END(L"RedBlackDelete");
		
		setNums.erase(iter);

		if (count % 100 == 0)
		{
			for (auto iter = setNums.begin(); iter != setNums.end(); ++iter)
			{
				PRO_BEGIN(L"RedBlackSearch");
				bool found = rbTree.SearchData(*iter);
				PRO_END(L"RedBlackSearch");

				if (!found)
				{
					printf("error\n");
				}
			}
		}
		
		count++;
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
