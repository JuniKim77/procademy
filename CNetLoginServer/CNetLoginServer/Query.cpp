#include "Query.h"
#include "CDBConnector.h"

bool procademy::SelectAccountInfo(CDBConnector* conn, INT64 accountNo)
{
    return conn->Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %lld;", accountNo);
}
