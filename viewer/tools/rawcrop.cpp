#include "codebase.h"
#include "rawbase.h"

static const float ratio=0.5f;

int main(int argc,char *argv[])
   {
   char *output;

   if (argc!=3)
      {
      printf("usage: %s <input.raw> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw cropped volume\n");
      exit(1);
      }

   output=cropRAWvolume(argv[1],argv[2],ratio);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }

   return(0);
   }
