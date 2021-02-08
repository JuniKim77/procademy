#include <iostream>
#include <Windows.h>
#include <time.h>
#include <conio.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

enum Result {
	GREAT = 1,
	GOOD,
	NOT_GOOD,
	BAD,
	FAIL
};

void printTime(clock_t time)
{
	int sec = time / 1000;
	int msec = time % 1000;
	char buf[64];

	if (sec > 9)
	{
		if (msec > 99)
		{
			sprintf_s(buf, "Time: %d:%d\n", sec, msec);
		}
		else if (msec > 9)
		{
			sprintf_s(buf, "Time: %d:0%d\n", sec, msec);
		}
		else {
			sprintf_s(buf, "Time: %d:00%d\n", sec, msec);
		}
	}
	else {
		if (msec > 99)
		{
			sprintf_s(buf, "Time: 0%d:%d\n", sec, msec);
		}
		else if (msec > 9)
		{
			sprintf_s(buf, "Time: 0%d:0%d\n", sec, msec);
		}
		else {
			sprintf_s(buf, "Time: 0%d:00%d\n", sec, msec);
		}
	}

	cout << buf;
}

int main()
{
	timeBeginPeriod(1);
	int count = 0;
	int expectedTimes[] = { 5, 8, 10, 12, 17, 21, 25 };
	int result[7];

	while (1)
	{
		system("cls");
		clock_t time = clock();

		if (_kbhit()) {
			char c = getc(stdin);

			if (count < 7)
			{
				result[count] = abs(time - expectedTimes[count] * 1000) / 200;
				count++;
			}
		}
		
		printTime(time);

		for (int i = 0; i < 7; ++i)
		{
			cout << expectedTimes[i] << " ";
		}
		cout << endl;

		for (int i = 0; i < count; ++i)
		{
			cout << "Expected Time: " << expectedTimes[i] << "  ";
			switch (result[i])
			{
			case 0:
			case GREAT:
				cout << "Great" << endl;
				break;
			case GOOD:
				cout << "Good" << endl;
				break;
			case NOT_GOOD:
				cout << "Not Good" << endl;
				break;
			case BAD:
				cout << "Bad" << endl;
				break;
			default:
				cout << "Fail" << endl;
				break;
			}
		}
	}

	timeEndPeriod(1);

	return 0;
}