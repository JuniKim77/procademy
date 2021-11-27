#pragma once

#include <wtypes.h>

namespace procademy
{
	class CDBConnector;

	bool SelectAccountInfo(CDBConnector* conn, INT64 accountNo);
}