// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "rawbase.h"

// analyze RAW file format
BOOLINT readRAWinfo(char *filename,
                    unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                    unsigned int *components,unsigned int *bits,
                    float *scalex,float *scaley,float *scalez)
   {
   unsigned int rawcomps=1;
   BOOLINT rawsigned=FALSE;
   BOOLINT rawbits=8;
   BOOLINT rawmsb=TRUE;
   int rawscalex=1,rawscaley=1,rawscalez=1;
   float rawmaxscale=1.0f;

   char *dot,*dotdot;
   int count;

   dot=strrchr(filename,'.');

   if (dot==NULL) return(FALSE);

   if (strcmp(dot,".raw")!=0) return(FALSE);

   *dot='\0';
   dotdot=strrchr(filename,'.');
   *dot='.';

   if (dotdot==NULL) return(FALSE);

   *width=*height=*depth=*steps=1;

   dotdot++;

   if (sscanf(dotdot,"%dx%d%n",width,height,&count)!=2) return(FALSE);

   dotdot+=count;

   if (*dotdot=='x')
      {
      dotdot++;

      if (sscanf(dotdot,"%d%n",depth,&count)!=1) return(FALSE);

      dotdot+=count;
      }

   if (*dotdot=='x')
      {
      dotdot++;

      if (sscanf(dotdot,"%d%n",depth,&count)!=1) return(FALSE);

      dotdot+=count;
      }

   if (*dotdot=='x')
      {
      dotdot++;

      if (sscanf(dotdot,"%d%n",steps,&count)!=1) return(FALSE);

      dotdot+=count;
      }

   if (*dotdot=='_')
      {
      dotdot++;

      while (*dotdot!='.' && *dotdot!='_')
         switch (*dotdot)
            {
            case '1': rawcomps=1; rawbits=8; break; // char
            case '2': rawcomps=2; rawbits=8; break; // short
            case '3': rawcomps=3; rawbits=8; break; // rgb
            case '4': rawcomps=4; rawbits=8; break; // rgba
            case '6': rawcomps=3; rawbits=16; break; // rgb 16-bit
            case '8': rawcomps=4; rawbits=16; break; // rgba 16-bit
            case 'f': rawcomps=1; rawbits=32; break; // float 32-bit
            case 'u': rawsigned=FALSE; break; // unsigned
            case 's': rawsigned=TRUE; break; // signed
            case 'm': rawmsb=TRUE; break; // MSB
            case 'l': rawmsb=FALSE; break; // LSB
            }
      }

   if (*dotdot=='_')
      {
      dotdot++;

      if (sscanf(dotdot,"%dx%d%n",&rawscalex,&rawscaley,&count)!=2) return(FALSE);

      dotdot+=count;

      if (*dotdot=='x')
         {
         dotdot++;

         if (sscanf(dotdot,"%d%n",&rawscalez,&count)!=1) return(FALSE);

         dotdot+=count;
         }
      }

   if (*dotdot!='.') return(FALSE);

   if (rawcomps!=1 && components==NULL) return(FALSE);
   if (rawbits!=8 && bits==NULL) return(FALSE);

   if (components!=0) *components=rawcomps;
   if (bits!=0) *bits=rawbits;

   if (rawscalex>rawmaxscale) rawmaxscale=rawscalex;
   if (rawscaley>rawmaxscale) rawmaxscale=rawscaley;
   if (rawscalez>rawmaxscale) rawmaxscale=rawscalez;

   if (scalex!=NULL) *scalex=rawscalex/rawmaxscale;
   if (scaley!=NULL) *scaley=rawscaley/rawmaxscale;
   if (scalez!=NULL) *scalez=rawscalez/rawmaxscale;

   return(TRUE);
   }

// read a RAW volume
unsigned char *readRAWvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                             unsigned int *components,unsigned int *bits,
                             float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   char *name;

   unsigned char *volume;
   unsigned long long bytes;

   // open RAW file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze header
   name=strdup(filename);
   if (!readRAWinfo(name,
                    width,height,depth,steps,
                    components,bits,
                    scalex,scaley,scalez))
      {
      free(name);
      fclose(file);
      return(NULL);
      }
   free(name);

   bytes=(*width)*(*height)*(*depth)*(*components)*(*steps);

   if (bits!=NULL)
      if (*bits==16) bytes*=2;

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
