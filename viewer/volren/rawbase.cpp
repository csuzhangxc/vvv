// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "rawbase.h"

unsigned short int RAW_INTEL=1;

#define RAW_ISINTEL (*((unsigned char *)(&RAW_INTEL)+1)==0)

inline void RAW_swapuint(unsigned int *x)
   {
   unsigned int tmp=*x;

   *x=((tmp&0xff)<<24)|
      ((tmp&0xff00)<<8)|
      ((tmp&0xff0000)>>8)|
      ((tmp&0xff000000)>>24);
   }

inline void RAW_swapfloat(float *x)
   {RAW_swapuint((unsigned int *)x);}

// analyze RAW file format
BOOLINT readRAWinfo(char *filename,
                    unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                    unsigned int *components,unsigned int *bits,BOOLINT *sign,BOOLINT *msb,
                    float *scalex,float *scaley,float *scalez)
   {
   unsigned int rawcomps=1;
   BOOLINT rawsign=FALSE;
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
         {
         switch (*dotdot)
            {
            case '1': rawcomps=1; rawbits=8; break; // char
            case '2': rawcomps=1; rawbits=16; break; // short
            case '3': rawcomps=3; rawbits=8; break; // rgb
            case '4': rawcomps=4; rawbits=8; break; // rgba
            case '6': rawcomps=3; rawbits=16; break; // rgb 16-bit
            case '8': rawcomps=4; rawbits=16; break; // rgba 16-bit
            case 'f': rawcomps=1; rawbits=32; break; // float 32-bit
            case 'u': rawsign=FALSE; break; // unsigned
            case 's': rawsign=TRUE; break; // signed
            case 'm': rawmsb=TRUE; break; // MSB
            case 'l': rawmsb=FALSE; break; // LSB
            default: return(FALSE);
            }

         dotdot++;
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

   if (bits==NULL)
      if (rawcomps==1 && rawbits==16)
         {
         rawcomps=2;
         rawbits=8;
         }

   if (rawcomps!=1 && components==NULL) return(FALSE);
   if (rawbits!=8 && bits==NULL) return(FALSE);
   if (rawsign!=FALSE && sign==NULL) return(FALSE);
   if (rawmsb!=TRUE && msb==NULL) return(FALSE);

   if (components!=NULL) *components=rawcomps;
   if (bits!=NULL) *bits=rawbits;
   if (sign!=NULL) *sign=rawsign;
   if (msb!=NULL) *msb=rawmsb;

   if (rawscalex>rawmaxscale) rawmaxscale=rawscalex;
   if (rawscaley>rawmaxscale) rawmaxscale=rawscaley;
   if (rawscalez>rawmaxscale) rawmaxscale=rawscalez;

   if (rawmaxscale==0.0f) return(FALSE);

   if (scalex!=NULL) *scalex=rawscalex/rawmaxscale;
   if (scaley!=NULL) *scaley=rawscaley/rawmaxscale;
   if (scalez!=NULL) *scalez=rawscalez/rawmaxscale;

   return(TRUE);
   }

// define RAW file format
char *makeRAWinfo(unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                  unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                  float scalex,float scaley,float scalez)
   {
   static const int maxlen=100;

   char info[maxlen];
   float maxscale=1.0f;

   snprintf(info,maxlen,".%dx%d",width,height);
   if (depth>1) snprintf(&info[strlen(info)],maxlen-strlen(info),"x%d",depth);
   if (steps>1) snprintf(&info[strlen(info)],maxlen-strlen(info),"x%d",steps);

   if (components!=1 || bits!=8 || sign!=FALSE || msb!=TRUE ||
       scalex!=1.0f || scaley!=1.0f || scalez!=1.0f)
      {
      snprintf(&info[strlen(info)],maxlen-strlen(info),"_");

      if (sign==FALSE) snprintf(&info[strlen(info)],maxlen-strlen(info),"u");
      else snprintf(&info[strlen(info)],maxlen-strlen(info),"s");

      if (components==1 && bits==8) snprintf(&info[strlen(info)],maxlen-strlen(info),"1");
      else if (components==1 && bits==16) snprintf(&info[strlen(info)],maxlen-strlen(info),"2");
      else if (components==2 && bits==8) snprintf(&info[strlen(info)],maxlen-strlen(info),"2");
      else if (components==3 && bits==8) snprintf(&info[strlen(info)],maxlen-strlen(info),"3");
      else if (components==4 && bits==8) snprintf(&info[strlen(info)],maxlen-strlen(info),"4");
      else if (components==3 && bits==16) snprintf(&info[strlen(info)],maxlen-strlen(info),"6");
      else if (components==4 && bits==16) snprintf(&info[strlen(info)],maxlen-strlen(info),"8");
      else return(NULL);

      if (msb==TRUE) snprintf(&info[strlen(info)],maxlen-strlen(info),"m");
      else snprintf(&info[strlen(info)],maxlen-strlen(info),"l");

      if (scalex>maxscale) maxscale=scalex;
      if (scaley>maxscale) maxscale=scaley;
      if (scalez>maxscale) maxscale=scalez;

      if (maxscale==0.0f) return(NULL);

      scalex/=maxscale;
      scaley/=maxscale;
      scalez/=maxscale;

      if (scalex!=1.0f || scaley!=1.0f || scalez!=1.0f)
         {
         snprintf(&info[strlen(info)],maxlen-strlen(info),"_%dx%d",int(1000.0f*scalex+0.5f),int(1000.0f*scaley+0.5f));
         if (depth>1) snprintf(&info[strlen(info)],maxlen-strlen(info),"x%d",int(1000.0f*scalez+0.5f));
         }
      }

   snprintf(&info[strlen(info)],maxlen-strlen(info),".raw");

   return(strdup(info));
   }

// append RAW file format suffix
char *appendRAWinfo(const char *filename,
                    unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                    unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                    float scalex,float scaley,float scalez)
   {
   char *filename2;
   char *info,*dot;
   char *filename3;

   // define RAW info
   info=makeRAWinfo(width,height,depth,steps,
                    components,bits,sign,msb,
                    scalex,scaley,scalez);

   if (info==NULL) return(NULL);

   // remove suffix
   filename2=strdup(filename);
   dot=strrchr(filename2,'.');
   if (dot!=NULL)
      if (strcmp(dot,".raw")==0) *dot='\0';

   // append RAW info to filename
   filename3=strdup2(filename2,info);
   free(filename2);
   free(info);

   return(filename3);
   }

// read a RAW volume
unsigned char *readRAWvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                             unsigned int *components,unsigned int *bits,BOOLINT *sign,BOOLINT *msb,
                             float *scalex,float *scaley,float *scalez)
   {
   FILE *file;

   char *name;

   unsigned char *volume;
   unsigned long long bytes;

   // open RAW file
   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   // analyze RAW info
   name=strdup(filename);
   if (!readRAWinfo(name,
                    width,height,depth,steps,
                    components,bits,sign,msb,
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
      else if (*bits==32) bytes*=4;

   if ((volume=(unsigned char *)malloc(bytes))==NULL) return(NULL);

   // read RAW volume
   if (fread(volume,bytes,1,file)!=1)
      {
      free(volume);
      fclose(file);
      return(NULL);
      }

   fclose(file);

   return(volume);
   }

// write a RAW volume
BOOLINT writeRAWvolume(const char *filename, // /wo suffix .raw
                       unsigned char *volume,
                       unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                       unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                       float scalex,float scaley,float scalez)
   {
   FILE *file;

   char *output;
   unsigned long long bytes;

   // make RAW info
   output=appendRAWinfo(filename,
                        width,height,depth,steps,
                        components,bits,sign,msb,
                        scalex,scaley,scalez);

   if (output==NULL) return(FALSE);

   // open RAW output file
   if ((file=fopen(output,"wb"))==NULL)
      {
      free(output);
      return(FALSE);
      }

   free(output);

   bytes=width*height*depth*components*steps;

   if (bits==16) bytes*=2;
   else if (bits==32) bytes*=4;

   // write RAW volume
   if (fwrite(volume,bytes,1,file)!=1)
      {
      fclose(file);
      return(FALSE);
      }

   fclose(file);

   return(TRUE);
   }

// copy a RAW volume
BOOLINT copyRAWvolume(FILE *file, // source file desc
                      const char *output, // destination file name /wo .raw
                      unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                      unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                      float scalex,float scaley,float scalez)
   {
   unsigned int i,j;

   unsigned char *slice;
   unsigned long long bytes;

   char *outname;
   FILE *outfile;

   // compute total number of cells per slice
   bytes=width*height*components;

   // compute total number of bytes per slice
   if (bits==16) bytes*=2;
   else if (bits==32) bytes*=4;

   // make RAW info
   outname=appendRAWinfo(output,
                         width,height,depth,steps,
                         components,bits,sign,msb,
                         scalex,scaley,scalez);

   if (outname==NULL) return(FALSE);

   // open RAW output file
   if ((outfile=fopen(outname,"wb"))==NULL)
      {
      free(outname);
      return(FALSE);
      }

   free(outname);

   if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

   // process out-of-core slice by slice
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         if (fwrite(slice,bytes,1,outfile)!=1)
            {
            free(slice);
            return(FALSE);
            }
         }

   free(slice);

   return(TRUE);
   }

// copy a RAW volume
BOOLINT copyRAWvolume(const char *filename, // source file
                      const char *output) // destination file name /wo suffix .raw
   {
   FILE *file;

   char *name;

   unsigned int rawwidth,rawheight,rawdepth,rawsteps,rawcomps,rawbits;
   BOOLINT rawsign,rawmsb;
   float rawscalex,rawscaley,rawscalez;

   BOOLINT success;

   // open RAW file
   if ((file=fopen(filename,"rb"))==NULL) return(FALSE);

   // analyze RAW info
   name=strdup(filename);
   if (!readRAWinfo(name,
                    &rawwidth,&rawheight,&rawdepth,&rawsteps,
                    &rawcomps,&rawbits,&rawsign,&rawmsb,
                    &rawscalex,&rawscaley,&rawscalez))
      {
      free(name);
      fclose(file);
      return(FALSE);
      }
   free(name);

   success=copyRAWvolume(file,filename,
                         rawwidth,rawheight,rawdepth,rawsteps,
                         rawcomps,rawbits,rawsign,rawmsb,
                         rawscalex,rawscaley,rawscalez);

   fclose(file);

   return(success);
   }

// convert a RAW array to a 16-bit unsigned array
unsigned short int *convert2short(unsigned char *source,unsigned long long cells,
                                  unsigned int bits,BOOLINT sign,BOOLINT msb)
   {
   unsigned long long i;

   unsigned short int *shorts;

   float v;

   if ((shorts=(unsigned short int *)malloc(cells*sizeof(unsigned short int)))==NULL) return(NULL);

   if (bits==8)
      if (sign)
         for (i=0; i<cells; i++) shorts[i]=source[i]+128;
      else
         for (i=0; i<cells; i++) shorts[i]=source[i];
   else if (bits==16)
      if (msb)
         if (sign)
            for (i=0; i<cells; i++) shorts[i]=(signed short)(256*source[i<<2]+source[(i<<2)+1])+32768;
         else
            for (i=0; i<cells; i++) shorts[i]=(unsigned short)(256*source[i<<2]+source[(i<<2)+1]);
      else
         if (sign)
            for (i=0; i<cells; i++) shorts[i]=(signed short)(source[i<<2]+256*source[(i<<2)+1])+32768;
         else
            for (i=0; i<cells; i++) shorts[i]=(unsigned short)(source[i<<2]+256*source[(i<<2)+1]);
   else if (bits==32)
      if (msb)
         if (RAW_ISINTEL)
            for (i=0; i<cells; i++)
               {
               v=fabs(*(float*)(&source[4*i]));
               RAW_swapfloat(&v);
               shorts[i]=v>1.0f?65535:(unsigned int)ffloor(65535.0f*v+0.5f);
               }
         else
            for (i=0; i<cells; i++)
               {
               v=fabs(*(float*)(&source[4*i]));
               shorts[i]=v>1.0f?65535:(unsigned int)ffloor(65535.0f*v+0.5f);
               }
      else
         if (RAW_ISINTEL)
            for (i=0; i<cells; i++)
               {
               v=fabs(*(float*)(&source[4*i]));
               shorts[i]=v>1.0f?65535:(unsigned int)ffloor(65535.0f*v+0.5f);
               }
         else
            for (i=0; i<cells; i++)
               {
               v=fabs(*(float*)(&source[4*i]));
               RAW_swapfloat(&v);
               shorts[i]=v>1.0f?65535:(unsigned int)ffloor(65535.0f*v+0.5f);
               }
   else ERRORMSG();

   return(shorts);
   }

// compute maximum 16-bit unsigned value
unsigned short int convert2maxval(unsigned short int *shorts,unsigned long long cells)
   {
   unsigned long long i;

   unsigned short maxval;

   maxval=0;
   for (i=0; i<cells; i++)
      if (shorts[i]>maxval) maxval=shorts[i];

   return(maxval);
   }

// quantize a 16-bit unsigned array to a char array
unsigned char *convert2char(unsigned short int *shorts,unsigned long long cells,
                            unsigned short maxval)
   {
   unsigned long long i;

   unsigned char *chars;

   if (maxval==0) maxval++;

   if ((chars=(unsigned char *)malloc(cells))==NULL) return(NULL);

   for (i=0; i<cells; i++)
      chars[i]=(int)ffloor(255.0f*shorts[i]/maxval+0.5f);

   return(chars);
   }

// copy a RAW volume with out-of-core linear quantization
BOOLINT copyRAWvolume_linear(FILE *file, // source file desc
                             const char *output, // destination file name /wo .raw
                             unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                             unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                             float scalex,float scaley,float scalez)
   {
   unsigned int i,j;

   unsigned char *slice;
   unsigned long long cells;
   unsigned long long bytes;

   unsigned short int *shorts;
   unsigned char *chars;

   unsigned long long tellpos;

   unsigned short int maxval0,maxval;

   char *outname;
   FILE *outfile;

   // compute total number of cells per slice
   cells=bytes=width*height*components;

   // compute total number of bytes per slice
   if (bits==16) bytes*=2;
   else if (bits==32) bytes*=4;

   // remember seek position
   tellpos=ftell(file);

   // scan for maximum value
   maxval0=0;
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         shorts=convert2short(slice,cells,bits,sign,msb);
         free(slice);

         maxval=convert2maxval(shorts,cells);
         free(shorts);

         if (maxval>maxval0) maxval0=maxval;
         }

   // seek back to start
   if (fseek(file,tellpos,SEEK_SET)==-1) return(FALSE);

   // make RAW info
   outname=appendRAWinfo(output,
                         width,height,depth,steps,
                         1,8,FALSE,TRUE,
                         scalex,scaley,scalez);

   if (outname==NULL) return(FALSE);

   // open RAW output file
   if ((outfile=fopen(outname,"wb"))==NULL)
      {
      free(outname);
      return(FALSE);
      }

   free(outname);

   // process out-of-core slice by slice
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         shorts=convert2short(slice,cells,bits,sign,msb);
         free(slice);

         chars=convert2char(shorts,cells,maxval0);
         free(shorts);

         if (fwrite(chars,cells,1,outfile)!=1)
            {
            free(chars);
            return(FALSE);
            }

         free(chars);
         }

   return(TRUE);
   }

