// (c) by Stefan Roettger

#ifndef OGLBASE_H
#define OGLBASE_H

#include "codebase.h" // universal code base

// OpenGL includes:

#ifdef WINOS
#include <windows.h>
#endif

#ifndef MACOSX
#ifdef LINUX
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#if defined(LINUX) || defined(WINOS)
#include <GL/glext.h>
#endif
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#endif

// OpenGL 1.0 workaround:

#ifndef GL_VERSION_1_1
#ifndef glGenTextures
#define glGenTextures glGenTexturesEXT
#endif
#ifndef glBindTexture
#define glBindTexture glBindTextureEXT
#endif
#ifndef glDeleteTextures
#define glDeleteTextures glDeleteTexturesEXT
#endif
#endif

// OpenGL 1.1 workaround:

#ifndef GL_VERSION_1_2
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D GL_TEXTURE_3D_EXT
#endif
#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R GL_TEXTURE_WRAP_R_EXT
#endif
#ifndef glTexImage3D
#define glTexImage3D glTexImage3DEXT
#endif
#endif

// window handling prototypes:

void clearwindow();
void invertwindow();
void swapbuffers();
void openwindow(int width,int height,float fps,char *title);
void setbackground(float R,float G,float B);
void setwindowinfo(char *info);
void addhandler(void workproc(float time));
void closewindow();

// event handling prototypes:

BOOLINT getbutton1();
BOOLINT getbutton2();
BOOLINT getbutton3();
float getmousex();
float getmousey();
char getkey();

// window query prototypes:

int getwinwidth();
int getwinheight();
float getaspect();

// Windows OpenGL extensions:

#ifdef WINOS

extern PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT;

#ifdef GL_ARB_multitexture
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
#endif

#ifdef GL_NV_register_combiners
extern PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
extern PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
extern PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
extern PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;
#endif

#ifdef GL_ARB_fragment_program
extern PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
extern PFNGLBINDPROGRAMARBPROC glBindProgramARB;
extern PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
extern PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB;
extern PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
#endif

#endif

#endif
