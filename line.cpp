#include "elran.h"
#include "colour.h"
#include "line.h"

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

	viewport.x = 15; 
	viewport.y = 15;
	viewport.x1 = SWIDTH-15;
	viewport.y1 = SHEIGHT-15;
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

