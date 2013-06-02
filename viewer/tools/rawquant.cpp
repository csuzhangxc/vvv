#include "codebase.h"
#include "rawbase.h"

static const float ratio=0.5f;
static const long long maxcells=250000000;

int main(int argc,char *argv[])
   {
   char *output;

   if (argc!=2)
      {
      printf("usage: %s <input.raw>\n",argv[0]);
      exit(1);
      }

   output=processRAWvolume(argv[1],ratio,maxcells);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }

   return(0);
   }