// copy a RAW volume with out-of-core linear quantization
BOOLINT copyRAWvolume_linear(const char *filename, // source file
                             const char *output) // destination file name /wo suffix .raw
   {
   FILE *file;

   char *name;

   unsigned int rawwidth,rawheight,rawdepth,rawsteps,rawcomps,rawbits;
   BOOLINT rawsign,rawmsb;
   float rawscalex,rawscaley,rawscalez;

   BOOLINT success;

   // open RAW file
   if ((file=fopen(filename,"rb"))==NULL) return(FALSE);

   // analyze RAW info
   name=strdup(filename);
   if (!readRAWinfo(name,
                    &rawwidth,&rawheight,&rawdepth,&rawsteps,
                    &rawcomps,&rawbits,&rawsign,&rawmsb,
                    &rawscalex,&rawscaley,&rawscalez))
      {
      free(name);
      fclose(file);
      return(FALSE);
      }
   free(name);

   success=copyRAWvolume_linear(file,filename,
                                rawwidth,rawheight,rawdepth,rawsteps,
                                rawcomps,rawbits,rawsign,rawmsb,
                                rawscalex,rawscaley,rawscalez);

   fclose(file);

   return(success);
   }

// populate histogram from short array
void convert2histogram(unsigned short int *shorts,unsigned long long cells,unsigned int *histo)
   {
   unsigned long long i;

   for (i=0; i<cells; i++)
      histo[shorts[i]]++;
   }

