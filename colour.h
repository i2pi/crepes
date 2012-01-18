#ifndef COLOUR__H
#define COLOUR__H

unsigned char	r_ (unsigned long c);
unsigned char	g_ (unsigned long c);
unsigned char 	b_ (unsigned long c);
unsigned long	rgb (unsigned char r, unsigned char g, unsigned char b);
unsigned long	mix (unsigned long a, unsigned long b, float r);
unsigned long	mix50 (unsigned long a, unsigned long b);
void			gfilt (int x, int y, int x1, int y1, int thres, unsigned char *tcono);
void			filt (int x, int y, int x1, int y1, unsigned long *tcono);
void			mfilt (int x, int y, int x1, int y1, int thres);
unsigned long	cmix (unsigned long c, unsigned long m);
unsigned long	bmix (unsigned long c, unsigned long m);

#endif
