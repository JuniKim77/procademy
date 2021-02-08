#include <iostream>
#include <Windows.h>

using namespace std;

void MyQuickSort(int left, int right, int* data);
void MyQuickSortStack(int left, int right, int* data);
int Partition(int left, int right, int* data);
void printInts(int* data, size_t size);

#define STACK_SIZE (2000)
int stk[STACK_SIZE];
int top = -1;

int main()
{
	int test1[16] = { 19, 5, 7, 2, 4, 11, 176, 2, 5, 7, 0, 11, 13, 12, 19, 50 };
	int test2[16] = { 19, 5, 7, 2, 4, 11, 176, 2, 5, 7, 0, 11, 13, 12, 19, 50 };

	printInts(test1, 16);

	LARGE_INTEGER begin;
	LARGE_INTEGER end;

	QueryPerformanceCounter(&begin);

	MyQuickSort(0, 15, test1);

	QueryPerformanceCounter(&end);

	printf("Time Duration: %I64d\n", end.QuadPart - begin.QuadPart);

	printInts(test1, 16);

	printInts(test2, 16);

	QueryPerformanceCounter(&begin);

	MyQuickSortStack(0, 15, test2);

	QueryPerformanceCounter(&end);

	printf("Time Duration: %I64d\n", end.QuadPart - begin.QuadPart);

	printInts(test2, 16);

	return 0;
}

void MySwap(int* left, int* right)
{
	int temp = *left;
	*left = *right;
	*right = temp;
}

void MyQuickSort(int left, int right, int* data)
{
	if (left < right)
	{
		int mid = Partition(left, right, data);

		MyQuickSort(left, mid - 1, data);
		MyQuickSort(mid + 1, right, data);
	}
}

void MyQuickSortStack(int left, int right, int* data)
{
	stk[++top] = left;
	stk[++top] = right;

	while (top >= 0)
	{
		int cur_left = stk[top--];
		int cur_right = stk[top--];

		int mid = Partition(cur_left, cur_right, data);

		if (cur_left < mid - 1)
		{
			stk[++top] = cur_left;
			stk[++top] = mid - 1;
		}
		if (mid + 1 < cur_right)
		{
			stk[++top] = mid + 1;
			stk[++top] = cur_right;
		}
	}
}

int Partition(int left, int right, int* data)
{
	int pivot = *(data + left);
	int* pLeft = data + left + 1;
	int* pRight = data + right;

	while (1)
	{
		while (*pLeft < pivot)
			++pLeft;

		while (*pRight > pivot)
			--pRight;
		
		if (pLeft >= pRight)
			break;

		MySwap(pLeft, pRight);
	}

	MySwap((data + left), pRight);

	return pRight - data;
}

void printInts(int* data, size_t size)
{
	for (int i = 0; i < size; ++i)
	{
		cout << data[i] << " ";
	}
	cout << endl;
}
