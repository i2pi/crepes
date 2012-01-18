#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>

#include "bass.h"
#include "tclib.h"
#include "font.h" 
#include "elran.h"


controller_setT	control_set[MAX_CONTROL_SETS];
backT			back[BACKS];
short			*data;
int				cur_back=0;
int				cur_set=0;
float			global_time;
enum EDIT_MODE	edit_mode;
int				MAX_POSITION;
int				DATA_SIZE;
char			DONE=0, timecode=0;
char			direction=0, paused=0;
unsigned long	position, pstep=22100;
unsigned long	frame;
HSTREAM			str;
int				effects=0;
effectT			*effect[MAX_FX];

struct
{
	unsigned char	sensitivity;
	unsigned char	offset;
	unsigned char	black;
	unsigned char	tv_rate;
	unsigned char	imoda, imodb;
	float	step;	
} globals;

struct 
{
	int	x,y;
	int	x1,y1;
	int	w,h;
	int	mx,my;
} viewport;

	 
void normalizeVector(VECTOR * v)
{
 float len=(float)sqrt((v->x*v->x)+(v->y*v->y)+(v->z*v->z));
 if (len<0.0000001)
  return;
 len=1/len;
 v->x*=len;
 v->y*=len;
 v->z*=len;
}

void createRotationMatrix(MATRIX * m,VECTOR * v,float a)
{
 float c;
 float s;
 float t;
 float x,y,z;
 x=v->x;
 y=v->y;
 z=v->z;

 c=(float)cos(a);
 s=(float)sin(a);
 t=1-c;

 m->a=t*x*x + c;
 m->b=t*x*y - s*z;
 m->c=t*x*z + s*y;

 m->d=t*y*x + s*z;
 m->e=t*y*y + c;
 m->f=t*y*z - s*x;

 m->g=t*z*x - s*y;
 m->h=t*z*y + s*x;
 m->i=t*z*z + c;

 m->x=0;
 m->y=0;
 m->z=0;
}

void scaleMatrix (MATRIX *m, float a)
{
	m->a *= a;
	m->e *= a;
	m->i *= a;
}

void vectorMatrixRot(VECTOR * v,MATRIX * m)
{
   VECTOR tmp;
   tmp.x=v->x*m->a + v->y*m->d + v->z*m->g;
   tmp.y=v->x*m->b + v->y*m->e + v->z*m->h;
   tmp.z=v->x*m->c + v->y*m->f + v->z*m->i;
   *v=tmp;
}

void vectorMatrixTrans(VECTOR * v,MATRIX * m)
{
	v->x+=m->x;
	v->y+=m->y;
	v->z+=m->z;
}

void vectorMatrixMul(VECTOR * v,MATRIX * m)
{
	vectorMatrixRot(v,m);
	vectorMatrixTrans(v,m);
}


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


void	hpf_back (int b)
{
	int	x,y;
	unsigned long s;

	for (x=1; x<WIDTH-1; x++)
	for (y=1; y<HEIGHT-1; y++)
	{
		s = back[b].buf[(x-1) + (y-1)*WIDTH] + back[b].buf[(x-0) + (y-1)*WIDTH] + back[b].buf[(x+1) + (y-1)*WIDTH] +
			back[b].buf[(x-1) + (y-0)*WIDTH] 								    + back[b].buf[(x+1) + (y-0)*WIDTH] +
			back[b].buf[(x-1) + (y+1)*WIDTH] + back[b].buf[(x-0) + (y+1)*WIDTH] + back[b].buf[(x+1) + (y+1)*WIDTH];
		s >>= 3;

		back[b].buf[x+y*WIDTH] = back[b].buf[x+y*WIDTH] - (unsigned char)s;
	}
}

void	contrast_back (int b, char thres)
{
	int	x, y;

	for (x=0; x<WIDTH; x++)
	for (y=0; y<HEIGHT; y++)
	{
		if (back[b].buf[x+y*WIDTH] < thres)
		{
			back[b].buf[x+y*WIDTH] = 0;
		} else
		{
			back[b].buf[x+y*WIDTH] = 255;
		}
	}
}


inline
unsigned long	cmix (unsigned char c, unsigned char m)
{
	unsigned char	g = (c*m) >> 8;
	return ((g<<16)+(g<<8)+g);
}

inline
unsigned long	rcmix (unsigned char r, unsigned char g, unsigned char b, unsigned char m)
{
	return ( rgb (((r*m)>>8),((g*m)>>8), ((b*m)>>8)) );
}

void	init_backs (void)
{
	int	i;

	for (i=0; i<BACKS; i++)
	{
		back[i].used = 0;
		back[i].alpha = 0.5f;
		back[i].brightness = 0.5f;
		back[i].alpha_threshold = -1.0f;
		back[i].active = 0;
		back[i].edit = 0;
		back[i].effects = 0;
		sprintf (back[i].filename, "empty");
	}
}

void	rpixel (int x, int y, unsigned long c)
{
	tcono[x+y*WIDTH] = c;
}

void	box (int x, int y, int x1, int y1, unsigned char c)
{
	int X, Y;

	for (X=x; X<x1-5; X++)
	for (Y=y; Y<y1; Y++) rpixel (X,Y,rgb(c,c,c));
	for (X=x1-5; X<x1; X++)
	for (Y=y; Y<y1; Y++) tcono[X+Y*WIDTH] = mix (tcono[X+Y*WIDTH], rgb (c,c,c), 0.5);
}

char	*f2s (char *s, float f)
{
	int	bigbit;
	int	smallbit;
	int	j;
	float	 J;
	char	zeros[10];

	
	bigbit = (int) floor (f);
	smallbit = (int) (fabs(f-floor(f)) * 10000.0f);	// should be enough decimal places
	J = 0.1f;
	zeros[0] = 0;
	for (j=0; j<5; j++)
	{
		if (fabs(f-floor(f)) < J) zeros[j] = '0'; else zeros[j] = '\0';
		J *= 0.1f;
	}
	if (zeros[0]!='0')
	sprintf (s, "%i.%s%i", bigbit, zeros, smallbit);
	else
	sprintf (s, "%i.%i",bigbit,smallbit);

	return (s);
}


