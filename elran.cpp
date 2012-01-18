#include "soundsys.h"
#include "tclib.h"
#include "colour.h"
#include "font.h" 
#include "elran.h"

#define PI 3.1415926537

extern "C" 
{	
	char	xm[];
	char	scroller[];
};

unsigned char	screen[SWIDTH*SHEIGHT];

void scroll (int line, int offset)
{
	int	j, i, pi,  l, y;
	char	str[128];
	
	l=0;
	i=0;
	pi=0;
	while (l<line)
	{
		if (scroller[++i]=='\n') 
		{	
			l++;
			if (l < line) pi = i;
		}
	}

	for (j=pi; j<i; j++)
	{
		str[j-pi] = scroller[j];
	}
	str[i-pi] = 0;

	y=-offset;
	print (str, y);
	while (y<HEIGHT)
	{
		i = 0;	
		while (scroller[j] != '\n') str[i++] = scroller[j++];
		j++;
		str[i] = 0;
		print (str, y+=20);
	}
}

inline
int	order (void)
{
	return (getpos_sound() & 0x0000ffff);
}

inline 
int	row (void)
{
	return (getpos_sound() >> 16);
}


inline 
double sin(double phase)
{
	__asm
	{
		fld		phase
		fsin	
		fstp	phase
	}
	
	return phase;
}

inline
double cos(double phase)
{
	__asm
	{
		fld		phase
		fcos	
		fstp	phase
	}
	
	return phase;
}

inline
double sqrt(double phase)
{
	__asm
	{
		fld		phase
		fsqrt	
		fstp	phase
	}
	
	return phase;
}


double	fabs (double x) { return ((x < 0.0) ? -1.0*x : x); }

void sline (float x, float y, float x1, float y1, unsigned char *screen)
{
	float	dx, dy, X, Y;
	float	m,c;
	
	struct 
	{
		int	x,y;
		int	x1,y1;
		int	w,h;
		int	mx,my;
	} viewport;

	viewport.x = 2; 
	viewport.y = 2;
	viewport.x1 = SWIDTH-2;
	viewport.y1 = SHEIGHT-2;
	viewport.w = SWIDTH;
	viewport.h = SHEIGHT;
	viewport.mx = SWIDTH/2;
	viewport.my = SHEIGHT/2;	

	
	// Klippen

	if ((y < viewport.y)&&(y1 < viewport.y)) return;
	if ((y > viewport.y1)&&(y1 > viewport.y1)) return;
	if ((x < viewport.x)&&(x1 < viewport.x)) return;
	if ((x > viewport.x1)&&(x1 > viewport.x1)) return;

	if (x1 - x == 0.0f) x1 += 0.0001f;

	m = (y1 - y) / (float)(x1 - x);
	c = y - m * x;


	if (y < viewport.y)
	{
		y = viewport.y+2.0f;
		x = (y - c) / m;
	} 
	if (y > viewport.y1) 
	{
		y = viewport.y1-2.0f;
		x = (y - c) / m;
	}
	
	if (y1 < viewport.y)
	{
		y1 = viewport.y+2.0f;
		x1 = (y1 - c) / m;
	} 
	if (y1 > viewport.y1)
	{
		y1 = viewport.y1-2.0f;
		x1 = (y1 - c) / m;
	}

	if (x < viewport.x)
	{		
		x = viewport.x+2.0f;
		y = m*x + c;
	} else
	if (x > viewport.x1)
	{		
		x = viewport.x1-2.0f;
		y = m*x + c;
	}	
	
	if (x1 < viewport.x)
	{		
		x1 = viewport.x+2.0f;
		y1 = m*x1 + c;
	} else
	if (x1 > viewport.x1)
	{		
		x1 = viewport.x1-2.0f;
		y1 = m*x1 + c;
	}	

	if ((x<viewport.x)||(x>viewport.x1)||(x1<viewport.x)||(x1>viewport.x1)||
		(y<viewport.y)||(y>viewport.y1)||(y1<viewport.y)||(y1>viewport.y1))
	{
		return;
	}

	dx=(x1-x);
	dy=(y1-y);

	if (fabs(dx)>fabs(dy))
	{
		if (x1<x)
		{
			m=x1; x1=x; x=m;
			m=y1; y1=y; y=m;
		}
		if (dx == 0.0) return;
		m = dy/dx;
		Y=y;
		for (X=x; X<x1; X++)	
		{
			screen[(int)(X+Y*SWIDTH)] = 0x00;
			Y += m;
		}
		return;
	} else
	{
		if (y1<y)
		{
			m=x1; x1=x; x=m;
			m=y1; y1=y; y=m;
		}
		if (dy == 0.0) return;
		m = dx/dy;
		X=x;
		for (Y=y; Y<y1; Y++)
		{
			screen[(int)(X+Y*SWIDTH)] = 0x00;
			X += m;
		}
		return;
	}
}



