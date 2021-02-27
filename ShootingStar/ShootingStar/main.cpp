#include "Process.h"
#include <Windows.h>

int main()
{
	while (1)
	{
		KeyProcess();

		Update();

		system("cls");
		Render();

		Sleep(30);
	}

	return 0;
}