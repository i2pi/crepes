// #define TC_NODD

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#ifndef __GNUC__
#include <ddraw.h>
#endif

// visual C++ define
#ifdef _MSC_VER
 #define __VISUALC__
#endif

#define __ASMOPT__

// virtual buffer
unsigned long int *tcono;

// videomode used
static int videomode;
// modelist               width height bpp  avail..!
static int modelist[100]={
                          320  ,200   ,32  ,0,
                          320  ,200   ,16  ,0,

                          320  ,240   ,32  ,0,
                          320  ,240   ,16  ,0,

                          512  ,384   ,32  ,0,
                          512  ,384   ,16  ,0,

                          640  ,480   ,32  ,0,
                          640  ,480   ,16  ,0,

                          800  ,600   ,16  ,0,
                          800  ,600   ,32  ,0,

                          0    ,0     ,0   ,0};

/*
*/

static HCURSOR cursor;

// resizable window?
static int resizable=0;

static char * windowtopic="(truecolor shit)";

static void memset_tc(void * d,char b,int c)
 {
  int l;
  for (l=0;l<c;l++)
   ((char *)d)[l]=b;
 }

// directdraw
#ifndef __GNUC__
#ifndef TC_NODD
static  int use_dd_tc=1;
#endif
#ifdef TC_NODD
static  int use_dd_tc=0;
#endif
  static LPDIRECTDRAW m_dd;
  static DDSURFACEDESC m_desc;
  static LPDIRECTDRAWSURFACE m_surf;

// correct pixelformat?
  static int fastdraw=0;

  static int Rmask;
  static int Gmask;
  static int Bmask;
  static int Rpos;
  static int Gpos;
  static int Bpos;

  static int needclear=1;

#else
static  int use_dd_tc=0;
#endif


// for watcom
#ifndef BI_BITFIELDS
#define BI_BITFIELDS	(3L)
#endif

// misc mousestuff
static int mouse_xc=0;
static int mouse_yc=0;
static int mouse_bs=0;

static char keystate[256];

// window proc
static LRESULT CALLBACK wtc_proc(HWND hwnd,unsigned int iMsg,WPARAM parm1,LPARAM parm2);
// keyboard buffers
static int wtc_bufferp=0;
static char wtc_buffer[200];

// misc gdi info.
static HWND wtc_handle;
static HINSTANCE wtc_instance;
static HDC wtc_hdc;
static HBITMAP wtc_obj;
static char wtc_windowname[]="trueworl";
static WNDCLASS wnd;

// misc gdi info
typedef struct {
 BITMAPINFOHEADER bi;
 int r;
 int g;
 int b;
} WTC_BITMAP;
static WTC_BITMAP bitmap;

static int width;
static int height;

#ifndef __GNUC__
static int get_bit_top(int a)
 {
  int counter=0;
  unsigned int b;
  b=a;
  while ((b&0x80000000)==0 && counter<32 )
   {
    counter++;
    b<<=1;
   }
  return counter;
 }

static void messagehandler_tc()
 {
   MSG msg;
   if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }
 }


static HRESULT CALLBACK ddraw_enum_video_modes_tc(LPDDSURFACEDESC desc,void * privparm)
 {
   int l=0;
   while(modelist[l*4]!=0)
    {
     if (desc->dwWidth==modelist[(l*4)])
      if (desc->dwHeight==modelist[(l*4)+1])
       if (desc->ddpfPixelFormat.dwRGBBitCount==modelist[(l*4)+2])
        if (desc->dwWidth>=width)
         if (desc->dwHeight>=height)
          {
           modelist[(l*4)+3]=1;
//           printf("mode: %ix%ix%i is okay\n",modelist[(l*4)],modelist[(l*4)+1],modelist[(l*4)+2]);
          }
     l++;
    }

   return DDENUMRET_OK;
 }
#endif

static HWND buttonfull;
static HWND buttonwindow;
static int modeselect=-1;
static int accelerated=0;

