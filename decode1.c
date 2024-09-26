#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"
/* Function Definitions */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dncInfo)
{
   if (strcmp(strstr(argv[2], "."), ".bmp") == 0)
	{
		dncInfo->stego_image_fname = argv[2];
		//printf("%s\n", dncInfo->stego_image_fname);
	}
	else
	{
		return e_failure;
	}
	if (argv[3] == NULL)
	{
		dncInfo->secret_fname = "decoded_secret";
	}
	else
	{
		dncInfo->secret_fname = argv[3];
	}
	return e_success;
}
/* Perform the decoding */
Status do_decoding(DecodeInfo *dncInfo)
{
	//open files for decoding
	if (open_stego_file(dncInfo) == e_success)
	{
		printf("File open successfully\n");
	}
	else
	{
		printf("Error in opening file\n");
		return e_failure; 
	}
	//Decode Magic String
	if (decode_magic_string(MAGIC_STRING, dncInfo) == e_success)
	{
		printf("MAGIC_STRING decoded Successfully\n");
	}
	else
	{
		printf("Error : Failed to decode MAGIC_STRING\n");
		return e_failure; 
	}
	//decode file extension size
	if(decode_file_extn_size(dncInfo) == e_success)
	{
		printf("File Extention size decoded successfully\n");
	}
	else
	{
		printf("Error : Failed to decode File Extention size\n");
		return e_failure;
	}
	//decode file extension
	if(decode_secret_file_extn(dncInfo) == e_success)
	{
		printf("File Extention decoded successfully\n");
	}
	else
	{
		printf("Error : Failed to decode File Extention\n");
		return e_failure;
	}
	//concatinate secret file name and extention 
	if(concatenate(dncInfo) == e_success)
	{
		printf("Output secret file created successfully\n");
	}
	else
	{
		printf("Error : Failed to Open Output file successfully\n");
		return e_failure;
	}
	//decode file size
	if(decode_file_size(dncInfo) == e_success)
	{
		printf("File Size decoded Successfully\n");
	}
	else
	{
		printf("Error : Failed to decode File Size\n");
		return e_failure;
	}
	//decode secret file data
	if(decode_secret_file_data(dncInfo) == e_success)
	{
		printf("File data decoded Successfully\n");
	}
	else
	{
		printf("Error : Failed to decode File data successfully\n");
		return e_failure;
	}
	return e_success;
}
/* Open Stego file for decoding */
Status open_stego_file(DecodeInfo *dncInfo)
{
	dncInfo->fptr_stego_image = fopen(dncInfo->stego_image_fname, "r");
	if (dncInfo->fptr_stego_image == NULL) 
	{
   		 perror("Error opening stego image file");
		 fprintf(stderr,"Error : Unable to open file %s\n",dncInfo->stego_image_fname);
		 return e_failure;
	}
	else
	{
		return e_success;
	}
}
/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *dncInfo)
{
    fseek(dncInfo->fptr_stego_image, 54, SEEK_SET);
	char image_data[strlen(MAGIC_STRING) + 1];
	if(decode_data_from_image((sizeof(MAGIC_STRING)-1),dncInfo->fptr_stego_image,image_data) == e_success)
	{
		image_data[strlen(MAGIC_STRING)]='\0';
		//printf("%s\n",image_data);
		if(strcmp(MAGIC_STRING,image_data)==0)
		{
			return e_success;
		}
		else
		{
			return e_failure;
		}
	}
}
/* Decode secret file extension size */
Status decode_file_extn_size(DecodeInfo *dncInfo)
{
    char buffer[32];
	fread(buffer,32,1,dncInfo->fptr_stego_image);
	dncInfo->extn_secret_size = decode_size_lsb(buffer);
	//printf("%ld\n",dncInfo->extn_secret_size);
	return e_success;
}
/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *dncInfo)
{
    char buffer[dncInfo->extn_secret_size];
	if(decode_data_from_image(dncInfo->extn_secret_size,dncInfo->fptr_stego_image,buffer)==e_success)
	{
		buffer[dncInfo->extn_secret_size]='\0';
		strcpy(dncInfo->extn_secret_file,buffer);
		//printf("%s\n",dncInfo->extn_secret_file);
	}
	else
	{
		return e_failure;
	}
	return e_success;
}
/* Decode secret file size */
Status decode_file_size(DecodeInfo *dncInfo)
{
    char buffer[32];
    fread(buffer,1,32,dncInfo->fptr_stego_image);
    dncInfo->size_secret_file = decode_size_lsb(buffer);
    //printf("%ld\n",dncInfo->size_secret_file);
    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *dncInfo)
{
    char secret[dncInfo->size_secret_file];
    if(decode_data_from_image(dncInfo->size_secret_file,dncInfo->fptr_stego_image,secret)==e_success)
    {
       secret[dncInfo->size_secret_file]='\0';
	   fprintf(dncInfo->fptr_secret,"%s",secret);
	   //printf("%s",secret);
    }
    return e_success;
}
/* Decode data from the image */
Status decode_data_from_image(int size, FILE *fptr_stego_image, char *decode_data)
{
	char buffer[8];
	for(int i=0;i<size;i++)
	{
		fread(buffer,1,8,fptr_stego_image);
		decode_data[i]=decode_byte_from_lsb(buffer);
	}
	return e_success;
}
/* Decode a byte into LSB of image data array */
char decode_byte_from_lsb(const char *image_buffer)
{
    char ch=0;
	for(int i=0;i<8;i++)
	{
		ch=((image_buffer[i]&1)<<i) | ch;
	}
	return ch;
}

/* Decode size from LSB of buffer */
char decode_size_lsb(const char *buffer)
{
    char ch=0;
	for(int i=0;i<32 ;i++)
	{
		ch=((buffer[i]&1)<<i) | ch;
	}
	return ch;
}
/* Concatenate the decoded information */
Status concatenate(DecodeInfo *dncInfo)
{
    char extn[20];
	strcpy(extn,dncInfo->secret_fname);
	strcat(extn,dncInfo->extn_secret_file);
	//printf("%s\n",extn);
	dncInfo->fptr_secret=fopen(extn,"w+");
	if(dncInfo->fptr_secret == NULL)
    {
		perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",dncInfo->secret_fname);
        return e_failure;
	}
    	return e_success;
}
