#include "codebase.h"
#include "rawbase.h"

static const double ratio=0.5;

int main(int argc,char *argv[])
   {
   if (argc!=3)
      {
      printf("usage: %s <input.raw> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw cropped volume\n");
      exit(1);
      }

   cropRAWvolume(argv[1],argv[2]);

   return(0);
   }
