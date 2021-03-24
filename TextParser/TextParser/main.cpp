#include "TextParser.h"
#include <locale.h>

int main()
{
	_wsetlocale(LC_ALL, L"");
	TextParser tp;

	tp.LoadFile(L"test.txt");

	int value;

	tp.GetValue(L"Version", &value);

	int value2;

	tp.GetValue(L"MaxUser", &value2);
	
	WCHAR value3[MAX_PARSER_LENGTH];

	tp.GetValue(L"Tag", value3);

	WCHAR value4[MAX_PARSER_LENGTH];

	tp.GetValue(L"ServerBindIP", value4);

	WCHAR value5[MAX_PARSER_LENGTH];

	tp.GetValue(L"ÀÛ¼ºÀÚ", value5);

	return 0;
}