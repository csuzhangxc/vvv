// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef GLUTBASE_H
#define GLUTBASE_H

#include "codebase.h" // universal code base
#include "oglbase.h" // opengl code base

// OpenGL includes:

#ifdef WINOS
#include <windows.h>
#endif

#ifndef MACOSX
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

// window handling prototypes:

void clearwindow();
void swapbuffers();
void openwindow(int width,int height,float fps,char *title);
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
BOOLINT getresized();

#endif
