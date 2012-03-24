// (c) by Stefan Roettger, licensed under GPL 2+

#include "ddsbase.h"

int main(int argc,char *argv[])
   {
   unsigned char *data;
   unsigned int bytes;

   if (argc!=2)
      {
      printf("usage: %s <volume.pvm>\n",argv[0]);
      exit(1);
      }

   if ((data=readDDSfile(argv[1],&bytes))==NULL)
      if ((data=readRAWfile(argv[1],&bytes))==NULL) exit(1);
      else writeDDSfile(argv[1],data,bytes);
   else writeRAWfile(argv[1],data,bytes);

   return(0);
   }
