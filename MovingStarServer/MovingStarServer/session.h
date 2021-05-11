#include <WinSock2.h>

#define SCREEN_WIDTH (80)
#define SCREEN_HEIGTH (23)

class Session
{
public:
	Session();

	// network
	SOCKET socket;
	u_short port;
	u_long IP;
	// content
	int x = SCREEN_WIDTH / 2;
	int y = SCREEN_HEIGTH / 2;
	int ID = 0;
};