// compute boundaries of volume crop box
void convert2boundary(unsigned short int *shorts,unsigned int width,unsigned int height,unsigned int slice,
                      unsigned int thres,
                      unsigned int &crop_x1,unsigned int &crop_x2,
                      unsigned int &crop_y1,unsigned int &crop_y2,
                      unsigned int &crop_z1,unsigned int &crop_z2)
   {
   unsigned int i,j;

   unsigned int count;

   // left side
   for (i=0; i<width; i++)
      {
      for (count=0,j=0; j<height; j++)
         if (shorts[i+j*width]>thres) count++;

      if (count>0)
         {
         if (i<crop_x1) crop_x1=i;
         break;
         }
      }

   // right side
   for (i=0; i<width; i++)
      {
      for (count=0,j=0; j<height; j++)
         if (shorts[i+j*width]>thres) count++;

      if (count>0)
         {
         if (width-1-i>crop_x2) crop_x2=width-1-i;
         break;
         }
      }

   // bottom side
   for (j=0; j<height; j++)
      {
      for (count=0,i=0; i<width; i++)
         if (shorts[i+j*width]>thres) count++;

      if (count>0)
         {
         if (j<crop_y1) crop_y1=j;
         break;
         }
      }

   // top side
   for (j=0; j<width; j++)
      {
      for (count=0,i=0; i<height; i++)
         if (shorts[i+j*width]>thres) count++;

      if (count>0)
         {
         if (height-1-j>crop_y2) crop_y2=height-1-j;
         break;
         }
      }

   // entire slice
   for (count=0,i=0; i<width; i++)
      for (j=0; j<height; j++)
         if (shorts[i+j*width]>thres) count++;

   // front slice
   if (count==0)
      if (slice==crop_z1) crop_z1=slice+1;

   // back slice
   if (count!=0) crop_z2=slice;
   }

