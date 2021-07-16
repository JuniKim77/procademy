#include "RedBlackTree.h"
#include <time.h>
#include "MyProfiler.h"
#include <unordered_set>
#include "MyHashMap.h"
#include "BinaryTree.h"
#include <iostream>

/// <summary>
/// 0 ~ 0x7ffff 까지 범위로 랜덤 숫자 반환
/// </summary>
/// <param name="num"></param>
void getRandNum(unsigned int* num);
/// <summary>
/// 맨 상위 비트 값을 결정하고 하위 비트에서 랜덤 하게 값을 생성
/// </summary>
/// <param name="output"></param>
/// <param name="msb"></param>
void getRandBiasedNum(unsigned int* output, int msb);
void insertNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased, int msb);
bool deleteNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased, int msb);
bool searchNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased, int msb);
/// <summary>
/// 일반 랜덤 숫자로 테스트
/// </summary>
void TestNormalDistributionNumber();
/// <summary>
/// 한 쪽으로 쏠려있는 숫자들로 테스트
/// </summary>
/// <param name="groupNum">나눌 계층 수</param>
void TestBiasedDistributionNumber(int groupNum);

int main()
{
	srand((unsigned int)time(NULL));

	int count;

	std::cout << "Count: ";
	std::cin >> count;

	for (int t = 1; t <= count; ++t)
	{
		TestNormalDistributionNumber();
	}

	for (int t = 1; t <= count; ++t)
	{
		TestBiasedDistributionNumber(8);
	}

	return 0;
}

void getRandNum(unsigned int* number)
{
	unsigned int num = rand();

	num <<= 15;

	num |= rand();

	num &= 0x7ffff;

	*number = num;
}

void getRandBiasedNum(unsigned int* output, int msb)
{
	unsigned int num = rand();

	num <<= 15;

	num |= rand();

	num &= 0xffff;

	num |= (msb << 16);

	*output = num;
}

void insertNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;

	if (biased)
	{
		getRandBiasedNum(&num, msb);
	}
	else
	{
		getRandNum(&num);
	}

	while (1)
	{
		auto ret = setNum.insert(num);

		if (ret.second == true)
			break;

		if (biased)
		{
			getRandBiasedNum(&num, msb);
		}
		else
		{
			getRandNum(&num);
		}
	}

	if (checkPerformance)
	{
		int depth = rbTree.GetDepthInsertNode(num);
		ProfileSetDepth(depth, L"RedBlackInsert");
		PRO_BEGIN(L"RedBlackInsert");
		rbTree.InsertNode(num);
		PRO_END(L"RedBlackInsert");

		depth = bTree.GetDepthInsertNode(num);
		ProfileSetDepth(depth, L"BinaryTreeInsert");
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

bool deleteNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;
	bool ret = true;
	bool ret2 = true;
	bool ret3 = true;

	if (biased)
	{
		getRandBiasedNum(&num, msb);
	}
	else
	{
		getRandNum(&num);
	}

	while (1)
	{
		auto iter = setNum.find(num);

		if (iter == setNum.end())
		{
			if (biased)
			{
				getRandBiasedNum(&num, msb);
			}
			else
			{
				getRandNum(&num);
			}
		}
		else
		{
			if (checkPerformance)
			{
				int depth = rbTree.GetDepthDeleteNode(num);
				ProfileSetDepth(depth, L"RedBlackDelete");
				PRO_BEGIN(L"RedBlackDelete");
				ret = rbTree.DeleteNode(*iter);
				PRO_END(L"RedBlackDelete");

				depth = bTree.GetDepthDeleteNode(num);
				ProfileSetDepth(depth, L"BinaryTreeDelete");
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

bool searchNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;

	if (biased)
	{
		getRandBiasedNum(&num, msb);
	}
	else
	{
		getRandNum(&num);
	}

	std::unordered_set<unsigned int>::iterator iter;

	while (1)
	{
		iter = setNum.find(num);

		if (iter == setNum.end())
		{
			if (biased)
			{
				getRandBiasedNum(&num, msb);
			}
			else
			{
				getRandNum(&num);
			}
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
		int depth = rbTree.GetDepthSearchData(num);
		ProfileSetDepth(depth, L"RedBlackSearch");
		PRO_BEGIN(L"RedBlackSearch");
		found = rbTree.SearchData(*iter);
		PRO_END(L"RedBlackSearch");

		depth = bTree.GetDepthSearchData(num);
		ProfileSetDepth(depth, L"BinaryTreeSearch");
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

void TestNormalDistributionNumber()
{
	ProfileReset();
	RedBlackTree rbTree;
	MyHashMap myHash;
	BinaryTree bTree;

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

		/*if (!rbTree.CheckBalance())
		{
			printf("Balance error\n");
		}*/
	}

	for (int i = 0; i < 100000; ++i)
	{
		searchNum(bTree, rbTree, myHash, setNums, true);
	}

	WCHAR file_name[32] = { 0, };
	swprintf_s(file_name, _countof(file_name), L"Profile_Normal");
	ProfileDataOutText(file_name);
}

void TestBiasedDistributionNumber(int groupNum)
{
	ProfileReset();
	RedBlackTree rbTree;
	MyHashMap myHash;
	BinaryTree bTree;

	int size = 100000;
	int eachSize = size / groupNum;

	std::unordered_set<unsigned int> setNums;

	for (int g = 0; g < groupNum; ++g)
	{
		for (int i = 0; i < eachSize; i++)
		{
			insertNum(bTree, rbTree, myHash, setNums, false, true, g);
		}
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

		/*if (!rbTree.CheckBalance())
		{
			printf("Balance error\n");
		}*/
	}

	for (int i = 0; i < 100000; ++i)
	{
		searchNum(bTree, rbTree, myHash, setNums, true);
	}

	WCHAR file_name[32] = { 0, };
	swprintf_s(file_name, _countof(file_name), L"Profile_Biased");
	ProfileDataOutText(file_name);
}
