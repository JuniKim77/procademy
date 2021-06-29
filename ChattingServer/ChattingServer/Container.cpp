#include "Container.h"
#include "User.h"

using namespace std;

extern DWORD g_SessionNo;
extern DWORD g_RoomNo;
extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

void InsertSessionData(DWORD sessionNo, Session* session)
{
	g_sessions[sessionNo] = session;
}

void DeleteSessionData(DWORD sessionNo)
{
	auto iter = g_sessions.find(sessionNo);

	if (iter == g_sessions.end())
	{
		return;
	}

	delete g_sessions[sessionNo];
	g_sessions[sessionNo] = nullptr;
	g_sessions.erase(iter);
}

void UpdateSessionData(DWORD sessionNo, Session* session)
{
	g_sessions[sessionNo] = session;
}

void InsertUserData(DWORD userNo, User* user)
{
	g_users[userNo] = user;
}

void DeleteUserData(DWORD userNo)
{
	auto iter = g_users.find(userNo);

	if (iter == g_users.end())
	{
		return;
	}

	delete g_users[userNo];
	g_users[userNo] = nullptr;
	g_users.erase(iter);
}

void UpdateUserData(DWORD userNo, User* user)
{
	g_users[userNo] = user;
}

void InsertRoomData(DWORD roomNo, Room* room)
{
	g_rooms[roomNo] = room;
}

void DeleteRoomData(DWORD roomNo)
{
	auto iter = g_rooms.find(roomNo);

	if (iter == g_rooms.end())
	{
		return;
	}

	delete g_rooms[roomNo];
	g_rooms[roomNo] = nullptr;
	g_rooms.erase(iter);
}

void UpdateRoomData(DWORD roomNo, Room* room)
{
	g_rooms[roomNo] = room;
}

void InsertUserToRoom(DWORD userNo, Room* room)
{
	room->mUserList.push_back(userNo);
	g_users[userNo]->mRoomNo = room->mRoomNo;
}

User* FindUser(DWORD userNo)
{
	auto iter = g_users.find(userNo);

	if (iter == g_users.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}

Room* FindRoom(DWORD roomNo)
{
	auto iter = g_rooms.find(roomNo);

	if (iter == g_rooms.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}

Session* FindSession(DWORD sessionNo)
{
	auto iter = g_sessions.find(sessionNo);

	if (iter == g_sessions.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}