// (c) by Stefan Roettger, licensed under GPL 2+

#include "oglbase.h"

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

#ifdef GL_EXT_framebuffer_object
   glGenFramebuffersEXT                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
   glDeleteFramebuffersEXT                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
   glBindFramebufferEXT                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
   glCheckFramebufferStatusEXT              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
   glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
   glGenerateMipmapEXT                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
   glFramebufferTexture2DEXT                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
   glFramebufferRenderbufferEXT             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
   glGenRenderbuffersEXT                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
   glDeleteRenderbuffersEXT                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
   glBindRenderbufferEXT                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
   glRenderbufferStorageEXT                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
   glGetRenderbufferParameterivEXT          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
   glIsRenderbufferEXT                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");
   glBlitFramebuffer                        = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");

   if (!(glGenFramebuffersEXT && glDeleteFramebuffersEXT && glBindFramebufferEXT && glCheckFramebufferStatusEXT &&
         glGetFramebufferAttachmentParameterivEXT && glGenerateMipmapEXT && glFramebufferTexture2DEXT && glFramebufferRenderbufferEXT &&
         glGenRenderbuffersEXT && glDeleteRenderbuffersEXT && glBindRenderbufferEXT && glRenderbufferStorageEXT &&
         glGetRenderbufferParameterivEXT && glIsRenderbufferEXT &&
         glBlitFramebuffer)) ERRORMSG();
#endif
   }

#endif

// OpenGL dependent functions:

void setbackground(float R,float G,float B,float A)
   {glClearColor(R,G,B,A);}

void clearbuffer()
   {
   glDisable(GL_DITHER);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DITHER);
   }

void invertbuffer()
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

   glDepthMask(GL_FALSE);

   glBegin(GL_QUADS);
   glColor3f(1.0f,1.0f,1.0f);
   glVertex2f(-1.0f,-1.0f);
   glVertex2f(1.0f,-1.0f);
   glVertex2f(1.0f,1.0f);
   glVertex2f(-1.0f,1.0f);
   glEnd();

   glDepthMask(GL_TRUE);

   glDisable(GL_BLEND);

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   }

void initogl()
   {
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

   setbackground(0.0f,0.0f,0.0f);
   clearbuffer();
   }

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

#ifdef GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSPROC                     glGenFramebuffersEXT = 0;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSPROC                  glDeleteFramebuffersEXT = 0;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFERPROC                     glBindFramebufferEXT = 0;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSPROC              glCheckFramebufferStatusEXT = 0;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameterivEXT = 0;  // return various FBO parameters
PFNGLGENERATEMIPMAPPROC                      glGenerateMipmapEXT = 0;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE2DPROC                glFramebufferTexture2DEXT = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFERPROC             glFramebufferRenderbufferEXT = 0;              // FBO renderbuffer attachement procedure
PFNGLGENRENDERBUFFERSPROC                    glGenRenderbuffersEXT = 0;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSPROC                 glDeleteRenderbuffersEXT = 0;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFERPROC                    glBindRenderbufferEXT = 0;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEPROC                 glRenderbufferStorageEXT = 0;                  // renderbuffer memory allocation procedure
PFNGLGETRENDERBUFFERPARAMETERIVPROC          glGetRenderbufferParameterivEXT = 0;           // return various renderbuffer parameters
PFNGLISRENDERBUFFERPROC                      glIsRenderbufferEXT = 0;                       // determine renderbuffer object type
PFNGLBLITFRAMEBUFFERPROC                     glBLitFramebuffer = 0;                         // FBO blit procedure
#endif

#endif
