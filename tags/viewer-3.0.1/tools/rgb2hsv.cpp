#include "codebase.h"

#include "ddsbase.h"

void rgb2hsv(float r,float g,float b,float *hsv)
   {
   float h,s,v;
   float maxv,minv,diff,rdist,gdist,bdist;

   maxv=fmax(r,fmax(g,b));
   minv=fmin(r,fmin(g,b));
   diff=maxv-minv;

   v=maxv;

   if (maxv!=0.0f) s=diff/maxv;
   else s=0.0f;

   if (s==0.0f) h=0.0f;
   else
      {
      rdist=(maxv-r)/diff;
      gdist=(maxv-g)/diff;
      bdist=(maxv-b)/diff;

      if (r==maxv) h=bdist-gdist;
      else if (g==maxv) h=2.0f+rdist-bdist;
      else h=4.0f+gdist-rdist;

      h*=60.0f;
      if (h<0.0f) h+=360.0f;
      }

   hsv[0]=h;
   hsv[1]=s;
   hsv[2]=v;
   }

int main(int argc,char *argv[])
   {
   unsigned int i;

   float hsv[3];

   unsigned char *volume,
                 *data1,*data2,*data3;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   if (argc<3 || argc>5)
      {
      printf("usage: %s <input.pvm> <hue.pvm> [<sat.pvm> [<val.pvm>]]\n",argv[0]);
      exit(1);
      }

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components,&scalex,&scaley,&scalez))==NULL) exit(1);
   if (components!=3) exit(1);

   if ((data1=(unsigned char *)malloc(width*height*depth))==NULL) exit(1);
   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) exit(1);
   if ((data3=(unsigned char *)malloc(width*height*depth))==NULL) exit(1);

   for (i=0; i<width*height*depth; i++)
      {
      rgb2hsv(volume[3*i]/255.0f,volume[3*i+1]/255.0f,volume[3*i+2]/255.0f,hsv);

      data1[i]=ftrc(255.0f*hsv[0]+0.5f);
      data2[i]=ftrc(255.0f*hsv[1]+0.5f);
      data3[i]=ftrc(255.0f*hsv[2]+0.5f);
      }

   free(volume);

   if (argc>=3) writePVMvolume(argv[2],data1,width,height,depth,1,scalex,scaley,scalez);
   if (argc>=4) writePVMvolume(argv[3],data2,width,height,depth,1,scalex,scaley,scalez);
   if (argc==5) writePVMvolume(argv[4],data3,width,height,depth,1,scalex,scaley,scalez);

   free(data1);
   free(data2);
   free(data3);

   return(0);
   }
