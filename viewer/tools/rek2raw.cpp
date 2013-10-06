// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#ifdef HAVE_MINI
#include <mini/rekbase.h>
#endif

int main(int argc,char *argv[])
   {
   if (argc!=3)
      {
      printf("usage: %s <input.rek> <output.raw>\n",argv[0]);
      printf(" input: 8bit or 16bit Fraunhofer volume file format (with 2048 byte header)\n");
      printf(" output: 8bit or 16bit LSB raw volume\n");
      exit(1);
      }

#ifdef HAVE_MINI

   char *output;

   output=copyREKvolume(argv[1],argv[2]);

   if (output)
      {
      printf("wrote %s\n",output);
      free(output);
      }
   else exit(1);

#else

   ERRORMSG();

#endif

   return(0);
   }
