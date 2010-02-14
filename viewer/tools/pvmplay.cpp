// (c) by Stefan Roettger

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base and window handling
#include "ddsbase.h" // volume file reader

#define STR_MAX (256)

unsigned char *volume,*volume2;

unsigned int width,height,depth,
             components;

unsigned int frame=0;

unsigned char *image;

unsigned int imgwidth,imgheight;

void handler(float time)
   {
   unsigned int i;

   GLuint texid;

   char str[STR_MAX];

   static BOOLINT pause=TRUE;

   if (frame==depth-1) pause=TRUE;

   snprintf(str,STR_MAX,"frame=%d",frame+1);
   setwindowinfo(str);

   switch (getkey())
      {
      case ' ':
         pause=!pause;
         break;
      case 'b':
         pause=TRUE;
         frame=0;
         break;
      case 'e':
         pause=TRUE;
         frame=depth-1;
         break;
      case '-':
      case '<':
      case '1':
         pause=TRUE;
         if (frame>0) frame--;
         break;
      case '+':
      case '=':
      case '>':
      case '2':
         pause=TRUE;
         if (frame<depth-1) frame++;
         break;
      case '\033':
      case 'q':
         free(image);
         free(volume);
         closewindow();
         exit(0);
      }

   clearwindow();

   for (i=0; i<height; i++)
      memcpy(&image[i*3*imgwidth],&volume[frame*3*width*height+i*3*width],3*width);

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,imgwidth,imgheight,0,GL_RGB,GL_UNSIGNED_BYTE,image);

   glDepthMask(GL_FALSE);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_DITHER);
   glDisable(GL_CULL_FACE);

   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glTranslatef(0.5f/imgwidth,0.5f/imgheight,0.0f);
   glScalef((float)(width-1)/imgwidth,(float)(height-1)/imgheight,0.0f);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0f,getwinwidth()-1,0.0f,getwinheight()-1,-1.0f,1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   if (width*getwinheight()<getwinwidth()*height)
      {
      glTranslatef((getwinwidth()-width*getwinheight()/height)/2.0f,0.0f,0.0f);
      glScalef(width*getwinheight()/height,getwinheight(),0.0f);
      }
   else
      {
      glTranslatef(0.0f,(getwinheight()-height*getwinwidth()/width)/2.0f,0.0f);
      glScalef(getwinwidth(),height*getwinwidth()/width,0.0f);
      }

   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

   glEnable(GL_TEXTURE_2D);

   glColor3f(1.0f,1.0f,1.0f);
   glBegin(GL_TRIANGLE_FAN);
   glTexCoord2f(0.0f,0.0f);
   glVertex3f(0.0f,0.0f,0.0f);
   glTexCoord2f(1.0f,0.0f);
   glVertex3f(1.0f,0.0f,0.0f);
   glTexCoord2f(1.0f,1.0f);
   glVertex3f(1.0f,1.0f,0.0f);
   glTexCoord2f(0.0f,1.0f);
   glVertex3f(0.0f,1.0f,0.0f);
   glEnd();

   glDisable(GL_TEXTURE_2D);
   glDeleteTextures(1,&texid);

   glDepthMask(GL_TRUE);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_DITHER);
   glEnable(GL_CULL_FACE);

   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   swapbuffers();

   if (!pause)
      if (frame<depth-1) frame++;
      else frame=0;
   }

int main(int argc,char *argv[])
   {
   unsigned int i;

   float fps=25.0f;

   if (argc<2 || argc>4)
      {
      printf("usage: %s <movie.pvm> [<frame number> [<output.pnm>]]\n",argv[0]);
      exit(1);
      }

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components))==NULL) exit(1);

  if (argc>2)
      {
      if (sscanf(argv[2],"%d",&frame)!=1) exit(1);
      if (frame<1 || frame>depth) exit(1);
      frame--;
      }

  if (components==2)
      {
      volume=quantize(volume,width,height,depth);
      components=1;
      }

   if (argc==4)
      {
      writePNMimage(argv[3],&volume[frame*width*height*components],width,height,components);
      free(volume);
      }
   else
      {
      if (components==1)
         {
         if ((volume2=(unsigned char *)malloc(3*width*height*depth))==NULL) exit(1);

         for (i=0; i<width*height*depth; i++)
            volume2[3*i]=volume2[3*i+1]=volume2[3*i+2]=volume[i];

         free(volume);
         volume=volume2;
         }
      else if (components!=3) exit(1);

      for (imgwidth=2; imgwidth<width; imgwidth*=2);
      for (imgheight=2; imgheight<height; imgheight*=2);

      if ((image=(unsigned char *)malloc(3*imgwidth*imgheight))==NULL) exit(1);
      memset(image,0,3*imgwidth*imgheight);

      openwindow(width,height,fps,argv[1]);
      addhandler(handler);
      }

   return(0);
   }
