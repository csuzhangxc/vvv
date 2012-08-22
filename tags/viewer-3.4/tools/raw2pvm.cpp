#include "codebase.h"

#include "ddsbase.h"

int main(int argc,char *argv[])
   {
   unsigned char *data;
   unsigned int bytes;

   unsigned int width,height,depth;
   int components=1;

   float scalex,scaley,scalez;

   if (argc!=6 && argc!=7 && argc!=10)
      {
      printf("usage: %s <input.raw> <width> <height> <depth>\n",argv[0]);
      printf("       [<components> [<scalex> <scaley> <scalez>]] <output.pvm>\n");
      printf(" input: raw volume\n");
      printf(" output: compressed PVM volume\n");
      printf("components:\n");
      printf(" 1=8bit\n");
      printf(" 2=16bit/MSB\n");
      printf(" -2=16bit/LSB\n");
      printf(" -32768=16bit/LSB/signed\n");
      printf(" 32767=16bit/MSB/signed\n");
      printf(" 4=IEEE/float\n");
      exit(1);
      }

   if (sscanf(argv[2],"%d",&width)!=1) exit(1);
   if (sscanf(argv[3],"%d",&height)!=1) exit(1);
   if (sscanf(argv[4],"%d",&depth)!=1) exit(1);
   if (argc>=7) if (sscanf(argv[5],"%d",&components)!=1) exit(1);
   if (width<1 || height<1 || depth<1 || (components<1 && components!=-2 && components!=-32768)) exit(1);

   if (argc==10)
      {
      if (sscanf(argv[6],"%g",&scalex)!=1) exit(1);
      if (sscanf(argv[7],"%g",&scaley)!=1) exit(1);
      if (sscanf(argv[8],"%g",&scalez)!=1) exit(1);
      }

   if ((data=readRAWfile(argv[1],&bytes))==NULL) exit(1);

   if (components==-2) {swapbytes(data,bytes); components=2;}
   if (components==-32768) {swapbytes(data,bytes); convbytes(data,bytes); components=2;}
   if (components==32767) {convbytes(data,bytes); components=2;}
   if (components==4) {convfloat(data,bytes); components=2; bytes/=2;}
   if (bytes<width*height*depth*components) exit(1);

   if (argc==6) writePVMvolume(argv[5],data,width,height,depth,components);
   else if (argc==7) writePVMvolume(argv[6],data,width,height,depth,components);
   else writePVMvolume(argv[9],data,width,height,depth,components,scalex,scaley,scalez);
   free(data);

   return(0);
   }
