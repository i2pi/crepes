#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "jmath.h"
#include "line.h"
#include "i2a.h"
#include "elran.h"
#include "fs.h"

i2aT	*load_i2a_file (char *filename)
{
	FILE	*fp;
	i2aT	*a = NULL;
	int		P, p, s;

	fp = fopen (filename, "rb");

	if (fp==NULL)
	{
		perror (filename);
		exit (-3);
	}

	a = (i2aT *) malloc (sizeof (i2aT));

	a->shapes = 0;

	while (!feof (fp))
	{
		s = a->shapes++; 
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
		}
		if (s >= MAX_SHAPES-2) 
		{
			goto done;
		}
	}
done:;
	a->shapes--;

	for (s=0; s<a->shapes-1; s++)
	for (p=0; p<a->shape[s]->points; p++)
	{
		a->shape[s]->opoint[p].x = a->shape[s]->point[p].x;
		a->shape[s]->opoint[p].y = a->shape[s]->point[p].y;
		a->shape[s]->opoint[p].z = a->shape[s]->point[p].z;

	}

	return (a);
}

i2aT	*load_i2a (char *file, unsigned long size)
{
	i2aT	*a = NULL;
	int		P, p, s;
	unsigned long b;

	a = (i2aT *) malloc (sizeof (i2aT));

	a->shapes = 0;
	
	b = 0;

	while (b < size)
	{
		s = a->shapes++; 
		//fread (&P, 1, sizeof (int), fp);
		file = mread (&P, 1, sizeof (int), file);
		b += sizeof (int);
		a->shape[s] = (shapeT *) malloc (sizeof (shapeT));
		a->shape[s]->points = P;
		a->shape[s]->point = (VECTOR *) malloc (sizeof (VECTOR) * a->shape[s]->points);
		a->shape[s]->opoint = (VECTOR *) malloc (sizeof (VECTOR) * a->shape[s]->points);
		for (p=0; p<P; p++)
		{
			//fread (&a->shape[s]->point[p].x, sizeof (float), 1, fp);
			file = mread (&a->shape[s]->point[p].x, sizeof (float), 1, file);
			b += sizeof (float);
			//fread (&a->shape[s]->point[p].y, sizeof (float), 1, fp);
			file = mread (&a->shape[s]->point[p].y, sizeof (float), 1, file);
			b += sizeof (float);
			//fread (&a->shape[s]->point[p].z, sizeof (float), 1, fp);
			file = mread (&a->shape[s]->point[p].z, sizeof (float), 1, file);
			b += sizeof (float);
		}
		if (s >= MAX_SHAPES-2) 
		{
			goto done;
		}
	}
done:;
	a->shapes--;

	for (s=0; s<a->shapes-1; s++)
	for (p=0; p<a->shape[s]->points; p++)
	{
		a->shape[s]->opoint[p].x = a->shape[s]->point[p].x;
		a->shape[s]->opoint[p].y = a->shape[s]->point[p].y;
		a->shape[s]->opoint[p].z = a->shape[s]->point[p].z;

	}

	return (a);
}


void	show_i2a (i2aT *a, unsigned long col, unsigned char thick)
{
	int	s,p,S;

	if (a->shapes < 150) S = 1; else
	if (a->shapes < 400) S = 4; else
	if (a->shapes < 1000) S = 20; else S=50;
	
	for (s=0; s<a->shapes-1; s+=S)
	{
		for (p=0; p<a->shape[s]->points-1; p++)
		{
			dline (&a->shape[s]->point[p], &a->shape[s]->point[p+1], col, thick);
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


void	zoom_i2a (i2aT *ai, float a, float b, float c)
{
	int	s, p, S;
	static unsigned char i=0;

	if (ai->shapes < 150) S = 1; else
	if (ai->shapes < 400) S = 4; else
	if (ai->shapes < 1000) S = 20; else S=50;
	
	for (s=0; s<ai->shapes; s+=S)
	{
		for (p=0; p<ai->shape[s]->points; p++)
		{	
			i++;
		
			ai->shape[s]->point[p].x = ai->shape[s]->opoint[p].x;
			ai->shape[s]->point[p].y = ai->shape[s]->opoint[p].y;
			ai->shape[s]->point[p].z = (float) (sin(get_global_time()/((float)((s+c)/a)))*b);
		}
	}
}


void	spin_i2a (i2aT *ai, float a, float b, float c, float d)
{
	int	s, p,S;
	static unsigned char i;
	VECTOR rvec;
	float	rang;
	MATRIX	rmat;
	float	gt = get_global_time();

	rvec.x = (float)sin(PI*sin(gt/a));
	rvec.z = (float)cos(PI*sin(gt/b));
	rvec.y = (float)cos(PI*cos(gt/c));
	normalizeVector (&rvec);
	rang = d;
	createRotationMatrix (&rmat, &rvec, rang);

	if (ai->shapes < 150) S = 1; else
	if (ai->shapes < 400) S = 4; else
	if (ai->shapes < 1000) S = 20; else S=50;

	for (s=0; s<ai->shapes; s+=S)
	{
		
		for (p=0; p<ai->shape[s]->points; p++)
		{
			ai->shape[s]->point[p].x = ai->shape[s]->point[p].x;
			ai->shape[s]->point[p].y = ai->shape[s]->point[p].y;
			ai->shape[s]->point[p].z = ai->shape[s]->point[p].z;
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

