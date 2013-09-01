// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"
#include "rawbase.h"

#include "rekbase.h"

// analyze 2048 byte REK header
BOOLINT readREKheader(FILE *file,
                      long long *width,long long *height,long long *depth,unsigned int *components,
                      float *scalex,float *scaley,float *scalez)
   {
   unsigned char data16[2];
   unsigned char data32[4];

   long long rekwidth,rekheight,rekdepth;
   unsigned int rekbits,rekcomps;

   unsigned int rekheader;
   unsigned int rekmajor,rekminor,rekrevision;

   unsigned int rekX,rekY,rekZ;
   float reknorm;
   float rekvoxelX,rekvoxelZ;

   // volume width
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekwidth=data16[0]+256*data16[1];

   // volume height
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekheight=data16[0]+256*data16[1];

   // volume bits (16 = unsigned short int)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekbits=data16[0]+256*data16[1];

   // volume depth (slices / images per file)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekdepth=data16[0]+256*data16[1];

   if (rekwidth<1 || rekheight<1 || rekdepth<1) return(FALSE);
   if (rekbits!=8 && rekbits!=16) return(FALSE);

   if (rekbits==8) rekcomps=1;
   else rekcomps=2;

   if (rekcomps!=1 && components==NULL) return(FALSE);

   // header size (2048)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekheader=data16[0]+256*data16[1];

   // major version (2)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekmajor=data16[0]+256*data16[1];

   // minor version (5)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekminor=data16[0]+256*data16[1];

   // revision (0)
   if (fread(&data16,1,2,file)!=2) return(FALSE);
   rekrevision=data16[0]+256*data16[1];

   if (rekmajor!=2 || rekminor<5) return(FALSE);

   // fseek to reconstruction params
   if (fseek(file,0x238,SEEK_SET)==-1) return(FALSE);

   // reconstruction width
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   rekX=data32[0]+256*(data32[1]+256*(data32[2]+256*data32[3]));

   // reconstruction height
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   rekY=data32[0]+256*(data32[1]+256*(data32[2]+256*data32[3]));

   // reconstruction depth
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   rekZ=data32[0]+256*(data32[1]+256*(data32[2]+256*data32[3]));

   if (rekX!=rekwidth || rekY!=rekheight || rekZ!=rekdepth) return(FALSE);

   // reconstruction norm (scaling factor, abscoeff = greyvalue / norm)
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   reknorm=*(float *)(&data32);

   // reconstruction voxel size X (micrometers, equal to voxel size Y)
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   rekvoxelX=*(float *)(&data32);

   // reconstruction voxel size Z (micrometers)
   if (fread(&data32,1,4,file)!=4) return(FALSE);
   rekvoxelZ=*(float *)(&data32);

   // seek to raw data
   if (fseek(file,rekheader,SEEK_SET)==-1) return(FALSE);

   *width=rekwidth;
   *height=rekheight;
   *depth=rekdepth;

   if (components!=0) *components=rekcomps;

   if (scalex!=NULL) *scalex=rekvoxelX;
   if (scaley!=NULL) *scaley=rekvoxelX;
   if (scalez!=NULL) *scalez=rekvoxelZ;

   return(TRUE);
   }

// read a REK volume (Fraunhofer EZRT volume format)
unsigned char *readREKvolume(const char *filename,
                             long long *width,long long *height,long long *depth,unsigned int *components,
                             float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   unsigned char *volume;
   long long bytes;

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

   if ((volume=(unsigned char *)malloc(bytes))==NULL) ERRORMSG();

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
                      long long *width,long long *height,long long *depth,unsigned int *components,
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

   long long width,height,depth;
   unsigned int components;
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
char *processREKvolume(const char *filename,const char *output,
                       void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL)
   {
   FILE *file;

   long long width,height,depth;
   unsigned int components;
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
                                  scalex,scaley,scalez,
                                  RAW_TARGET_RATIO,
                                  RAW_TARGET_CELLS,
                                  feedback,obj)))
      {
      fclose(file);
      return(NULL);
      }

   fclose(file);

   return(outname);
   }

// read a REK volume out-of-core
unsigned char *readREKvolume_ooc(const char *filename,
                                 long long *width,long long *height,long long *depth,unsigned int *components,
                                 float *scalex,float *scaley,float *scalez,
                                 void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   char *output,*dot;
   char *outname;

   unsigned char *volume;
   long long steps;

   volume=NULL;

   output=strdup(filename);

   dot=strrchr(output,'.');

   if (dot!=NULL)
      if (strcasecmp(dot,".rek")==0) *dot='\0';

   outname=processREKvolume(filename,output,feedback,obj);
   free(output);

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
