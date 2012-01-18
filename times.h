#define MAX_SECTIMES	50


typedef struct 
{
	float	time[MAX_SECTIMES];
	unsigned char rank[MAX_SECTIMES];
	int	num;
	int	marker;
} secT;

float	times[85];
secT	rowantimes;
secT	dantimes;


void sectime (secT *st, int num, int min, float sec, unsigned char rank)
{
	st->time[num] = min*60.0f + sec;
	st->rank[num] = rank;
	st->num = num+1;
	st->marker = 0;
}

void init_times ()
{
times[ 0]=9.382789f;
times[ 1]=15.682336f;
times[ 2]=23.272835f;
times[ 3]=30.486303f;
times[ 4]=55.975739f;
times[ 5]=62.465443f;
times[ 6]=73.169640f;
times[ 7]=82.072426f;
times[ 8]=90.975189f;
times[ 9]=94.779045f;
times[10]=117.571205f;
times[11]=124.675354f;
times[12]=140.664215f;
times[13]=151.367645f;
times[14]=171.272720f;
times[15]=192.670822f;
times[16]=213.570969f;
times[17]=229.655670f;
times[18]=231.365509f;
times[19]=249.166412f;
times[20]=261.556519f;
times[21]=272.170654f;
times[22]=306.055511f;
times[23]=334.462250f;
times[24]=338.059967f;
times[25]=344.355499f;
times[26]=346.915985f;
times[27]=357.452820f;
times[28]=361.959961f;
times[29]=363.050507f;
times[30]=364.760681f;
times[31]=370.950104f;
times[32]=386.550171f;
times[33]=388.758362f;
times[34]=400.247620f;
times[35]=409.746490f;
times[36]=410.938904f;
times[37]=417.953735f;
times[38]=419.344543f;
times[39]=424.552643f;
times[40]=429.147858f;
times[41]=431.657288f;
times[42]=441.156403f;
times[43]=441.953278f;
times[44]=442.852661f;
times[45]=443.737701f;
times[46]=445.547485f;
times[47]=446.446136f;
times[48]=447.342224f;
times[49]=448.254150f;
times[50]=449.155731f;
times[51]=454.450745f;
times[52]=455.353302f;
times[53]=456.351105f;
times[54]=458.949158f;
times[55]=459.847534f;
times[56]=460.646271f;
times[57]=462.545715f;
times[58]=462.644226f;
times[59]=462.840729f;
times[60]=463.049469f;
times[61]=464.245300f;
times[62]=465.046539f;
times[63]=467.951080f;
times[64]=473.447205f;
times[65]=478.447021f;
times[66]=480.251434f;
times[67]=482.046906f;
times[68]=487.348846f;
times[69]=488.246307f;
times[70]=489.146484f;
times[71]=489.947174f;
times[72]=490.146667f;
times[73]=490.246521f;
times[74]=497.937775f;
times[75]=499.742676f;
times[76]=501.538177f;
times[77]=518.247803f;
times[78]=525.649414f;
times[79]=526.336426f;
times[80]=528.130981f;
times[81]=528.637817f;
times[82]=529.730530f;
times[83]=530.445923f;
times[84]=551.139160f;

sectime (&rowantimes, 0, 1, 45.882f, 100);
sectime (&rowantimes, 1, 2, 46.875f, 99);
sectime (&rowantimes, 2, 2, 47.514f, 98);
sectime (&rowantimes, 3, 2, 48.100f, 97);
sectime (&rowantimes, 4, 3, 21.012f, 45);
sectime (&rowantimes, 5, 4, 4.776f,  70);
sectime (&rowantimes, 6, 4, 19.939f, 30);
sectime (&rowantimes, 7, 4, 45.996f, 96);
sectime (&rowantimes, 8, 5, 45.641f, 25);
sectime (&rowantimes, 9, 6, 2.431f,  50);
sectime (&rowantimes, 10, 6, 11.955f, 96);
sectime (&rowantimes, 11, 7, 22.927f, 10);
sectime (&rowantimes, 12, 8, 33.059f, 5);
sectime (&rowantimes, 13, 8, 37.051f, 15);
sectime (&rowantimes, 14, 8, 47.952f, 95);
sectime (&rowantimes, 15, 8, 55.730f, 94);

sectime (&dantimes, 0, 0, 16.532f, 10); 
sectime (&dantimes, 1, 1, 20.2f, 17);
sectime (&dantimes, 2, 0, 26.690f, 5);
sectime (&dantimes, 3, 0, 47.055f, 8);
sectime (&dantimes, 4, 1, 4.249f, 30);
sectime (&dantimes, 5, 1, 23.559f, 20);
sectime (&dantimes, 6, 1, 52.256f, 2);
sectime (&dantimes, 7, 1, 56.263f, 35);
sectime (&dantimes, 8, 2, 2.903f, 22);
sectime (&dantimes, 9, 2, 50.933f, 40);
sectime (&dantimes, 10, 3, 3.347f, 45);
sectime (&dantimes, 11, 3, 13.146f, 70);
sectime (&dantimes, 12, 3, 16.162f, 35);
sectime (&dantimes, 13, 3, 36.158f, 29);
sectime (&dantimes, 14, 4, 4.230f, 56);
sectime (&dantimes, 15, 4, 10.97f, 6);
sectime (&dantimes, 16, 4, 16.238f, 62);
sectime (&dantimes, 17, 4, 25.147f, 57);
sectime (&dantimes, 18, 4, 42.496f, 65);
sectime (&dantimes, 19, 5, 26.940f, 80);
sectime (&dantimes, 20, 5, 34.926f, 95);
sectime (&dantimes, 21, 5, 36.162f, 69);
sectime (&dantimes, 22, 5, 57.863f, 82);
sectime (&dantimes, 23, 6, 11.809f, 51);
sectime (&dantimes, 24, 6, 49.121f, 89);
sectime (&dantimes, 25, 6, 59.79f, 67);
sectime (&dantimes, 26, 7, 10.913f, 39);
sectime (&dantimes, 27, 7, 31.7f, 100);
sectime (&dantimes, 28, 7, 52.244f, 74);
sectime (&dantimes, 29, 8, 5.587f, 29);
sectime (&dantimes, 30, 8, 51.231f, 9);

}