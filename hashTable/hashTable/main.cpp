#include <stdlib.h>
#include <stdio.h>

#define bucket_size (10)

enum Process
{
	ADD,
	DELETE,
	PRINT_ALL,
	FIND
};

void Add();
void Delete();
void Print();
void Find();
void Run();
int select_option();

struct node
{
	int key;
	int data;
	node* next;

	node(int k, int d)
		: key(k)
		, data(d)
		, next(NULL)
	{

	}
};

node* bucket[bucket_size] = { 0, };

void Run()
{
	while (1)
	{
		int option = select_option();
		
		switch(option) {
		case ADD:
			Add();
			break;
		case DELETE:
			Delete();
			break;
		case PRINT_ALL:
			Print();
			break;
		case FIND:
			Find();
			break;
		default:
			break;
		}

		printf("press any key...\n");
		fgetc(stdin);
		fgetc(stdin);
		system("cls");
	}
	
}

int select_option()
{
	printf("## MENU ##\n");
	printf("1. 데이터 추가\n");
	printf("2. 데이터 삭제\n");
	printf("3. 전체보기\n");
	printf("4. 찾기\n");
	printf(": ");
	int ret;
	scanf_s("%d", &ret);
	printf("\n");

	return ret - 1;
}

void Add()
{
	int key;
	int value;

	printf("Key : ");
	scanf_s("%d", &key);
	printf("Data : ");
	scanf_s("%d", &value);

	int index = key % bucket_size;

	node* pNode = new node(key, value);

	node* pCur = bucket[index];

	if (pCur == nullptr)
	{
		bucket[index] = pNode;

		printf("insert success\n\n");

		return;
	}

	while (pCur->next != nullptr)
	{
		pCur = pCur->next;
	}

	pCur->next = pNode;

	printf("insert success\n\n");

	return;
}
void Delete()
{
	int key;

	printf("Key : ");
	scanf_s("%d", &key);

	int index = key % bucket_size;

	node* pCur = bucket[index];

	if (pCur == nullptr)
	{
		printf("Key Not Found\n\n");

		return;
	}

	node* prev = bucket[index];

	if (pCur->key == key)
	{
		bucket[index] = pCur->next;
		delete pCur;
		return;
	}

	pCur = pCur->next;

	while (pCur != nullptr)
	{
		if (pCur->key == key) {
			prev->next = pCur->next;
			delete pCur;

			return;
		}

		prev = pCur;
		pCur = pCur->next;
	}
}
void Print()
{
	for (int i = 0; i < bucket_size; ++i)
	{
		printf("[0%d]", i);

		node* pCur = bucket[i];

		while (pCur != nullptr)
		{
			printf(" > Key:%d | Data:%d", pCur->key, pCur->data);
			pCur = pCur->next;
		}

		printf("\n");
	}
	printf("\n");
}
void Find()
{
	int key;

	printf("Find Key : ");
	scanf_s("%d", &key);

	int index = key % bucket_size;

	node* pCur = bucket[index];

	if (pCur == nullptr)
	{
		printf("Not found\n\n");

		return;
	}

	while (pCur != nullptr)
	{
		if (pCur->key == key)
		{
			printf("Find Data : %d\n\n", pCur->data);

			return;
		}

		pCur = pCur->next;
	}

	printf("Not found\n\n");

	return;
}

int main()
{
	Run();

	return 0;
}