#include "codebase.h"

#ifdef HAVE_MINI
#include <mini/rawbase.h>
#endif

static const float ratio=0.5f;
static const unsigned long long maxcells=250000000;

int main(int argc,char *argv[])
   {
   if (argc!=2)
      {
      printf("usage: %s <input.raw>\n",argv[0]);
      exit(1);
      }

#ifdef HAVE_MINI

   char *output;

   output=processRAWvolume(argv[1],ratio,maxcells);

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