void	init (void)
{
	init_font ();

	settopic_tc("crepes");
	init_tc(WIDTH, HEIGHT);
	start_tc();
	memset (tcono, 0, WIDTH*HEIGHT*4);
	blit_tc();
	
}

void weare (char *str)
{
	memset (screen, 0xff, SWIDTH*SHEIGHT);
	prints ("we are", 50, 20, screen);
	prints (str, 80, 50, screen);
}

void justlike (void)
{
	memset (screen, 0xff, SWIDTH*SHEIGHT);
	prints ("just like everybody else", 10, 90, screen);
}

void show (void)
{
	int	x, y, X, Y;
	unsigned char c;
	unsigned long C;

	for (x=0; x<SWIDTH; x++)
	for (y=0; y<SHEIGHT; y++)
	{
		c = screen[x+y*SWIDTH];
		X = x<<2;
		Y = y<<2;
		C = (c) + (c<<8) + (c<<16);
		tcono [X   + (Y+0)*WIDTH] = C;
		tcono [X+1 + (Y+0)*WIDTH] = C;
		tcono [X+2 + (Y+0)*WIDTH] = C;
		tcono [X+3 + (Y+0)*WIDTH] = C;

		tcono [X   + (Y+1)*WIDTH] = C;
		tcono [X+1 + (Y+1)*WIDTH] = C;
		tcono [X+2 + (Y+1)*WIDTH] = C;
		tcono [X+3 + (Y+1)*WIDTH] = C;

		tcono [X   + (Y+2)*WIDTH] = C;
		tcono [X+1 + (Y+2)*WIDTH] = C;
		tcono [X+2 + (Y+2)*WIDTH] = C;
		tcono [X+3 + (Y+2)*WIDTH] = C;
		
		tcono [X   + (Y+3)*WIDTH] = C;
		tcono [X+1 + (Y+3)*WIDTH] = C;
		tcono [X+2 + (Y+3)*WIDTH] = C;
		tcono [X+3 + (Y+3)*WIDTH] = C;
	}
}

void elipse (int x, int y, int rx, int ry)
{
	float	t;
	int		X,Y;

	for (t=0.0; t<2.0*PI; t+=0.02f)
	{
		X = (int)(rx*sin (t) + x);
		Y = (int)(ry*cos (t) + y);
		screen[X+Y*SWIDTH] = 0;
	}
}


void normal (int x, int y, int cs, int ns)
{
	sline (x-cs, y, x, y-cs, screen);
	sline (x, y-cs, x+cs, y, screen);
	sline (x+cs, y, x, y+cs, screen);
	sline (x, y+cs, x-cs, y, screen);
	sline (x, y, x+ns, y-ns, screen);
}

void box (int x, int y, int x1, int y1)
{
	sline (x,y, x1, y, screen);
	sline (x1, y, x1, y1, screen);
	sline (x1, y1, x, y1, screen);
	sline (x, y1, x, y, screen);
}

void thumb (int x, int y, float sx, float sy)
{
	// thumb
	box (x-(10*sx), y-(20*sy), x-( 2*sx), y-(5*sy));
	box (x-(20*sx), y-( 8*sy), x-(10*sx), y-(3*sy));
	box (x-(25*sx), y-( 3*sy), x-(13*sx), y+(2*sy));
	box (x-(21*sx), y+( 2*sy), x-(10*sx), y+(6*sy));
	sline (x-(10*sx), y+(7*sy), x+(15*sx), y+(7*sy), screen);
	sline (x+(15*sx), y+(7*sy), x+(15*sx), y-(8*sy), screen);
	sline (x+(15*sx), y-(8*sy), x-( 1*sx), y-(8*sy), screen);
}

