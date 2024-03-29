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

Item gItems[5] = {
	{_T("일반 아이템"), 100 },
	{_T("고급 아이템"), 50 },
	{_T("레어 아이템"), 20 },
	{_T("(희귀) 아이템"), 4 },
	{_T("((레전드)) 아이템"), 1 } // 175
};

int initial[5] = { 100, 50, 20, 4, 1 };
int index[5];
int gCount = 175;

void update();

void GetItem() {
	int idx = rand() % gCount;

	wcout << _T("남은 횟수 : ") << gCount << _T(" ==== ");

	if (idx < index[0]) {
		wcout << gItems[0].name << " " << gItems[0].freq--;
	}
	else if (idx < index[1]) {
		wcout << gItems[1].name << " " << gItems[1].freq--;
	}
	else if (idx < index[2]) {
		wcout << gItems[2].name << " " << gItems[2].freq--;
	}
	else if (idx < index[3]) {
		wcout << gItems[3].name << " " << gItems[3].freq--;
	}
	else {
		wcout << gItems[4].name << " " << gItems[4].freq--;
	}

	update();
	gCount--;

	if (gCount == 0)
	{
		for (int i = 0; i < 5; ++i)
		{
			gItems[i].freq = initial[i];
		}

		gCount = 175;
		update();
	}
}

void update() {
	int count = 0;
	for (int i = 0; i < 5; ++i)
	{
		count += gItems[i].freq;
		index[i] = count;
	}
}

int main()
{
	update();
	srand(time(nullptr));
	wcout.imbue(std::locale("kor")); // 이것을 추가하면 된다.
	wcin.imbue(std::locale("kor")); // cin은 이것을 추가

	while (1)
	{
		getchar();
		GetItem();
	}


	return 0;
}