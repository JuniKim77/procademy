#pragma once

#include <wtypes.h>
#include "Sector.h"

struct stTrackData
{
	DWORD mAction;
	COORD mServer;
	COORD mClient;
	BYTE mMoveDirection;
};

struct User
{
	void UpdateTrack()
	{
		mExExTrack.mAction = mExTrack.mAction;
		mExExTrack.mMoveDirection = mExTrack.mMoveDirection;
		mExExTrack.mServer.X = mExTrack.mServer.X;
		mExExTrack.mServer.Y = mExTrack.mServer.Y;
		mExExTrack.mClient.X = mExTrack.mClient.Y;
		mExExTrack.mClient.X = mExTrack.mClient.Y;
	}

	DWORD userNo;
	DWORD sessionNo;

	DWORD action;
	BYTE direction;
	BYTE moveDirection;

	short x;
	short y;

	st_Sector curSector;
	st_Sector oldSector;

	stTrackData mExTrack;
	stTrackData mExExTrack;

	char hp;
};