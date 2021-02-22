enum logType;
class myNew
{
public:
	friend void* operator new (size_t size, const char* File, int Line);
	friend void operator delete (void* p);
	friend void operator delete[] (void* p);
	friend void writeLog(logType type, void* pMemory);
	~myNew();

private:
	void* mAddr;
	size_t mSize;
	char mFileName[256];
	int mLine;
};