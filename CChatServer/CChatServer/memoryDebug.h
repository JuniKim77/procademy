#pragma once

struct memDebug
{
	void* address;
	const char* fileName;
	int line;
	bool bArray;
	bool bAlignment;
	int hour;
	int minute;
	int second;
};