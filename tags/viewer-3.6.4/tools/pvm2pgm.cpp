// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "ddsbase.h"
#include "rawbase.h"

#define MAX_STR 1000

int main(int argc,char *argv[])
   {
   unsigned int i,j;

   unsigned char *volume;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   unsigned char *image;

   char filename[MAX_STR];

   if (argc!=3)
      {
      printf("usage: %s <input.pvm> <output.pgm>\n",argv[0]);
      printf(" input: compressed PVM volume\n");
      printf(" output: pgm image series containing the volume slices\n");
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

   if ((image=(unsigned char *)malloc(width*height*components))==NULL) exit(1);

   for (i=0; i<depth; i++)
      {
      printf("writing PGM file #%d\n",i+1);

      for (j=0; j<height; j++)
         memcpy(&image[(height-1-j)*width*components],&volume[i*width*height*components+j*width*components],width*components);

      snprintf(filename,MAX_STR,"%s-%04d.pgm",argv[2],i+1);
      writePNMimage(filename,image,width,height,components);
      }

   free(image);

   free(volume);

   return(0);
   }
