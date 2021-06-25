#include "Container.h"

using namespace std;

extern DWORD g_SessionNo = 1;
extern DWORD g_RoomNo = 1;
extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

void InsertSessionData(Session* session)
{
	g_sessions[g_SessionNo] = session;
}

void DeleteSessionData(DWORD sessionNo)
{
	delete g_sessions[sessionNo];
	g_sessions[sessionNo] = nullptr;
}

void UpdateSessionData(DWORD sessionNo, Session* session)
{
	g_sessions[sessionNo] = session;
}

void InsertUserData(User* user)
{
	g_users[g_SessionNo] = user;
}

void DeleteUserData(DWORD userNo)
{
	delete g_users[userNo];
	g_users[userNo] = nullptr;
}

void UpdateUserData(DWORD userNo, User* user)
{
	g_users[userNo] = user;
}