/***********************************************************************
* init_tc(); <- initiate lib, ie get settings,decide mode,etc
***********************************************************************/
void init_tc(int x,int y) {
 int kloop;
 HRESULT result;
 char str[60];
 int mode;
 int starttime;

 width=x;
 height=y;

 for (kloop=0;kloop<256;kloop++)
  keystate[kloop]=0;

  wtc_instance=GetModuleHandle(NULL);


// Set window infos
 wnd.style=CS_HREDRAW | CS_VREDRAW;;
 wnd.lpfnWndProc=&wtc_proc;
 wnd.cbClsExtra=0;
 wnd.cbWndExtra=0;
 wnd.hInstance=wtc_instance;
 wnd.hIcon=LoadIcon(NULL,IDI_APPLICATION);
 wnd.hCursor=LoadCursor(NULL,IDC_ARROW);
 wnd.hbrBackground=(HBRUSH)COLOR_WINDOW;
 wnd.lpszMenuName=NULL;
 wnd.lpszClassName=wtc_windowname;
// register window
 RegisterClass(&wnd);

// create window
#ifndef __GNUC__
/*
  if (use_dd_tc==1)
   wtc_handle=CreateWindow(wtc_windowname,
                           windowtopic,
                           WS_POPUP
                           ,0,0,646,505,NULL,NULL,wtc_instance,NULL);
  else
*/
#endif
     wtc_handle=CreateWindow(wtc_windowname,
                             windowtopic,
                             WS_CAPTION|WS_SYSMENU
                             ,0,0,606,20+25,NULL,NULL,wtc_instance,NULL);
   

/* show window */
 ShowWindow(wtc_handle,SW_SHOW);
 SetForegroundWindow(wtc_handle);
 UpdateWindow(wtc_handle);

 Sleep(100);

 mode=WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON;
 if (use_dd_tc==1)
  mode|=BS_DEFPUSHBUTTON;
 buttonfull=CreateWindow("BUTTON","Fullscreen mode",mode,0,0,300,20,wtc_handle,NULL,wtc_instance,NULL);

 mode=WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON;
 if (use_dd_tc!=1)
  mode|=BS_DEFPUSHBUTTON;
 buttonwindow=CreateWindow("BUTTON","Windowed mode",mode,300,0,300,20,wtc_handle,NULL,wtc_instance,NULL);



 starttime=timeGetTime();
 while(modeselect==-1)
  {
   messagehandler_tc();
   sprintf(str,"%s : Select mode : %i seconds left",windowtopic,6-((timeGetTime()-starttime)/1000));
   SetWindowText(wtc_handle,str);
   if ((timeGetTime()-starttime)>5500)
    modeselect=5;

   Sleep(40);
  }
 SetWindowText(wtc_handle,windowtopic);

 DestroyWindow(buttonfull);
 DestroyWindow(buttonwindow);

// 
// ^- 2'a

 SetWindowPos(wtc_handle,use_dd_tc&1?HWND_TOPMOST:HWND_TOP,0,0,
 use_dd_tc&1?width:width+10,
 use_dd_tc&1?height:height+30,
 SWP_SHOWWINDOW);

 UpdateWindow(wtc_handle);

#ifndef __GNUC__
 if (use_dd_tc==1)
  {
//   SetWindowLong(wtc_handle,GWL_STYLE,WS_VISIBLE|WS_POPUP|);
   SetWindowLong(wtc_handle,GWL_STYLE,WS_POPUP);
   result=DirectDrawCreate(NULL,&m_dd,NULL);
   if (result!=DD_OK)
    {
     use_dd_tc=0;
     return;
    }
   // check modelist
   IDirectDraw_EnumDisplayModes(m_dd,0,NULL,(LPVOID)0,(LPDDENUMMODESCALLBACK)ddraw_enum_video_modes_tc);
  }
#endif
}

