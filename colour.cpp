#include "colour.h"
#include "tclib.h"
#include "elran.h"

inline
unsigned char	r_ (unsigned long c)
{
	return ((unsigned char)((c >> 16) & 0xFF));
}

inline
unsigned char	g_ (unsigned long c)
{
	return ((unsigned char)((c >> 8) & 0xFF));
}

inline
unsigned char 	b_ (unsigned long c)
{
	return ((unsigned char) (c & 0xFF));
}

unsigned long rgb (unsigned char r, unsigned char g, unsigned char b)
{
	return (r << 16 | g << 8 | b);
}

unsigned long mix (unsigned long a, unsigned long b, float r)
{
	return ( rgb (
					(unsigned char) (r_(a)*r + r_(b)*(1.0f-r)),
					(unsigned char) (g_(a)*r + g_(b)*(1.0f-r)),
					(unsigned char) (b_(a)*r + b_(b)*(1.0f-r))
				 ));
}

void	gfilt (int x, int y, int x1, int y1, int thres, unsigned char *tcono)
{
	int	j,k;
	unsigned long	r;
	unsigned char	c;

	for (j=x; j<x1; j++)
	for (k=y; k<y1; k++)
	if (tcono[j+k*160] > thres)
	{
		tcono[j+k*160]=0xff;
	} else
	{
		r = (tcono[j-1+(k-1)*160]) + (tcono[j+(k-1)*160]) + (tcono[j+1+(k-1)*160]) + 
			(tcono[j-1+    k*160])                          + (tcono[j+1+    k*160]) + 
			(tcono[j-1+(k+1)*160]) + (tcono[j+(k+1)*160]) + (tcono[j+1+(k+1)*160]);
		
		c = (unsigned char)(r>>3);
		if (c > thres) c = 255;

		tcono[j+k*160] = c;
	}
}

void	filt (int x, int y, int x1, int y1, unsigned long *tcono)
{
	int	j,k;
	unsigned long	r,g,b;

	for (j=x+1; j<x1-1; j++)
	for (k=y+1; k<y1-1; k++)
	{
		b = b_(tcono[j-1+(k-1)*WIDTH]) + b_(tcono[j+(k-1)*WIDTH]) + b_(tcono[j+1+(k-1)*WIDTH]) + 
			b_(tcono[j-1+    k*WIDTH])                            + b_(tcono[j+1+    k*WIDTH]) + 
			b_(tcono[j-1+(k+1)*WIDTH]) + b_(tcono[j+(k+1)*WIDTH]) + b_(tcono[j+1+(k+1)*WIDTH]);
		r = r_(tcono[j-1+(k-1)*WIDTH]) + r_(tcono[j+(k-1)*WIDTH]) + r_(tcono[j+1+(k-1)*WIDTH]) + 
			r_(tcono[j-1+    k*WIDTH])                            + r_(tcono[j+1+    k*WIDTH]) + 
			r_(tcono[j-1+(k+1)*WIDTH]) + r_(tcono[j+(k+1)*WIDTH]) + r_(tcono[j+1+(k+1)*WIDTH]);
		g = g_(tcono[j-1+(k-1)*WIDTH]) + g_(tcono[j+(k-1)*WIDTH]) + g_(tcono[j+1+(k-1)*WIDTH]) + 
			g_(tcono[j-1+    k*WIDTH])                            + g_(tcono[j+1+    k*WIDTH]) + 
			g_(tcono[j-1+(k+1)*WIDTH]) + g_(tcono[j+(k+1)*WIDTH]) + g_(tcono[j+1+(k+1)*WIDTH]);

		tcono[j+k*WIDTH] = rgb ((unsigned char)(r>>3),(unsigned char)(g>>3),(unsigned char)(b>>3));
	}
}

void	mfilt (int x, int y, int x1, int y1, int thres)
{
	int	j,k;
	unsigned long	r;
	unsigned char	c;

	for (j=x; j<x1; j++)
	for (k=y; k<y1; k++)
	{
		r = b_(tcono[j+    k*WIDTH]) + b_(tcono[j+1+    (k-2)*WIDTH]) +
			b_(tcono[j-3+    k*WIDTH])   + b_(tcono[j+2+    k*WIDTH]);
			
		
		c = (unsigned char)(r>>2);
		if (c < thres) c = 0;

		if (c-40 > 0) c-=40;

		tcono[j+k*WIDTH] = rgb (c,c,c);
	}
}

unsigned long mix50 (unsigned long a, unsigned long b)
{
	return (
			rgb (
					(unsigned char)((r_(a)+r_(b))>>1),
					(unsigned char)((g_(a)+g_(b))>>1),
					(unsigned char)((b_(a)+b_(b))>>1)
				)
		);
}


unsigned long	cmix (unsigned long c, unsigned long m)
{
	return ((((c&0xff00ff)*m)>>8)&0xff00ff)+((((c&0xff00)*m)>>8)&0xff00);
}

unsigned long	bmix (unsigned long c, unsigned long m)
{
	return (
			(((unsigned long)(r_(c)+m)<<14) & 0xff0000) +
			(((unsigned long)(g_(c)+m)<< 6) & 0x00ff00) +
			(((unsigned long)(b_(c)+m)>> 2) & 0x0000ff)
		 );

}
