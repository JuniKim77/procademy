#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main()
{
	FILE* fout;
	fopen_s(&fout, "output.txt", "w"); // fout 포인터의 주소 자체를 바꾸기 위해 이중 포인터

	if (fout == NULL)
		return -1;

	fprintf(fout, "%d %d", 100, 200); // 숫자가 문자열로 변환되어 저장됨

	fputc(97, fout); // 숫자가 아스키 값으로 인식되어 저장됨

	char buffer[32] = "Hojun Kim\n";

	int count1 = fwrite(buffer, strlen(buffer), 1, fout); // 덩어리의 크기, 덩어리 개수. 그러나 반환 값은 보내진 덩어리의 개수로 나옴. 여기서는 1

	int count2 = fwrite(buffer, 1, strlen(buffer), fout); // 위와 같지만 반환값이 10이 됨.

	printf("%d %d\n", count1, count2);

	fseek(fout, 0, SEEK_SET); // 첫번째 바이트로 돌아감

	fwrite(buffer, strlen(buffer), 1, fout);

	fseek(fout, -1, SEEK_END);  // 가장 뒤로부터 얼마만큼

	fseek(fout, 0, SEEK_CUR); // 현재 지점

	fwrite(buffer, strlen(buffer), 1, fout);

	fseek(fout, 0, SEEK_SET);

	int begin = ftell(fout); // 현재 커서의 위치를 반환

	fseek(fout, 0, SEEK_END);

	int end = ftell(fout);

	printf("File Size: %d\n", end - begin);

	sprintf_s(buffer, "Procademy");

	rewind(fout); // 처음으로 돌아감 fflush는 밀어주는 의미이고, 더이상 작동이 안됨

	fwrite(buffer, strlen(buffer), 1, fout);

	fclose(fout);

	return 0;
}