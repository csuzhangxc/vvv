// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef HAVE_MINI
#include "codebase.h"
#else
#include <mini/minibase.h>

#include <mini/rawbase.h>
#include <mini/rekbase.h>
#endif

void feedback(const char *info,float percent,void *obj)
   {
   static float last=0.0f;

   if (fabs(percent-last)>=0.01f)
      {
      printf("%s: %d%%\n",info,(int)(100.0f*percent+0.5f));
      last=percent;
      }
   }

int main(int argc,char *argv[])
   {
   double isovalue=0.5;

   if (argc!=3 && argc!=4)
      {
      printf("usage: %s <volume.raw> <iso.geo> [<iso value>]\n",argv[0]);
      printf(" load volume data (raw and rek volume format)\n");
      printf(" and convert it into an iso surface (libmini geometry format)\n");
      exit(1);
      }

   if (argc==4)
      if (sscanf(argv[3],"%lg",&isovalue)!=1) exit(1);

#ifdef HAVE_MINI

   char *outname;

   outname=extractRAWvolume(argv[1],argv[2],isovalue,feedback);
   if (outname==NULL) outname=extractREKvolume(argv[1],argv[2],isovalue,feedback);

   if (outname==NULL) exit(1);
   else free(outname);

#else

   ERRORMSG();

#endif

   return(0);
   }
