#pragma once
#include <unordered_map>
#include <list>
#include <wtypes.h>

class Session;
struct User;
struct Room;

void InsertSessionData(DWORD sessionNo, Session* session);
void DeleteSessionData(DWORD sessionNo);
void UpdateSessionData(DWORD sessionNo, Session* session);
void InsertUserData(DWORD userNo, User* user);
void DeleteUserData(DWORD userNo);
void UpdateUserData(DWORD userNo, User* user);
void InsertRoomData(DWORD roomNo, Room* room);
void DeleteRoomData(DWORD roomNo);
void UpdateRoomData(DWORD roomNo, Room* room);
void InsertUserToRoom(DWORD userNo, Room* room);
User* FindUser(DWORD userNo);
Room* FindRoom(DWORD roomNo);
Session* FindSession(DWORD sessionNo);
