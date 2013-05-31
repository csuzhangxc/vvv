#include "codebase.h"
#include "rawbase.h"

int main(int argc,char *argv[])
   {
   char *output;

   if (argc!=2)
      {
      printf("usage: %s <input.raw>\n",argv[0]);
      exit(1);
      }

   output=processRAWvolume(argv[1]);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }

   return(0);
   }
