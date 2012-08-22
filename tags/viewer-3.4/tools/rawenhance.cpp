#include "codebase.h"

double sigmoid(double x,double e)
   {return(1.0/(1.0+exp(-e*x))-0.5);}

int main(int argc,char *argv[])
   {
   unsigned int i,j;

   FILE *file,*out;

   unsigned char *data;
   unsigned int width,height,depth;
   int components;

   double vmin,vmax;
   double exponent,norm;

   double v;

   if (argc!=8)
      {
      printf("usage: %s <input.raw> <width> <height <depth> <components> <exponent> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw contrast-enhanced volume\n");
      printf(" components: 1=8bit 2=16bit/MSB/unsigned\n");
      printf(" exponent:\n");
      printf("  1 -> almost linear\n");
      printf("  2 -> about 10%% enhanced\n");
      printf("  4 -> about 50%% enhanced\n");
      exit(1);
      }

   if (sscanf(argv[2],"%d",&width)!=1) exit(1);
   if (sscanf(argv[3],"%d",&height)!=1) exit(1);
   if (sscanf(argv[4],"%d",&depth)!=1) exit(1);
   if (sscanf(argv[5],"%d",&components)!=1) exit(1);
   if (sscanf(argv[6],"%lf",&exponent)!=1) exit(1);
   if (width<1 || height<1 || depth<1 || (components!=1 && components!=2)) exit(1);

   if ((data=(unsigned char *)malloc(width*height*components))==NULL) exit(1);

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   vmin=1.0;
   vmax=0.0;

   for (i=0; i<depth; i++)
      {
      if (fread(data,width*height*components,1,file)!=1) exit(1);

      for (j=0; j<width*height; j++)
         {
         if (components==1) v=data[j]/255.0;
         else v=(256*data[2*j]+data[2*j+1])/65535.0;

         if (v<vmin) vmin=v;
         if (v>vmax) vmax=v;
         }
      }

   if (vmin>=vmax) vmax=vmin+1;

   printf("found volume with normalized range [%.4g,%.4g]\n",vmin,vmax);

   fclose(file);

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   if ((out=fopen(argv[7],"wb"))==NULL) exit(1);

   norm=sigmoid(1.0,exponent);

   for (i=0; i<depth; i++)
      {
      if (fread(data,width*height*components,1,file)!=1) exit(1);

      for (j=0; j<width*height; j++)
         {
         if (components==1) v=data[j]/255.0;
         else v=(256*data[2*j]+data[2*j+1])/65535.0;

         v=(v-vmin)/(vmax-vmin);

         v=2.0*v-1.0;
         v=sigmoid(v,exponent)/norm;
         v=(v+1.0)/2.0;

         if (components==1)
            {
            v=255.0*v;
            data[j]=ftrc(v+0.5);
            }
         else
            {
            v=65535.0*v;
            data[2*j]=ftrc(v+0.5)/256;
            data[2*j+1]=ftrc(v+0.5)%256;
            }
         }

      fwrite(data,components,width*height,out);
      }

   fclose(out);

   fclose(file);

   free(data);

   printf("wrote %dbit contrast-enhanced volume with dimensions %dx%dx%d\n",
          8*components,width,height,depth);

   return(0);
   }
