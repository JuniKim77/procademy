#define SUB_SIZE (1000)
#define TOTAL_SIZE (1024)

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
		TestBiasedDistributionNumber(32);
	}

	return 0;
}

void getRandNum(unsigned int* number)
{
	unsigned int num = rand();

	num <<= 15;

	num |= rand();

	num &= 0x3fffff;

	*number = num;
}

void getRandBiasedNum(unsigned int* output, int msb)
{
	unsigned int num = rand();

	num <<= 15;

	num |= rand();

	num &= 0x1ffff;

	num |= (msb << 17);

	*output = num;
}

void insertNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;
	unsigned int num_list[SUB_SIZE] = { 0, };

	if (biased)
	{
		getRandBiasedNum(&num, msb);
	}
	else
	{
		getRandNum(&num);
	}

	int count = 0;

	while (1)
	{
		if (count == SUB_SIZE)
			break;

		auto ret = setNum.insert(num);

		if (ret.second == true)
		{
			num_list[count] = num;
			count++;
		}

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
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = rbTree.GetDepthInsertNode(num_list[i]);
			ProfileSetDepth(depth, L"RedBlackInsert");
		}

		PRO_BEGIN(L"RedBlackInsert");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			rbTree.InsertNode(num_list[i]);
		}
		PRO_END(L"RedBlackInsert");

		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = bTree.GetDepthInsertNode(num_list[i]);
			ProfileSetDepth(depth, L"BinaryTreeInsert");
		}

		PRO_BEGIN(L"BinaryTreeInsert");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			bTree.InsertNode(num_list[i]);
		}
		PRO_END(L"BinaryTreeInsert");

		PRO_BEGIN(L"HashTableInsert");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			hash.InsertNode(num_list[i]);
		}
		PRO_END(L"HashTableInsert");
	}
	else
	{
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			rbTree.InsertNode(num_list[i]);
			bTree.InsertNode(num_list[i]);
			hash.InsertNode(num_list[i]);
		}
	}
}

bool deleteNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;
	unsigned int num_list[SUB_SIZE] = { 0, };

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

	int count = 0;

	while (1)
	{
		if (count == SUB_SIZE)
			break;

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
			num_list[count++] = num;

			setNum.erase(iter);

			if (biased)
			{
				getRandBiasedNum(&num, msb);
			}
			else
			{
				getRandNum(&num);
			}
		}
	}

	if (checkPerformance)
	{
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = rbTree.GetDepthDeleteNode(num_list[i]);
			ProfileSetDepth(depth, L"RedBlackDelete");
		}

		PRO_BEGIN(L"RedBlackDelete");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			ret = rbTree.DeleteNode(num_list[i]);
		}
		PRO_END(L"RedBlackDelete");

		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = bTree.GetDepthDeleteNode(num_list[i]);
			ProfileSetDepth(depth, L"BinaryTreeDelete");
		}

		PRO_BEGIN(L"BinaryTreeDelete");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			ret = bTree.DeleteNode(num_list[i]);
		}
		PRO_END(L"BinaryTreeDelete");

		PRO_BEGIN(L"HashTableDelete");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			ret3 = hash.DeleteNode(num_list[i]);
		}
		PRO_END(L"HashTableDelete");
	}
	else
	{
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			ret = rbTree.DeleteNode(num_list[i]);
			ret2 = bTree.DeleteNode(num_list[i]);
			ret3 = hash.DeleteNode(num_list[i]);
		}
	}

	return ret && ret2 && ret3;
}

bool searchNum(BinaryTree& bTree, RedBlackTree& rbTree, MyHashMap& hash, std::unordered_set<unsigned int>& setNum, bool checkPerformance, bool biased = false, int msb = 0)
{
	unsigned int num;
	unsigned int num_list[SUB_SIZE] = { 0, };

	if (biased)
	{
		getRandBiasedNum(&num, msb);
	}
	else
	{
		getRandNum(&num);
	}

	std::unordered_set<unsigned int>::iterator iter;

	int count = 0;

	while (1)
	{
		if (count == SUB_SIZE)
			break;

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
			num_list[count++] = num;

			if (biased)
			{
				getRandBiasedNum(&num, msb);
			}
			else
			{
				getRandNum(&num);
			}
		}
	}

	bool found;
	bool found1;
	bool found2;

	if (checkPerformance)
	{
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = rbTree.GetDepthSearchData(num_list[i]);
			ProfileSetDepth(depth, L"RedBlackSearch");
		}

		PRO_BEGIN(L"RedBlackSearch");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			found = rbTree.SearchData(num_list[i]);
		}
		PRO_END(L"RedBlackSearch");

		for (int i = 0; i < SUB_SIZE; ++i)
		{
			int depth = bTree.GetDepthSearchData(num_list[i]);
			ProfileSetDepth(depth, L"BinaryTreeSearch");
		}

		PRO_BEGIN(L"BinaryTreeSearch");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			found1 = bTree.SearchData(num_list[i]);
		}
		PRO_END(L"BinaryTreeSearch");

		PRO_BEGIN(L"HashTableSearch");
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			found2 = hash.SearchData(num_list[i]);
		}
		PRO_END(L"HashTableSearch");
	}
	else
	{
		for (int i = 0; i < SUB_SIZE; ++i)
		{
			found = rbTree.SearchData(num_list[i]);
			found1 = bTree.SearchData(num_list[i]);
			found2 = hash.SearchData(num_list[i]);
		}
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
	
	// pre setting 100만 건
	for (int i = 0; i < TOTAL_SIZE; i++)
	{
		insertNum(bTree, rbTree, myHash, setNums, false);
	}

	// run
	for (int i = 0; i < TOTAL_SIZE * 20; ++i)
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
	}

	for (int i = 0; i < TOTAL_SIZE * 10; ++i)
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

	std::unordered_set<unsigned int> setNums;

	// 32 * 32 = 1024(TOTAL_SIZE)
	for (int g = 0; g < groupNum; ++g)
	{
		for (int i = 0; i < 32; i++)
		{
			insertNum(bTree, rbTree, myHash, setNums, false, true, g);
		}
	}
	
	for (int i = 0; i < TOTAL_SIZE * 20; ++i)
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
	}

	for (int i = 0; i < TOTAL_SIZE  * 10; ++i)
	{
		searchNum(bTree, rbTree, myHash, setNums, true);
	}

	WCHAR file_name[32] = { 0, };
	swprintf_s(file_name, _countof(file_name), L"Profile_Biased");
	ProfileDataOutText(file_name);
}
