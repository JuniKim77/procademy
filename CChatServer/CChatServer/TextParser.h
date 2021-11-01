#pragma once
#include <Windows.h>
#define MAX_PARSER_LENGTH (256)

class TextParser
{
public:
	TextParser();
	~TextParser();
	bool LoadFile(const WCHAR* fileName);
	bool GetValue(const WCHAR* key, int* value);
	bool GetValue(const WCHAR* key, WCHAR* value);

private:
	bool SkipNoneCommand(WCHAR** retBuffer);
	bool SkipComment(WCHAR** retBuffer);
	bool GetNextWord(WCHAR** retBuffer, int* pLength);
	bool GetNextStringWord(WCHAR** retBuffer, int* pLength);
	void GetEndWord(WCHAR** retBuffer);
	void GetEndStringWord(WCHAR** retBuffer);

private:
	WCHAR* pBuffer;
};