void	print (char *s, int X, int Y)
{
	int	x,y,n,l;
	int	j,k;
	unsigned long c, b;

	x = X;
	y = Y;
	l=0;
	b=0;

	while (s[l] != '\0')
	if (s[l]=='_') { b=b?0:1; l++; } else
	{
		if ((s[l] >= 'a')&&(s[l] <= 'z')) n = s[l]-'a'; else
		if ((s[l] >= '0')&&(s[l] <= '9')) n = s[l]-'0' + 26; else
		if (s[l] == ',') n = 36; else 
		if (s[l] == '.') n = 37; else n = -65;
		
		if (s[l] == '\n')
		{
			x = X-6;
			y += 15;
		}

		if (n != -65)
		{
			for (j=0; j<font_width[n]; j++)
			for (k=0; k<16; k++)
			{
				if (font[n][j][k] != 255)
				{
					c = rgb (font[n][j][k], font[n][j][k], font[n][j][k]);
					if ((x+j<WIDTH)&&(y+k<HEIGHT))
					{
						if (!b)
							rpixel (x+j, y+k, c);
						else
						{
							rpixel (x+j, y+k, mix (0x0000ff, c, 0.5));
							rpixel (x+j, y+16, 0x0000ff);
						}
					}
				}
			}	
			if (b)
			{
				for (j=0; j<font_width[n]-2; j++)
				{
					rpixel (x+j, y+16, 0x0000ff);
				}
			}
			x+=font_width[n]-2;
		} else
		{
			x+=6;
		}
		
		l++;
	}
}



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
	 getch();
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


inline
void pixel (int x, int y, unsigned char c)
{
	back[cur_back].buf[x+y*WIDTH] = c;
}

void dline (VECTOR *a, VECTOR *b, int bk)
{
	float	za, zb;
	float	x,y, x1,y1, dx, dy, X, Y;
	float	m,c;

	za = PERSPECTIVE/(a->z + PERSPECTIVE);
	zb = PERSPECTIVE/(b->z + PERSPECTIVE);
	x =  (a->x*SCALEX*za + viewport.mx);
	y =  (a->y*SCALEY*za + viewport.my);
	x1 = (b->x*SCALEX*zb + viewport.mx);
	y1 = (b->y*SCALEY*zb + viewport.my);

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
			back[bk].buf[(int)X+(int)Y*WIDTH] = 255; 
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
			
			back[bk].buf[(int)X+(int)Y*WIDTH] = 255; 
			X += m;
		}
		return;
	}
}



i2aT	*load_i2a (char *filename)
{
	FILE	*fp;
	i2aT	*a = NULL;
	int		P, p, s;
	float	maxx=-100.0f, minx=100.0f;
	float	maxy=-100.0f, miny=100.0f;
	float	maxz=-100.0f, minz=100.0f;

	fp = fopen (filename, "rb");

	a = (i2aT *) malloc (sizeof (i2aT));

	a->shape = NULL;
	a->shapes = 0;

	while (!feof (fp))
	{
		s = (a->shapes++) - 1; 
		a->shape = (shapeT **) realloc (a->shape, sizeof (shapeT *) * a->shapes);
		fread (&P, 1, sizeof (int), fp);
		a->shape[s] = (shapeT *) malloc (sizeof (shapeT));
		a->shape[s]->points = P;
		a->shape[s]->point = (VECTOR *) malloc (sizeof (VECTOR) * a->shape[s]->points);
		a->shape[s]->opoint = (VECTOR *) malloc (sizeof (VECTOR) * a->shape[s]->points);
		for (p=0; p<P; p++)
		{
			fread (&a->shape[s]->point[p].x, sizeof (float), 1, fp);
			fread (&a->shape[s]->point[p].y, sizeof (float), 1, fp);
			fread (&a->shape[s]->point[p].z, sizeof (float), 1, fp);

			if ( (a->shape[s]->point[p].x > -10.0f) && (a->shape[s]->point[p].x < 10.0f) &&
				 (a->shape[s]->point[p].y > -10.0f) && (a->shape[s]->point[p].y < 10.0f) &&
				 (a->shape[s]->point[p].z > -10.0f) && (a->shape[s]->point[p].z < 10.0f) )
			{
				if (a->shape[s]->point[p].x > maxx) maxx = a->shape[s]->point[p].x; 
				if (a->shape[s]->point[p].x < minx) minx = a->shape[s]->point[p].x; 
				if (a->shape[s]->point[p].y > maxy) maxy = a->shape[s]->point[p].y; 
				if (a->shape[s]->point[p].y < miny) miny = a->shape[s]->point[p].y; 
				if (a->shape[s]->point[p].z > maxz) maxz = a->shape[s]->point[p].z; 
				if (a->shape[s]->point[p].z < minz) minz = a->shape[s]->point[p].z; 
			}
		}
	}

	a->shapes--;

	if (maxx - minx == 0.0f) maxx += 0.001f;
	if (maxy - miny == 0.0f) maxy += 0.001f;
	if (maxz - minz == 0.0f) maxz += 0.001f;

	// Normalise 

	for (s=0; s<a->shapes-1; s++)
	for (p=0; p<a->shape[s]->points; p++)
	{
		a->shape[s]->point[p].x -= minx;
		a->shape[s]->point[p].x /= (maxx - minx);
		a->shape[s]->point[p].y -= miny;
		a->shape[s]->point[p].y /= (maxy - miny);
		a->shape[s]->point[p].z -= minz;
		a->shape[s]->point[p].z /= (maxz - minz);

		a->shape[s]->opoint[p].x = a->shape[s]->point[p].x;
		a->shape[s]->opoint[p].y = a->shape[s]->point[p].y;
		a->shape[s]->opoint[p].z = a->shape[s]->point[p].z;

	}

	printf ("Loaded '%s' with %d shapes\n", filename, s-1);

	return (a);
}

void	show_i2a (i2aT *a, int b)
{
	int	s,p;
	
	memset(back[b].buf, 0, WIDTH*HEIGHT);
	back[b].used = 1;
	for (s=0; s<a->shapes-1; s++)
	{
		for (p=0; p<a->shape[s]->points-1; p++)
		{
			dline (&a->shape[s]->point[p], &a->shape[s]->point[p+1], b);
		}
	}

}

void	rotate_i2a (i2aT *a, MATRIX *m)
{
	int	s,p;
	
	for (s=0; s<a->shapes; s++)
	for (p=0; p<a->shape[s]->points; p++)
	{
		vectorMatrixRot (&a->shape[s]->point[p], m);
	}
}

void	tga_to_back (char *name, char *tgabuf, int b, char rx, char ry)
{
	FILE	*fp;
	int		*texture, i, x, y, X, Y;

	fp = fopen (name, "rb");
	if (!fp) { printf ("Couldn't find '%s'\n", name); return; }
	fread (&tgabuf[2], 1, 1055360,  fp);	
	texture = load_tga ((void *)tgabuf);
	sprintf (back[b].filename, name);
	i=3;
	back[b].used = 1;
	if ((texture[2] != HEIGHT) || (texture[1] != WIDTH))
	{
		printf ("Incorrect size for %s - should be %d x %d\n", name, WIDTH, HEIGHT);
		fclose (fp);
		return;
	}
	for (y=0; y<HEIGHT; y++)
	for (x=0; x<WIDTH; x++)
	{	
		if (rx) X=WIDTH-x; else X=x;
		if (ry) Y=HEIGHT-y; else Y=y;
		
		back[b].buf[X+Y*WIDTH] = b_(texture[i++]);
	}
	fclose (fp);
}

