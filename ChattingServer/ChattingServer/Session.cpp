#include "Session.h"

Session::Session(SOCKET socket, u_short port, u_long ip, DWORD idNum)
	: mSocket(socket)
	, mPort(port)
	, mIP(ip)
	, mIDNum(idNum)
{
}

Session::~Session()
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
	}
}

void Session::SetName(const WCHAR* name)
{
	wcscpy_s(mName, _countof(mName), name);

	mName[_countof(mName) - 1] = L'\0';
}

void Session::printInfo() const
{
}
