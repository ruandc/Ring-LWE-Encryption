#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

void read_binary_file(char *name, char * buffer)
{
	FILE *file;
	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	//Do what ever with buffer

	free(buffer);
}

int read_text_file(char * file_name, char * buffer)
{
	FILE *ptr_file;

	ptr_file = fopen(file_name,"r");
	if (!ptr_file)
		return 1;

	if (fgets(buffer,1000, ptr_file)==NULL) //Try to read 1000 characters in one go
		return 1;

	fclose(ptr_file);
	return 0;
}

int write_hex_file(char * file_name, uint32_t * output, int len)
{
	FILE *ptr_file;

	ptr_file = fopen(file_name,"w");
	if (!ptr_file)
		return 1;

	int i,j;
	for (i=0; i<len; i++)
	{
		//for (j=0; j<4; j++)
		//{
		//	fprintf(ptr_file, "%x", ((output[i]>>(8*j))&0xff));
		//}
		fprintf(ptr_file, "%.8x", output[i]);
    }

	fclose(ptr_file);
	return 0;
}

int read_hex_file(char * file_name, uint32_t * out, int max_size)
{
	FILE *ptr_file;

	ptr_file = fopen(file_name,"r");
	if (!ptr_file)
		return 1;

	int i=0;
	char buffer[9]; //Apparently zero is also stored at end of buffer.
					//Therefore read '9' characters for each 32-bit element
	while ((i<max_size) && (fgets(buffer,9,ptr_file)!=NULL))
	{
		out[i++] = strtol(buffer,NULL,16);
	}

	fclose(ptr_file);
	return 0;
}
