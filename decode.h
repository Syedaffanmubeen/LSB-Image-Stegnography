#include <stdio.h>
#include "types.h"		// Contains user defined types
/* 
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */
#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[4];
    long size_secret_file;
	long extn_secret_size;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dncInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *dncInfo);

Status open_stego_file(DecodeInfo *dncInfo);

/*Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *dncInfo);

/*Decode secret file extension size*/
Status decode_file_extn_size(DecodeInfo *dncInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *dncInfo);

/* Decode secret file size */
Status decode_file_size( DecodeInfo *dncInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *dncInfo);

/* Decode function, which does the real decoding */
Status decode_data_from_image( int size, FILE *fptr_src_image, char *decode_data);

/* Decode a byte into LSB of image data array */
char decode_byte_from_lsb(const char *image_buffer); 

char decode_size_lsb(const char *buffer);

Status concatenate(DecodeInfo *dncInfo);
