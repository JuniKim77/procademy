class myNew
{
public:
	friend void* operator new (size_t size, const char* File, int Line);
	friend void* operator new[] (size_t size, const char* File, int Line);
	friend void operator delete (void* p);
	friend void operator delete[] (void* p);
	myNew(const char* fileName = "ALLOC");
	~myNew();

	enum
	{
		FILE_NAME_LENGTH = 128,
		ALLOC_MAX = 1500
	};
	enum logType
	{
		LOG_TYPE_NOALLOC,
		LOG_TYPE_ARRAY,
		LOG_TYPE_LEAK
	};

	struct stMemory_Info
	{
		void* mAddr;
		size_t mSize;
		char mFileName[FILE_NAME_LENGTH];
		int mLine;
		bool mIsArray;
	};

private:
	stMemory_Info mMemory[ALLOC_MAX];
	char mLogFileName[FILE_NAME_LENGTH];

	void writeLog(logType type, void* pMemory);
	bool newAlloc(void** pPtr, const char* fileName, int line, size_t size, bool bArray = false);
	bool deleteAlloc(void* pPtr, bool bArray = false);
};