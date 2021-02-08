#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

struct MyColor
{
	BYTE B;
	BYTE G;
	BYTE R;
	BYTE A;
};

struct BMP
{
	tagBITMAPFILEHEADER fileHeader;
	tagBITMAPINFOHEADER infoHeader;
	BYTE* image = NULL;

	~BMP()
	{
		if (image != NULL)
		{
			free(image);
		}
	}

	BMP(const BMP* other)
		: fileHeader(other->fileHeader)
		, infoHeader(other->infoHeader)
		, image(nullptr)
	{
		size_t size = infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8;
		image = (BYTE*)malloc(size);
		memcpy_s(image, size, other->image, size);
	}

	BMP(const char* file_name)
	{
		FILE* fin = fopen(file_name, "rb");

		if (fin == NULL) return;

		fread(&fileHeader, sizeof(tagBITMAPFILEHEADER), 1, fin);
		fread(&infoHeader, sizeof(tagBITMAPINFOHEADER), 1, fin);

		size_t size = infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8;

		image = (BYTE*)malloc(size);

		if (image != NULL)
			fread(image, size, 1, fin);

		fclose(fin);
	}

	void print_info() {
		printf("File Size: %d\n", fileHeader.bfSize);
		printf("Image Size: %d\n", infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8);
		printf("Image Height: %d, Image Width: %d\n", infoHeader.biHeight, infoHeader.biWidth);
	}

	void copy_image(const BMP* other) {
		memcpy_s(&fileHeader, sizeof(tagBITMAPFILEHEADER), &other->fileHeader, sizeof(tagBITMAPFILEHEADER));
		memcpy_s(&infoHeader, sizeof(tagBITMAPINFOHEADER), &other->infoHeader, sizeof(tagBITMAPINFOHEADER));
	}

	void write_file(const char* file_name)
	{
		FILE* fout = fopen(file_name, "wb");

		fwrite(&fileHeader, sizeof(tagBITMAPFILEHEADER), 1, fout);
		fwrite(&infoHeader, sizeof(tagBITMAPINFOHEADER), 1, fout);
		fwrite(image, infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8, 1, fout);

		fclose(fout);
	}

	void alpha_merge(const BMP* left, const BMP* right, const char* file_name)
	{
		FILE* fout = fopen(file_name, "wb");

		fwrite(&fileHeader, sizeof(tagBITMAPFILEHEADER), 1, fout);
		fwrite(&infoHeader, sizeof(tagBITMAPINFOHEADER), 1, fout);

		MyColor* spot1 = (MyColor *)left->image;
		MyColor* spot2 = (MyColor *)right->image;

		for (int i = 0; i < infoHeader.biHeight; ++i)
		{
			for (int j = 0; j < infoHeader.biWidth; ++j)
			{
				MyColor color;

				color.B = (spot1->B + spot2->B) / 2;
				color.G = (spot1->G + spot2->G) / 2;
				color.R = (spot1->R + spot2->R) / 2;
				color.A = (spot1->A + spot2->A) / 2;

				fwrite(&color, sizeof(MyColor), 1, fout);

				spot1++;
				spot2++;
			}
		}

		fclose(fout);
	}

	void alpha_merge2(const BMP* left, const BMP* right, const char* file_name)
	{
		FILE* fout = fopen(file_name, "wb");

		fwrite(&fileHeader, sizeof(tagBITMAPFILEHEADER), 1, fout);
		fwrite(&infoHeader, sizeof(tagBITMAPINFOHEADER), 1, fout);

		DWORD* spot1 = (DWORD*)left->image;
		DWORD* spot2 = (DWORD*)right->image;

		for (int i = 0; i < infoHeader.biHeight; ++i)
		{
			for (int j = 0; j < infoHeader.biWidth; ++j)
			{
				DWORD sum = ((*spot1 >> 1) & 0x7f7f7f7f) + ((*spot2 >> 1) & 0x7f7f7f7f);

				fwrite(&sum, sizeof(DWORD), 1, fout);
				spot1++;
				spot2++;
			}
		}

		fclose(fout);
	}

	void alpha_merge_ratio(const BMP* left, const BMP* right, double left_ratio, double right_ratio, const char* file_name)
	{
		FILE* fout = fopen(file_name, "wb");

		fwrite(&fileHeader, sizeof(tagBITMAPFILEHEADER), 1, fout);
		fwrite(&infoHeader, sizeof(tagBITMAPINFOHEADER), 1, fout);

		MyColor* spot1 = (MyColor*)left->image;
		MyColor* spot2 = (MyColor*)right->image;

		for (int i = 0; i < infoHeader.biHeight; ++i)
		{
			for (int j = 0; j < infoHeader.biWidth; ++j)
			{
				MyColor color;

				color.B = spot1->B * left_ratio + spot2->B * right_ratio;
				color.G = spot1->G * left_ratio + spot2->G * right_ratio;
				color.R = spot1->R * left_ratio + spot2->R * right_ratio;
				color.A = spot1->A * left_ratio + spot2->A * right_ratio;

				fwrite(&color, sizeof(MyColor), 1, fout);

				spot1++;
				spot2++;
			}
		}

		fclose(fout);
	}
};

int main()
{
	BMP bmp1("sample.bmp");

	BMP bmp2("sample2.bmp");

	BMP bmp4("pngwing.bmp");

	bmp1.print_info();
	bmp2.print_info();

	BMP bmp3(&bmp1);

	bmp3.print_info();

	bmp3.write_file("output.bmp");
	bmp3.alpha_merge(&bmp1, &bmp2, "output2.bmp");
	bmp3.alpha_merge2(&bmp1, &bmp2, "output3.bmp");
	bmp3.alpha_merge_ratio(&bmp1, &bmp2, 0.85, 0.15, "output4.bmp");

	bmp4.print_info();
	bmp4.write_file("output5.bmp");
}