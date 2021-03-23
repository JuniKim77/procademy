#define MAX_PARSER_LENGTH (256)

class TextParser
{
public:
	TextParser();
	~TextParser();
	bool LoadFile(const char* fileName);
	bool GetValue(const char* key, int* value);
	bool GetValue(const char* key, char* value);

private:
	bool SkipNoneCommand(char** retBuffer);
	bool SkipComment(char** retBuffer);
	bool GetNextWord(char** retBuffer, int* pLength);
	bool GetNextStringWord(char** retBuffer, int* pLength);
	void GetEndWord(char** retBuffer);
	void GetEndStringWord(char** retBuffer);

private:
	char* pBuffer;
};