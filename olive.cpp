#include <math.h>

#include "tclib.h"
#include "colour.h"
#include "jmath.h"
#include "elran.h"
#include "olive.h"

inline
void	bfill (int x, int y, float z, int w, int h, unsigned long c)
{
	int	j,k;

	if (x<0) x = 0; else
	if (x+w > 639) w=0;

	if (y<0) y = 0; else
	if (y+w > 479) w=0;


	for (k=0; k<h; k++)
	for (j=0; j<w; j++) tcono [x+j + (k+y)*WIDTH] = c;
}
void do_back (int j, unsigned char bri)
{
	int	x,X;
	int	y,Y,s;
	unsigned long  c;
	unsigned long  r,g,b;
	
	for (x=0;x<640; x+=20)
	for (y=0;y<480; y+=20)
	{
		X = (int)((639-x)*fabs(sin (j+x/4040.0f)));
		Y = (int)((479-y)*fabs(cos (y+j/4300.0f)));
		c = (tcono[X+Y*WIDTH]&0xff);

		if (c>0xc0) 
		{
			//c = FLESH; 
			r = 0xce;
			g = 0x5a;
			b = 0x5a;
		} else
		if (c>0xa0) 
		{
			//c = SKIN; 
			r = 0xff;
			g = 0xce;
			b = 0x84;
		} else
		if (c>0x70)
		{
			//c = OLIVE;
			r = 0xb5;
			g = 0xb5;
			b = 0x6b;
		} else 
		{
			//c = TAR;
			r = 0x6b;
			g = 0x63;
			b = 0x5a;
		}

		r+=bri;
		g+=bri;
		b+=bri;

		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) b=255;

		c = (r<<16) + (g<<8) + (b);
		
		s = (int)(20 * sin ((j + x + y) / 1204.0f) * cos ((x*x-(y+j))/14056.0f));
		bfill (x-s,y-s,s*0.1f,   20+2*s,20+2*s, c);			
	}		
}



void olive (int j, unsigned char bri)
{
	do_back (j, bri);
}
