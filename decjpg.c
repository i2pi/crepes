#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"

FILE	*MYJPEG_fp;


static void init_source(j_decompress_ptr ci)
{   	
}

static boolean fill_input_buffer(j_decompress_ptr ci)
{
	unsigned char	b;

	fread (&b, 1, 1, MYJPEG_fp);

	ci->src->next_input_byte=&b;
	ci->src->bytes_in_buffer=1;
	
	return TRUE;
}

static void skip_input_data(j_decompress_ptr ci,long desired)
{  
	int	i;
	char	dummy;
		
	for (i=0; i<desired; i++)
	{
		fread (&dummy, 1,1, MYJPEG_fp);
	}

	// FSEEK, but cant remember the flag name, cant be fucked reading the .h
}

static void term_source(j_decompress_ptr ci) { }

int * decjpg(char *name)
{
      struct jpeg_decompress_struct cinfo;
      struct jpeg_error_mgr jerr;
      struct jpeg_source_mgr * src;
      JSAMPARRAY buffer;

      int lop;
      int l2;
	  int * dest;


	  MYJPEG_fp = fopen (name, "rb");
	  if (MYJPEG_fp == NULL) 
	  {
		printf ("Couldnt open '%s' as a JPG\n", name);
		return (NULL);
	  }

      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_decompress(&cinfo);

      cinfo.src = (struct jpeg_source_mgr *)

      (*cinfo.mem->alloc_small) (
                                (j_common_ptr) &cinfo,
                                JPOOL_PERMANENT,
                                sizeof(struct jpeg_source_mgr)

                                );
      src=cinfo.src;
      src->init_source = init_source;
      src->fill_input_buffer = fill_input_buffer;
      src->skip_input_data = skip_input_data;
      src->resync_to_restart = jpeg_resync_to_restart;
      src->term_source = term_source;
      src->bytes_in_buffer = 0;
      src->next_input_byte = NULL;
      

      jpeg_read_header(&cinfo, TRUE);
	  jpeg_start_decompress(&cinfo);

      buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width*4, 1);

      dest=(int *)malloc(12+(cinfo.output_width*cinfo.output_height*4));

      dest[0]=32;
      dest[1]=cinfo.output_width;
      dest[2]=cinfo.output_height;

      for (lop=0;lop<cinfo.output_height;lop++)
      {
         jpeg_read_scanlines(&cinfo, buffer,1);

         for (l2=0;l2<cinfo.output_width;l2++)
         {
//           tcono[lop*512+l2]=
           dest[cinfo.output_width*lop+l2+3]=
               ( buffer[0][l2*3+0]<<16 )+
               ( buffer[0][l2*3+1]<<8 )+
               ( buffer[0][l2*3+2] );
         }

      }

      jpeg_finish_decompress(&cinfo);
	  jpeg_destroy_decompress(&cinfo);

	  return dest;
}
