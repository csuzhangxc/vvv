// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef HAVE_MINI
#include "codebase.h"
#else
#include <mini/minibase.h>

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

   if (!ministrip::convertGEO2PLYfile(argv[1],argv[2]))
      {
      printf("conversion failure\n");
      exit(1);
      }

#else

   ERRORMSG();

#endif

   return(0);
   }
