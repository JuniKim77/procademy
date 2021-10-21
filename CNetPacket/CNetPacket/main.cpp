#include "CNetPacket.h"
#include <wtypes.h>
#include <iostream>

using namespace std;

int main()
{
	procademy::CNetPacket* packet = procademy::CNetPacket::AllocAddRef();
	
	unsigned char byValue = 100;
	char chValue = 50;
	short shValue = 0xAA;
	unsigned short ushValue = 0xBB;
	int iValue = 500;
	long lValue = 600;
	float fValue = 10.0f;
	__int64 iiValue = 1500;
	double dValue = 150.0;

	cout << "변경 전 : " << byValue << ", " << chValue << ", " << shValue << ", " << ushValue << ", " << iValue << ", " << endl;
	cout << lValue << ", " << fValue << ", " << iiValue << ", " << dValue << ", " << endl;

	*packet << byValue << chValue << shValue << ushValue << iValue << lValue << fValue << iiValue << dValue;

	*packet >> byValue >> chValue >> shValue >> ushValue >> iValue >> lValue >> fValue >> iiValue >> dValue;
	
	cout << "변경 후 : " << byValue << ", " << chValue << ", " << shValue << ", " << ushValue << ", " << iValue << ", " << endl;
	cout << lValue << ", " << fValue << ", " << iiValue << ", " << dValue << ", " << endl;

	char msg[] = "Hojun Kim, Procademy";
	int size = strlen(msg);

	packet->PutData(msg, size);

	packet->GetData(msg, size);

	cout << msg << endl;

	WCHAR wMsg[] = L"Hojun Kim, W version";
	int wSize = wcslen(wMsg);

	packet->PutData(wMsg, wSize);

	packet->GetData((char*)wMsg, sizeof(wMsg));

	wcout << wMsg << endl;

	*packet << L"Test W operator";

	WCHAR wMsg2[100] = { 0, };

	packet->GetData(wMsg2, 15);

	wcout << wMsg2 << endl;

	packet->SubRef();

	procademy::CNetPacket* code = procademy::CNetPacket::AllocAddRef();

	char orin[] = "aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn";

	*code << orin;
	*code << '\0';
	code->SetHeader(false);
	code->Encode();
	code->Decode();

	code->SubRef();
	
	return 0;
}