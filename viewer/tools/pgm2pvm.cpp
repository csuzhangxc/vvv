#include "codebase.h"

#include "ddsbase.h"

int main(int argc,char *argv[])
   {
   unsigned int i,j;

   unsigned char *data,*image;
   unsigned int width,height,depth;
   unsigned int components;

   unsigned int width2,height2;
   unsigned int components2;

   if (argc<3)
      {
      printf("usage: %s {<input.pgm>} <output.pvm>\n",argv[0]);
      printf(" input: pnm image series with consistent size and type\n");
      printf(" output: compressed PVM volume containing the image series\n");
      exit(1);
      }

   depth=argc-2;

   if ((image=readPNMimage(argv[1],&width,&height,&components))==NULL) exit(1);
   if (components!=1 && components!=3 && components!=4) exit(1);

   data=(unsigned char *)malloc(width*height*depth*components);

   for (j=0; j<height; j++)
      memcpy(&data[j*width*components],&image[(height-1-j)*width*components],width*components);
   free(image);

   for (i=2; i<depth; i++)
      {
      if ((image=readPNMimage(argv[i],&width2,&height2,&components2))==NULL) exit(1);
      if (components2!=components || width2!=width || height2!=height) exit(1);

      for (j=0; j<height; j++)
         memcpy(&data[i*width*height*components+j*width*components],&image[(height-1-j)*width*components],width*components);
      free(image);
      }

   writePVMvolume(argv[argc-1],data,width,height,depth,components);
   free(data);

   return(0);
   }
