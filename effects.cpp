#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "elran.h"

/*
typedef struct 
{
	float	value;
	char	value_control;
	float	value_control_amount;
	char	name[64];
} fxparamT;

typedef struct _effectT
{
	char			name[128];
	enum LAYER_TYPE	layer_type;
	char			output[WIDTH*HEIGHT];
	void			*data;
	void			(*init)(struct _effectT *);
	void			(*method)(struct _effectT *, struct _backT *, unsigned char *);
	char			params;
	fxparamT		param[MAX_FX_PARAM];
} effectT;
*/

effectT	*new_effect (void (*fxinit)(struct _effectT *))
{
	effectT	*e;

	e = (effectT *) malloc (sizeof (effectT));

	e->layer_type = 0;
	e->data = NULL;
	e->init = fxinit;
	e->method = NULL;
	e->params = 0;

	fxinit(e);

	printf ("Loaded effect '%s'\n", e->name);

	return (e);
}


void	fxmethod_crop (effectT *e, backT *b, unsigned char *buf)
{
	int	x,y;

	memset (e->output, 0x00, WIDTH*HEIGHT);

	for (x=(int)((e->param[0].value+1.0f)* (WIDTH>>1)); x<(int)((e->param[2].value+1.0f)* (WIDTH>>1)); x++)
	for (y=(int)((e->param[1].value+1.0f)*(HEIGHT>>1)); y<(int)((e->param[3].value+1.0f)*(HEIGHT>>1)); y++)
	{
		e->output[x+y*WIDTH] = buf[x+y*WIDTH];
	}
	
}

void	fxinit_crop (effectT *e)
{
	sprintf (e->name, "crop");
	e->layer_type = I2A_LAYER | TGA_LAYER;
	e->method = fxmethod_crop;	
	e->params = 4;

	sprintf (e->param[0].name, "top x");
	e->param[0].value = -1.0f;
	e->param[0].value_control = 0;
	e->param[0].value_control_amount = 0.0f;

	sprintf (e->param[1].name, "top y");
	e->param[1].value = -1.0f;
	e->param[1].value_control = 0;
	e->param[1].value_control_amount = 0.0f;

	sprintf (e->param[2].name, "bottom x");
	e->param[2].value = 1.0f;
	e->param[2].value_control = 0;
	e->param[2].value_control_amount = 0.0f;

	sprintf (e->param[3].name, "bottom y");
	e->param[3].value = 1.0f;
	e->param[3].value_control = 0;
	e->param[3].value_control_amount = 0.0f;

	e->cur_param = 0;
}