/***********************************************************************
* start_tc(); <- setup desired mode,etc,etc
***********************************************************************/
void start_tc(void) {
 int respawns;
 HDC hdc;
 RECT rect;
 HRESULT result;
 int flag;
 int yl,xl;
 char *videoptr;

#ifndef __GNUC__
 cursor=SetCursor(NULL);
#endif

#ifndef __GNUC__
 if (use_dd_tc==1)
  {
   result=IDirectDraw_SetCooperativeLevel(m_dd,wtc_handle,DDSCL_EXCLUSIVE|DDSCL_ALLOWREBOOT|DDSCL_FULLSCREEN); // |DDSCL_NOWINDOWCHANGES
   flag=0;
   videomode=0;


   while(modelist[videomode*4]!=0 && modelist[(videomode*4)+3]==0 )
    {
     videomode++;
    }

   if (modelist[videomode*4]!=0)
   {
    result=IDirectDraw_SetDisplayMode(m_dd,
                           modelist[(videomode*4)],
                           modelist[(videomode*4)+1],
                           modelist[(videomode*4)+2]);
    
    IDirectDraw_WaitForVerticalBlank(m_dd,DDWAITVB_BLOCKBEGIN,NULL);
   }


   if (modelist[videomode*4]==0)
    {
     printf("failure at modeset\n");
     DestroyWindow(wtc_handle);
     use_dd_tc=0;
     init_tc(width,height);
     start_tc();
     return;
    }

   memset_tc(&m_desc,0,sizeof(m_desc));
   m_desc.dwSize=sizeof(m_desc);
   m_desc.dwFlags=DDSD_CAPS;
   m_desc.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;

   result=IDirectDraw_CreateSurface(m_dd,&m_desc,&m_surf,NULL);
   respawns=0;
   while (respawns<5 && result!=DD_OK)
    {
     Sleep(500);
     result=IDirectDraw_CreateSurface(m_dd,&m_desc,&m_surf,NULL);
     respawns++;
    }

   if (result!=DD_OK)
    {
     printf("failure at create\n");
     DestroyWindow(wtc_handle);
     use_dd_tc=0;
     init_tc(width,height);
     start_tc();
     return;
    }


   tcono=(unsigned long int *)malloc(width*height*4);

   respawns=0;


   result=IDirectDrawSurface_Lock(m_surf,NULL,&m_desc,0,NULL);
   modeselect=2;

   while (respawns<13 && result!=DD_OK)
    {
      Sleep(500);
      if (result==DDERR_SURFACELOST)
       {
        result=IDirectDrawSurface_Restore(m_surf);
       }
     result=IDirectDrawSurface_Lock(m_surf,NULL,&m_desc,0,NULL);
     respawns++;
    }
   if (result==DD_OK)
    {
     videoptr=(char *)m_desc.lpSurface;
//     for (yl=0;yl<m_desc.dwHeight;yl++)
     for (yl=0;yl<modelist[(videomode*4)+1];yl++)
      {
//       for (xl=0;xl<m_desc.dwWidth*modelist[(videomode*4)+2]/8;xl++)
       for (xl=0;xl<modelist[videomode*4]*modelist[(videomode*4)+2]/8;xl++)
        videoptr[xl]=0;
       videoptr+=m_desc.lPitch;
      }
     IDirectDrawSurface_Unlock(m_surf,NULL);
    }



   Rmask=m_desc.ddpfPixelFormat.dwRBitMask;
   Gmask=m_desc.ddpfPixelFormat.dwGBitMask;
   Bmask=m_desc.ddpfPixelFormat.dwBBitMask;

   Rpos=get_bit_top(Rmask);
   Gpos=get_bit_top(Gmask);
   Bpos=get_bit_top(Bmask);

 // is the bit ordering correct?
   if (modelist[(videomode*4)+2]==32)
    if (Rpos==8)
     if (Gpos==16)
      if (Bpos==24)
       {
        fastdraw=1;
       }

   if (modelist[(videomode*4)+2]==16)
    if (Rpos==16)
     if (Gpos==21)
      if (Bpos==27)
       {
        fastdraw=1;
       }
   Sleep(1400);
  }
 else
  {
#endif
   hdc=GetWindowDC(wtc_handle);
   wtc_hdc=CreateCompatibleDC(hdc);
   ReleaseDC(wtc_handle,hdc);

   bitmap.bi.biSize=40; //sizeof(struct WTC_BITMAP);
   bitmap.bi.biWidth=width;
   bitmap.bi.biHeight=-height;
   bitmap.bi.biPlanes=1;
   bitmap.bi.biBitCount=32;
   bitmap.bi.biCompression=BI_BITFIELDS;
   bitmap.bi.biSizeImage=0;
   bitmap.bi.biXPelsPerMeter=0;
   bitmap.bi.biYPelsPerMeter=0;
   bitmap.bi.biClrUsed=0;
   bitmap.bi.biClrImportant=0;
   bitmap.r=0xff0000;
   bitmap.g=0x00ff00;
   bitmap.b=0x0000ff;

   GetClientRect(wtc_handle,&rect);

   wtc_obj=CreateDIBSection(wtc_hdc,(BITMAPINFO *)&bitmap,DIB_RGB_COLORS,(void *)&tcono,NULL,0);
   if (!wtc_obj)
    {
        MessageBox(NULL,"PErkele.. something fucked up","Woorlic TrueColor",0);
        exit(-1);
    }
   SelectObject(wtc_hdc,wtc_obj);
#ifndef __GNUC__
  }
#endif
}

