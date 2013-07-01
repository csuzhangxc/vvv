// (c) by Stefan Roettger, licensed under GPL 2+

#include "guibase.h"

GUI::GUI(int hooks)
   {
   int i;

   maxhooks=hooks;

   hx=new float[hooks];
   hy=new float[hooks];
   hw=new float[hooks];
   hh=new float[hooks];
   hr=new float[hooks];
   hg=new float[hooks];
   hb=new float[hooks];
   ha=new float[hooks];

   hd1=new void *[hooks];
   hd2=new void *[hooks];

   hookptr=new GUI_hooktype[hooks];
   drawptr=new GUI_drawtype[hooks];

   for (i=0; i<hooks; i++) hookptr[i]=NULL;
   }

GUI::~GUI()
   {
   delete hx;
   delete hy;
   delete hw;
   delete hh;
   delete hr;
   delete hg;
   delete hb;
   delete ha;

   delete hd1;
   delete hd2;

   delete hookptr;
   delete drawptr;
   }

int GUI::addhook(float x,float y,float width,float height,
                 float hue,float sat,float val,float alpha,
                 GUI_hooktype hook,void *data1,
                 GUI_drawtype draw,void *data2)
   {
   int i;

   float rgb[3];

   if (width<=0.0f || x<0.0f || x+width>1.0f) ERRORMSG();
   if (height<=0.0f || y<0.0f || y+height>1.0f) ERRORMSG();

   if (hue<0.0f || hue>360.0f) ERRORMSG();
   if (sat<0.0f || sat>1.0f) ERRORMSG();
   if (val<0.0f || val>1.0f) ERRORMSG();
   if (alpha<0.0f || alpha>1.0f) ERRORMSG();

   if (hook==NULL) ERRORMSG();

   hsv2rgb(hue,sat,val,rgb);

   for (i=0; i<maxhooks; i++)
      if (hookptr[i]==NULL)
         {
         hx[i]=x;
         hy[i]=y;
         hw[i]=width;
         hh[i]=height;
         hr[i]=rgb[0];
         hg[i]=rgb[1];
         hb[i]=rgb[2];
         ha[i]=alpha;

         hd1[i]=data1;
         hd2[i]=data2;

         hookptr[i]=hook;
         drawptr[i]=draw;

         return(i+1);
         }

   return(0);
   }

void GUI::delhook(int id)
   {if (id>0 && id<=maxhooks) hookptr[id-1]=NULL;}

void GUI::delhooks()
   {
   int i;

   for (i=0; i<maxhooks; i++) hookptr[i]=NULL;
   }

int GUI::click(float x,float y)
   {
   int i;

   x+=0.5f;
   y+=0.5f;

   for (i=0; i<maxhooks; i++)
      if (hookptr[i]!=NULL)
         if (x>=hx[i] && x<=hx[i]+hw[i] && y>=hy[i] && y<=hy[i]+hh[i])
            {
            hookptr[i]((x-hx[i])/hw[i],(y-hy[i])/hh[i],hd1[i]);
            return(i+1);
            }

   return(0);
   }

void GUI::release(float x,float y)
   {
   int i;

   x+=0.5f;
   y+=0.5f;

   for (i=0; i<maxhooks; i++)
      if (hookptr[i]!=NULL)
         if (x>=hx[i] && x<=hx[i]+hw[i] && y>=hy[i] && y<=hy[i]+hh[i])
            hookptr[i](-1.0f,-1.0f,hd1[i]);
   }

void GUI::refresh()
   {
   int i;

   glDisable(GL_CULL_FACE);
   glDisable(GL_DEPTH_TEST);

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   for (i=0; i<maxhooks; i++)
      if (hookptr[i]!=NULL)
         {
         glColor4f(hr[i],hg[i],hb[i],ha[i]);
         glBegin(GL_TRIANGLE_FAN);
         glVertex2f(hx[i],hy[i]);
         glVertex2f(hx[i]+hw[i],hy[i]);
         glVertex2f(hx[i]+hw[i],hy[i]+hh[i]);
         glVertex2f(hx[i],hy[i]+hh[i]);
         glEnd();
         }

   for (i=0; i<maxhooks; i++)
      if (hookptr[i]!=NULL)
         if (drawptr[i]!=NULL)
            {
            glLoadIdentity();
            glTranslatef(hx[i],hy[i],0.0f);
            glScalef(hw[i],hh[i],0.0f);

            drawptr[i](hd1[i],hd2[i]);
            }

   glDisable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   }

