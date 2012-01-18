#ifndef TGA__H
#define TGA__H

#define COLORMAP_NO          0
#define COLORMAP_YES         1

#define IMAGETYPE_NODATA     0
#define IMAGETYPE_COLORMAP   1
#define IMAGETYPE_TRUECOLOR  2

typedef struct {
 unsigned short  firstent;
 unsigned short  numents;
 unsigned char   entsize;
}COLORMAP;

typedef struct {
 unsigned short  xorig;
 unsigned short  yorig;
 unsigned short  width;
 unsigned short  height;
 unsigned char   bpp;
 unsigned char   desc;
}IMAGESPEC;

typedef struct {
 unsigned char idlen;
 unsigned char colormaptype;
 unsigned char imagetype;
 COLORMAP      colormap;
 IMAGESPEC     imagespec;
}TGAHEAD;

int *load_tga(void *bild);

#endif

