#ifndef TXTREADER
#define TXTREADER

class TxtReader
{
public:
	TxtReader(const char* fileName);
	~TxtReader();
	void GetNextLine(char** pBegin, char** pEnd, char* buffer);
	void ReadNextParagraph(char** pBegin, char** pEnd, int* lineSize, char*** dest);
	char* getBuffer() { return mBuffer; }

private:
	void SaveFileToBuffer();

private:
	const char* mFileName;
	char* mBuffer;
};

#endif
