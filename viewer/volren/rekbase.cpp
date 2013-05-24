// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

unsigned char *readREKvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                             float *scalex,float *scaley,float *scalez)
   {
   int i;

   FILE *file;

   unsigned char data[2];
   unsigned char *volume;

   unsigned int rekwidth,rekheight,rekdepth,rekbits,rekcomps;
   unsigned int rekdummy;

   // open file to analyze 2048 byte header
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // volume width
   if (fread(&data,1,2,file)!=2) return(NULL);
   rekwidth=data[0]+256*data[1];

   // volume height
   if (fread(&data,1,2,file)!=2) return(NULL);
   rekheight=data[0]+256*data[1];

   // volume bits
   if (fread(&data,1,2,file)!=2) return(NULL);
   rekbits=data[0]+256*data[1];

   // volume depth (slices)
   if (fread(&data,1,2,file)!=2) return(NULL);
   rekdepth=data[0]+256*data[1];

   if (rekwidth<1 || rekheight<1 || rekdepth<1) return(NULL);
   if (rekbits!=8 && rekbits!=16) return(NULL);

   if (rekbits==8) rekcomps=1;
   else rekcomps=2;

   if (rekcomps!=1 && components==NULL) return(NULL);

   // check 3x 0x01000000 signature at 0x3b8
   if (fseek(file,0x3b8,SEEK_SET)==-1) return(NULL);
   for (i=0; i<3; i++)
      {
      if (fread(&data,1,2,file)!=2) return(NULL);
      rekdummy=data[0]+256*data[1];
      if (rekdummy!=1) return(NULL);

      if (fread(&data,1,2,file)!=2) return(NULL);
      rekdummy=data[0]+256*data[1];
      if (rekdummy!=0) return(NULL);
      }

   // seek to raw data
   if (fseek(file,2048,SEEK_SET)==-1) return(NULL);

   if ((volume=(unsigned char *)malloc(rekwidth*rekheight*rekdepth*rekcomps))==NULL) return(NULL);

   if (fread(volume,rekwidth*rekheight*rekdepth*rekcomps,1,file)!=1)
      {
      free(volume);
      return(NULL);
      }

   fclose(file);

   *width=rekwidth;
   *height=rekheight;
   *depth=rekdepth;

   if (components!=0) *components=rekcomps;

   if (scalex!=NULL) *scalex=1.0f;
   if (scaley!=NULL) *scaley=1.0f;
   if (scalez!=NULL) *scalez=1.0f;

   return(volume);
   }