#ifndef __GNUC__

 static void blit_tc_16bpp_slow(char *dest,int skip);
 static void blit_tc_32bpp_slow(char *dest,int skip);

 static void blit_tc_32bpp_fast(char *dest,int skip);

 #ifdef __ASMOPT__
  void __cdecl blit_tc_fastcopy     (void *dest,void * src,int bytes);
  void __cdecl blit_tc_16bpp_fast   (char *dest,int skip,int x,int y);
 #endif

#endif

/***********************************************************************
* blit_tc(void); copy to screen
***********************************************************************/
void blit_tc(void) {
 PAINTSTRUCT ps;
 HDC hdc;
 RECT rect;
 MSG msg;
 HRESULT result;
 char *videoptr;
 int yl,xl;

  if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

#ifndef __GNUC__
 if (use_dd_tc==1)
  {


   result=IDirectDrawSurface_Lock(m_surf,NULL,&m_desc,0,NULL);
   if (result==DDERR_SURFACELOST)
    {
     result=IDirectDrawSurface_Restore(m_surf);
     needclear=1;
     result=IDirectDrawSurface_Lock(m_surf,NULL,&m_desc,0,NULL);
    }
   if (result!=DD_OK)
    {
     return;
    }


   if (needclear)
    {
//     printf("clearink\n");
     videoptr=(char *)m_desc.lpSurface;
     for (yl=0;yl<modelist[(videomode*4)+1];yl++)
      {
       for (xl=0;xl<modelist[videomode*4]*modelist[(videomode*4)+2]/8;xl++)
        videoptr[xl]=0;
       videoptr+=m_desc.lPitch;
      }

     needclear=0;
    }


   videoptr=(char *)m_desc.lpSurface;
   videoptr+=(((modelist[(videomode*4)+1]-height)>>1)*m_desc.lPitch)+
             (((modelist[(videomode*4)]-width)>>1)*(modelist[(videomode*4)+2]/8));
   if (modelist[videomode*4+2]==32)
    {                                    
      if (fastdraw==1)
       blit_tc_32bpp_fast(videoptr,m_desc.lPitch);
      else
       blit_tc_32bpp_slow(videoptr,m_desc.lPitch);
    } else {
      if (fastdraw==1)
       blit_tc_16bpp_fast(videoptr,m_desc.lPitch,width,height);
      else
       blit_tc_16bpp_slow(videoptr,m_desc.lPitch);
    }
   IDirectDrawSurface_Unlock(m_surf,NULL);
  }
 else
 {
#endif  
//  GetClientRect(wtc_handle,&rect);
  
  hdc=GetDC(wtc_handle);
  BitBlt(hdc,0,0,width,height,wtc_hdc,0,0,SRCCOPY);
  ReleaseDC(wtc_handle,hdc);
  UpdateWindow(wtc_handle);
#ifndef __GNUC__
 }
#endif


}

/***********************************************************************
* kill_tc(void); stop system
***********************************************************************/
void kill_tc(void) {
#ifndef __GNUC__
 if (use_dd_tc==1)
  {
   IDirectDrawSurface_Release(m_surf);
  }
  SetCursor(cursor);
#endif
 DestroyWindow(wtc_handle);
}

#ifndef __GNUC__

static void blit_tc_32bpp_fast(char *dest,int skip)
 {
  int yl;
  unsigned long int *c_tcono;
 
    c_tcono=tcono;
    for (yl=0;yl<height;yl++)
     {
//      memcpy(dest,c_tcono,width*4);
      blit_tc_fastcopy(dest,c_tcono,width*4);
      c_tcono+=width;
      dest+=skip;
     }

 }

static void blit_tc_32bpp_slow(char *dest,int skip)
 {
  int yl,xl;
  unsigned long int *c_tcono;
  int rr,gg,bb;
  int * ldest;

    c_tcono=tcono;
    for (yl=0;yl<height;yl++)
     {
//      memcpy(dest,c_tcono,width*4);
      ldest=(int *)dest;
      for (xl=0;xl<width;xl++)
       {
        rr=c_tcono[xl]<<8;
        gg=rr<<8; bb=rr<<16;
        ldest[xl]=
         ((rr>>Rpos)&Rmask)+
         ((gg>>Gpos)&Gmask)+
         ((bb>>Bpos)&Bmask);
       }
      c_tcono+=width;
      dest+=skip;
     }
 }

