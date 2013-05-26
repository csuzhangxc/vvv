// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "rawbase.h"

// analyze RAW file format
BOOLINT readRAWinfo(char *filename,
                    unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                    float *scalex,float *scaley,float *scalez)
   {
   char *dot,*dotdot;
   int count;

   dot=strrchr(filename,'.');

   if (dot==NULL) return(FALSE);

   if (strcmp(dot,".raw")!=0) return(FALSE);

   *dot='\0';
   dotdot=strrchr(filename,'.');
   *dot='.';

   if (dotdot==NULL) return(FALSE);

   if (sscanf(dotdot,".%dx%dx%d%n",width,height,depth,&count)!=3) return(FALSE);

   if (components!=0) *components=1;

   if (scalex!=NULL) *scalex=1.0f;
   if (scaley!=NULL) *scaley=1.0f;
   if (scalez!=NULL) *scalez=1.0f;

   return(TRUE);
   }

// read a RAW volume
unsigned char *readRAWvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                             float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   char *name;

   unsigned char *volume;
   unsigned long long bytes;

   // open REK file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze header
   name=strdup(filename);
   if (!readRAWinfo(name,width,height,depth,components,
                    scalex,scaley,scalez))
      {
      free(name);
      fclose(file);
      return(NULL);
      }
   free(name);

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
