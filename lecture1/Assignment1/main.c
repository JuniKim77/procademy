#include <stdio.h>

void printBits(unsigned char num)
{
	char buf[9] = { 0, };
	for (int i = 7; i >= 0; --i)
	{
		if ((num & (1 << i)) > 0) {
			buf[7 - i] = '1';
		}
		else {
			buf[7 - i] = '0';
		}
	}
	buf[8] = '\0';

	fputs(buf, stdout);
}

int main()
{
	unsigned char num1 = 40;

	printf("%d 의 바이너리: ", num1);

	printBits(num1);
	fputc('\n', stdout);

	unsigned short num2 = 0;

	for(;;)
	{
		unsigned int bit = 0;
		fputs("종료는 0 입력\n", stdout);
		fputs("비트위치: ", stdout);
		scanf_s("%d", &bit);

		if (bit == 0) {
			fputs("종료합니다.\n\n", stdout);
			break;
		}

		if (bit >= 1 && bit <= 16) {
			int isOn;
			fputs("OFF/ON [0,1] : ", stdout);
			scanf_s("%d", &isOn);

			if (isOn == 0) {
				num2 &= ~(1 << bit - 1);
			}
			else {
				num2 |= (1 << bit - 1);
			}
			
			for (int i = 16; i >= 1; --i)
			{
				printf("%d 번 Bit : ", i);
				if ((num2 & (1 << i - 1)) == 0) {
					fputs("OFF\n",stdout);
				}
				else {
					fputs("ON\n", stdout);
				}
			}
		}
		else {
			fputs("비트 범위를 초과하였습니다.\n", stdout);
		}
	}
	
	unsigned int num3 = 0;
	while (1)
	{
		int byte = 0;
		fputs("위치 (1~4) : ", stdout);
		scanf_s("%d", &byte);
		int value = 0;
		fputs("값 [0~255] : ", stdout);
		scanf_s("%d", &value);

		value = value % 256;
		unsigned int fullBitNumber = 255;

		num3 &= ~((fullBitNumber) << (byte - 1) * 8);

		for (int i = 0; i < 8; ++i)
		{
			num3 |= ((value & (1 << i)) << (byte - 1) * 8);
		}

		for (int i = 1; i < 5; ++i)
		{
			printf("%d 번째 바이트 값 : ", i);
			int temp = num3 & (fullBitNumber << (i - 1) * 8);

			temp >>= ((i - 1) * 8);

			printf("%d \n", temp);
		}

		printf("\n전체 4바이트 값 : %#010x\n\n", num3);
	}

	return 0;
}