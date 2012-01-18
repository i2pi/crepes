#ifndef MATH__H
#define MATH__H

#define PI					3.1415926537f
#define MAXMESH				600
#define MAXFRAMES			256

typedef struct
{
	float	x, y, z;
} VECTOR;

struct MATRIX
{
	float a,b,c, d,e,f, g,h,i ,x,y,z;
};

typedef struct
{
	int	a,b,c;
} triT;

typedef struct
{
	VECTOR	*mpoint;		// Matrixed points
	VECTOR	*tpoint;
	VECTOR	*point;
	int		points;
	triT	*triangle;
	int		triangles;
} meshT;

typedef struct
{
	VECTOR	rvec;
	float	angle;
	VECTOR	position;
	float	scale;
	VECTOR	light;
	float	bright;
	unsigned long	time;
} cameraT;

typedef struct
{
	meshT	*mesh[MAXMESH];
	int		meshes;	
	cameraT	keyframe[MAXFRAMES];
	int		keyframes;
} sceneT;

void normalizeVector(VECTOR * v);
void createRotationMatrix(MATRIX * m,VECTOR * v,float a);
void scaleMatrix (MATRIX *m, float a);
void vectorMatrixRot(VECTOR * v,MATRIX * m);
void vectorMatrixTrans(VECTOR * v,MATRIX * m);
void vectorMatrixMul(VECTOR * v,MATRIX * m);

#endif
