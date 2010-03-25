// (c) by Stefan Roettger

#ifndef GUIBASE_H
#define GUIBASE_H

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base and window handling

typedef void (*GUI_hooktype)(float x,float y,void *data);
typedef void (*GUI_drawtype)(void *data1,void *data2);

// GUI class
class GUI
   {
   public:

   //! default constructor
   GUI(int hooks=100);

   //! destructor
   ~GUI();

   int addhook(float x,float y,float width,float height,
               float hue,float sat,float val,float alpha,
               GUI_hooktype hook,void *data1,
               GUI_drawtype draw,void *data2);

   void delhook(int id);
   void delhooks();

   int click(float x,float y);
   void release(float x,float y);

   void refresh();

   static void nullhook(float x,float y);

   static void buttonhook(float x,float y,void *data);
   static void buttondraw(void *data1,void *data2);

   static void pushbuttonhook(float x,float y,void *data);
   static void pushbuttondraw(void *data1,void *data2);

   static void sliderhookX(float x,float y,void *data);
   static void sliderdrawX(void *data1,void *data2);

   static void sliderhookY(float x,float y,void *data);
   static void sliderdrawY(void *data1,void *data2);

   static void wheelhookX(float x,float y,void *data);
   static void wheeldrawX(void *data1,void *data2);

   static void wheelhookY(float x,float y,void *data);
   static void wheeldrawY(void *data1,void *data2);

   static void drawline(float x1,float y1,float x2,float y2,
                        float hue,float sat,float val,float alpha);

   static void drawlineRGBA(float x1,float y1,float x2,float y2,
                            float r,float g,float b,float alpha);

   static void drawquad(float x,float y,float width,float height,
                        float hue,float sat,float val,float alpha);

   static void drawquadRGBA(float x,float y,float width,float height,
                            float r,float g,float b,float alpha);

   static void drawframe(float x,float y,float width,float height,
                         float hue,float sat,float val,float alpha);

   static void drawframeRGBA(float x,float y,float width,float height,
                             float r,float g,float b,float alpha);

   static void drawstring(float x,float y,float width,float height,
                          float hue,float sat,float val,float alpha,char *str);

   static void drawtexture(float x,float y,float width,float height,
                           float hue,float sat,float val,float alpha,
                           int texid,int sizex,int sizey);

   static int buildtexmap2DL(unsigned char *image,
                             int width,int height);

   static int buildtexmap2DL(float *image,
                             int width,int height);

   static int buildtexmap2DRGB(unsigned char *image,
                               int width,int height);

   static int buildtexmap2DRGB(float *image,
                               int width,int height);

   static int buildtexmap2DRGBA(unsigned char *image,
                                int width,int height);

   static int buildtexmap2DRGBA(float *image,
                                int width,int height);

   static void deletetexmap(int texid);

   static void hsv2rgb(float hue,float sat,float val,float *rgb);

   protected:

   static void drawsymbol(float hue,float sat,float val,float alpha,const char *symbol);
   static void drawletter(float hue,float sat,float val,float alpha,char letter);

   private:

   int maxhooks;

   float *hx,*hy,*hw,*hh,*hr,*hg,*hb,*ha;
   void **hd1,**hd2;

   GUI_hooktype *hookptr;
   GUI_drawtype *drawptr;
   };

#endif
