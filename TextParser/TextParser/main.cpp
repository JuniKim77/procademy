#include "TextParser.h"
#include <locale.h>

int main()
{
	setlocale(LC_ALL, "");
	TextParser tp;

	tp.LoadFile("test.txt");

	int value;

	tp.GetValue("Version", &value);

	int value2;

	tp.GetValue("MaxUser", &value2);
	
	char value3[MAX_PARSER_LENGTH];

	tp.GetValue("Tag", value3);

	char value4[MAX_PARSER_LENGTH];

	tp.GetValue("ServerBindIP", value4);


	return 0;
}