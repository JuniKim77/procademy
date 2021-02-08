#include <random>
#include <iostream>
#include <tchar.h>
#include <ctime>

using namespace std;

struct Item
{
	TCHAR name[32];
	int freq;
};

int order[175];

void swap(int* left, int* right);

void knuthShuffle() {
	for (int i = 0; i < 175; ++i)
	{
		order[i] = i;
	}

	for (int i = 174; i > 0; --i)
	{
		int pickedNum = rand() % i;
		swap(&order[pickedNum], &order[i]);
	}
}

void swap(int* left, int* right)
{
	int temp = *left;
	*left = *right;
	*right = temp;
}

Item gItems[5] = {
	{_T("�Ϲ� ������"), 100 },
	{_T("��� ������"), 50 },
	{_T("���� ������"), 20 },
	{_T("(���) ������"), 4 },
	{_T("((������)) ������"), 1 } // 175
};

int index[5] = { 100, 150, 170, 174, 175 };
int gCount = 0;

void update();

void GetItem() {
	int idx = order[gCount];

	wcout << _T("Ƚ�� : ") << gCount + 1 << _T(" ==== ");

	if (idx < index[0]) {
		wcout << gItems[0].name << " " << ++gItems[0].freq;
	}
	else if (idx < index[1]) {
		wcout << gItems[1].name << " " << ++gItems[1].freq;
	}
	else if (idx < index[2]) {
		wcout << gItems[2].name << " " << ++gItems[2].freq;
	}
	else if (idx < index[3]) {
		wcout << gItems[3].name << " " << ++gItems[3].freq;
	}
	else {
		wcout << gItems[4].name << " " << ++gItems[4].freq;
	}

	gCount++;

	if (gCount == 175)
	{
		knuthShuffle();
		update();

		gCount = 0;
	}
}

void update() {
	for (int i = 0; i < 5; ++i)
	{
		gItems[i].freq = 0;
	}
}

int main()
{
	update();
	srand(time(nullptr));
	wcout.imbue(std::locale("kor")); // �̰��� �߰��ϸ� �ȴ�.
	wcin.imbue(std::locale("kor")); // cin�� �̰��� �߰�

	knuthShuffle();

	while (1)
	{
		getchar();
		GetItem();
	}


	return 0;
}