#include <wtypes.h>

//------------------------------------------------------
//  패킷헤더
//
//	| PacketCode | PayloadSize | * Payload * |
//		1Byte         2Byte       Size Byte     
//
//------------------------------------------------------

#pragma pack(push, 1)   

struct st_NETWORK_HEADER
{
	BYTE	byCode;
	WORD	wPayloadSize;
};

#pragma pack(pop)