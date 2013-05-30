#include "codebase.h"
#include "rawbase.h"

int main(int argc,char *argv[])
   {
   if (argc!=3)
      {
      printf("usage: %s <input.raw> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw quantized volume\n");
      exit(1);
      }

   copyRAWvolume_nonlinear(argv[1],argv[2]);

   return(0);
   }