void GUI::nullhook(float x,float y) {}

void GUI::buttonhook(float x,float y,void *data)
   {
   void (*trigger)(float x,float y)=(void (*)(float x,float y))data;
   if (x>=0.0f && y>=0.0f && trigger!=NULL) trigger(x,y);
   }

void GUI::buttondraw(void *data1,void *data2)
   {
   char *text=(char *)data2;

   drawstring(0.2f,0.2f,0.6f,0.6f,0.0f,0.0f,0.0f,0.9f,text);
   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::pushbuttonhook(float x,float y,void *data)
   {
   static BOOLINT clicked=FALSE;

   BOOLINT *button=(BOOLINT *)data;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (!clicked)
      {
      *button=!*button;
      clicked=TRUE;
      }
   }

void GUI::pushbuttondraw(void *data1,void *data2)
   {
   BOOLINT *button=(BOOLINT *)data1;
   char *text=(char *)data2;

   drawstring(0.2f,0.2f,0.6f,0.6f,0.0f,0.0f,0.0f,0.9f,text);

   if (*button) drawquad(0.8f,0.2f,0.15f,0.6f,30.0f,0.9f,0.9f,0.9f);
   drawframe(0.8f,0.2f,0.15f,0.6f,0.0f,0.0f,0.75f,0.75f);

   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::sliderhookX(float x,float y,void *data)
   {
   const float mouseaccel=1.0f/(0.9f-0.075f);

   static BOOLINT clicked=FALSE;

   static float lx;

   float *slider=(float *)data;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (clicked) *slider+=mouseaccel*(x-lx);
   else clicked=TRUE;

   if (*slider<0.0f) *slider=0.0f;
   else if (*slider>1.0f) *slider=1.0f;

   lx=x;
   }

void GUI::sliderdrawX(void *data1,void *data2)
   {
   float *slider=(float *)data1;
   char *text=(char *)data2;

   drawstring(0.55f,0.3f,0.4f,0.4f,0.0f,0.0f,0.0f,0.9f,text);

   drawquad(0.05f+*slider*(0.9f-0.075f),0.2f,0.075f,0.6f,30.0f,0.9f,0.9f,0.9f);
   drawframe(0.05f,0.2f,0.9f,0.6f,0.0f,0.0f,0.75f,0.75f);

   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::sliderhookY(float x,float y,void *data)
   {
   const float mouseaccel=1.0f/(0.9f-0.075f);

   static BOOLINT clicked=FALSE;

   static float ly;

   float *slider=(float *)data;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (clicked) *slider+=mouseaccel*(y-ly);
   else clicked=TRUE;

   if (*slider<0.0f) *slider=0.0f;
   else if (*slider>1.0f) *slider=1.0f;

   ly=y;
   }

void GUI::sliderdrawY(void *data1,void *data2)
   {
   float *slider=(float *)data1;
   char *text=(char *)data2;

   drawstring(0.3f,0.55f,0.4f,0.375f,0.0f,0.0f,0.0f,0.9f,text);

   drawquad(0.2f,0.05f+*slider*(0.9f-0.075f),0.6f,0.075f,30.0f,0.9f,0.9f,0.9f);
   drawframe(0.2f,0.05f,0.6f,0.9f,0.0f,0.0f,0.75f,0.75f);

   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::wheelhookX(float x,float y,void *data)
   {
   const float mouseaccel=1.5f;

   static BOOLINT clicked=FALSE;

   static float lx;

   float *slider=(float *)data;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (clicked) *slider+=mouseaccel*(x-lx);
   else clicked=TRUE;

   while (*slider<0.0f) *slider+=1.0f;
   while (*slider>1.0f) *slider-=1.0f;

   lx=x;
   }

void GUI::wheeldrawX(void *data1,void *data2)
   {
   float *slider=(float *)data1;
   char *text=(char *)data2;

   float x;

   for (x=0.05f+*slider*(0.9f-0.025f)+0.025f; x<0.95f; x+=0.025f) drawframe(x,0.2f,0.0f,0.6f,30.0f,0.9f,0.9f,0.9f);
   for (x=0.05f+*slider*(0.9f-0.025f)-0.025f; x>0.05f; x-=0.025f) drawframe(x,0.2f,0.0f,0.6f,30.0f,0.9f,0.9f,0.9f);

   drawstring(0.55f,0.3f,0.4f,0.4f,0.0f,0.0f,0.0f,0.9f,text);

   drawquad(0.05f+*slider*(0.9f-0.025f),0.2f,0.025f,0.6f,30.0f,0.9f,0.9f,0.9f);
   drawframe(0.05f,0.2f,0.9f,0.6f,0.0f,0.0f,0.75f,0.75f);

   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::wheelhookY(float x,float y,void *data)
   {
   const float mouseaccel=1.5f;

   static BOOLINT clicked=FALSE;

   static float ly;

   float *slider=(float *)data;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (clicked) *slider+=mouseaccel*(y-ly);
   else clicked=TRUE;

   while (*slider<0.0f) *slider+=1.0f;
   while (*slider>1.0f) *slider-=1.0f;

   ly=y;
   }

void GUI::wheeldrawY(void *data1,void *data2)
   {
   float *slider=(float *)data1;
   char *text=(char *)data2;

   float y;

   for (y=0.05f+*slider*(0.9f-0.025f)+0.025f; y<0.95f; y+=0.025f) drawframe(0.2f,y,0.6f,0.0f,30.0f,0.9f,0.9f,0.9f);
   for (y=0.05f+*slider*(0.9f-0.025f)-0.025f; y>0.05f; y-=0.025f) drawframe(0.2f,y,0.6f,0.0f,30.0f,0.9f,0.9f,0.9f);

   drawstring(0.3f,0.55f,0.4f,0.375f,0.0f,0.0f,0.0f,0.9f,text);

   drawquad(0.2f,0.05f+*slider*(0.9f-0.025f),0.6f,0.025f,30.0f,0.9f,0.9f,0.9f);
   drawframe(0.2f,0.05f,0.6f,0.9f,0.0f,0.0f,0.75f,0.75f);

   drawframe(0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,0.75f,0.75f);
   }

void GUI::drawline(float x1,float y1,float x2,float y2,
                   float hue,float sat,float val,float alpha)
   {
   float rgb[3];

   hsv2rgb(hue,sat,val,rgb);

   glColor4f(rgb[0],rgb[1],rgb[2],alpha);
   glBegin(GL_LINES);
   glVertex2f(x1,y1);
   glVertex2f(x2,y2);
   glEnd();
   }

void GUI::drawlineRGBA(float x1,float y1,float x2,float y2,
                       float r,float g,float b,float alpha)
   {
   glColor4f(r,g,b,alpha);
   glBegin(GL_LINES);
   glVertex2f(x1,y1);
   glVertex2f(x2,y2);
   glEnd();
   }

void GUI::drawquad(float x,float y,float width,float height,
                   float hue,float sat,float val,float alpha)
   {
   float rgb[3];

   hsv2rgb(hue,sat,val,rgb);

   glColor4f(rgb[0],rgb[1],rgb[2],alpha);
   glBegin(GL_TRIANGLE_FAN);
   glVertex2f(x,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y+height);
   glVertex2f(x,y+height);
   glEnd();
   }

void GUI::drawquadRGBA(float x,float y,float width,float height,
                       float r,float g,float b,float alpha)
   {
   glColor4f(r,g,b,alpha);
   glBegin(GL_TRIANGLE_FAN);
   glVertex2f(x,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y+height);
   glVertex2f(x,y+height);
   glEnd();
   }

void GUI::drawframe(float x,float y,float width,float height,
                    float hue,float sat,float val,float alpha)
   {
   float rgb1[3],rgb2[3];

   hsv2rgb(hue,sat,fmax(val-0.25f,0.0f),rgb1);
   hsv2rgb(hue,sat,fmin(val+0.25f,1.0f),rgb2);

   glBegin(GL_LINES);
   glColor4f(rgb1[0],rgb1[1],rgb1[2],alpha);
   glVertex2f(x,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y+height);
   glColor4f(rgb2[0],rgb2[1],rgb2[2],alpha);
   glVertex2f(x+width,y+height);
   glVertex2f(x,y+height);
   glVertex2f(x,y+height);
   glVertex2f(x,y);
   glEnd();
   }

void GUI::drawframeRGBA(float x,float y,float width,float height,
                        float r,float g,float b,float alpha)
   {
   glColor4f(r,g,b,alpha);
   glBegin(GL_LINES);
   glVertex2f(x,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y);
   glVertex2f(x+width,y+height);
   glVertex2f(x+width,y+height);
   glVertex2f(x,y+height);
   glVertex2f(x,y+height);
   glVertex2f(x,y);
   glEnd();
   }

void GUI::drawsymbol(float hue,float sat,float val,float alpha,const char *symbol)
   {
   float rgb[3];

   float px,py,lx,ly;

   BOOLINT draw=TRUE;

   hsv2rgb(hue,sat,val,rgb);

   glColor4f(rgb[0],rgb[1],rgb[2],alpha);
   glBegin(GL_LINES);

   px=py=0.0f;

   while (*symbol!='\0')
      {
      lx=px;
      ly=py;

      switch (*symbol++)
         {
         case 'u': draw=FALSE; break;
         case 'd': draw=TRUE; break;
         case 'n': py+=1.0f; break;
         case 's': py-=1.0f; break;
         case 'e': px+=1.0f; break;
         case 'w': px-=1.0f; break;
         case 'N': py+=1.0f; px+=1.0f; break;
         case 'S': py-=1.0f; px-=1.0f; break;
         case 'E': px+=1.0f; py-=1.0f; break;
         case 'W': px-=1.0f; py+=1.0f; break;
         }

      if (draw)
         if (px!=lx || py!=ly)
            {
            glVertex2f(lx,ly);
            glVertex2f(px,py);
            }
      }

   glEnd();
   }

void GUI::drawletter(float hue,float sat,float val,float alpha,char letter)
   {
   glPushMatrix();
   glScalef(1.0f/4,1.0f/6,0.0f);

   switch (toupper(letter))
      {
      case 'A': drawsymbol(hue,sat,val,alpha,"nnnnnNeEsssssunnndwww"); break;
      case 'B': drawsymbol(hue,sat,val,alpha,"nnnnnneeEsSwwueedEsSww"); break;
      case 'C': drawsymbol(hue,sat,val,alpha,"ueeNdSwWnnnnNeE"); break;
      case 'D': drawsymbol(hue,sat,val,alpha,"nnnnnneeEssssSww"); break;
      case 'E': drawsymbol(hue,sat,val,alpha,"ueeedwwwnnnnnneeeussswdww"); break;
      case 'F': drawsymbol(hue,sat,val,alpha,"nnnnnneeeussswdww"); break;
      case 'G': drawsymbol(hue,sat,val,alpha,"unnneedessSwWnnnnNeE"); break;
      case 'H': drawsymbol(hue,sat,val,alpha,"nnnnnnueeedssssssunnndwww"); break;
      case 'I': drawsymbol(hue,sat,val,alpha,"uedeeuwdnnnnnneuwdw"); break;
      case 'J': drawsymbol(hue,sat,val,alpha,"undEeNnnnnnwww"); break;
      case 'K': drawsymbol(hue,sat,val,alpha,"nnnnnnusssdNNNuSSSdEEE"); break;
      case 'L': drawsymbol(hue,sat,val,alpha,"ueeedwwwnnnnnn"); break;
      case 'M': drawsymbol(hue,sat,val,alpha,"nnnnnnEeNssssss"); break;
      case 'N': drawsymbol(hue,sat,val,alpha,"nnnnnnusdEEEssunndnnnn"); break;
      case 'O': drawsymbol(hue,sat,val,alpha,"uedWnnnnNeEssssSw"); break;
      case 'P': drawsymbol(hue,sat,val,alpha,"nnnnnneeEsSww"); break;
      case 'Q': drawsymbol(hue,sat,val,alpha,"uedWnnnnNeEssssSwuNdE"); break;
      case 'R': drawsymbol(hue,sat,val,alpha,"nnnnnneeEsSwwuedEEs"); break;
      case 'S': drawsymbol(hue,sat,val,alpha,"undEeNnWwWnNeE"); break;
      case 'T': drawsymbol(hue,sat,val,alpha,"ueednnnnnnwwueedee"); break;
      case 'U': drawsymbol(hue,sat,val,alpha,"unnnnnndsssssEeNnnnnn"); break;
      case 'V': drawsymbol(hue,sat,val,alpha,"unnnnnndsssssENNnnnn"); break;
      case 'W': drawsymbol(hue,sat,val,alpha,"unnnnnndssssssNeEnnnnnn"); break;
      case 'X': drawsymbol(hue,sat,val,alpha,"nnNNNnuwwwdsEEEss"); break;
      case 'Y': drawsymbol(hue,sat,val,alpha,"ueednnWWnnueeedsssS"); break;
      case 'Z': drawsymbol(hue,sat,val,alpha,"ueeendswwwnnNNNnwwws"); break;
      case '0': drawsymbol(hue,sat,val,alpha,"uedWnnnnNeEssssSwuNNdWWW"); break;
      case '1': drawsymbol(hue,sat,val,alpha,"ueednnnnnnSS"); break;
      case '2': drawsymbol(hue,sat,val,alpha,"ueeedwwwNNNnnWwSs"); break;
      case '3': drawsymbol(hue,sat,val,alpha,"undEeNnWwuedNnWwS"); break;
      case '4': drawsymbol(hue,sat,val,alpha,"ueednnnnnnuwwdssseee"); break;
      case '5': drawsymbol(hue,sat,val,alpha,"undEeNnWwwnnneee"); break;
      case '6': drawsymbol(hue,sat,val,alpha,"unndNeEsSwWnnnnNeE"); break;
      case '7': drawsymbol(hue,sat,val,alpha,"ueednnnnnnwwusssedee"); break;
      case '8': drawsymbol(hue,sat,val,alpha,"uedWnNWnNeEsSwuedEsSw"); break;
      case '9': drawsymbol(hue,sat,val,alpha,"undEeNnnnnWwSsEee"); break;
      }

   glPopMatrix();
   }

void GUI::drawstring(float x,float y,float width,float height,
                     float hue,float sat,float val,float alpha,char *str)
   {
   const float linefeed=0.2f;

   int c,cmax,l;
   char *ptr;

   if (str==NULL) return;

   for (c=0,cmax=l=1,ptr=str; *ptr!='\0'; ptr++)
      {
      if (*ptr!='\n') c++;
      else {c=0; l++;}
      if (c>cmax) cmax=c;
      }

   glPushMatrix();
   glTranslatef(x,y,0.0f);
   glScalef(width/cmax,height/(l+(l-1)*linefeed),0.0f);
   glTranslatef(0.0f,(l-1)*(1.0f+linefeed),0.0f);
   glPushMatrix();

   while (*str!='\0')
      {
      if (*str=='\n')
         {
         glPopMatrix();
         glTranslatef(0.0f,-(1.0f+linefeed),0.0f);
         glPushMatrix();
         }
      else
         {
         if (islower(*str))
            {
            glPushMatrix();
            glTranslatef(0.2f,0.0f,0.0f);
            glScalef(0.6f,0.75f,1.0f);
            }

         drawletter(hue,sat,val,alpha,*str);

         if (islower(*str)) glPopMatrix();

         glTranslatef(1.0f,0.0f,0.0f);
         }

      str++;
      }

   glPopMatrix();
   glPopMatrix();
   }

void GUI::drawtexture(float x,float y,float width,float height,
                      float hue,float sat,float val,float alpha,
                      int texid,int sizex,int sizey)
   {
   float rgb[3];

   hsv2rgb(hue,sat,val,rgb);

   glEnable(GL_TEXTURE_2D);

   glBindTexture(GL_TEXTURE_2D,texid);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glTranslatef(0.5f/sizex,0.5f/sizey,0.0f);
   glScalef((float)(sizex-1)/sizex,(float)(sizey-1)/sizey,1.0f);
   glMatrixMode(GL_MODELVIEW);

   glColor4f(rgb[0],rgb[1],rgb[2],alpha);
   glBegin(GL_TRIANGLE_FAN);
   glTexCoord2f(0.0f,0.0f);
   glVertex2f(x,y);
   glTexCoord2f(1.0f,0.0f);
   glVertex2f(x+width,y);
   glTexCoord2f(1.0f,1.0f);
   glVertex2f(x+width,y+height);
   glTexCoord2f(0.0f,1.0f);
   glVertex2f(x,y+height);
   glEnd();

   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   glBindTexture(GL_TEXTURE_2D,0);

   glDisable(GL_TEXTURE_2D);
   }

int GUI::buildtexmap2DL(unsigned char *image,
                        int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,width,height,0,
                GL_LUMINANCE,GL_UNSIGNED_BYTE,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

int GUI::buildtexmap2DL(float *image,
                        int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,width,height,0,
                GL_LUMINANCE,GL_FLOAT,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

int GUI::buildtexmap2DRGB(unsigned char *image,
                          int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,
                GL_RGB,GL_UNSIGNED_BYTE,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

int GUI::buildtexmap2DRGB(float *image,
                          int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,
                GL_RGB,GL_FLOAT,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

int GUI::buildtexmap2DRGBA(unsigned char *image,
                           int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,
                GL_RGBA,GL_UNSIGNED_BYTE,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

int GUI::buildtexmap2DRGBA(float *image,
                           int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,
                GL_RGBA,GL_FLOAT,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

void GUI::deletetexmap(int texid)
   {
   GLuint GLtexid=texid;
   if (texid>0) glDeleteTextures(1,&GLtexid);
   }

void GUI::hsv2rgb(float hue,float sat,float val,float *rgb)
   {
   float hue6,r,s,t;

   if (hue<0.0f || sat<0.0f || sat>1.0f || val<0.0f || val>1.0f) ERRORMSG();

   hue/=60.0f;
   hue=hue-6.0f*ftrc(hue/6.0f);
   hue6=hue-ftrc(hue);

   r=val*(1.0f-sat);
   s=val*(1.0f-sat*hue6);
   t=val*(1.0f-sat*(1.0f-hue6));

   switch (ftrc(hue))
        {
        case 0: // red -> yellow
           rgb[0] = val;
           rgb[1] = t;
           rgb[2] = r;
           break;
        case 1: // yellow -> green
           rgb[0] = s;
           rgb[1] = val;
           rgb[2] = r;
           break;
        case 2: // green -> cyan
           rgb[0] = r;
           rgb[1] = val;
           rgb[2] = t;
           break;
        case 3: // cyan -> blue
           rgb[0] = r;
           rgb[1] = s;
           rgb[2] = val;
           break;
        case 4: // blue -> magenta
           rgb[0] = t;
           rgb[1] = r;
           rgb[2] = val;
           break;
        case 5: // magenta -> red
           rgb[0] = val;
           rgb[1] = r;
           rgb[2] = s;
           break;
        }
   }
