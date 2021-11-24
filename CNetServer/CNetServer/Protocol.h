#include <wtypes.h>

#define dfNETWORK_CODE (0x89)

//------------------------------------------------------
//  ��Ŷ���
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