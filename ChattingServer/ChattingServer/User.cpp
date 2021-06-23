#include "User.h"

User::User(DWORD userNo, const WCHAR* name)
	: mUserNo(userNo)
{
	wcscpy_s(mName, _countof(mName), name);

	mName[_countof(mName) - 1] = L'\0';
}

User::~User()
{
}
