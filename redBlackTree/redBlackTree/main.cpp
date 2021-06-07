#include "RedBlackTree.h"
#include <time.h>
#include "MyProfiler.h"
#include <unordered_set>
#include "MyHashMap.h"
#include "BinaryTree.h"

void getRandNum(unsigned int* num);
void insertNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance);
bool deleteNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance);
bool searchNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance);

int main()
{
	ProfileReset();
	RedBlackTree rbTree;
	MyHashMap myHash;
	BinaryTree bTree;
	srand(time(NULL));

	std::unordered_set<unsigned int> setNums;

	for (int i = 0; i < 100000; i++)
	{
		insertNum(bTree, rbTree, myHash, setNums, false);
	}

	for (int i = 0; i < 200000; ++i)
	{
		unsigned int num;
		getRandNum(&num);

		if ((num & 1) == 1)
		{
			insertNum(bTree, rbTree, myHash, setNums, true);
		}
		else
		{
			if (!deleteNum(bTree, rbTree, myHash, setNums, true))
			{
				printf("Delete error\n");
			}
		}

		if (!rbTree.CheckBalance())
		{
			printf("Balance error\n");
		}
	}

	for (int i = 0; i < 100000; ++i)
	{
		searchNum(bTree, rbTree, myHash, setNums, true);
	}

	ProfileDataOutText(TEXT("Profile"));

	return 0;
}

void getRandNum(unsigned int* number)
{
	unsigned int num = rand();

	num <<= 16;

	num |= rand();

	num &= 0x7ffff;

	*number = num;
}

void insertNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance)
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

		PRO_BEGIN(L"BinaryTreeInsert");
		bTree.InsertNode(num);
		PRO_END(L"BinaryTreeInsert");

		PRO_BEGIN(L"HashTableInsert");
		hash.InsertNode(num);
		PRO_END(L"HashTableInsert");
	}
	else
	{
		rbTree.InsertNode(num);
		bTree.InsertNode(num);
		hash.InsertNode(num);
	}
}

bool deleteNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance)
{
	unsigned int num;
	bool ret = true;
	bool ret2 = true;
	bool ret3 = true;

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

				PRO_BEGIN(L"BinaryTreeDelete");
				ret2 = bTree.DeleteNode(*iter);
				PRO_END(L"BinaryTreeDelete");

				PRO_BEGIN(L"HashTableDelete");
				ret3 = hash.DeleteNode(*iter);
				PRO_END(L"HashTableDelete");
			}
			else
			{
				ret = rbTree.DeleteNode(*iter);
				ret2 = bTree.DeleteNode(*iter);
				ret3 = hash.DeleteNode(*iter);
			}

			setNum.erase(iter);
			break;
		}
	}

	return ret && ret2 && ret3;
}

bool searchNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance)
{
	unsigned int num;

	getRandNum(&num);

	std::unordered_set<unsigned int>::iterator iter;

	while (1)
	{
		iter = setNum.find(num);

		if (iter == setNum.end())
		{
			getRandNum(&num);
		}
		else
		{
			break;
		}
	}

	bool found;
	bool found1;
	bool found2;

	if (checkPerformance)
	{
		PRO_BEGIN(L"RedBlackSearch");
		found = rbTree.SearchData(*iter);
		PRO_END(L"RedBlackSearch");

		PRO_BEGIN(L"BinaryTreeSearch");
		found1 = bTree.SearchData(*iter);
		PRO_END(L"BinaryTreeSearch");

		PRO_BEGIN(L"HashTableSearch");
		found2 = hash.SearchData(*iter);
		PRO_END(L"HashTableSearch");
	}
	else
	{
		found = rbTree.SearchData(*iter);
		found1 = bTree.SearchData(*iter);
		found2 = hash.SearchData(*iter);
	}

	if (!found)
	{
		printf("Search RedBlack error\n");
	}
	if (!found1)
	{
		printf("Search Binary error\n");
	}
	if (!found2)
	{
		printf("Search HashTable error\n");
	}

	return found && found1 && found2;
}
