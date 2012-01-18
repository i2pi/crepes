#include "jmath.h"
#include <math.h>


	 
void normalizeVector(VECTOR * v)
{
	float len=(float)sqrt((v->x*v->x)+(v->y*v->y)+(v->z*v->z));
	if (len<0.0000001)
		return;
	
	len=1.0f/len;
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

