#include <stdio.h>
#include "CSVReader.h"
#include <locale>

int main()
{
	setlocale(LC_ALL, "");
	CSVFile test(L"test3.csv");

	bool result = test.SelectRow(0);

	result = test.NextRow();
	result = test.PreRow();
	result = test.PreRow();
	result = test.SelectRow(0);

	WCHAR str[4];

	test.GetColumn(str, _countof(str));

	wprintf_s(str);
	result = test.NextColumn();

	for (int i = 0; i < test.GetCol() - 1; ++i)
	{
		int value;

		if (test.GetColumn(&value))
		{
			wprintf(L" %d", value);
		}

		result = test.NextColumn();
	}

	wprintf(L"\n");

	for (int i = 0; i < test.GetCol() - 1; ++i)
	{
		result = test.PreColumn();
		int value;

		if (test.GetColumn(&value))
		{
			wprintf(L"%d ", value);
		}
	}

	WCHAR str2[4];
	result = test.PreColumn();
	test.GetColumn(str2, _countof(str2));

	wprintf_s(L"%s\n", str2);

	result = test.MoveColumn(5);

	{
		int value;

		if (test.GetColumn(&value))
		{
			wprintf(L"%d\n", value);
		}

		test.NextRow();

		if (test.GetColumn(&value))
		{
			wprintf(L"%d\n", value);
		}

		test.PreRow();

		if (test.GetColumn(&value))
		{
			wprintf(L"%d\n", value);
		}
	}

	return 0;
}