static void blit_tc_16bpp_slow(char *dest,int skip)
 {
  int yl,xl;
  unsigned long int *c_tcono;
  int rr,gg,bb;
  short int * ldest;

    c_tcono=tcono;
    for (yl=0;yl<height;yl++)
     {
//      memcpy(dest,c_tcono,width*4);
      ldest=(short int *)dest;
      for (xl=0;xl<width;xl++)
       {
        rr=c_tcono[xl]<<8;
        gg=rr<<8; bb=rr<<16;
        ldest[xl]=
         ((rr>>Rpos)&Rmask)+
         ((gg>>Gpos)&Gmask)+
         ((bb>>Bpos)&Bmask);
       }
      c_tcono+=width;
      dest+=skip;
     }
 }

#endif

static LRESULT CALLBACK wtc_proc(HWND hwnd,unsigned int iMsg,WPARAM parm1,LPARAM parm2){
 int tmp;
 RECT rect;


 if (use_dd_tc)
 {
  GetClientRect(wtc_handle,&rect);
 }
/*
  case WM_COMMAND :
     for (tmp=0;tmp<8;tmp++)
      {
       if (buttonlist[tmp]==(HWND)parm2)
        { videomode=tmp; return 0; }
      }
   break;
*/

 switch (iMsg) {
  case WM_COMMAND :
    if ((HWND)parm2==buttonfull)
     {
      use_dd_tc=1;
      modeselect=1;
     }
    if ((HWND)parm2==buttonwindow)
     {
      use_dd_tc=0;
      modeselect=1;
     }
   break;
  case WM_KEYUP :
    keystate[parm1]=0;
   break;
  case WM_KEYDOWN :
    keystate[parm1]=1;
   break;
  case WM_CHAR :
    wtc_buffer[wtc_bufferp++]=parm1;
   return(0);
  case WM_DESTROY :
    wtc_buffer[wtc_bufferp++]=27;
   return(0);
  case  WM_LBUTTONDOWN:
  case  WM_LBUTTONUP:
  case  WM_MBUTTONDOWN:
  case  WM_MBUTTONUP:
  case  WM_RBUTTONDOWN:
  case  WM_RBUTTONUP:
  case  WM_MOUSEMOVE:
   cursor=SetCursor(NULL);

    tmp=parm2&0xffff;
    tmp*=width;
    tmp/=rect.right;
    if (tmp>(width-1))
     tmp=(width-1);

    mouse_xc=tmp;

    tmp=(parm2>>16)&0xffff;
    tmp*=height;
    tmp/=rect.bottom;
    if (tmp>(height-1))
     tmp=(height-1);

    mouse_yc=tmp;

    mouse_bs=0;

    if (parm1&MK_LBUTTON)
     mouse_bs+=1;
    if (parm1&MK_RBUTTON)
     mouse_bs+=2;
    if (parm1&MK_MBUTTON)
     mouse_bs+=4;
   return 0;
//   break;
 }


 if (modeselect==2)
  {
   if (iMsg==WM_NCPAINT)
    {
//   printf("wm ncpaint\n");
     return 0;
    }
   if (iMsg==WM_PAINT)
    {
//   printf("wm paint\n");
     return 0;
    }
   if (iMsg==WM_ERASEBKGND)
    {
//   printf("erase background\n");
     return 0;
    }
  }

 return(DefWindowProc(hwnd,iMsg,parm1,parm2));
}

/***********************************************************************
* get last key pressed down
***********************************************************************/
int getch_tc(void) {
 MSG msg;

 while(wtc_bufferp==0)
  {

   if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
     TranslateMessage(&msg);
     DispatchMessage(&msg);
    }

  }
 return( (int)wtc_buffer[--wtc_bufferp] );
}

/***********************************************************************
* check if key in kbd buffer
***********************************************************************/
int kbhit_tc(void) {

 MSG msg;
   if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

 if (wtc_bufferp)
  return(1);
 else
  return(0);
}

/***********************************************************************
* returns mouse X
***********************************************************************/
int mousex_tc(void){
 return(mouse_xc);
}

/***********************************************************************
* returns mouse Y
***********************************************************************/
int mousey_tc(void){
 return(mouse_yc);
}

/***********************************************************************
* returns mouse button status
***********************************************************************/
int mouseb_tc(void){
 return(mouse_bs);
}

/***********************************************************************
* returns the state of the current key,
* should use platform specific defines
***********************************************************************/
int keystate_tc(int key){
 return(keystate[key]);
}

void settopic_tc(char * name)
 {
  windowtopic=name;
 }

void usewindow_tc(int aa)
 {
  if (aa)
   use_dd_tc=0;
  else
   use_dd_tc=1;
 }