void	circle (int x, int y, int r, unsigned long col)
{
	float	t;
	int		X,Y;

	for (t=0; t<2.0f*PI; t+=0.1f)
	{
		X = (int)(r * sin (t) + x);
		Y = (int)(r * cos (t) + y);
		tcono[X+Y*WIDTH]=col;
	}
}

rwormT	*rworm (rwormT *parent, int b, float x, float y, float radius, float itheta, float tstep, float dtheta, int maxchilds, unsigned long col)
{
	rwormT	*worm, *new_worm;
	float	theta, rx, ry;
	int		X,Y;
	

	worm = (rwormT *) malloc (sizeof (rwormT));
	
	worm->point.x = x;
	worm->point.y = y;
	worm->point.z = 0.0f;
	worm->childs = 0;
	worm->child = NULL;
	worm->parent = parent;

	// Mark our point as found

	X = (int) ((x + 1.0f) * WIDTH * 0.5f);
	Y = (int) ((y + 1.0f) * HEIGHT* 0.5f);

	if (X >= WIDTH)  return (NULL);
	if (X <  0)		 return (NULL);
	if (Y >= HEIGHT) return (NULL);
	if (Y <  0)		 return (NULL);

	// Mark as used
	back[b].buf[X + Y*WIDTH] = 0xff;
	
	// And for visualisation
	tcono[X+Y*WIDTH] = col;
	blit_tc();

	// Find the children

	for (theta = itheta-dtheta; theta < itheta + dtheta; theta += tstep)
	{
		rx = (float) (radius * sin (theta) + x);
		ry = (float) (radius * cos (theta) + y);

		X = (int) ((rx + 1.0f) * WIDTH * 0.5f);
		Y = (int) ((ry + 1.0f) * HEIGHT* 0.5f);	


		if ((X < WIDTH) && (X > 0) && (Y < HEIGHT) && (Y > 0))
		{
			if (back[b].buf[X + Y*WIDTH] == 0x00)
			{
				// Found a child!!
	
				if (worm->childs < maxchilds)
				{
					new_worm = rworm (worm, b, rx, ry, radius, theta, tstep, dtheta, maxchilds, col);
					if (new_worm)
					{
						worm->childs++;
						worm->child = (rwormT **) realloc (worm->child, sizeof (rwormT *) * worm->childs);
						worm->child[worm->childs-1] = new_worm;
					}
				} 

				if (kbhit_tc()) exit (-1);
			} else
			{
				//tcono[X+Y*WIDTH] = 0x404060;
			}
		}
	}

	return (worm);
}

unsigned long rcol (void)
{
	unsigned char	r, g, b;

	r = (unsigned char)rand () & 0xff;
	g = (unsigned char)rand () & 0xff;
	b = (unsigned char)rand () & 0xff;

	return (rgb (r,g,b));
}

rwormT *worm_back (int b, float radius, float tstep, float dtheta, int maxchilds)
{
	int		X, Y;
	float	x, y;
	unsigned long c;

	for (X=120; X<WIDTH; X++)
	for (Y=80; Y<HEIGHT; Y++)
	{
		x = (2.0f * X/(float)WIDTH) - 1.0f;
		y = (2.0f * Y/(float)HEIGHT) - 1.0f;

		if (back[b].buf[X + Y*WIDTH] == 0x00)
		{
			c = rcol();
			circle (X,Y, 5, c);
			rworm (NULL, b, x,  y,  radius, 0.0f, tstep, dtheta, maxchilds, c);
		}
	}

	return (NULL);
}
/*
int	main (int argc, char **argv)
{
	char	*tgabuf;

	settopic_tc("FUCK YOU");
	init_tc(WIDTH, HEIGHT);
	start_tc();

	
	viewport.x = 0; 
	viewport.y = 0;
	viewport.x1 = WIDTH;
	viewport.y1 = HEIGHT;
	viewport.w = WIDTH;
	viewport.h = HEIGHT;
	viewport.mx = WIDTH/2;
	viewport.my = HEIGHT/2;


	tgabuf = (char *) malloc (1055360);
	init_backs ();
	
	tga_to_back ("texture2.tga", tgabuf, 1, 0, 0);
	
	hpf_back (1);
	
	contrast_back (1, 0x40);
	memset (tcono, 0x0, WIDTH*HEIGHT*4);	

		
	worm_back (1, 0.01f, 0.01f, 0.5f, 64);		// back, radius, tstep, dtheta, maxchilds
	
	while (!kbhit_tc())
	{
	
		blit_tc ();
	}

	kill_tc ();

	return (0);
}



*/









void init_controls (void)
{
	int	s,c;

	for (s=0; s<MAX_CONTROL_SETS; s++)
	{
		control_set[s].gain = 0.0f;
		control_set[s].value = 0.0f;
		control_set[s].cur_control = 0;

		for (c=0; c<MAX_CONTROLS; c++)
		{
			control_set[s].control[c].phase     = 0.0f;
			control_set[s].control[c].amplitude = 0.0f;
			control_set[s].control[c].frequency = 0.0f;
			control_set[s].control[c].phase_mod_data = 0.0f;
			control_set[s].control[c].amp_mod_data   = 0.0f;
			control_set[s].control[c].freq_mod_data  = 0.0f;
			control_set[s].control[c].mode = 0;
			control_set[s].control[c].data_point = 0;
			control_set[s].control[c].edit = 0;
		}
	}
}

float controller (controllerT *c)
{
	float a,f,p,d, v;
	
	if (fabs (c->amplitude) > 0.01)
	{
		d = data[c->data_point]/66000.0f;
		
		a = c->amplitude * (1.0f + c->amp_mod_data * d);
		f = c->frequency * (1.0f + c->freq_mod_data * d) * 1.5f;
		p = c->phase	 * (1.0f + c->phase_mod_data * d) * PI;
		if (f>0.0f)
		{
			v = (float)(a * sin (f*global_time + p));
		} else
		{
			v = (float)(a * sin (global_time/f + p));
		}

		c->value = v;
		return v;
	}
		
	c->value = 0.0f;

	return (0.0f);
}

void controller_set (controller_setT *s)
{
	int		c;
	float	v;

	v = 1.0f;

	for (c=0; c<MAX_CONTROLS; c++)
	{
		if (s->control[c].mode == 0) 
		{
			// Additive
			
			v += controller (&s->control[c]);
		} else
		{
			// Multiplicative

			v *= controller (&s->control[c]);
		}
	}

	s->value = v * s->gain;
}

void	calc_control_sets (void)
{
	int	s;

	for (s=0; s<MAX_CONTROL_SETS; s++)
	{
		controller_set (&control_set[s]);
	}
}


