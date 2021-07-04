#pragma once

#include <wtypes.h>
#include "Sector.h"

struct User
{
	DWORD userNo;
	DWORD sessionNo;

	DWORD action;
	BYTE direction;
	BYTE moveDirection;

	short x;
	short y;

	st_Sector curSector;
	st_Sector oldSector;

	char hp;
};