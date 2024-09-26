#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
/* Function Definitions */
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
Status read_and_validate_encode_args(char **argv, EncodeInfo *encInfo)
{
	FILE *fptr;
	char arr[2];
	fptr = fopen(argv[2], "r");
	if (fptr == NULL)
	{
		printf("Error opening file");
		return e_failure;
	}
	fread(arr, sizeof(char), 2, fptr);
	if (strcmp(arr, "BM") == 0)
	{
		//valid BMP file
		encInfo->src_image_fname = argv[2];

		//Determine the extension of the secret file
		if (strstr(argv[3], ".txt"))
		{
			encInfo->secret_fname = argv[3];
			strcpy(encInfo->extn_secret_file, ".txt");
		}
		else if (strstr(argv[3], ".c"))
		{
			strcpy(encInfo->extn_secret_file, ".c");
			encInfo->secret_fname = argv[3];
		}
		else if (strstr(argv[3], ".sh"))
		{
			encInfo->secret_fname = argv[3];
			strcpy(encInfo->extn_secret_file, ".sh");
		}
		else if(strstr(argv[3],".exe"))
		{
			encInfo->secret_fname = argv[3];
			strcpy(encInfo->extn_secret_file,".exe");
		}
		//check if stego imahe file is provided,otherwise use default
		if(argv[4] != NULL)
		{
			if(strstr(argv[4],".bmp"))
			{
				encInfo->stego_image_fname = argv[4];
				printf("%s\n",encInfo->stego_image_fname);
			}
			else
			{
				return e_failure;
			}
		}
		else
		{
			encInfo -> stego_image_fname = "stego.bmp";
		}
		//fclose(fptr);
		return e_success;
	}
	else
	{
		//fclose(fptr);
		return e_failure;
	}
}
//Function to perform the encoding process
Status do_encoding(EncodeInfo *encInfo)
{
	//open files for encoding
	if(open_files(encInfo)==e_success)
	{
		printf("Files open successfully\n");
	}
	else {
		printf("Error in opening files");
		return e_failure;
	}
	//check if there is enoungh capacity in both source and stego files
	if(check_capacity(encInfo)==e_success)
	{
		printf("Size is Sufficient in Both Source file & stego file\n");
	}
	else
	{
		printf("Error : Sufficient Size is not available\n");
		return e_failure;
	}
	//copy BMP header to stego file
	if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
	{
		printf("Successfully copied bmp header to stego file\n");
	}
	else {
		printf("Error : Failed to  copy bmp header to stego file\n");
	}
	//Encode magic string
	if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
	{
		printf("Magic String Encoded Successfully\n");
	}
	else {
		printf("Error : Failed to  encode Magic string\n");
		return e_failure;
	}
	//encode file size
	if(encode_file_size(strlen(encInfo->extn_secret_file),encInfo) == e_success)		//passing file size and structure to encode size function
	{
		printf("Secret file extention size encoded succesfully\n");
	}
	else
	{
		printf("Error : Failed to encode secret file extention size\n");
		return e_failure;
	}
	//encode secret file extention
	if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_success){
		printf("secret file extension encoded successfully\n");
	}else {
	printf("Error : Failed to encode secret file extension\n");
	}
	//encode file size
	if(encode_file_size(encInfo->size_secret_file,encInfo) == e_success)
	{
		printf("secret file size encoded successful.\n");
	}
	else {
	printf("Error : Failed to encode secret file size\n");
	return e_failure;
	}
	//encode secret file data
	if(encode_secret_file_data(encInfo) == e_success)
	{
		printf("secret file data encoded.\n");
	}
	else {
		printf("Error : Failed to encode secret file data \n");
	}
	//copy remaining image data
	if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
	{
		printf("Remaining image data is copied\n");
		return e_success;
    }
	else {
		printf("Failed to copy remaining data\n");
		return e_failure;
	}
}
/* Function to open source image, secret file, and stego image for encoding */
Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
		return e_failure;
	}
	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
		return e_failure;
	}
	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
		return e_failure;
	}
	// No failure return e_success
	return e_success;
}
/* Function to check if there is enough capacity in both source and stego files */
Status check_capacity(EncodeInfo *encInfo)
{
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);			//to check and store the sie of source bmp file
	encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);					//to check and store the size of secret file
	if(encInfo->image_capacity > 54+((strlen(MAGIC_STRING)+sizeof(int)+sizeof(int)+sizeof(int)+encInfo->size_secret_file)*8))
	{
		return e_success;
	}
	else {
		return e_failure;
	}
}
/* Function to copy BMP header from source to stego image file */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char array[54];
	fseek(fptr_src_image,0,SEEK_SET);				//to make filepointer to point at first position
	fread(array,sizeof(char),54,fptr_src_image);
	fwrite(array, sizeof(char), 54, fptr_stego_image);
	return e_success;
}
/* Function to encode the magic string into the stego image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	if(encode_data_to_image(MAGIC_STRING,(sizeof(MAGIC_STRING)-1),encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
	{
		return e_success;
	}
	else {
		return e_failure;
	}
}
/* Function to encode data into the stego image */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char buffer[8];
	for(int i=0;i<size;i++)
	{
		fread(buffer,8,1,fptr_src_image);		//fetch 8 bytes of data from sorce image and store in an array
		encode_byte_to_lsb(data[i],buffer);		//passing array index and image data to encode byte to lsb
		fwrite(buffer,8,1,fptr_stego_image);
	}
	return e_success;
}
/* Function to encode file size into the stego image */
Status encode_file_size(long file_size, EncodeInfo *encInfo)
{
	char arr[32];
	fread(arr,32,1,encInfo->fptr_src_image);
	for(int i=0;i<32;i++)
	{
		char temp=((file_size & (1<<i))>>i);
		arr[i] = (arr[i] & ~1) | temp;
	}
	fwrite(arr,32,1,encInfo->fptr_stego_image);
	return e_success;
}
/* Function to encode a byte into the least significant bit of an array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
	for(int i=0;i<8;i++)
	{
		char res=((data & (1<<i))>>i);			//fetch each bit from data and move to lsb position
		image_buffer[i] = (image_buffer[i] & ~1) | res;		//store buffer array with data lsb
	}
}
/* Function to encode the secret file extension into the stego image */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
	encode_data_to_image((char *)file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
	return e_success;
}
/* Function to encode the data of the secret file into the stego image */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    char str[encInfo->size_secret_file];
    fread(str,encInfo->size_secret_file,1,encInfo->fptr_secret);
    encode_data_to_image(str,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}
/* Function to copy the remaining image data from source to stego image */
Status copy_remaining_img_data(FILE *fptr_src,FILE *fptr_dest)
{
    char ch;
    while((fread(&ch,1,1,fptr_src)) > 0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}
/* Function to get the size of a file */
uint get_file_size(FILE *fptr)
{
	fseek(fptr,0,SEEK_END);
	long size=ftell(fptr);
	rewind(fptr);
	return size;
}
/* Function to get the image size of BMP */
uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height,size;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);
	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);
	//printf("width = %u\n", width);
	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);
	//printf("height = %u\n", height);
	// Return image capacity
	size=width * height *3;
	//printf("%u\n",size);
	return size;
	//return width * height * 3;
}
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
