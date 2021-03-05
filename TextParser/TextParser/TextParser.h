class TextParser
{
public:
	TextParser();
	~TextParser();
	void LoadFile(const char* fileName);
	bool GetValue(char* key, int* pLength);

private:
	bool SkipNoneCommand();
	bool SkipComment();
	bool GetNextWord(char** retBuffer, int* pLength);
	bool GetStringWord(char** retBuffer, int* pLength);

	char* pCurrent;
	char* pBuffer;
};