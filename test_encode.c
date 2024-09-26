#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
OperationType check_operation_type(char *argv[]);
Status read_and_validate_encode_args(char **argv, EncodeInfo *encInfo);
Status do_encoding(EncodeInfo *encInfo);
OperationType check_operation_type(char *argv[])
{
	if (strcmp(argv[1], "-e") == 0)
	{
		return e_encode;
	}
	else if (strcmp(argv[1], "-d") == 0)
	{
		return e_decode;
	}
	else
	{
		return e_unsupported;
	}
}
int main(int argc, char *argv[])
{
	EncodeInfo encInfo;
	DecodeInfo dncInfo;
	uint img_size;
	if (check_operation_type(argv) == e_encode)
	{
		printf("Selected Encoding\n");
		if(argc>=4)
		{
			if (read_and_validate_encode_args(argv, &encInfo) == e_success)
			{
				do_encoding(&encInfo);
			}
			else if(read_and_validate_encode_args(argv,&encInfo) == e_failure)
			{
				printf("Error : Source image file is not a .bmp file \n");
			}
		}
		else
		{
			printf("Error : Pass correct number of arguments through command line\n");
		}
	}
	else if (check_operation_type(argv) == e_decode)
	{
		printf("Selected Decoding\n");
		if(argc>=3)
		{
			if(read_and_validate_decode_args(argv,&dncInfo) == e_success)
			{
				do_decoding(&dncInfo);
			}
			else if(read_and_validate_decode_args(argv,&dncInfo) == e_failure)
			{
				printf("Error : stego image file is not a .bmp file \n");
			}
		}
		else
		{
			printf("Error : Pass correct number of arguments through command line\n");
		}
	}
	else
	{
		printf("Pass Valid Type\n");
	}
	return 0;
}
