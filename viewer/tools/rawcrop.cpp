// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#ifdef HAVE_MINI
#include <mini/rawbase.h>
#endif

static const float ratio=0.5f;

int main(int argc,char *argv[])
   {
   if (argc!=3)
      {
      printf("usage: %s <input.raw> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw cropped volume\n");
      exit(1);
      }

#ifdef HAVE_MINI

   char *output;

   output=cropRAWvolume(argv[1],argv[2],ratio);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }

#else

   ERRORMSG();

#endif

   return(0);
   }
