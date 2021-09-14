#include "CLanServerNoLock.h"
#include <unordered_set>

class CEchoServerNoLock : public CLanServerNoLock
{
public:
	CEchoServerNoLock();
	virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
	virtual void OnClientJoin(SESSION_ID SessionID) override;
	virtual void OnClientLeave(SESSION_ID SessionID) override;
	virtual void OnRecv(SESSION_ID SessionID, CPacket* packet) override;
	virtual void OnError(int errorcode, WCHAR* log) override;

private:
	void InsertSessionID(u_int64 sessionNo);
	void DeleteSessionID(u_int64 sessionNo);
	void CompletePacket(SESSION_ID SessionID, CPacket* packet);
	void EchoProc(SESSION_ID sessionID, CPacket* packet);
	void LockSet();
	void UnlockSet();

private:
	std::unordered_set<u_int64> mSessionSet;
	SRWLOCK mSetLock;
};