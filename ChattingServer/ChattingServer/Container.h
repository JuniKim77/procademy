#pragma once
#include <unordered_map>
#include <list>
#include <wtypes.h>

class Session;
struct User;
struct Room;

void InsertSessionData(Session* session);
void DeleteSessionData(DWORD sessionNo);
void UpdateSessionData(DWORD sessionNo, Session* session);
void InsertUserData(User* user);
void DeleteUserData(DWORD userNo);
void UpdateUserData(DWORD userNo, User* user);
