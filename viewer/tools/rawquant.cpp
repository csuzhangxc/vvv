// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#ifdef HAVE_MINI
#include <mini/rawbase.h>
#include <mini/rekbase.h>
#endif

static float ratio=0.5f;
static unsigned long long maxsize=512;

int main(int argc,char *argv[])
   {
   unsigned long long cell_limit;

   if (argc!=2 && argc!=3)
      {
      printf("usage: %s <input.raw> [<volume size limit in mb>]\n",argv[0]);
      exit(1);
      }

   if (argc==3)
      if (sscanf(argv[2],"%llu",&maxsize)!=1) exit(1);

   cell_limit=maxsize*maxsize*maxsize;

#ifdef HAVE_MINI

   char *output;

   output=processRAWvolume(argv[1],NULL,ratio,cell_limit);
   if (output==NULL) output=processREKvolume(argv[1],NULL,ratio,cell_limit);

   if (output)
      {
      printf("output %s\n",output);
      free(output);
      }
   else exit(1);

#else

   ERRORMSG();

#endif

   return(0);
   }