void	amp_box (int x, int y, int d, float v)
{
	unsigned char	c;

	c = (unsigned char)(127.0f * (v+1.0f));

	box (x,y, x+d*2, y+d, c);
	box (x,y, x+(d), y+(d>>1), 127);
	
}

void	edit_control_set (void)
{
	int	s,S,x,y;
	char str[50];

	box (100, 100, 620, 475,0xff);
	print ("edit mixer,", 105,100);
	
	y = 110;
	for (s=0; s<MAX_CONTROL_SETS; s+=8)
	{
		x = 110;
		for (S=s; S<s+8; S++)
		{
			if (S!=cur_set)
			{
				sprintf (str, "%d", S);
			} else
			{
				sprintf (str, "_%d_", S);
			} 
			
			print (str, x,y);
			amp_box (x+18,y+10, 15, control_set[S].value);
			amp_box (x+18,y+27, 15, control_set[S].gain);
			x+=65;
		}
		y+= 45;
	}
}

void	edit_control ()
{
	int		c,x,y,Y;
	char	str[50];

	box (100,90, 620, 285, 0xff);
	sprintf (str, "edit generator, channel %d,", cur_set);
	print (str, 100,95);
	amp_box(258, 95, 15, control_set[cur_set].value);

	y = 110;
	x = 110;
	for (c=0; c<MAX_CONTROLS; c++)
	{
		if (c!=control_set[cur_set].cur_control)
		{
			sprintf (str, "%d", c);
			print (str, x, y);
		} else	
		{
			sprintf (str, "_%d_", c);
			print (str, x, y);
			
			for (Y=y + (control_set[cur_set].control[c].edit) * 20 + 35; Y<y+150; Y++)
			{
				tcono[x+10+Y*WIDTH] = 0x000000;
			}

			switch (control_set[cur_set].control[c].edit)
			{
				case 0: sprintf (str, "amplitude"); break;
				case 1: sprintf (str, "phase"); break;
				case 2: sprintf (str, "frequency"); break;
				case 3: sprintf (str, "amp mod"); break;
				case 4: sprintf (str, "phase mod"); break;
				case 5: sprintf (str, "freq mod"); break;
				case 6: sprintf (str, "tap point: %d", control_set[cur_set].control[c].data_point); break;
				case 7: sprintf (str, "mode: %s", (control_set[cur_set].control[c].mode?"multiply":"add")); break;
			}
			
			print (str, x-10, y+155);
		} 
		
		amp_box (x+18, y+ 10, 15, control_set[cur_set].control[c].value);
		amp_box (x+18, y+ 30, 15, control_set[cur_set].control[c].amplitude);
		amp_box (x+18, y+ 50, 15, control_set[cur_set].control[c].phase);
		amp_box (x+18, y+ 70, 15, control_set[cur_set].control[c].frequency);
		amp_box (x+18, y+ 90, 15, control_set[cur_set].control[c].amp_mod_data);
		amp_box (x+18, y+110, 15, control_set[cur_set].control[c].phase_mod_data);
		amp_box (x+18, y+130, 15, control_set[cur_set].control[c].freq_mod_data);
		
		x += 65;
	}
}

void	edit_back (void)
{
	int		b,B,x,y,Y;
	char	str[50];
	unsigned char c;
	

	box (70,130, 500, 475, 0xff);
	sprintf (str, "edit layer,");
	print (str, 70,130);

	y = 130;
	x = 140;
		
	for (b=0; b<BACKS; b+=8)
	{
		x=140;
		for (B=b; B<b+8; B++)
		{
			if (B == cur_back)
			{
				sprintf (str, "_%d_", B);
			} else
			{
				sprintf (str, "%d", B);
			}
			print (str, x, y);
			x+=30;
		}
		y += 30;
	}
	
	// show current layer

	
	if (back[cur_back].filename)
	{
		sprintf (str, "layer %d [%s],", cur_back, back[cur_back].filename);
	}else
	{
		sprintf (str, "layer %d EMPTY", cur_back);
	}

	print (str, 100,300);
	
	for (x=0; x<160; x++)
	for (y=0; y<120; y++)
	{
		c = back[cur_back].buf[(x<<2)+(y<<2)*WIDTH];
		tcono[x+105+(y+350)*WIDTH] = rcmix (0xf0, 0x20, 0x50, c);
	}

	x=275;
	y=310;
			

	amp_box (x, y    , 15, back[cur_back].alpha + 0.5f*(control_set[back[cur_back].alpha_control].value + 1.0f) *
																	back[cur_back].alpha_control_amount);
	amp_box (x, y+ 20, 15, back[cur_back].alpha_threshold + 0.5f*(control_set[back[cur_back].alpha_threshold_control].value+1.0f) * 
																	back[cur_back].alpha_threshold_control_amount);
	amp_box (x, y+ 40, 15, back[cur_back].brightness + 0.5f*(control_set[back[cur_back].brightness_control].value+1.0f) * 
																	back[cur_back].brightness_control_amount);
	amp_box (x, y+ 60, 15, back[cur_back].alpha_control_amount);
	amp_box (x, y+ 80, 15, back[cur_back].alpha_threshold_control_amount);
	amp_box (x, y+100, 15, back[cur_back].brightness_control_amount);
	
	for (Y=y + 5 +(back[cur_back].edit) * 20; Y<y+120; Y++)
	{
		tcono[x-5+Y*WIDTH] = 0x000000;
	}

	switch (back[cur_back].edit)
	{
		case 0: sprintf (str, "alpha"); break;
		case 1: sprintf (str, "threshold"); break;
		case 2: sprintf (str, "brightness"); break;
		case 3: sprintf (str, "alpha mod"); break;
		case 4: sprintf (str, "threshold mod"); break;
		case 5: sprintf (str, "brighness mod"); break;
		case 6: sprintf (str, "alpha channel: %d", back[cur_back].alpha_control); break;
		case 7: sprintf (str, "threshold channel: %d", back[cur_back].alpha_threshold_control); break;
		case 8: sprintf (str, "brightness channel: %d", back[cur_back].brightness_control); break;
		default: 
		{
			b = back[cur_back].edit - 9;
			back[cur_back].cur_effect= b / (MAX_FX_PARAM);
			back[cur_back].cur_param = b - (back[cur_back].cur_effect * MAX_FX_PARAM);


			if (back[cur_back].cur_effect >= back[cur_back].effects)
			{
				back[cur_back].edit += MAX_FX_PARAM;
				sprintf (str, "next");
			} else
			{
				if (back[cur_back].cur_param < back[cur_back].effect[back[cur_back].cur_effect].params)
				{
					sprintf (str, "param mod %02.4f", back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount);
					print (str, x, y+145);
					sprintf (str, "osc %03d", back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control);
					print (str, x+130, y+145);
					sprintf (str, "effect %d _%s_, %s, %02.4f", 
							back[cur_back].cur_effect,
							back[cur_back].effect[back[cur_back].cur_effect].name,
							back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].name,
							back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value,
						    back[cur_back].cur_param);
					
				} else
				{
					sprintf (str, "next");
					back[cur_back].edit += (MAX_FX_PARAM - back[cur_back].cur_param);
				}
			}

			if (back[cur_back].edit >= MAX_EDIT_BACK + MAX_FX_PARAM*MAX_FX_PER_LAYER)
			{
				back[cur_back].edit = 0;
			}
		}break;
	}
			
	print (str, x, y+125);
	
}

