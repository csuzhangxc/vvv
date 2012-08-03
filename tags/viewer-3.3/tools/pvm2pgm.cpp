// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "ddsbase.h"

#define MAX_STR 1000

int main(int argc,char *argv[])
   {
   unsigned int i;

   unsigned char *volume;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   char filename[MAX_STR];

   if (argc!=3)
      {
      printf("usage: %s <input.pvm> <output>\n",argv[0]);
      exit(1);
      }

   printf("reading PVM file\n");

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components,&scalex,&scaley,&scalez))==NULL) exit(1);
   if (volume==NULL) exit(1);

   printf("found volume with width=%d height=%d depth=%d components=%d\n",
          width,height,depth,components);

   if (scalex!=1.0f || scaley!=1.0f || scalez!=1.0f)
      printf("and edge length %g/%g/%g\n",scalex,scaley,scalez);

   printf("and data checksum=%08X\n",checksum(volume,width*height*depth*components));

   if (components==2)
      {
      volume=quantize(volume,width,height,depth);
      components=1;
      }

   for (i=0; i<depth; i++)
      {
      printf("writing PGM file #%d\n",i+1);

      snprintf(filename,MAX_STR,"%s-%04d.pgm",argv[2],i+1);
      writePNMimage(filename,volume+i*width*height*components,width,height,components);
      }

   free(volume);

   return(0);
   }
