// from minifmod
typedef void (*SAMPLELOADCALLBACK)(void *buff, int lenbytes, int numbits, int instno, int sampno);

extern "C" {
 void init_soundpack(char * filename,SAMPLELOADCALLBACK slc);
 void init_sound(char * filename);
 void start_sound(void);
 void stop_sound(void);
 int getpos_sound(void);
 int gettime_sound();
 void setpos_sound(int);
 void setvol_sound(int vol);
 void setamp_sound(int amp);
}