void	edit_effect (void)
{
	int		b,B,x,y,Y;
	char	str[50];
	unsigned char c;
	

	box (70,130, 500, 475, 0xff);
	sprintf (str, "edit effect,");
	print (str, 70,130);

	y = 130;
	x = 140;
		
	for (b=0; b<BACKS; b+=8)
	{
		x=140;
		for (B=b; B<b+8; B++)
		{
			if (B == cur_back)
			{
				sprintf (str, "_%d_", B);
			} else
			{
				sprintf (str, "%d", B);
			}
			print (str, x, y);
			x+=30;
		}
		y += 30;
	}
	
	// show current layer with ONLY this effect

	
	if (back[cur_back].filename)
	{
		sprintf (str, "layer %d [%s],", cur_back, back[cur_back].filename);
	}else
	{
		sprintf (str, "layer %d EMPTY", cur_back);
	}

	print (str, 100,300);
	
	for (x=0; x<160; x++)
	for (y=0; y<120; y++)
	{
		back[cur_back].effect[back[cur_back].cur_effect].method (&back[cur_back].effect[back[cur_back].cur_effect],
																 &back[cur_back], &back[cur_back].buf[0]);
		
		c = back[cur_back].effect[back[cur_back].cur_effect].output[(x<<2)+(y<<2)*WIDTH];
		tcono[x+105+(y+350)*WIDTH] = rcmix (0x20, 0xf0, 0x50, c);
	}	
}

