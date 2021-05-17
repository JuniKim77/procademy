#include <WinSock2.h>

#define SCREEN_WIDTH (80)
#define SCREEN_HEIGTH (23)

class RingBuffer;

class Session
{
public:
	Session();
	~Session();
	void printInfo() const;

	// network
	SOCKET socket;
	u_short port;
	u_long IP;
	RingBuffer* sendRingBuffer;
	RingBuffer* receiveRingBuffer;

	// content
	int x = SCREEN_WIDTH / 2;
	int y = SCREEN_HEIGTH / 2;
	int ID = 0;
};