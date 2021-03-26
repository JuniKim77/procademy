#pragma once

class ScreenDib
{
public:
	ScreenDib(int width, int height, int colorBit);
	virtual ~ScreenDib();

protected:
	void CreateDibBuffer(int width, int height, int colorBit);
	void ReleaseDibBuffer()

public:

protected:

};