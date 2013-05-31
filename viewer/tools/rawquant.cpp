#include "codebase.h"
#include "rawbase.h"

int main(int argc,char *argv[])
   {
   char *output;

   if (argc!=3)
      {
      printf("usage: %s <input.raw> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw quantized volume\n");
      exit(1);
      }

   output=processRAWvolume(argv[1],argv[2]);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }

   return(0);
   }
