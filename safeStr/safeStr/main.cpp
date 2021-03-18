#include <strsafe.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <locale.h>

int main()
{
	setlocale(LC_ALL, "");

	WCHAR source1[32] = L"�ȳ��ϼ���. ����ī���� 19�� ��ȣ���Դϴ�.";
	WCHAR dest1[16];
	WCHAR dest2[16];

	wprintf_s(L"Buffer Size: %d, : %s\n", _countof(source1), source1);

	HRESULT hr = StringCchCopy(dest1, _countof(dest1), source1); // cch : count char ���ڿ� ����� ���� _countof
	StringCbCopy(dest2, sizeof(dest2), source1); // cb : count byte ���� ������ ���� sizeof

	if (hr == S_OK) {
		wprintf_s(L"����!\n");
	}
	else if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
		wprintf_s(L"Dest ���۰� ���ġ ���� ���\n");
	}

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest1), dest1);
	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest2), dest2);

	hr = StringCchCopyN(dest1, _countof(dest1), source1, _countof(dest1) - 1); // Source���� �ʿ��� ��ŭ�� ������ �� ����

	if (hr == S_OK) {
		wprintf_s(L"����!\n");
	}
	else if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
		wprintf_s(L"Dest ���۰� ���ġ ���� ���\n");
	}

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest1), dest1);

	WCHAR dest3[16] = L"Concat: ";

	StringCchCat(dest3, _countof(dest3), source1);

	wprintf_s(L"Buffer Size: %d : %s\n", _countof(dest3), dest3); // �̾� ���̱� \0�� �ʿ��ϴ�.

	WCHAR dummy1[] = L"Hello";
	WCHAR dummy2[] = L"World";

	StringCchPrintf(dest1, _countof(dest1), L"%s + %s\n", dummy1, dummy2);

	wprintf_s(dest1);

	size_t len;
	StringCchLength(dest1, MAX_PATH, &len);

	wprintf_s(L"%d\n", len); // dest�� ����ũ�� ��ŭ�� ���̸� üũ

	return 0;
}