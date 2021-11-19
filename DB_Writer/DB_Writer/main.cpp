#include "CDBConnector.h"

int main()
{
	procademy::CDBConnector db(L"127.0.0.1", L"root", L"123456789", L"test", 3306);

	db.Disconnect();

	return 0;
}