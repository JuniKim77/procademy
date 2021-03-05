#include "TextParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

TextParser::TextParser()
	: pBuffer(nullptr)
{
}

TextParser::~TextParser()
{
	if (!pBuffer)
	{
		free(pBuffer);
	}
}

void TextParser::LoadFile(const char* fileName)
{
	FILE* fin;
	fopen_s(&fin, fileName, "r");

	fseek(fin, 0, SEEK_END);
	int fileSize = ftell(fin) + 1;
	fseek(fin, 0, SEEK_SET);

	pBuffer = (char*)malloc(fileSize);
	memset(pBuffer, 0, fileSize);
	
	int result = fread_s(pBuffer, fileSize, fileSize, 1, fin);
	pBuffer[fileSize] = '\0';

	printf(pBuffer);

	fclose(fin);
}

bool TextParser::GetValue(char* key, int* pLength)
{
	pCurrent = pBuffer;



	return false;
}

bool TextParser::SkipNoneCommand()
{
	while (1)
	{
		if (*pCurrent == ' ' || *pCurrent == '\n' || *pCurrent == '\t')
		{
			++pCurrent;
		}
		else if (*pCurrent == '/')
		{
			// �ּ� ó��
			if (!SkipComment())
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}

	return *pCurrent != '\0';
}

bool TextParser::SkipComment()
{
	++pCurrent;
	// ���� �ּ� ó��
	if (*pCurrent == '/')
	{
		++pCurrent;

		while (1)
		{
			if (*pCurrent == '/' && *(pCurrent + 1) == '/')
			{
				pCurrent += 2;

				return true;
			}
			if (*pCurrent == '\0')
			{
				return false;
			}

			++pCurrent;
		}
	}
	// ������ �ּ� ó��
	else if (*pCurrent == '*')
	{
		++pCurrent;

		while (1)
		{
			if (*pCurrent == '*' && *(pCurrent + 1) == '/')
			{
				pCurrent += 2;

				return true;
			}
			if (*pCurrent == '\0')
			{
				return false;
			}

			++pCurrent;
		}
	}

	return false;
}

bool TextParser::GetNextWord(char** retBuffer, int* pLength)
{
	return false;
}

bool TextParser::GetStringWord(char** retBuffer, int* pLength)
{
	return false;
}
