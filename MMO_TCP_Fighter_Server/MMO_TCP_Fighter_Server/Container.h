#pragma once
#include <unordered_map>
#include <list>
#include <wtypes.h>

class Session;
struct User;

void InsertSessionData(DWORD sessionNo, Session* session);
void DeleteSessionData(DWORD sessionNo);
void UpdateSessionData(DWORD sessionNo, Session* session);
void InsertUserData(DWORD userNo, User* user);
void DeleteUserData(DWORD userNo);
void UpdateUserData(DWORD userNo, User* user);
User* FindUser(DWORD userNo);
Session* FindSession(DWORD sessionNo);
