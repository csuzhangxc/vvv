#include "codebase.h"

static const double ratio=0.1;

int main(int argc,char *argv[])
   {
   unsigned int i,j;

   FILE *file;

   unsigned char *data;
   unsigned int width,height,depth;

   unsigned int histo[65536];

   unsigned int v;

   double tsum,wsum;

   unsigned int thres;

   if (argc!=6)
      {
      printf("usage: %s <input.raw> <width> <height <depth> <output.raw>\n",argv[0]);
      printf(" input: 16bit MSB raw volume\n");
      printf(" output: 16bit MSB raw cropped volume\n");
      exit(1);
      }

   if (sscanf(argv[2],"%d",&width)!=1) exit(1);
   if (sscanf(argv[3],"%d",&height)!=1) exit(1);
   if (sscanf(argv[4],"%d",&depth)!=1) exit(1);
   if (width<1 || height<1 || depth<1);

   if ((data=(unsigned char *)malloc(width*height*2))==NULL) exit(1);

   for (i=0; i<65536; i++) histo[i]=0;

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   for (i=0; i<depth; i++)
      {
      if (fread(data,width*height*2,1,file)!=1) exit(1);

      for (j=0; j<width*height; j++)
         {
         v=256*data[2*j]+data[2*j+1];
         histo[v]++;
         }
      }

   fclose(file);

   free(data);

   tsum=0.0;
   for (i=0; i<65536; i++)
      tsum+=histo[i]*i/65535.0;

   wsum=0.0;
   for (thres=0; thres<65536; thres++)
      {
      wsum+=histo[thres]*thres/65535.0;
      if (wsum>ratio*tsum) break;
      }

   printf("empty volume has normalized threshold %.3g\n",thres/65535.0);

   return(0);
   }
