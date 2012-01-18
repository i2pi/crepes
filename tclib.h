#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* general function names */

 extern unsigned long int *tcono;

 void init_tc(int x,int y);

 void start_tc(void);

 void blit_tc(void);

 void kill_tc(void);

 void settopic_tc(char *);
 void usewindow_tc(int);

/* keyboard functions */
 int getch_tc(void);
 int kbhit_tc(void);
 int keystate_tc(int);        /* win32 only atm */

/* mouse stuff */
 int mousex_tc(void);         /* also win32 only atm */
 int mousey_tc(void);         /* also win32 only atm */
 int mouseb_tc(void);         /* also win32 only atm */

/************************************************************************/
/* Pragmas sealed incase of any other compiler using aux */

#ifdef __cplusplus
};
#endif


