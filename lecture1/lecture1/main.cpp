#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

using namespace std;

struct DATA
{
	int a;
	char b;
	int c;
	LONG64 d;
	char e;
};

void Test(DATA data)
{
	data.a = 1;
	data.b = 2;
	data.c = 3;
	data.d = 0x00ffffffffffffff;
	data.e = 3;
}

void Test2()
{
	int a = 4;
	char c = 'c';
	int b = 2;
	LONG64 k = 5;
	char d = 'a';

	printf("%d %c %d\n", a, c, b);
	printf("%lld %c", k, d);
}
union bitFlag
{
	struct
	{
		char a : 3;
		char b : 3;
		char c : 2;
	} bits;
	char ch;
};

int main()
{
	bitFlag flg;

	flg.bits.a = 0;
	flg.bits.b = 1;
	flg.bits.c = 2;

	flg.ch = 100;

	DATA data;

	data.a = 0;
	data.b = 0;
	data.c = 0;

	Test(data);

	Test2();

	//int a = 1000;

	//int mod = a % 3;
	//char buf[16];
	//strcpy_s(buf, 16, "Starting");
	//strcat_s(buf, 16, " End");               // Correct

	//char buffer[1500] = "test";
	//buffer[4] = 'K';
	//char adder[] = "addStr";


	//strcat_s(buffer, _countof(buffer), adder);

	//char test[4];
	//char test2[4];
	//scanf_s("%3c", test, _countof(test));
	//scanf_s("%4c", test2, _countof(test2));
	//
	//printf("%s\n", test);
	//printf("%s\n", test2);
	//printf("%d", _countof(test));

	return 0;
}