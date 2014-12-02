// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "ddsbase.h"

int main(int argc,char *argv[])
   {
   const char info1[]="quantized from 16 bit to 8 bit by pvm2pvm tool";
   const char info2[]="\nquantized from 16 bit to 8 bit by pvm2pvm tool";

   unsigned char *volume,*volume2;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   unsigned char *desc,*cour,*scan,*comm;

   if (argc!=2 && argc!=3)
      {
      printf("usage: %s <16 bit input.pvm> [<8 bit output.pvm>]\n",argv[0]);
      exit(1);
      }

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components,&scalex,&scaley,&scalez,&desc,&cour,&scan,&comm))==NULL) exit(1);

   printf("found volume with width=%d height=%d depth=%d components=%d\n",
          width,height,depth,components);

   if (components==2)
      {
      volume2=quantize(volume,width,height,depth,FALSE,TRUE);

      printf("quantized 16 bit volume to 8 bit using a non-linear mapping\n");

      if (comm!=NULL)
         {
         if ((comm=(unsigned char *)strdup((char *)comm))==NULL) ERRORMSG();
         if ((comm=(unsigned char *)realloc(comm,strlen((char *)comm)+strlen(info2)+1))==NULL) ERRORMSG();
         strcat((char *)comm,info2);
         }
      else
         if ((comm=(unsigned char *)strdup(info1))==NULL) ERRORMSG();

      if (argc>2) writePVMvolume(argv[2],volume2,width,height,depth,1,scalex,scaley,scalez,desc,cour,scan,comm);

      free(volume2);
      free(comm);
      }

   free(volume);

   return(0);
   }
