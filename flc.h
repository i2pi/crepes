#ifndef FLC__H
#define FLC__H

#include "elran.h"
#include <stdio.h>

typedef struct _flicT
{
	int	width, height;
	int frames;
	int f;
	unsigned long *frame_data;
} flicT;

#define FLI_MAXFRAMES (4000)	  /* Max number of frames... */
#define FLI_FILE_MAGIC 0xaf11	  /* File header Magic old FLI */
#define FLC_FILE_MAGIC 0xaf12	  /* File header Magic new FLC */
#define FLI_FRAME_MAGIC 0xf1fa		/* Frame Magic FLI/FLC */
#define FILE_HEAD_SIZE 128
#define FRAME_HEAD_SIZE 16
#define CHUNK_HEAD_SIZE 6
#define FLI_256_COLOR 4
#define FLI_DELTA 7
#define FLI_COLOR 11
#define FLI_LC	12
#define FLI_BLACK 13
#define FLI_BRUN 15
#define FLI_COPY 16
#define MAXCOLORS 256


int buffer_brun_chunk(unsigned char *pcc, unsigned char *image_cbuff,\
		      int image_height, int image_width);
int buffer_delta_chunk(unsigned char *pcc, unsigned char *image_cbuff,\
		      int image_height, int image_width);
int buffer_lc_chunk(unsigned char *pcc, unsigned char *image_cbuff,\
		      int image_height, int image_width);

int out_image(unsigned char *image_buffer, unsigned char *color_table,\
	    int width, int height, int num, flicT *flic);

//flicT *unfli(FILE *input);
//flicT *load_flc (char *name);
flicT *unfli(char *file);


#endif
