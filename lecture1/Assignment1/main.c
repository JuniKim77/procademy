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

	printf("%d �� ���̳ʸ�: ", num1);

	printBits(num1);
	fputc('\n', stdout);

	unsigned short num2 = 0;

	for(;;)
	{
		unsigned int bit = 0;
		fputs("����� 0 �Է�\n", stdout);
		fputs("��Ʈ��ġ: ", stdout);
		scanf_s("%d", &bit);

		if (bit == 0) {
			fputs("�����մϴ�.\n\n", stdout);
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
				printf("%d �� Bit : ", i);
				if ((num2 & (1 << i - 1)) == 0) {
					fputs("OFF\n",stdout);
				}
				else {
					fputs("ON\n", stdout);
				}
			}
		}
		else {
			fputs("��Ʈ ������ �ʰ��Ͽ����ϴ�.\n", stdout);
		}
	}
	
	unsigned int num3 = 0;
	while (1)
	{
		int byte = 0;
		fputs("��ġ (1~4) : ", stdout);
		scanf_s("%d", &byte);
		int value = 0;
		fputs("�� [0~255] : ", stdout);
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
			printf("%d ��° ����Ʈ �� : ", i);
			int temp = num3 & (fullBitNumber << (i - 1) * 8);

			temp >>= ((i - 1) * 8);

			printf("%d \n", temp);
		}

		printf("\n��ü 4����Ʈ �� : %#010x\n\n", num3);
	}

	return 0;
}