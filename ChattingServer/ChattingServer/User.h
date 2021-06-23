#pragma once

#include <wtypes.h>

class User
{
public:
	User(DWORD userNo, const WCHAR* name);
	~User();

private:
	DWORD mUserNo;
	WCHAR mName[15];
};