// copy a RAW volume with out-of-core cropping
BOOLINT cropRAWvolume(FILE *file, // source file desc
                      const char *output, // destination file name /wo .raw
                      unsigned int width,unsigned int height,unsigned int depth,unsigned int steps,
                      unsigned int components,unsigned int bits,BOOLINT sign,BOOLINT msb,
                      float scalex,float scaley,float scalez,
                      double ratio)
   {
   unsigned int i,j,k;

   unsigned char *slice;
   unsigned long long cells;
   unsigned long long bytes;

   unsigned short int *shorts;

   unsigned long long tellpos;

   unsigned int histo[65536];

   double tsum,wsum;

   unsigned int thres;

   unsigned int crop_x1,crop_y1;
   unsigned int crop_x2,crop_y2;
   unsigned int crop_z1,crop_z2;

   char *outname;
   FILE *outfile;

   // compute total number of cells per slice
   cells=bytes=width*height*components;

   // compute total number of bytes per slice
   if (bits==16) bytes*=2;
   else if (bits==32) bytes*=4;

   // remember seek position
   tellpos=ftell(file);

   // initialize histogram
   for (i=0; i<65536; i++) histo[i]=0;

   // populate histogram
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         shorts=convert2short(slice,cells,bits,sign,msb);
         free(slice);

         convert2histogram(shorts,cells,histo);
         free(shorts);
         }

   // integrate histogram
   tsum=0.0;
   for (i=0; i<65536; i++)
      tsum+=histo[i]*i/65535.0;

   // compute threshold from integrated histogram
   wsum=0.0;
   for (thres=0; thres<65536; thres++)
      {
      wsum+=histo[thres]*thres/65535.0;
      if (wsum>ratio*tsum) break;
      }

   // seek back to start
   if (fseek(file,tellpos,SEEK_SET)==-1) return(FALSE);

   crop_x1=width-1;
   crop_x2=0;

   crop_y1=height-1;
   crop_y2=0;

   crop_z1=0;
   crop_z2=depth-1;

   // compute crop boundaries
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         shorts=convert2short(slice,cells,bits,sign,msb);
         free(slice);

         convert2boundary(shorts,width,height,j,thres,crop_x1,crop_x2,crop_y1,crop_y2,crop_z1,crop_z2);
         free(shorts);
         }

   // seek back to start
   if (fseek(file,tellpos,SEEK_SET)==-1) return(FALSE);

   // make RAW info
   outname=appendRAWinfo(output,
                         width,height,depth,steps,
                         2,8,FALSE,!RAW_ISINTEL,
                         scalex,scaley,scalez);

   if (outname==NULL) return(FALSE);

   // open RAW output file
   if ((outfile=fopen(outname,"wb"))==NULL)
      {
      free(outname);
      return(FALSE);
      }

   free(outname);

   // process out-of-core slice by slice
   for (i=0; i<steps; i++)
      for (j=0; j<depth; j++)
         {
         if ((slice=(unsigned char *)malloc(bytes))==NULL) return(FALSE);

         if (fread(slice,bytes,1,file)!=1)
            {
            free(slice);
            return(FALSE);
            }

         shorts=convert2short(slice,cells,bits,sign,msb);
         free(slice);

         if (j>=crop_z1 && j<=crop_z2)
            for (k=crop_y1; k<=crop_y2; k++)
               if (fwrite(&shorts[crop_x1+k*width],2*(crop_x2-crop_x1+1),1,outfile)!=1)
                  {
                  free(shorts);
                  return(FALSE);
                  }

         free(shorts);
         }

   return(TRUE);
   }

// copy a RAW volume with out-of-core cropping
BOOLINT cropRAWvolume(const char *filename, // source file
                      const char *output, // destination file name /wo suffix .raw
                      double ratio)
   {
   FILE *file;

   char *name;

   unsigned int rawwidth,rawheight,rawdepth,rawsteps,rawcomps,rawbits;
   BOOLINT rawsign,rawmsb;
   float rawscalex,rawscaley,rawscalez;

   BOOLINT success;

   // open RAW file
   if ((file=fopen(filename,"rb"))==NULL) return(FALSE);

   // analyze RAW info
   name=strdup(filename);
   if (!readRAWinfo(name,
                    &rawwidth,&rawheight,&rawdepth,&rawsteps,
                    &rawcomps,&rawbits,&rawsign,&rawmsb,
                    &rawscalex,&rawscaley,&rawscalez))
      {
      free(name);
      fclose(file);
      return(FALSE);
      }
   free(name);

   success=cropRAWvolume(file,filename,
                         rawwidth,rawheight,rawdepth,rawsteps,
                         rawcomps,rawbits,rawsign,rawmsb,
                         rawscalex,rawscaley,rawscalez,
                         ratio);

   fclose(file);

   return(success);
   }
