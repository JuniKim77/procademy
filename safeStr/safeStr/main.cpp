#include <strsafe.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <locale.h>

int main()
{
	setlocale(LC_ALL, "");

	WCHAR source1[32] = L"안녕하세요. 프로카데미 19기 김호준입니다.";
	WCHAR dest1[16];
	WCHAR dest2[16];

	wprintf_s(L"Buffer Size: %d, : %s\n", _countof(source1), source1);

	HRESULT hr = StringCchCopy(dest1, _countof(dest1), source1); // cch : count char 문자열 몇개인지 따라서 _countof
	StringCbCopy(dest2, sizeof(dest2), source1); // cb : count byte 버퍼 사이즈 따라서 sizeof

	if (hr == S_OK) {
		wprintf_s(L"성공!\n");
	}
	else if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
		wprintf_s(L"Dest 버퍼가 충분치 않은 경우\n");
	}

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest1), dest1);
	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest2), dest2);

	hr = StringCchCopyN(dest1, _countof(dest1), source1, _countof(dest1) - 1); // Source에서 필요한 만큼을 지정할 수 있음

	if (hr == S_OK) {
		wprintf_s(L"성공!\n");
	}
	else if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
		wprintf_s(L"Dest 버퍼가 충분치 않은 경우\n");
	}

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest1), dest1);

	WCHAR dest3[16] = L"Concat: ";

	StringCchCat(dest3, _countof(dest3), source1);

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest3), dest3); // 이어 붙이기 \0이 필요하다.

	WCHAR dummy1[] = L"Hello";
	WCHAR dummy2[] = L"World";

	StringCchPrintf(dest1, _countof(dest1), L"%s + %s\n", dummy1, dummy2);

	wprintf_s(dest1);

	size_t len;
	StringCchLength(dest1, MAX_PATH, &len);

	wprintf_s(L"%d\n", len); // dest의 버퍼크기 만큼만 길이를 체크

	return 0;
}