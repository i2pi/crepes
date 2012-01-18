#include "tga.h"
#include <stdio.h>
#include <stdlib.h>

int *load_tga(void *bild)
{
  TGAHEAD * header=(TGAHEAD *)bild;
  int * outpic;
  unsigned char * palette;
  unsigned char * picdata;
  int l1,l2;
  int tmp;

  

  outpic=(int *)malloc(
       (header->imagespec.width*header->imagespec.height*4)
           +12);
  outpic[0]=32;
  outpic[1]=header->imagespec.width;
  outpic[2]=header->imagespec.height;

  switch (header->imagespec.bpp)
   {
    case 8 :
      palette=(unsigned char *)
               (
                (int)(((int)bild)+18)
               );
               /* just to make sure the compiler understand */
      picdata=(unsigned char *)
               (
                (int)(((int)bild)+786)
               );
               /* just to make sure the compiler understand */
      for (l1=0;l1<(header->imagespec.width*header->imagespec.height);l1++)
       {
        outpic[l1+3]=
          (palette[(picdata[l1]*3)  ]    )+
          (palette[(picdata[l1]*3)+1]<<8 )+
          (palette[(picdata[l1]*3)+2]<<16);
       }
     break;
    case 24 :
      picdata=(unsigned char *)
               (
                (int)(((int)bild)+18)
               );
               /* just to make sure the compiler understand */
      for (l1=0;l1<(header->imagespec.width*header->imagespec.height);l1++)
       {
        outpic[l1+3]= 
          (picdata[l1*3  ]    )+
          (picdata[l1*3+1]<<8 )+
          (picdata[l1*3+2]<<16);
       }
     break;
    default :
	{
      printf("Error, unsupported TGA type!(bitdepth problem) [%d bpp] (%d x %d)\n", header->imagespec);
	}
     
   }

  if (! ( header->imagespec.desc & 0x20 ) )
   {
    for (l2=0;l2<(header->imagespec.height>>1);l2++)
     {
      for (l1=0;l1<header->imagespec.width;l1++)
       {
        tmp=
          outpic[ 3+(l2*header->imagespec.width)+l1 ];
        outpic[ 3+(l2*header->imagespec.width)+l1 ]=
          outpic[ 3+((header->imagespec.height-l2-1)*header->imagespec.width)+l1 ];
        outpic[ 3+((header->imagespec.height-l2-1)*header->imagespec.width)+l1 ]=
          tmp;
       }
     }
   }

  return outpic;
}
