#pragma once
#include <stdio.h>

class BaseObject
{
public:
	virtual bool Update() = 0;
	virtual void Render() = 0;

protected:
	enum
	{
		MAX_RANGE = 75
	};
	int mX = 0;
};