void	keypress (char key)
{
	switch (key)
	{
		case ' ': edit_mode = NONE; break;
		case 'm': edit_mode = CONTROL_SET; break;
		case 'g': edit_mode = CONTROL; break;
		case 'l': edit_mode = BACK; break;
		case '7': 
		{
			if (edit_mode == BACK) 
			{
				back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount 
																		-= globals.step;
				if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount < -1.0f)
				{
					back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount = -1.0f;
				}
			}
		} break;
		case '9':
		{
			if (edit_mode == BACK) 
			{
				back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount 
																		+= globals.step;
				if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount > 1.0f)
				{
					back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control_amount = 1.0f;
				}
			}
		} break;
		case '3':
		{
			if (edit_mode == BACK) 
			{
				back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control++;
				if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control >= MAX_CONTROL_SETS-1)
				{
					back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control = 0;
				}
			}
		} break;
		case '1':
		{
			if (edit_mode == BACK) 
			{
				back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control--;
				if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control < 0)
				{
					back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value_control = MAX_CONTROL_SETS-1;
				}
			}
		} break;

		case 's': globals.sensitivity--; break;
		case 'S': globals.sensitivity++; break;
		case 'o': globals.offset--; break;
		case 'O': globals.offset++; break;
		case 'b': globals.black--; break;
		case 'B': globals.black++; break;
		case 't': globals.tv_rate--; break;
		case 'T': globals.tv_rate++; break;
		case '/': globals.step *= 0.1f; break;
		case '*': globals.step *= 10.0f; break;
		case '[': globals.imoda--; break;
		case '{': globals.imoda++; break;
		case '}': globals.imodb--; break;
		case ']': globals.imodb++; break;

		case '6': 
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { cur_set++; if (cur_set>=MAX_CONTROL_SETS) cur_set=0; } break;
				case CONTROL: { control_set[cur_set].cur_control++; if (control_set[cur_set].cur_control>=MAX_CONTROLS)
																control_set[cur_set].cur_control = 0; } break;
				case BACK: { cur_back++; if (cur_back>=BACKS) cur_back=0; } break;
			}
		} break;
		case '4': 
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { cur_set--; if (cur_set<0) cur_set=MAX_CONTROL_SETS-1; } break;
				case CONTROL: { control_set[cur_set].cur_control--; if (control_set[cur_set].cur_control<0)
																control_set[cur_set].cur_control = MAX_CONTROLS-1; } break;
				case BACK: { cur_back--; if (cur_back<0) cur_back=BACKS; } break;
			}
		} break;
		case '8':
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { cur_set-=8; if (cur_set<0) cur_set=MAX_CONTROL_SETS-1; } break;
				case CONTROL: { 
								control_set[cur_set].control[control_set[cur_set].cur_control].edit--; 
								if (control_set[cur_set].control[control_set[cur_set].cur_control].edit < 0)
								{
									control_set[cur_set].control[control_set[cur_set].cur_control].edit = MAX_EDIT_CONT-1;
								}
							  } break;
				case BACK: {
								back[cur_back].edit--;
								if (back[cur_back].edit < 0)
								{
									back[cur_back].edit = MAX_EDIT_BACK+MAX_FX_PER_LAYER*MAX_FX_PARAM;
								}
						   } break;
				
			}
		} break;
		case '2':
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { cur_set+=8; if (cur_set>=MAX_CONTROL_SETS) cur_set=0; } break;
				case CONTROL: { 
								control_set[cur_set].control[control_set[cur_set].cur_control].edit++; 
								if (control_set[cur_set].control[control_set[cur_set].cur_control].edit >= MAX_EDIT_CONT)
								{
									control_set[cur_set].control[control_set[cur_set].cur_control].edit = 0;
								}
							  } break;
				case BACK: {
								back[cur_back].edit++;
								if (back[cur_back].edit > MAX_EDIT_BACK+MAX_FX_PER_LAYER*MAX_FX_PARAM)
								{
									back[cur_back].edit = 0;
								}
						   } break;
			}
		} break;
		case '=':
		case '+': 
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { control_set[cur_set].gain += globals.step; if (control_set[cur_set].gain>1.0f)
																		control_set[cur_set].gain = 1.0f; } break;
				case CONTROL: 
				{
					switch (control_set[cur_set].control[control_set[cur_set].cur_control].edit%8)
					{
						case 0: // Amplitude
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].amplitude += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].amplitude > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].amplitude = 1.0f;
							}
						} break;
						case 1: // Phase
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].phase += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].phase > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].phase = 1.0f;
							}
						} break;
						case 2: // frequency
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].frequency += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].frequency > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].frequency = 1.0f;
							}
						} break;
						case 3: // amp_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data = 1.0f;
							}
						} break;
						case 4: // phase_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data = 1.0f;
							}
						} break;
						case 5: // freq_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data += globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data > 1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data = 1.0f;
							}
						} break;
						case 6: // data_point
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].data_point ++;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].data_point >= DATA_SIZE)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].data_point = DATA_SIZE;
							}
						} break;
						case 7: // mode
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].mode =
								control_set[cur_set].control[control_set[cur_set].cur_control].mode ? 0 : 1;
						} break;
					} break; 
				} // CONTROL
				case BACK:
				{
					switch (back[cur_back].edit)
					{
						case 0: // alpha
						{
							back[cur_back].alpha += globals.step;
							if (back[cur_back].alpha > 1.0f) back[cur_back].alpha = 1.0f;
						} break;
						case 1: // alpha threshold
						{
							back[cur_back].alpha_threshold +=0.1f;
							if (back[cur_back].alpha_threshold > 1.0f) back[cur_back].alpha_threshold = 1.0f;
						} break;
						case 2: // brightness
						{
							back[cur_back].brightness += globals.step;
							if (back[cur_back].brightness > 1.0f) back[cur_back].brightness = 1.0f;
						} break;
						case 3: // alpha mod
						{
							back[cur_back].alpha_control_amount += globals.step;
							if (back[cur_back].alpha_control_amount > 1.0f) back[cur_back].alpha_control_amount = 1.0f;
						} break;
						case 4: // alpha threshold amount
						{
							back[cur_back].alpha_threshold_control_amount += globals.step;
							if (back[cur_back].alpha_threshold_control_amount > 1.0f) 
																	back[cur_back].alpha_threshold_control_amount = 1.0f;
						} break;
						case 5: // brightness amount
						{
							back[cur_back].brightness_control_amount += globals.step;
							if (back[cur_back].brightness_control_amount > 1.0f) back[cur_back].brightness_control_amount = 1.0f;
						} break;
						case 6: // alpha osc
						{
							back[cur_back].alpha_control ++;
							if (back[cur_back].alpha_control >= MAX_CONTROL_SETS) back[cur_back].alpha_control = 0;
						} break;
						case 7: // alpha_threshold osc
						{
							back[cur_back].alpha_threshold_control ++;
							if (back[cur_back].alpha_threshold_control >= MAX_CONTROL_SETS) back[cur_back].alpha_threshold_control = 0;
						} break;
						case 8: // brightness osc
						{
							back[cur_back].brightness_control ++;
							if (back[cur_back].brightness_control >= MAX_CONTROL_SETS) back[cur_back].brightness_control = 0;
						} break;
						default: // effect parameters
						{
							back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value += globals.step;
							if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value > 1.0f)
							{
								back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value = 1.0f;
							}
						} break;
						
					}
				}

			}	
		} break;
		case '_':
		case '-': 
		{
			switch (edit_mode)
			{
				case CONTROL_SET: { control_set[cur_set].gain -= globals.step; if (control_set[cur_set].gain<-1.0f)
																		control_set[cur_set].gain = -1.0f; } break;
				case CONTROL: 
				{
					switch (control_set[cur_set].control[control_set[cur_set].cur_control].edit%8)
					{
						case 0: // Amplitude
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].amplitude -= globals.step;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].amplitude < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].amplitude = -1.0f;
							}
						} break;
						case 1: // Phase
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].phase -= globals.step;;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].phase < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].phase = -1.0f;
							}
						} break;
						case 2: // frequency
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].frequency -= globals.step;;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].frequency < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].frequency = -1.0f;
							}
						} break;
						case 3: // amp_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data -= globals.step;;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].amp_mod_data = -1.0f;
							}
						} break;
						case 4: // phase_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data -= globals.step;;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].phase_mod_data = -1.0f;
							}
						} break;
						case 5: // freq_mod_data
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data -= globals.step;;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data < -1.0f)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data = -1.0f;
							}
						} break;
						case 6: // data_point
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].data_point --;
							if (control_set[cur_set].control[control_set[cur_set].cur_control].data_point < 0)
							{
								control_set[cur_set].control[control_set[cur_set].cur_control].freq_mod_data = 0;
							}
						} break;
						case 7: // mode
						{
							control_set[cur_set].control[control_set[cur_set].cur_control].mode =
								control_set[cur_set].control[control_set[cur_set].cur_control].mode ? 0 : 1;
						} break;
					};
				} break;  // CONTROL
				case BACK:
				{
					switch (back[cur_back].edit)
					{
						case 0: // alpha
						{
							back[cur_back].alpha -= globals.step;
							if (back[cur_back].alpha < -1.0f) back[cur_back].alpha = -1.0f;
						} break;
						case 1: // alpha threshold
						{
							back[cur_back].alpha_threshold -=0.1f;
							if (back[cur_back].alpha_threshold < -1.0f) back[cur_back].alpha_threshold = -1.0f;
						} break;
						case 2: // brightness
						{
							back[cur_back].brightness -= globals.step;
							if (back[cur_back].brightness < -1.0f) back[cur_back].brightness = -1.0f;
						} break;
						case 3: // alpha mod
						{
							back[cur_back].alpha_control_amount -= globals.step;
							if (back[cur_back].alpha_control_amount < -1.0f) back[cur_back].alpha_control_amount = -1.0f;
						} break;
						case 4: // alpha threshold amount
						{
							back[cur_back].alpha_threshold_control_amount -= globals.step;
							if (back[cur_back].alpha_threshold_control_amount < -1.0f) 
																	back[cur_back].alpha_threshold_control_amount = -1.0f;
						} break;
						case 5: // brightness amount
						{
							back[cur_back].brightness_control_amount -= globals.step;
							if (back[cur_back].brightness_control_amount < -1.0f) back[cur_back].brightness_control_amount = -1.0f;
						} break;
						case 6: // alpha osc
						{
							back[cur_back].alpha_control --;
							if (back[cur_back].alpha_control < 0) back[cur_back].alpha_control = MAX_CONTROL_SETS-1;
						} break;
						case 7: // alpha_threshold osc
						{
							back[cur_back].alpha_threshold_control --;
							if (back[cur_back].alpha_threshold_control < 0) back[cur_back].alpha_threshold_control = MAX_CONTROL_SETS-1;
						} break;
						case 8: // brightness osc
						{
							back[cur_back].brightness_control --;
							if (back[cur_back].brightness_control < 0) back[cur_back].brightness_control = MAX_CONTROL_SETS-1;
						} break;
						default: // effect parameters
						{
							back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value -= globals.step;
							if (back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value < -1.0f)
							{
								back[cur_back].effect[back[cur_back].cur_effect].param[back[cur_back].cur_param].value = -1.0f;
							}
						} break;
						
					}
				} break; // BACK
			}
		} break;
	}

	if (globals.sensitivity > 15) globals.sensitivity=15;
	if (globals.tv_rate > 7) globals.tv_rate = 7;
	if (globals.imoda == 0) globals.imoda = 1;
	if (globals.imodb == 0) globals.imodb = 1;
	if (globals.step < 0.000001f) globals.step = 0.00001f;
	if (globals.step > 1.0f) globals.step = 1.0f;
}

