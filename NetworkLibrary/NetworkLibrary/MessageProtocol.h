#include <wtypes.h>

//------------------------------------------------------
//  �޼��� ���
//
//	| MessageType |
//		 2Byte
//
//------------------------------------------------------

#pragma pack(push, 1)   

struct st_MESSAGE_HEADER
{
	WORD	wPayloadSize;
};

#pragma pack(pop)

#define dfMESSAGE_ECHO			1
/// ------------------------
/// echo message
/// ------------------------