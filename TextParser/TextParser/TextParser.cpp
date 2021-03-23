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

bool TextParser::LoadFile(const char* fileName)
{
	FILE* fin;
	fopen_s(&fin, fileName, "r");

	fseek(fin, 0, SEEK_END);
	int fileSize = ftell(fin) + 1;
	fseek(fin, 0, SEEK_SET);

	pBuffer = (char*)malloc(fileSize);
	memset(pBuffer, 0, fileSize);
	
	int result = fread_s(pBuffer, fileSize, fileSize, 1, fin);

	fclose(fin);

	return result == 1;
}

bool TextParser::GetValue(const char* key, int* value)
{
	char* pCurrent = pBuffer;
	char chWord[MAX_PARSER_LENGTH];
	int length;

	while (GetNextWord(&pCurrent, &length))
	{
		memset(chWord, 0, MAX_PARSER_LENGTH);
		memcpy(chWord, pCurrent, length);

		if (strcmp(key, chWord) == 0)
		{
			if (GetNextWord(&pCurrent, &length))
			{
				memset(chWord, 0, MAX_PARSER_LENGTH);
				memcpy(chWord, pCurrent, length);

				if (strcmp(chWord, "=") == 0)
				{
					if (GetNextWord(&pCurrent, &length))
					{
						memset(chWord, 0, MAX_PARSER_LENGTH);
						memcpy(chWord, pCurrent, length);
						*value = atoi(chWord);

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

bool TextParser::GetValue(const char* key, char* value)
{
	char* pCurrent = pBuffer;
	char chWord[MAX_PARSER_LENGTH];
	int length;

	while (GetNextWord(&pCurrent, &length))
	{
		memset(chWord, 0, MAX_PARSER_LENGTH);
		memcpy(chWord, pCurrent, length);

		if (strcmp(key, chWord) == 0)
		{
			if (GetNextWord(&pCurrent, &length))
			{
				memset(chWord, 0, MAX_PARSER_LENGTH);
				memcpy(chWord, pCurrent, length);

				if (strcmp(chWord, "=") == 0)
				{
					if (GetNextStringWord(&pCurrent, &length))
					{
						memset(value, 0, MAX_PARSER_LENGTH);
						memcpy(value, pCurrent + 1, length - 2);

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

bool TextParser::SkipNoneCommand(char** retBuffer)
{
	while (1)
	{
		if (**retBuffer == ' ' || **retBuffer == '\n' || **retBuffer == '\t' 
			|| **retBuffer == '{' || **retBuffer == '}')
		{
			++(*retBuffer);
		}
		else if (**retBuffer == '/')
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

bool TextParser::SkipComment(char** retBuffer)
{
	++(*retBuffer);
	// 茄临 林籍 贸府
	if (**retBuffer == '/')
	{
		++(*retBuffer);

		while (1)
		{
			if (**retBuffer == '\n')
			{
				++(*retBuffer);

				return true;
			}
			if (**retBuffer == '\0')
			{
				return false;
			}

			++(*retBuffer);
		}
	}
	// 咯矾临 林籍 贸府
	else if (**retBuffer == '*')
	{
		++(*retBuffer);

		while (1)
		{
			if (**retBuffer == '*' && *(*retBuffer + 1) == '/')
			{
				(*retBuffer) += 2;

				return true;
			}
			if (**retBuffer == '\0')
			{
				return false;
			}

			++(*retBuffer);
		}
	}

	return false;
}

bool TextParser::GetNextWord(char** retBuffer, int* pLength)
{
	GetEndWord(retBuffer);

	if (**retBuffer == '"')
	{
		GetEndStringWord(retBuffer);
	}

	if (SkipNoneCommand(retBuffer) == false)
	{
		return false;
	}

	char* wordBuffer = *retBuffer;

	if (*wordBuffer == '"')
	{
		wordBuffer++;
	}

	GetEndWord(&wordBuffer);

	*pLength = wordBuffer - *retBuffer;

	return true;
}

bool TextParser::GetNextStringWord(char** retBuffer, int* pLength)
{
	GetEndWord(retBuffer);

	if (SkipNoneCommand(retBuffer) == false)
	{
		return false;
	}

	char* wordBuffer = *retBuffer;
	
	GetEndStringWord(&wordBuffer);

	*pLength = wordBuffer - *retBuffer;

	return true;
}

void TextParser::GetEndWord(char** retBuffer)
{
	while (1)
	{
		if (**retBuffer == '.' || **retBuffer == '"' || **retBuffer == 0x20 ||
			**retBuffer == 0x08 || **retBuffer == 0x09 || **retBuffer == 0x0a ||
			**retBuffer == 0x0d)
		{
			break;
		}
		++(*retBuffer);
	}
}

void TextParser::GetEndStringWord(char** retBuffer)
{
	++(*retBuffer);

	while (1)
	{
		if (**retBuffer == '"')
		{
			++(*retBuffer);
			break;
		}
		++(*retBuffer);
	}
}
