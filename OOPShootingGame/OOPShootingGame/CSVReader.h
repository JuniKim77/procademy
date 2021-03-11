#ifndef CSVFILE
#define CSVFILE

class CSVFile
{
public:
	CSVFile(const char* fileName);
	~CSVFile();
	void readFile();
	int GetRow() { return mRowSize; }
	int GetCol() { return mColSize; }
	const char* GetRowAddress(int line);
	const char* GetTitleAddress();
	void GetNextComma(const char** pBuf);

private:
	int countRow(const char* buffer) const;
	int countCol(const char* buffer) const;

private:
	int mRowSize;
	int mColSize;
	const char* mFileName;
	char* mBuffer;
};

#endif