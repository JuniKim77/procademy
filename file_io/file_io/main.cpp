#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main()
{
	FILE* fout;
	fopen_s(&fout, "output.txt", "w"); // fout �������� �ּ� ��ü�� �ٲٱ� ���� ���� ������

	if (fout == NULL)
		return -1;

	fprintf(fout, "%d %d", 100, 200); // ���ڰ� ���ڿ��� ��ȯ�Ǿ� �����

	fputc(97, fout); // ���ڰ� �ƽ�Ű ������ �νĵǾ� �����

	char buffer[32] = "Hojun Kim\n";

	int count1 = fwrite(buffer, strlen(buffer), 1, fout); // ����� ũ��, ��� ����. �׷��� ��ȯ ���� ������ ����� ������ ����. ���⼭�� 1

	int count2 = fwrite(buffer, 1, strlen(buffer), fout); // ���� ������ ��ȯ���� 10�� ��.

	printf("%d %d\n", count1, count2);

	fseek(fout, 0, SEEK_SET); // ù��° ����Ʈ�� ���ư�

	fwrite(buffer, strlen(buffer), 1, fout);

	fseek(fout, -1, SEEK_END);  // ���� �ڷκ��� �󸶸�ŭ

	fseek(fout, 0, SEEK_CUR); // ���� ����

	fwrite(buffer, strlen(buffer), 1, fout);

	fseek(fout, 0, SEEK_SET);

	int begin = ftell(fout); // ���� Ŀ���� ��ġ�� ��ȯ

	fseek(fout, 0, SEEK_END);

	int end = ftell(fout);

	printf("File Size: %d\n", end - begin);

	sprintf_s(buffer, "Procademy");

	rewind(fout); // ó������ ���ư� fflush�� �о��ִ� �ǹ��̰�, ���̻� �۵��� �ȵ�

	fwrite(buffer, strlen(buffer), 1, fout);

	fclose(fout);

	return 0;
}