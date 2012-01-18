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