void straight (int x, int y, float sx, float sy)
{
	sline (x-10*sx, y-3*sy, x+10*sx, y-3*sy, screen);
	sline (x+10*sx, y-3*sy, x+10*sx, y-10*sy, screen);
	sline (x+10*sx, y-10*sy,x+20*sx, y, screen);
	sline (x+20*sx, y,      x+10*sx, y+10*sy, screen);
	sline (x+10*sx, y+10*sy,x+10*sx, y+3*sy, screen);
	sline (x+10*sx, y+3*sy, x-10*sx, y+3*sy, screen);
	sline (x-10*sx, y+3*sy, x-10*sx, y-3*sy, screen);
}

void collapse (int x, int y)
{
	sline (x-5, y-10, x+5, y-10, screen);
	sline (x+5, y-10, x+5, y, screen);
	sline (x+5, y, x+10, y, screen);
	sline (x+10,y, x,y+5, screen);
	sline (x,y+5, x-10, y, screen);
	sline (x-10, y, x-5, y, screen);
	sline (x-5,y, x-y, y-10, screen);
	box (x-15, y+5, x+15, y+10);
}

void timey (int x, int y, int r)
{
	float	t;
	int		X,Y;

	for (t=0; t<2*PI; t+=0.02f)
	{
		X = x + r*sin (t);
		Y = y + r*cos (t);

		screen [X + Y*SWIDTH] = 0;
	}

	sline (x-r, y, x-0.5*r, y, screen);
	sline (x+r, y, x+0.5*r, y, screen);
	sline (x, y-r, x, y-0.5*r, screen);
	sline (x, y+r, x, y+0.5*r, screen);
}

void happy (int x, int y, int r, float n)
{
	float	t;
	int		X,Y;

	for (t=0-n; t<n; t+=0.02f)
	{
		X = x + r*sin (t);
		Y = y + r*cos (t);

		screen [X + Y*SWIDTH] = 0;
	}

	box (x-r, y-r, (x-r)+5, (y-r)+5);
	box (x+r, y-r, (x+r)-5, (y-r)+5);
}

void baguette (int x, int y, float s)
{
	int	X, Y;

	sline (x-10*s, y+5, x+10*s, y+5,screen);
	sline (x-10*s, y-5, x-10*s, y+5,screen);
	sline (x+10*s, y-5, x+10*s, y+5,screen);
		
	for (X=0; X<20*s; X++)
	{
		Y = (y-5) + 2*sin (X);
		screen [x-(int)(10*s)+X + Y*SWIDTH] = 0;
	}
}

void shoulder (int x, int y, float r)
{
	float	t;
	int		X,Y;

	for (t=0; t<2*PI; t+=0.02f)
	{
		X=x+10*sin(t);
		Y=y+10*cos(t);
		screen [X+Y*SWIDTH] = 0;
	}
	
	X = x+20*sin(r);
	Y = y+20*cos(r);
	sline (x,y, X,Y, screen);
	sline (X,Y, x, y+40, screen);
}