void do_backs (char even)
{
	unsigned char	b;
	char			e;
	unsigned char	c, thresh, bri;
#ifdef ALPHA
	unsigned char  alp;
#endif
	short			*d;
	unsigned long	*bufp, *ob;
	unsigned char	*backp;


	if (!back[BACKS-1].active) memset (tcono, 0, WIDTH*HEIGHT*4);
	for (b=BACKS-1; b>0; b--) if (back[b].active)
	{
		
			thresh = (unsigned char)(127*(back[b].alpha_threshold + 0.5f*(control_set[back[b].alpha_threshold_control].value+1.0f) * 
																	back[b].alpha_threshold_control_amount+1.0f));
#ifdef ALPHA
			alp = (unsigned char)(127*(back[b].alpha + 0.5f*(control_set[back[b].alpha_control].value + 1.0f) *
																	back[b].alpha_control_amount+1.0f));
#endif
			bri = (unsigned char)(127*(back[b].brightness + 0.5f*(control_set[back[b].brightness_control].value+1.0f) * 
																	back[b].brightness_control_amount + 1.0f));
			

#ifdef ALPHA
			if (alp < 250)
#endif
			{
				// Do effects
				e = back[b].effects-1;
				if (e >= 0)
				{
					back[b].effect[e].method (&back[b].effect[e], &back[b], back[b].buf);

					if (e > 0)
					for (e=back[b].effects-2; e<=0; e++)
					{
						back[b].effect[e].method (&back[b].effect[e], &back[b], &back[b].effect[e+1].output[0]);
					}	

					if (even)
					{
						backp = &back[b].effect[0].output[0];
					} else
					{
						backp = &back[b].effect[0].output[WIDTH*HEIGHT-1];
					}
				} else
				{
					if (even)
					{
						backp = &back[b].buf[0];
					} else
					{
						backp = &back[b].buf[WIDTH*HEIGHT-1];
					}
				}

			
				if (even)
				{
					bufp = tcono;
					d = data;
					while (bufp < tcono+WIDTH*HEIGHT)
					{
						ob = bufp + WIDTH;
						if ( (abs (*(d+=globals.tv_rate)>>(globals.sensitivity)) + globals.offset) > globals.black)
						{
							while (bufp < ob)
							{
								c = (unsigned char)( ((*backp) * bri) >> 8);
					
								if (c > thresh)
								{
									*bufp = (unsigned long) (ALPHA_MIX);
								}
		
								bufp++;
								backp++;
							}
							backp += WIDTH;
							bufp += WIDTH;
						} else
						{
							backp += WIDTH<<1;
							bufp += WIDTH<<1;
						}
						
					}
				} else
				{
					bufp = tcono+WIDTH*HEIGHT-1;
					d = &data[HEIGHT*2-1];
					while (bufp > tcono)
					{
						ob = bufp - WIDTH;
						if ( (abs (*(d-=globals.tv_rate)>>(globals.sensitivity)) + globals.offset) > globals.black)
						{
							while (bufp > ob)
							{
								c = (unsigned char)( ((*backp) * bri) >> 8);
					
								if (c > thresh)
								{
									*bufp = (unsigned long) (ALPHA_MIX);
								}
		
								bufp--;
								backp--;
							}
							backp -= WIDTH;
							bufp -= WIDTH;
						} else
						{
							backp -= WIDTH<<1;
							bufp -= WIDTH<<1;
						}
					}
				}
			}
	}
}

void	do_timecode ()
{
	static char dummy[500];
	static float otime;
	short x, y;

	box (0,10, 640,35, 0xff);		

	x = (short)((global_time / 2) + 80);
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0x1010ff;
	for (x=80; x<620; x+= 8)
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0xefffef;			
	for (x=80; x<620; x+= 15)
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0xe0f0e0;			
	for (x=80; x<620; x+= 30)
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0xa0e0a0;
	for (x=80; x<620; x+= 60)
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0x90a090;
	for (x=81; x <(short)((global_time / 2) + 80); x++)
	for (y=10; y<35; y++) tcono[x+y*WIDTH] = 0xa0a0f0;		

	print (f2s (dummy, global_time), 15,13);
	print (f2s (dummy, 2.0f/(global_time - otime)), 550,13);

	box (0,HEIGHT-30, WIDTH, HEIGHT-1, 0xff);
	sprintf (dummy, "_s_ens,%03d  _o_ffset,%03d  _b_lack,%03d  _t_v.rate,%03d imoda,%03d _%03d_ imodb,%03d _%03d_ step,%02.5f", 
						globals.sensitivity, globals.offset, globals.black,
						globals.tv_rate, globals.imoda, frame%globals.imoda, globals.imodb, frame%globals.imodb,
						globals.step);
	print (dummy, 0, HEIGHT-25);

	otime = global_time;
}

void	explode_i2a (i2aT *ai)
{
	int	s, p;
	static unsigned char i;
	VECTOR rvec;
	float	rang;
	MATRIX	rmat;

	for (s=(int)(30 * (1.0f+sin (global_time*7.0f))); s<ai->shapes; s+=13)
	{
		rvec.x = (float)sin((data[s%(DATA_SIZE)]+120.0*sin(global_time)) /15.0 );
		rvec.z = (float)cos((data[s%(DATA_SIZE)]+250.0*sin(global_time)) /32.0 );
		rvec.y = (float)cos((data[s%(DATA_SIZE)]+190.0*cos(global_time) ) /10.0 );
		normalizeVector (&rvec);
		rang = (float)(sin(global_time*0.07));
		createRotationMatrix (&rmat, &rvec, rang);

		for (p=0; p<ai->shape[s]->points; p++)
		{
			ai->shape[s]->point[p].x = ai->shape[s]->opoint[p].x;
			ai->shape[s]->point[p].y = ai->shape[s]->opoint[p].y;
			ai->shape[s]->point[p].z = ai->shape[s]->opoint[p].z;
 			vectorMatrixRot (&ai->shape[s]->point[p], &rmat);
		}
	}
}

void	restore_i2a (i2aT *ai)
{
	int	s,p;
	
	for (s=0; s<ai->shapes; s++)
	for (p=0; p<ai->shape[s]->points; p++)
	{
		ai->shape[s]->point[p].x = ai->shape[s]->opoint[p].x;
		ai->shape[s]->point[p].y = ai->shape[s]->opoint[p].y;
		ai->shape[s]->point[p].z = ai->shape[s]->opoint[p].z;
	}		
}
	

