#include "codebase.h"

static const double ratio=0.5;

int main(int argc,char *argv[])
   {
   unsigned int i,j,k;

   FILE *file,*out;

   unsigned char *data;
   unsigned int width,height,depth;

   unsigned int histo[65536];

   unsigned int v;

   double tsum,wsum;

   unsigned int thres;
   unsigned int count;

   unsigned int crop_x1,crop_y1;
   unsigned int crop_x2,crop_y2;
   unsigned int crop_z1,crop_z2;

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

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   crop_x1=width-1;
   crop_x2=0;

   crop_y1=height-1;
   crop_y2=0;

   crop_z1=0;
   crop_z2=depth-1;

   for (i=0; i<depth; i++)
      {
      if (fread(data,width*height*2,1,file)!=1) exit(1);

      // left side
      for (j=0; j<width; j++)
         {
         for (count=0,k=0; k<height; k++)
            {
            v=256*data[2*(j+k*width)]+data[2*(j+k*width)+1];
            if (v>thres) count++;
            }
         if (count>0) break;
         }
      if (j<crop_x1) crop_x1=j;

      // right side
      for (j=0; j<width; j++)
         {
         for (count=0,k=0; k<height; k++)
            {
            v=256*data[2*((width-1-j)+k*width)]+data[2*((width-1-j)+k*width)+1];
            if (v>thres) count++;
            }
         if (count>0) break;
         }
      if (width-1-j>crop_x2) crop_x2=width-1-j;

      // bottom side
      for (k=0; k<height; k++)
         {
         for (count=0,j=0; j<width; j++)
            {
            v=256*data[2*(j+k*width)]+data[2*(j+k*width)+1];
            if (v>thres) count++;
            }
         if (count>0) break;
         }
      if (k<crop_y1) crop_y1=k;

      // top side
      for (k=0; k<width; k++)
         {
         for (count=0,j=0; j<height; j++)
            {
            v=256*data[2*(j+(height-1-k)*width)]+data[2*(j+(height-1-k)*width)+1];
            if (v>thres) count++;
            }
         if (count>0) break;
         }
      if (height-1-k>crop_y2) crop_y2=height-1-k;

      // entire slice
      for (count=0,j=0; j<width; j++)
         for (k=0; k<height; k++)
            {
            v=256*data[2*(j+k*width)]+data[2*(j+k*width)+1];
            if (v>thres) count++;
            }

      // front slice
      if (count==0)
         if (i==crop_z1) crop_z1=i+1;

      // back slice
      if (count!=0) crop_z2=i;
      }

   fclose(file);

   printf("non-empty volume has crop box [%d,%d]x[%d,%d]x[%d,%d]\n",
          crop_x1,crop_x2,crop_y1,crop_y2,crop_z1,crop_z2);

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   if ((out=fopen(argv[5],"wb"))==NULL) exit(1);

   for (i=0; i<depth; i++)
      {
      if (fread(data,width*height*2,1,file)!=1) exit(1);

      if (i>=crop_z1 && i<=crop_z2)
         for (j=crop_y1; j<=crop_y2; j++)
            fwrite(&data[2*(crop_x1+j*width)],2,crop_x2-crop_x1+1,out);
      }

   fclose(out);

   fclose(file);

   free(data);

   printf("wrote 16bit MSB volume with dimensions %dx%dx%d\n",
          crop_x2-crop_x1+1,crop_y2-crop_y1+1,crop_z2-crop_z1+1);

   return(0);
   }
