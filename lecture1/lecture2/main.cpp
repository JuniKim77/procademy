#include <iostream>
#include <string>
#include <locale>

using namespace std;

int myStrLen(const char* s)
{
	int count = 0;
	const char* pS = s;

	while (*pS++ != '\0')
	{
		count++;
	}

	return count;
}

bool myStrCpy_s(char* dest, size_t size, const char* src)
{
	char* pDest = dest;
	const char* pSrc = src;

	while (*pSrc != '\0' && size-- > 0)
	{
		*pDest++ = *pSrc++;
	}

	*pDest = '\0';

	return true;
}

int myStrCmp(const char* left, const char* right)
{
	while (*left != '\0' && *left == *right)
	{
		left++;
		right++;
	}

	return *left - *right;
}

bool myStrCat_s(char* dest, size_t size, const char* src)
{
	char* pDest = dest;

	while (*pDest != '\0' && size > 0) {
		pDest++;
		size--;
	}

	if (size == 0) return false;

	while (*src != '\0' && size-- > 0) {
		*pDest++ = *src++;
	}

	*pDest = '\0';

	return true;
}

char wordList[10][2][32] = {
	{"i",		"나"},
	{"you",		"너"},
	{"we",		"우리"},
	{"go",		"가다"},
	{"market",	"가게"},
	{"meet",	"만나다"},
	{"friend",	"친구"},
	{"and",		"그리고"},
	{"boy",		"소년"},
	{"girl",	"소녀"}

};

int searchDictionary(const char* word)
{
	char buf[32];
	myStrCpy_s(buf, sizeof(buf), word);

	_strlwr_s(buf, sizeof(buf));

	for (int i = 0; i < 10; ++i)
	{
		if (myStrCmp(buf, wordList[i][0]) == 0) {
			return i;
		}
	}

	return -1;
}

int myTranslator(char* dest, const char* str)
{
	char buf[256];
	char* pDest = dest;
	*dest = '\0';
	myStrCpy_s(buf, sizeof(buf), str);

	char* pBuf = buf;
	char* prev = buf;
	int count = 0;

	if (*pBuf == '\0') return count;

	while (1)
	{
		if (*pBuf == ' ' || *pBuf == '\0')
		{
			char temp = *pBuf;
			*pBuf = '\0';

			myStrCat_s(dest, 256, prev);

			int idx = searchDictionary(prev);

			if (idx != -1) {
				char temp[256];
				sprintf_s(temp, " [%s] ", wordList[idx][1]);
				myStrCat_s(pDest, sizeof(buf), temp);
				count++;
			}
			else {
				myStrCat_s(pDest, sizeof(buf), " ");
			}

			if (temp == '\0') return count;

			prev = ++pBuf;
			continue;
		}
		pBuf++;
	}

	return count;
}

int main()
{
	char buf[32] = "Hojun";

	printf("strlen: %d, myStrLen: %d\n", strlen(buf), myStrLen(buf));

	char dest[32];
	strcpy_s(dest, sizeof(dest), buf);
	printf("strcpy_s: %s, len: %d\n", dest, strlen(dest));

	char dest2[32];
	myStrCpy_s(dest2, sizeof(dest2), buf);
	printf("myStrCpy_s: %s, len: %d\n", dest2, myStrLen(dest2));

	char cmp1[32] = "abc";
	char cmp2[32] = "abc";
	char cmp3[32] = "adc";

	printf("%d\n", myStrCmp(cmp1, cmp2));
	printf("%d\n", myStrCmp(cmp1, cmp3));

	myStrCat_s(cmp1, sizeof(cmp1), buf);

	printf("myStrCat: %s\n", cmp1);

	char translated[256];
	char input[128];

	gets_s(input, sizeof(input));

	int count = myTranslator(translated, input);

	printf("%s\nTranslated word count: %d\n", translated, count);

	count = myTranslator(translated, "I meet my Friend at the market");

	printf("%s\nTranslated word count: %d\n", translated, count);

	return 0;
}