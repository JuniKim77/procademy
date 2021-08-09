#ifndef __DEFINE__
#define __DEFINE__


struct st_SESSION
{
	int SessionID;
};


struct st_PLAYER
{
	int SessionID;
	int Content[3];
};

#define dfSESSION_NUM (5000)
#define dfTHREAD_NUM	(3)

#define dfSESSION_NONE (0)
#define dfSESSION_CONNECT (1)
#define dfSESSION_LOGIN (2)
#endif