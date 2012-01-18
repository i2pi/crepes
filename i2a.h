#ifndef I2A__H
#define I2A__H

#include "jmath.h"

#define MAX_SHAPES	2000

typedef struct
{
	int		points;
	VECTOR	*point;
	VECTOR	*opoint;
} shapeT;

typedef struct
{
	int		shapes;
	shapeT	*shape[MAX_SHAPES];
} i2aT;

i2aT	*load_i2a_file (char *filename);
i2aT	*load_i2a (char *file, unsigned long size);
void	show_i2a (i2aT *a, unsigned long col, unsigned char thick);
void	rotate_i2a (i2aT *a, MATRIX *m);
void	explode_i2a (i2aT *ai);
void	zoom_i2a (i2aT *ai, float a, float b, float c);
void	spin_i2a (i2aT *ai, float a, float b, float c, float d);
void	restore_i2a (i2aT *ai);

#endif