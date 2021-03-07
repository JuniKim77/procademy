class CSVFile
{
public:
	CSVFile(const char* fileName);
	~CSVFile();
	void readFile();
	const char** mMemory;
	int mLineLength;

private:
	int countLine(const char* buffer) const;

	const char* mFileName;
};