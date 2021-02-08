#include <stdio.h>
#include <stdlib.h>
#include <cerrno>
#include <string.h>
#include <Windows.h>

struct File_Header
{
	short code;
	short file_num;
	size_t file_size;
};

struct Element 
{
	struct Element_Header
	{
		size_t size;
		char name[32];
		size_t offset;
	} header;
	char* element;

	~Element()
	{
		if (element != NULL)
		{
			free(element);
		}
	}
};


enum TYPE
{
	PACKKING = 1,
	UNPACKKING,
	UPDATE
};

void mainMenu();
void packMenu();
bool readFile(char* file_name, BYTE** data);
bool updateFile(const char* file_name, const char* update_file_name, BYTE* data);
void updateMenu();
void unpackMenu();
bool makeElementHeader();
void packFiles();
bool unpackFiles(BYTE* data);

File_Header g_fileHeader;
Element* g_elementHeaders;

int main()
{
	mainMenu();

	return 0;
}

void mainMenu()
{
	int select;
	printf("1. Packking\n");
	printf("2. UnPacking\n");
	printf("3. Update\n");
	printf("Select: ");

	scanf_s("%d", &select);
	fgetc(stdin);

	switch (select)
	{
	case PACKKING:
		packMenu();
		break;
	case UNPACKKING:
		unpackMenu();
		break;
	case UPDATE:
		updateMenu();
		break;
	default:
		break;
	}
}

void packMenu()
{
	g_fileHeader.code = 100;
	printf("Enter File Num: ");
	scanf_s("%hd", &g_fileHeader.file_num);
	fgetc(stdin);

	g_elementHeaders = (Element* )malloc(sizeof(Element) * g_fileHeader.file_num);
	if (g_elementHeaders == NULL)
		return;

	if (makeElementHeader() == false)
	{
		return;
	}

	packFiles();

	free(g_elementHeaders);
}

bool readFile(char* file_name, BYTE** data)
{
	FILE* fin;
	char name[32];

	printf("Enter packed file name (Max 31): ");
	fgets(name, sizeof(name), stdin);
	name[strlen(name) - 1] = '\0';

	sprintf_s(name, "%s%s", name, ".dat");
	memcpy(file_name, name, sizeof(name));

	fopen_s(&fin, name, "rb");
	fseek(fin, 0, SEEK_END);
	size_t fileSize = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	*data = (BYTE*)malloc(fileSize);

	fread(*data, fileSize, 1, fin);

	fclose(fin);

	return true;
}

bool updateFile(const char* file_name, const char* update_file_name, BYTE* data)
{
	File_Header* fileHeader = (File_Header*)data;

	Element::Element::Element_Header* pHeaders = (Element::Element::Element_Header*)(fileHeader + 1);

	for (int i = 0; i < fileHeader->file_num; ++i)
	{
		if (strcmp(pHeaders->name, update_file_name) == 0)
		{
			FILE* fin;

			fopen_s(&fin, update_file_name, "rb");

			fseek(fin, 0, SEEK_END);
			size_t updated_size = ftell(fin);
			fseek(fin, 0, SEEK_SET);

			if (pHeaders->size < updated_size)
			{
				pHeaders->offset = fileHeader->file_size;
			}

			BYTE* update_data = (BYTE*)malloc(updated_size);
			if (update_data == NULL)
			{
				fclose(fin);
				return false;
			}

			fread(update_data, updated_size, 1, fin);

			FILE* fout;
			fopen_s(&fout, file_name, "wb");

			if (pHeaders->size < updated_size)
			{
				pHeaders->offset = fileHeader->file_size;
				pHeaders->size = updated_size;
				fwrite(data, fileHeader->file_size, 1, fout);
				fwrite(update_data, updated_size, 1, fout);
			}
			else
			{
				pHeaders->size = updated_size;
				memcpy(data + pHeaders->offset, update_data, updated_size);
				fwrite(data, fileHeader->file_size, 1, fout);
			}

			fclose(fout);

			fclose(fin);

			return true;
		}
		pHeaders++;
	}

	printf("The update file is not found\n");

	return false;
}