int	main (void)
{
	int	o, r, oo, or, x,y,X,Y,li,of,lop, lopt;
	float A,B,C,D;

	init ();
	init_sound(xm);
	start_sound ();
	A=0.34f;
	B=-4.0f;
	C=0.98f;
	D=-0.12f;

	li = 0;
	of = -15;
	lop = 0;
	lopt = 0;
	
	while (!kbhit_tc()&&(li<174))
	{
		oo = o;
		o = order ();	
		or = r;	
		r = row ();

		if ((oo==15)&&(o==14)&&(!lopt))
		{
			lop++;
		}

		if (r == 3) lopt = 0;
		
		memset (screen, 0xff, SWIDTH*SHEIGHT);

		if (o<2)
		{
			prints ("loading crepes..", 20,20,screen);
			for (y=0; y<r>>1; y++)
			{
				for (A=-2.5; A<2.5; A+=0.02f)
				{
					X=20+10*sin(A);
					Y=100+5*(cos(A)-y*1.5);
					screen[X+Y*SWIDTH] = 0;
				}
			}
		}
				
		
		if (r<3)
		{
			
		} else
		if (r < 10)
		{
			switch (o)
			{
				case 4: weare ("ellipse"); break;
				case 5: weare ("normal"); break;
				case 6:	weare ("thumb"); break;
				case 7: weare ("straight"); break;
				case 8: weare ("collapse"); break;
				case 9: weare ("time"); break;
				case 10: weare ("happy"); break;
				case 11: weare ("baguette"); break;
				case 12: weare ("shoulder"); break;

				case 14: if (lop < 2) weare ("not"); break;
				case 15: prints ("i2pi", 40, 60, screen); break;
			}
		} else
		if (r < 17)
		{
			if (o > 3)
			{
				A *= 0.01*cos(B);
				B += 0.01-sin (C);
				C *= 0.01+cos (D);
				D += 0.01*sin (A);

				switch (o)
				{
					case 4: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (cos (sin(B*y*cos(x+r/30.0f))) + cos (o*D*x/34.0) * sin ((r*x + o*y)/cos (C*x*y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 5: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(x*A*(r+o) + y*B*o + o*r/87.0f) + sin ((r*x + o*y)/cos (C*x*y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 6: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(x*D*(r+o) + y*C*o + o*r/87.0f) + sin ((y*x + o*r)/cos (C*x/y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 7: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (cos (sin(B*y*cos(x+r/30.0f))) + cos (o*D*x/34.0) * sin ((r*x)/cos (C))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 8: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(y*B*o + o*r/87.0f) * cos (cos (o*D*x/34.0) * sin ((r*x + o*y)/cos (C*x*y)))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 9: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(y*B*o + o*r/87.0f) * cos (cos (o*D*x/34.0) * sin ((r*x + o*x)/cos (D*y)))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 10: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(x*A*(r+o) + y*B*o + o*r/87.0f)  + cos (o*D*x/34.0) * sin ((r*x + o*y)/cos (C*x*y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 11: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(y*B*(r+o) + y*D*o + o*r/87.0f) * sin ((r*x + o*y)/cos (C*x*y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
					case 12: 	
							for (x=10; x<SWIDTH-10; x++) for (y=10; y<SHEIGHT-10; y++)
							if (sin(x*A*(r+o) + y*B*o + o*r/87.0f) * cos (sin(B*y*cos(x+r/30.0f))) + cos (o*D*x/34.0) * sin ((r*x + o*y)/cos (C*x*y))
								> 0.0f)	screen[x+y*SWIDTH] = 0xd9; break;
				}	
			}
			switch (o)
			{
				case 4: elipse (80, 60, 30+r, 20-r); break;
				case 5: normal (80, 60, 20+r, 30-r); break;
				case 6: thumb (80,60, 2.0f+(r/18.0f), 2.0f); break;
				case 7:	straight (80, 60, 3.0f+(r/25.0f), 1.0f); break;
				case 8: collapse (80,60); break;
				case 9: timey (80,60, 20+r); break;
				case 10: happy (80,60, 20, r/21.0f); break;
				case 11: baguette (80,60, 1.0f + r/19.0f); break;
				case 12: shoulder (80,60, 0.4+r/13.0f); break;
				case 15: prints ("i2pi", 50, 30, screen); break;
			}
		} else
		{
			if (o==15) prints ("i2pi", 50, 30, screen); else
			if (lop < 2) justlike ();
		}

		show ();
		if (lop > 1)
		{
			scroll (li,of);
			if (or != r)
			{		
				of++;
			}
			if (of==0)
			{
				li++;
				of=-20;
			}
			memset (tcono, 0xff, WIDTH*55*4);
		}

		blit_tc();
	}
	if (li > 173)
	{
		while (r != 0)
		{
			memset (tcono, 0xff, WIDTH*HEIGHT*4);
			blit_tc();
			o = order ();	
			r = row ();
		}
	}

	kill_tc();
	
	return (0);
}



