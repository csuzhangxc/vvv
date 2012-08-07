#include "codebase.h"

int main(int argc,char *argv[])
   {
   unsigned int i,j,k;

   FILE *file,*out;

   unsigned char *data1,*data2,*data3,*grad;
   unsigned int width,height,depth;
   int components;

   unsigned int v0,v1,v2,v3,v4,v5,v6,v;

   if (argc!=7)
      {
      printf("usage: %s <input.raw> <width> <height <depth> <components> <output.raw>\n",argv[0]);
      printf(" input: raw volume\n");
      printf(" output: raw gradient volume\n");
      printf(" components: 1=8bit 2=16bit/MSB/unsigned\n");
      exit(1);
      }

   if (sscanf(argv[2],"%d",&width)!=1) exit(1);
   if (sscanf(argv[3],"%d",&height)!=1) exit(1);
   if (sscanf(argv[4],"%d",&depth)!=1) exit(1);
   if (sscanf(argv[5],"%d",&components)!=1) exit(1);
   if (width<1 || height<1 || depth<1 || (components!=1 && components!=2)) exit(1);

   if ((data1=(unsigned char *)malloc(width*height*components))==NULL) exit(1);
   if ((data2=(unsigned char *)malloc(width*height*components))==NULL) exit(1);
   if ((data3=(unsigned char *)malloc(width*height*components))==NULL) exit(1);
   if ((grad=(unsigned char *)malloc(width*height*components))==NULL) exit(1);

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   if ((out=fopen(argv[6],"wb"))==NULL) exit(1);

   if (fread(data2,width*height*components,1,file)!=1) exit(1);

   for (i=0; i<depth; i++)
      {
      if (i<depth-1)
         if (fread(data3,width*height*components,1,file)!=1) exit(1);

      // central gradient
      for (j=0; j<width; j++)
         for (k=0; k<height; k++)
            {
            if (components==1) v0=data2[j+k*width];
            else v0=256*data2[2*(j+k*width)]+data2[2*(j+k*width)+1];

            if (j>0)
               if (components==1) v1=data2[(j-1)+k*width];
               else v1=256*data2[2*((j-1)+k*width)]+data2[2*((j-1)+k*width)+1];
            else v1=v0;

            if (j<width-1)
               if (components==1) v2=data2[(j+1)+k*width];
               else v2=256*data2[2*((j+1)+k*width)]+data2[2*((j+1)+k*width)+1];
            else v2=v0;

            if (k>0)
               if (components==1) v3=data2[j+(k-1)*width];
               else v3=256*data2[2*(j+(k-1)*width)]+data2[2*(j+(k-1)*width)+1];
            else v3=v0;

            if (k<height-1)
               if (components==1) v4=data2[j+(k+1)*width];
               else v4=256*data2[2*(j+(k+1)*width)]+data2[2*(j+(k+1)*width)+1];
            else v4=v0;

            if (i>0)
               if (components==1) v5=data1[j+k*width];
               else v5=256*data1[2*(j+k*width)]+data1[2*(j+k*width)+1];
            else v5=v0;

            if (i<depth-1)
               if (components==1) v6=data3[j+k*width];
               else v6=256*data3[2*(j+k*width)]+data3[2*(j+k*width)+1];
            else v6=v0;

            v=ftrc(v0/65535.0*sqrt(((v2-v1)*(v2-v1)+(v4-v3)*(v4-v3)+(v6-v5)*(v6-v5))/3.0)+0.5);

            grad[2*(j+k*width)]=v/256;
            grad[2*(j+k*width)+1]=v%256;
            }

      fwrite(grad,components,width*height,out);

      unsigned char *tmp=data1;
      data1=data2;
      data2=data3;
      data3=tmp;
      }

   fclose(out);

   fclose(file);

   free(data1);
   free(data2);
   free(data3);
   free(grad);

   printf("wrote %dbit gradient volume with dimensions %dx%dx%d\n",
          8*components,width,height,depth);

   return(0);
   }