void updateMenu()
{
	BYTE* buffer;
	char file_name[32];

	readFile(file_name , &buffer);

	// Code validation
	File_Header* fileHeader = (File_Header*)buffer;
	if (fileHeader->code != 100)
	{
		printf("Code is not matched\n");
		goto EXIT;
	}

	char update_file[32];

	printf("Enter a file name to update(Max 31): ");
	fgets(update_file, sizeof(update_file), stdin);
	update_file[strlen(update_file) - 1] = '\0';

	updateFile(file_name, update_file, buffer);

EXIT:
	free(buffer);
}

void unpackMenu()
{
	BYTE* buffer;
	char file_name[32];

	readFile(file_name, &buffer);

	// Code validation
	File_Header* fileHeader = (File_Header*)buffer;
	if (fileHeader->code != 100)
	{
		printf("Code is not matched\n");
		goto EXIT;
	}

	if (!unpackFiles(buffer))
	{
		printf("Fail: Unpack\n");
	}

EXIT:
	free(buffer);
}

bool makeElementHeader()
{
	Element* pHeaders = g_elementHeaders;

	for (short i = 0; i < g_fileHeader.file_num; ++i)
	{
		printf("Enter File Name (Max Length 31): ");
		fgets(pHeaders->header.name, sizeof(pHeaders->header.name), stdin);
		pHeaders->header.name[strlen(pHeaders->header.name) -1 ] = '\0';

		FILE* fin;

		errno_t err = fopen_s(&fin, pHeaders->header.name, "rb");

		if (err == EINVAL)
			return false;

		fseek(fin, 0, SEEK_END);
		pHeaders->header.size = ftell(fin);
		pHeaders->element = (char*)malloc(sizeof(char) * pHeaders->header.size);
		fseek(fin, 0, SEEK_SET);

		fread(pHeaders->element, pHeaders->header.size, 1, fin);
		
		fclose(fin);

		pHeaders++;
	}

	return true;
}

void packFiles()
{
	int size = sizeof(File_Header);
	size += sizeof(Element::Element_Header) * g_fileHeader.file_num;

	Element* pElement = g_elementHeaders;

	for (int i = 0; i < g_fileHeader.file_num; ++i)
	{
		pElement->header.offset = size;
		size += pElement->header.size;
		pElement++;
	}

	g_fileHeader.file_size = size;

	char buffer[32];
	printf("Enter packed file name(Max 31): ");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strlen(buffer) - 1] = '\0';

	sprintf_s(buffer, "%s%s", buffer, ".dat");

	FILE* fout;
	fopen_s(&fout, buffer, "wb");

	fwrite(&g_fileHeader, sizeof(File_Header), 1, fout);

	Element* pHeader = g_elementHeaders;
	
	for (int i = 0; i < g_fileHeader.file_num; ++i)
	{
		fwrite(&pHeader->header, sizeof(Element::Element_Header), 1, fout);
		pHeader++;
	}

	pHeader = g_elementHeaders;

	for (int i = 0; i < g_fileHeader.file_num; ++i)
	{
		fwrite(pHeader->element, pHeader->header.size, 1, fout);
		pHeader++;
	}

	fclose(fout);
}

bool unpackFiles(BYTE* data)
{
	File_Header* fileHeader = (File_Header*)data;

	Element::Element_Header* headers = (Element::Element_Header*)(fileHeader + 1);

	for (int i = 0; i < fileHeader->file_num; ++i)
	{
		FILE* fout;
		fopen_s(&fout, headers->name, "wb");
		if (fout == NULL)
			return false;

		size_t ret = fwrite(data + headers->offset, headers->size, 1, fout);

		if (ret != 1)
		{
			printf("Error in fwrite\n");
			fclose(fout);
			return false;
		}

		fclose(fout);

		headers++;
	}

	return true;
}