void	render (unsigned char a, unsigned char b, char even)
{
	unsigned char	c;
	short			*d;
	unsigned long	*tc, *otc, *iotc;

	if (even)
	{
		d = data;
		tc=tcono;
		while (tc < &tcono[WIDTH*HEIGHT])
		{	
			c= (unsigned char)(abs((*(d+=globals.tv_rate)) >> 6));
			if (c>globals.black)
			{
				otc = tc+WIDTH;
				while ((tc < otc)) 
				{
		
					iotc = tc;
					while (((tc-iotc)<(a+3))&&(tc < otc))
					{
						*tc = cmix ((unsigned char)(*tc), c);
						tc++;
					}
					if (c>globals.black) c--;
				}
			} else
			{
				otc = tc+WIDTH;
				while (tc < otc)
				{
					*(tc++) = (c << 16) + (c << 8) + c;
				}
			}
			tc+=WIDTH;
		}
	} else
	{
		d = data+HEIGHT*2-1;
		tc=tcono+WIDTH*HEIGHT-1;
		while (tc > tcono)
		{	
			c= (unsigned char)(abs((*(d-=globals.tv_rate)) >> 6));
			if (c>globals.black)
			{
				otc = tc-WIDTH;
				while ((tc > otc)) 
				{
		
					iotc = tc;
					while (((iotc-tc)<(b+3))&&(tc > otc))
					{
						*tc = cmix ((unsigned char)(*tc), c);
						tc--;
					}
					if (c>globals.black) c--;
				}
			} else
			{
				otc = tc-WIDTH;
				while (tc > otc)
				{
					*(tc--) = (c << 16) + (c << 8) + c;
				}
			}
			tc-=WIDTH;
		}
	}
}
	

void	init (void)
{
	if (BASS_GetVersion()!=MAKELONG(0,8)) 
	{
		printf ("Bass not loaded - or incorrect virgin\n");
		exit (-1);
	}

	if (!BASS_Init(-1,44100,BASS_DEVICE_NOSYNC,GetForegroundWindow())) 
	{
		printf ("Can't get it up\n");
		exit (-1);
	}

	globals.sensitivity = 1;
	globals.offset = 0;
	globals.black = 20;

	globals.tv_rate=2;
	DATA_SIZE= HEIGHT * globals.tv_rate;
	data = (short *) malloc (sizeof (short) * MAX_TV_RATE * HEIGHT);

	globals.imoda = 19;
	globals.imodb = 15;

	globals.step = 0.01f;

	BASS_Start();	
	init_backs ();
	init_controls ();
	init_font ();

	settopic_tc("FUCK YOU");
	init_tc(WIDTH, HEIGHT);
	start_tc();

	viewport.x = 0; 
	viewport.y = 0;
	viewport.x1 = WIDTH;
	viewport.y1 = HEIGHT;
	viewport.w = WIDTH;
	viewport.h = HEIGHT;
	viewport.mx = WIDTH/2;
	viewport.my = HEIGHT/2;	
}

void init_frame ()
{
	position = BASS_ChannelGetPosition (str);
	if (position >= (unsigned)MAX_POSITION) BASS_ChannelSetPosition (str, 0);
	global_time =  position / (88200.0f*2.0f);
}

void	gui (void)
{
	if (timecode) do_timecode ();	
	switch (edit_mode)
	{
		case NONE:				break;
		case CONTROL_SET: edit_control_set(); break;
		case CONTROL: edit_control (); break;	
		case BACK: edit_back(); break;
	}
}

void	keys (void)
{
	char	key;
	char	b,a,bbri;
	float	maxbri;

	back[0].active = 1;
	for (b=1; b<BACKS; b++) back[b].active = 0;

	
	for (a=0; a<MAX_ACTIVE; a++)
	{
		maxbri = -1.0f;
		bbri = 0;
		for (b=0; b<BACKS; b++) if (back[b].used)
		{
			if ((!back[b].active)&&(back[b].brightness > maxbri))
			{
				maxbri = back[b].brightness;
				bbri = b;
			}
		}
		back[bbri].active = 1;
	}

	if (kbhit_tc())
	{
		key=getch_tc();
		if (key==27) DONE=1; else
		if (key=='t') timecode?timecode=0:timecode=1; else
		if (key=='p') 
		{
			if (!paused) 
			{
				BASS_ChannelPause (str);
				paused = 1;
			} else
			{
				BASS_ChannelResume (str);
				paused = 0;
			} 
		} else
		if (key=='r') BASS_ChannelSetPosition(str, 0); else
		if ((key==',')&&(position>pstep)) 
		{ 
			direction = 1; 
			if (paused) BASS_ChannelResume (str);
			paused = 0;
			BASS_ChannelSetPosition(str, position - pstep); 
		} else
		if ((key=='.')&&(position<MAX_POSITION-pstep)) 
		{ 
			direction = 1; 
			if (paused) BASS_ChannelResume (str);
			paused = 0;
			BASS_ChannelSetPosition(str, position + pstep); 
		} else
		keypress (key); 
	} else
	{
		direction=0;
	}
	if (!direction)  pstep = 22100;  else 
	{
		if (pstep < (unsigned)(MAX_POSITION >> 3)) pstep = (unsigned long)(pstep*1.1f);
	}
}

void field (char f)
{
	init_frame ();
	do_backs(f);		
	render ((unsigned char)frame%globals.imoda, (unsigned char)frame%globals.imodb, f);
	frame++;
	gui();
	blit_tc();	
}


void add_effect (effectT *e)
{
	if (effects < MAX_FX-1)
	{
		effect[effects++] = e;
	}
}




int	main (int argc, char **argv)
{
	int				i;
	char			*tgabuf;
	i2aT			*ai;
	char			name[255]; 

	init ();

	str = BASS_StreamCreateFile(FALSE, "elran.mp3", 0, 0, BASS_MP3_SETPOS);
	
	add_effect (new_effect (fxinit_crop));
	memcpy ((void *)&back[3].effect[0], (void *)effect[0], sizeof (effectT));
	back[3].effects++;

	tgabuf = (char *) malloc (WIDTH*HEIGHT*4);
	
	for (i=2;i<BACKS; i++)
	{
		sprintf (name, "texture%d.tga", i-1);
		tga_to_back (name, tgabuf, i, 0, 0);
	}

	free (tgabuf);
	
	ai = load_i2a ("boxers.i2a");

	MAX_POSITION = BASS_StreamGetLength (str);
	BASS_StreamPlay(str, 0,0);
	
	while (!DONE)
	{	
		BASS_ChannelGetData (str, data, sizeof(short)*DATA_SIZE);
	
		calc_control_sets ();
		keys();

		show_i2a (ai,1);
		explode_i2a (ai);

		field (1);
		field (0);
	}

	kill_tc();
	
	return (0);
}



