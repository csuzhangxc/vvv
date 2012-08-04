#include "codebase.h"

static const double gamma_correction=0.5;

int main(int argc,char *argv[])
   {
   FILE *file,*out;

   unsigned char data[2];

   unsigned int width,height,depth;

   int v;
   double vg;
   unsigned char vc;

   if (argc!=6)
      {
      printf("usage: %s <input.rek> <width> <height> <depth> <output.raw>\n",argv[0]);
      printf(" input: 16bit fraunhofer volume file format (with 2048 byte header)\n");
      printf(" output: 8bit raw volume\n");
      exit(1);
      }

   if (sscanf(argv[2],"%d",&width)!=1) exit(1);
   if (sscanf(argv[3],"%d",&height)!=1) exit(1);
   if (sscanf(argv[4],"%d",&depth)!=1) exit(1);
   if (width<1 || height<1 || depth<1) exit(1);

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);
   if (fseek(file,-width*height*depth*2,SEEK_END)==-1) exit(1);

   if ((out=fopen(argv[5],"wb"))==NULL) exit(1);

   // totally unoptimized:
   //  one 16bit LSB value in
   //  one gamma-corrected 8bit value out
   while (fread(&data,1,2,file)==2)
      {
      v=data[0]+256*data[1];
      vg=pow(v/65535.0,gamma_correction);
      vc=ftrc(vg*255.0+0.5);

      fwrite(&vc,1,1,out); 
      }

   fclose(file);
   fclose(out);

   return(0);
   }
