#pragma once

#define dfTYPE_ADD_STR 0
#define dfTYPE_DEL_STR 1
#define dfTYPE_PRINT_LIST 2
#define dfTYPE_QUIT 3

struct st_MSG_HEAD
{
	short shType;
	short shStrLen;
};
