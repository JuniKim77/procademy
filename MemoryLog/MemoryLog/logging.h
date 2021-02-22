enum logType
{
	LOG_TYPE_NOALLOC,
	LOG_TYPE_ARRAY,
	LOG_TYPE_LEAK
};

void writeLog(logType type, void* pMemory);