#include "codebase.h"

#include "ddsbase.h"

int main(int argc,char *argv[])
   {
   unsigned char *volume;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   if (argc!=2 && argc!=3)
      {
      printf("usage: %s <input.pvm> [<output.raw>]\n",argv[0]);
      exit(1);
      }

   printf("reading PVM file\n");

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components,&scalex,&scaley,&scalez))==NULL) exit(1);

   printf("found volume with width=%d height=%d depth=%d components=%d\n",
          width,height,depth,components);

   if (scalex!=1.0f || scaley!=1.0f || scalez!=1.0f)
      printf("and edge length %g/%g/%g\n",scalex,scaley,scalez);

   printf("and data checksum=%08X\n",checksum(volume,width*height*depth*components));

   if (argc>2)
      {
      writeRAWfile(argv[2],volume,width*height*depth*components,1);

      printf("wrote RAW file with size=%d\n",width*height*depth*components);
      }

   free(volume);

   return(0);
   }
