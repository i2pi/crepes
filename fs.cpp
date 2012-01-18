#include <string.h>


char	*mread (void *dest, int count, int size, char *src)
{
	memcpy (dest, src, count * size);
	return (src+(count*size));
}