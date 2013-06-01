// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"
#include "rawbase.h"

#include "rekbase.h"

// analyze 2048 byte REK header
BOOLINT readREKheader(FILE *file,
                      unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                      float *scalex,float *scaley,float *scalez)
   {
   int i;

   unsigned char data[2];

   unsigned int rekwidth,rekheight,rekdepth,rekbits,rekcomps;
   unsigned int rekdummy;

   // volume width
   if (fread(&data,1,2,file)!=2) return(FALSE);
   rekwidth=data[0]+256*data[1];

   // volume height
   if (fread(&data,1,2,file)!=2) return(FALSE);
   rekheight=data[0]+256*data[1];

   // volume bits
   if (fread(&data,1,2,file)!=2) return(FALSE);
   rekbits=data[0]+256*data[1];

   // volume depth (slices)
   if (fread(&data,1,2,file)!=2) return(FALSE);
   rekdepth=data[0]+256*data[1];

   if (rekwidth<1 || rekheight<1 || rekdepth<1) return(FALSE);
   if (rekbits!=8 && rekbits!=16) return(FALSE);

   if (rekbits==8) rekcomps=1;
   else rekcomps=2;

   if (rekcomps!=1 && components==NULL) return(FALSE);

   // check 3x 0x01000000 signature at 0x3b8
   if (fseek(file,0x3b8,SEEK_SET)==-1) return(FALSE);
   for (i=0; i<3; i++)
      {
      if (fread(&data,1,2,file)!=2) return(FALSE);
      rekdummy=data[0]+256*data[1];
      if (rekdummy!=1) return(FALSE);

      if (fread(&data,1,2,file)!=2) return(FALSE);
      rekdummy=data[0]+256*data[1];
      if (rekdummy!=0) return(FALSE);
      }

   // seek to raw data
   if (fseek(file,2048,SEEK_SET)==-1) return(FALSE);

   *width=rekwidth;
   *height=rekheight;
   *depth=rekdepth;

   if (components!=0) *components=rekcomps;

   if (scalex!=NULL) *scalex=1.0f;
   if (scaley!=NULL) *scaley=1.0f;
   if (scalez!=NULL) *scalez=1.0f;

   return(TRUE);
   }

// read a REK volume (Fraunhofer EZRT volume format)
unsigned char *readREKvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                             float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   unsigned char *volume;
   unsigned long long bytes;

   // open REK file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze REK header
   if (!readREKheader(file,width,height,depth,components,
                      scalex,scaley,scalez))
      {
      fclose(file);
      return(NULL);
      }

   bytes=(*width)*(*height)*(*depth)*(*components);

   if ((volume=(unsigned char *)malloc(bytes))==NULL) return(NULL);

   // read volume
   if (fread(volume,bytes,1,file)!=1)
      {
      free(volume);
      fclose(file);
      return(NULL);
      }

   fclose(file);

   return(volume);
   }

// read REK file format header
BOOLINT readREKheader(const char *filename,
                      unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                      float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   // open REK file
   if ((file=fopen(filename,"rb"))==NULL) return(FALSE);

   // analyze REK header
   if (!readREKheader(file,width,height,depth,components,
                      scalex,scaley,scalez))
      {
      fclose(file);
      return(FALSE);
      }

   fclose(file);

   return(TRUE);
   }

// copy a REK volume to a RAW volume
char *copyREKvolume(const char *filename,const char *output)
   {
   FILE *file;

   unsigned int width,height,depth,components;
   float scalex,scaley,scalez;

   char *outname;

   // open REK file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze REK header
   if (!readREKheader(file,&width,&height,&depth,&components,
                      &scalex,&scaley,&scalez))
      {
      fclose(file);
      return(NULL);
      }

   // copy REK data to RAW file
   if (!(outname=copyRAWvolume(file,output,
                               width,height,depth,1,
                               components,8,FALSE,FALSE,
                               scalex,scaley,scalez)))
      {
      fclose(file);
      return(NULL);
      }

   fclose(file);

   return(outname);
   }

// copy a REK volume to a RAW volume with out-of-core cropping and non-linear quantization
char *processREKvolume(const char *filename,const char *output)
   {
   FILE *file;

   unsigned int width,height,depth,components;
   float scalex,scaley,scalez;

   char *outname;

   // open REK file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze REK header
   if (!readREKheader(file,&width,&height,&depth,&components,
                      &scalex,&scaley,&scalez))
      {
      fclose(file);
      return(NULL);
      }

   // copy REK data to RAW file
   if (!(outname=processRAWvolume(file,output,
                                  width,height,depth,1,
                                  components,8,FALSE,FALSE,
                                  scalex,scaley,scalez)))
      {
      fclose(file);
      return(NULL);
      }

   fclose(file);

   return(outname);
   }

// read a REK volume out-of-core
unsigned char *readREKvolume_ooc(const char *filename,
                                 unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                                 float *scalex,float *scaley,float *scalez)
   {
   char *outname;

   unsigned char *volume;
   unsigned int steps;

   volume=NULL;

   outname=processREKvolume(filename,filename);

   if (outname!=NULL)
      {
      volume=readRAWvolume(outname,
                           width,height,depth,&steps,
                           components,NULL,NULL,NULL,
                           scalex,scaley,scalez);

      free(outname);
      }

   return(volume);
   }
