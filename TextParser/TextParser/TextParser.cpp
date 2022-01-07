#include "TextParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

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

bool TextParser::LoadFile(const WCHAR* fileName)
{
	FILE* fin;
	_wfopen_s(&fin, fileName, L"r,ccs=UTF-16LE");

	fseek(fin, 0, SEEK_END);
	int fileSize = ftell(fin) + 1;
	fseek(fin, 0, SEEK_SET);

	pBuffer = (WCHAR*)malloc(fileSize);
	memset(pBuffer, 0, fileSize);
	
	int result = fread_s(pBuffer, fileSize, fileSize, 1, fin); // check

	fclose(fin);

	return result == 1;
}

bool TextParser::GetValue(const WCHAR* key, int* value)
{
	WCHAR* pCurrent = pBuffer;
	WCHAR chWord[MAX_PARSER_LENGTH];
	int length;

	chWord[0] = '\0';

	while (GetNextWord(&pCurrent, &length))
	{
		memset(chWord, 0, sizeof(chWord));
		memcpy(chWord, pCurrent, sizeof(WCHAR) * length);

		if (wcscmp(key, chWord) == 0)
		{
			if (GetNextWord(&pCurrent, &length))
			{
				memset(chWord, 0, sizeof(chWord));
				memcpy(chWord, pCurrent, sizeof(WCHAR) * length);

				if (wcscmp(chWord, L"=") == 0)
				{
					if (GetNextWord(&pCurrent, &length))
					{
						memset(chWord, 0, sizeof(chWord));
						memcpy(chWord, pCurrent, sizeof(WCHAR) * length);
						*value = _wtoi(chWord);

						return true;
					}

					return false;
				}
			}

			return false;
		}
	}

	return false;
}

bool TextParser::GetValue(const WCHAR* key, WCHAR* value)
{
	WCHAR* pCurrent = pBuffer;
	WCHAR chWord[MAX_PARSER_LENGTH];
	int length;

	chWord[0] = '\0';

	while (GetNextWord(&pCurrent, &length))
	{
		memset(chWord, 0, sizeof(chWord));
		memcpy(chWord, pCurrent, sizeof(WCHAR) * length);

		if (wcscmp(key, chWord) == 0)
		{
			if (GetNextWord(&pCurrent, &length))
			{
				memset(chWord, 0, sizeof(chWord));
				memcpy(chWord, pCurrent, sizeof(WCHAR) * length);

				if (wcscmp(chWord, L"=") == 0)
				{
					if (GetNextStringWord(&pCurrent, &length))
					{
						memcpy(value, pCurrent + 1, sizeof(WCHAR) * length - 4);
						value[length - 2] = L'\0';

						return true;
					}

					return false;
				}
			}

			return false;
		}
	}

	return false;
}

bool TextParser::SkipNoneCommand(WCHAR** retBuffer)
{
	while (1)
	{
		if (**retBuffer == L' ' || **retBuffer == L'\n' || **retBuffer == L'\t' 
			|| **retBuffer == L'{' || **retBuffer == L'}')
		{
			++(*retBuffer);
		}
		else if (**retBuffer == L'/')
		{
			// 林籍 贸府
			if (!SkipComment(retBuffer))
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}

	return **retBuffer != '\0';
}

bool TextParser::SkipComment(WCHAR** retBuffer)
{
	++(*retBuffer);
	// 茄临 林籍 贸府
	if (**retBuffer == L'/')
	{
		++(*retBuffer);

		while (1)
		{
			if (**retBuffer == L'\n')
			{
				++(*retBuffer);

				return true;
			}
			if (**retBuffer == L'\0')
			{
				return false;
			}

			++(*retBuffer);
		}
	}
	// 咯矾临 林籍 贸府
	else if (**retBuffer == L'*')
	{
		++(*retBuffer);

		while (1)
		{
			if (**retBuffer == L'*' && *(*retBuffer + 1) == L'/')
			{
				(*retBuffer) += 2;

				return true;
			}
			if (**retBuffer == L'\0')
			{
				return false;
			}

			++(*retBuffer);
		}
	}

	return false;
}

bool TextParser::GetNextWord(WCHAR** retBuffer, int* pLength)
{
	GetEndWord(retBuffer);

	if (**retBuffer == L'"')
	{
		GetEndStringWord(retBuffer);
	}

	if (SkipNoneCommand(retBuffer) == false)
	{
		return false;
	}

	WCHAR* wordBuffer = *retBuffer;

	if (*wordBuffer == L'"')
	{
		wordBuffer++;
	}

	GetEndWord(&wordBuffer);

	*pLength = (int)(wordBuffer - *retBuffer);

	return true;
}

bool TextParser::GetNextStringWord(WCHAR** retBuffer, int* pLength)
{
	GetEndWord(retBuffer);

	if (SkipNoneCommand(retBuffer) == false)
	{
		return false;
	}

	WCHAR* wordBuffer = *retBuffer;
	
	GetEndStringWord(&wordBuffer);

	*pLength =(int)(wordBuffer - *retBuffer);

	return true;
}

void TextParser::GetEndWord(WCHAR** retBuffer)
{
	while (1)
	{
		if (**retBuffer == L'.' || **retBuffer == L'"' || **retBuffer == (WCHAR)0x20 ||
			**retBuffer == (WCHAR)0x08 || **retBuffer == (WCHAR)0x09 || **retBuffer == (WCHAR)0x0a ||
			**retBuffer == (WCHAR)0x0d)
		{
			break;
		}
		++(*retBuffer);
	}
}

void TextParser::GetEndStringWord(WCHAR** retBuffer)
{
	++(*retBuffer);

	while (1)
	{
		if (**retBuffer == L'"')
		{
			++(*retBuffer);
			break;
		}
		++(*retBuffer);
	}
}
