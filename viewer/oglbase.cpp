// (c) by Stefan Roettger, licensed under GPL 2+

#include "oglbase.h"

// OpenGL/GLUT window handling:

#define OGL_MAXSTR (256)
#define OGL_UPDATE (0.5)

char OGL_wintitle[OGL_MAXSTR];
char OGL_wininfo[OGL_MAXSTR];

int OGL_winwidth,OGL_winheight,OGL_winid;
float OGL_fps;

typedef void OGL_proctype(float time);
OGL_proctype *OGL_handler;

int OGL_frame;
double OGL_time,OGL_otime,OGL_atime;
int OGL_tcount;

BOOLINT OGL_button1D=FALSE,OGL_button2D=FALSE,OGL_button3D=FALSE;
int OGL_button1DF,OGL_button2DF,OGL_button3DF;
BOOLINT OGL_button1U=FALSE,OGL_button2U=FALSE,OGL_button3U=FALSE;
int OGL_button1UF,OGL_button2UF,OGL_button3UF;
float OGL_mousex=0.0f,OGL_mousey=0.0f;
char OGL_key='\0';

// Windows OpenGL extension setup:

#ifdef WINOS

static void initwglprocs()
   {
   if ((glTexImage3DEXT=(PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT"))==NULL) ERRORMSG();

#ifdef GL_ARB_multitexture
   if ((glActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB"))==NULL) ERRORMSG();
   if ((glMultiTexCoord3fARB=(PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB"))==NULL) ERRORMSG();
   if ((glMultiTexCoord4fARB=(PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB"))==NULL) ERRORMSG();
#endif

#ifdef GL_ARB_fragment_program
   if ((glGenProgramsARB=(PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB"))==NULL) ERRORMSG();
   if ((glBindProgramARB=(PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB"))==NULL) ERRORMSG();
   if ((glProgramStringARB=(PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB"))==NULL) ERRORMSG();
   if ((glGetProgramivARB=(PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress("glGetProgramivARB"))==NULL) ERRORMSG();
   if ((glProgramEnvParameter4fARB=(PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress("glProgramEnvParameter4fARB"))==NULL) ERRORMSG();
   if ((glDeleteProgramsARB=(PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB"))==NULL) ERRORMSG();
#endif

#ifdef GL_ARB_framebuffer_object
   glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
   glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
   glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
   glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
   glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
   glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
   glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
   glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
   glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
   glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
   glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
   glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
   glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
   glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");

   if (!(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
         glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture2D && glFramebufferRenderbuffer &&
         glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
         glGetRenderbufferParameteriv && glIsRenderbuffer)) ERRORMSG();
#endif
   }

#endif

// OpenGL dependent functions:

void clearwindow()
   {
   glDisable(GL_DITHER);
   glViewport(0,0,OGL_winwidth,OGL_winheight);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DITHER);
   }

void invertwindow()
   {
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(-1.0f,1.0f,-1.0f,1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ZERO);
   glEnable(GL_BLEND);

   glBegin(GL_QUADS);
   glColor3f(1.0f,1.0f,1.0f);
   glVertex2f(-1.0f,-1.0f);
   glVertex2f(1.0f,-1.0f);
   glVertex2f(1.0f,1.0f);
   glVertex2f(-1.0f,1.0f);
   glEnd();

   glDisable(GL_BLEND);

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   }

void swapbuffers()
   {
   char str[OGL_MAXSTR],
        tmp[OGL_MAXSTR];

   glFinish();

   OGL_frame++;
   OGL_atime+=gettime()-OGL_time;
   OGL_tcount++;

   glutSwapBuffers();

   glFinish();
   OGL_time=gettime()-OGL_time;

   waitfor(fmax(1.0/OGL_fps-OGL_time,0.0));
   OGL_time+=fmax(1.0/OGL_fps-OGL_time,0.0);
   OGL_otime+=OGL_time;

   if (OGL_otime>=OGL_UPDATE)
      {
      snprintf(str,OGL_MAXSTR,"%.1fhz %d%%",
               OGL_tcount/OGL_otime,ftrc(100.0*OGL_atime/OGL_otime+0.5));

      strncpy(tmp,OGL_wintitle,OGL_MAXSTR-1);

      strncat(tmp," (",OGL_MAXSTR-1-strlen(tmp));
      strncat(tmp,str,OGL_MAXSTR-1-strlen(tmp));

      if (strlen(OGL_wininfo)>0)
         {
         strncat(tmp," ",OGL_MAXSTR-1-strlen(tmp));
         strncat(tmp,OGL_wininfo,OGL_MAXSTR-1-strlen(tmp));
         }

      strncat(tmp,")",OGL_MAXSTR-1-strlen(tmp));

      glutSetWindowTitle(tmp);

      OGL_otime=OGL_atime=0.0;
      OGL_tcount=0;
      }
   }

void openwindow(int width,int height,float fps,char *title)
   {
   int argc=1;
   char **argv=&title;

   strncpy(OGL_wintitle,title,OGL_MAXSTR-1);
   strncpy(OGL_wininfo,"",OGL_MAXSTR-1);

   glutInit(&argc,argv);
   glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH|GLUT_DOUBLE);
   glutInitWindowSize(width,height);

   OGL_winid=glutCreateWindow(title);

   OGL_winwidth=glutGet((GLenum)GLUT_WINDOW_WIDTH);
   OGL_winheight=glutGet((GLenum)GLUT_WINDOW_HEIGHT);

#ifdef WINOS
   initwglprocs();
#endif

   glEnable(GL_DITHER);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);

   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);
   glEnable(GL_CULL_FACE);

   glShadeModel(GL_SMOOTH);
   glDisable(GL_BLEND);

   OGL_fps=fps;

   OGL_frame=0;
   OGL_time=OGL_otime=OGL_atime=0.0;
   OGL_tcount=0;

   glClearColor(0.0f,0.0f,0.0f,1.0f);

   clearwindow();
   swapbuffers();
   }

void setbackground(float R,float G,float B)
   {glClearColor(R,G,B,1.0f);}

void setwindowinfo(char *info)
   {strncpy(OGL_wininfo,info,OGL_MAXSTR-1);}

void reshapefunc(int width,int height)
   {
   OGL_winwidth=width;
   OGL_winheight=height;
   }

void mousefunc(int button,int state,int mx,int my)
   {
   if (button==GLUT_LEFT_BUTTON)
      if (state==GLUT_DOWN) {OGL_button1D=TRUE; OGL_button1DF=OGL_frame;}
      else {OGL_button1U=TRUE; OGL_button1UF=OGL_frame;}
   else if (button==GLUT_MIDDLE_BUTTON)
      if (state==GLUT_DOWN) {OGL_button2D=TRUE; OGL_button2DF=OGL_frame;}
      else {OGL_button2U=TRUE; OGL_button2UF=OGL_frame;}
   else if (button==GLUT_RIGHT_BUTTON)
      if (state==GLUT_DOWN) {OGL_button3D=TRUE; OGL_button3DF=OGL_frame;}
      else {OGL_button3U=TRUE; OGL_button3UF=OGL_frame;}
   }

void motionfunc(int mx,int my)
   {
   OGL_mousex=(float)mx/OGL_winwidth-0.5f;
   OGL_mousey=(float)(OGL_winheight-1-my)/OGL_winheight-0.5f;
   }

void keyboardfunc(unsigned char c,int mx,int my)
   {OGL_key=c;}

void wrapper()
   {
   float time;

   time=OGL_time;
   OGL_time=gettime();

   OGL_handler(time);
   }

void addhandler(void workproc(float time))
   {
   OGL_handler=workproc;
   glutDisplayFunc(wrapper);
   glutReshapeFunc(reshapefunc);
   glutMouseFunc(mousefunc);
   glutMotionFunc(motionfunc);
   glutPassiveMotionFunc(motionfunc);
   glutKeyboardFunc(keyboardfunc);
   glutIdleFunc(wrapper);
   glutMainLoop();
   }

void closewindow()
   {glutDestroyWindow(OGL_winid);}

BOOLINT getbutton1()
   {
   if (OGL_button1D && !OGL_button1U) return(TRUE);
   else if (OGL_button1D && OGL_button1U)
      {
      OGL_button1D=FALSE;
      return(OGL_button1DF==OGL_button1UF);
      }
   else if (!OGL_button1D && OGL_button1U)
      {
      OGL_button1U=FALSE;
      return(FALSE);
      }
   return(FALSE);
   }

BOOLINT getbutton2()
   {
   if (OGL_button2D && !OGL_button2U) return(TRUE);
   else if (OGL_button2D && OGL_button2U)
      {
      OGL_button2D=FALSE;
      return(OGL_button2DF==OGL_button2UF);
      }
   else if (!OGL_button2D && OGL_button2U)
      {
      OGL_button2U=FALSE;
      return(FALSE);
      }
   return(FALSE);
   }

BOOLINT getbutton3()
   {
   if (OGL_button3D && !OGL_button3U) return(TRUE);
   else if (OGL_button3D && OGL_button3U)
      {
      OGL_button3D=FALSE;
      return(OGL_button3DF==OGL_button3UF);
      }
   else if (!OGL_button3D && OGL_button3U)
      {
      OGL_button3U=FALSE;
      return(FALSE);
      }
   return(FALSE);
   }

float getmousex()
   {return(OGL_mousex);}
float getmousey()
   {return(OGL_mousey);}

char getkey()
   {
   char c=OGL_key;
   OGL_key='\0';
   return(c);
   }

int getwinwidth()
   {return(OGL_winwidth);}
int getwinheight()
   {return(OGL_winheight);}

float getaspect()
   {return((float)OGL_winwidth/OGL_winheight);}

// Windows OpenGL extensions:

#ifdef WINOS

PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT=NULL;

#ifdef GL_ARB_multitexture
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB=NULL;
PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB=NULL;
PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB=NULL;
#endif

#ifdef GL_ARB_fragment_program
PFNGLGENPROGRAMSARBPROC glGenProgramsARB=NULL;
PFNGLBINDPROGRAMARBPROC glBindProgramARB=NULL;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB=NULL;
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB=NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB=NULL;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB=NULL;
#endif

#ifdef GL_ARB_framebuffer_object
PFNGLGENFRAMEBUFFERSPROC                     glGenFramebuffersARB = 0;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSPROC                  glDeleteFramebuffersARB = 0;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFERPROC                     glBindFramebufferARB = 0;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSPROC              glCheckFramebufferStatusARB = 0;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameterivARB = 0;  // return various FBO parameters
PFNGLGENERATEMIPMAPPROC                      glGenerateMipmapARB = 0;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE2DPROC                glFramebufferTexture2DARB = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFERPROC             glFramebufferRenderbufferARB = 0;              // FBO renderbuffer attachement procedure
PFNGLGENRENDERBUFFERSPROC                    glGenRenderbuffersARB = 0;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSPROC                 glDeleteRenderbuffersARB = 0;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFERPROC                    glBindRenderbufferARB = 0;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEPROC                 glRenderbufferStorageARB = 0;                  // renderbuffer memory allocation procedure
PFNGLGETRENDERBUFFERPARAMETERIVPROC          glGetRenderbufferParameterivARB = 0;           // return various renderbuffer parameters
PFNGLISRENDERBUFFERPROC                      glIsRenderbufferARB = 0;                       // determine renderbuffer object type
#endif

#endif
