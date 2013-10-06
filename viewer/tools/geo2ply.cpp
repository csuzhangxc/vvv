// (c) by Stefan Roettger, licensed under GPL 2+

#ifdef HAVE_MINI
#include <mini/ministrip.h>
#endif

int main(int argc,char *argv[])
   {
   if (argc!=3)
      {
      printf("usage: %s <geometry.geo> <geometry.ply>\n",argv[0]);
      printf(" load geometry data (libmini geometry format)\n");
      printf(" and convert it into a ply file (polygon file format)\n");
      exit(1);
      }

#ifdef HAVE_MINI

   ministrip strip;

   printf("loading geometry\n");

   if (!strip.readGEOfile(argv[1]))
      {
      printf("load failure\n");
      exit(1);
      }

   printf("saving geometry\n");

   strip.writePLYfile(argv[2]);

#else

   ERRORMSG();

#endif

   return(0);
   }
