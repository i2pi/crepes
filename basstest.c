#include <stdlib.h>
#include <stdio.h>
#include "bass.h"
#include "tclib.h"

#define WIDTH	640
#define HEIGHT	480
#define BACKS	5

typedef struct _backT
{
	unsigned char	used;
	unsigned char	buf[WIDTH*HEIGHT];
	unsigned long	color;
} backT;

backT	back[BACKS];


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

inline
unsigned long rgb (unsigned char r, unsigned char g, unsigned char b)
{
	return (r << 16 | g << 8 | b);
}

inline 
unsigned long mix (unsigned long a, unsigned long b, float r)
{
	return ( rgb (
					(unsigned char) (r_(a)*r + r_(b)*(1.0f-r)),
					(unsigned char) (g_(a)*r + g_(b)*(1.0f-r)),
					(unsigned char) (b_(a)*r + b_(b)*(1.0f-r))
				 ));
}

inline 
unsigned long gmix (unsigned long a, unsigned char b, float r)
{
	return ( rgb (
					(unsigned char) (r_(a)*r + b*(1.0f-r)),
					(unsigned char) (g_(a)*r + b*(1.0f-r)),
					(unsigned char) (b_(a)*r + b*(1.0f-r))
				 ));
}


void	gfilt (int x, int y, int x1, int y1, int thres)
{
	int	j,k;
	unsigned long	r;
	unsigned char	c;

	for (j=x; j<x1; j++)
	for (k=y; k<y1; k++)
	if ((unsigned char)(tcono[j+k*WIDTH]&0xff) > thres)
	{
		tcono[j+k*WIDTH]=0xffffff;
	} else
	{
		r = b_(tcono[j-1+(k-1)*WIDTH]) + b_(tcono[j+(k-1)*WIDTH]) + b_(tcono[j+1+(k-1)*WIDTH]) + 
			b_(tcono[j-1+    k*WIDTH])                            + b_(tcono[j+1+    k*WIDTH]) + 
			b_(tcono[j-1+(k+1)*WIDTH]) + b_(tcono[j+(k+1)*WIDTH]) + b_(tcono[j+1+(k+1)*WIDTH]);
		
		c = (unsigned char)(r>>3);
		if (c > thres) c = 255;

		tcono[j+k*WIDTH] = rgb (c,c,c);
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
		r = b_(tcono[j+    k*WIDTH])                            + b_(tcono[j+1+    (k-2)*WIDTH]) +
			b_(tcono[j-3+    k*WIDTH])                            + b_(tcono[j+2+    k*WIDTH]);
			
		
		c = (unsigned char)(r>>2);
		if (c < thres) c = 0;

		if (c-40 > 0) c-=40;

		tcono[j+k*WIDTH] = rgb (c,c,c);
	}
}


inline
unsigned long	cmix (unsigned long c, unsigned char m)
{
	return (rgb ((r_(c)*m)>>8, (g_(c)*m)>>8, (b_(c)*m)>>8));
}

void	init_backs (void)
{
	int	i;

	for (i=0; i<BACKS; i++)
	{
		back[i].used = 0;
	}
}

int	main (int argc, char **argv)
{
	HSTREAM			str;
	int				x,y;
	unsigned char	c;
	short			data[HEIGHT*2*2];

	if (BASS_GetVersion()!=MAKELONG(0,8)) {
		printf ("Bass not loaded - or incorrect virgin\n");
		return 0;
	}

	if (!BASS_Init(-1,44100,BASS_DEVICE_NOSYNC,GetForegroundWindow())) 
	{
		printf ("Can't get it up\n");
		return 0;
	}

	settopic_tc("FUCK YOU");
	init_tc(WIDTH, HEIGHT);
	start_tc();
	
	BASS_Start();	

	str = BASS_StreamCreateFile(FALSE, "c:\\windows\\desktop\\bass\\test\\basstest\\sense_forme.mp3", 0, 0, 0);

	if (str==0)
	{
		printf ("Where is C:\test.mp3 -- or some other shit\n");
		return (0);
	}

	BASS_StreamPlay(str, 0,0);

	init_backs ();

	back[0].used = 1;
	for (x=0; x<WIDTH; x++)
	for (y=0; y<HEIGHT; y++)
	{	
		back[0].buf[x+y*WIDTH] = 255 * sin (x/23.0f) * cos (y/47.0f) * sin ((x*y)/100.0f);
	}

	while (!kbhit_tc ())
	{
		BASS_ChannelGetData(str, data, HEIGHT*sizeof(short)*2);

		for (y=0; y<HEIGHT-1; y++)
		{
			c = (unsigned char)(abs(data[y>>1] >> 7));
			for (x=0; x<WIDTH; x++)
			{
				tcono[x+y*WIDTH] = cmix (back[0].buf[x+y*WIDTH], c);	
			}
		}
		blit_tc();
	}

	kill_tc();
	
	return (0);
}