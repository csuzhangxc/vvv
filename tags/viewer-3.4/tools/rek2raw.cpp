#include "codebase.h"

int main(int argc,char *argv[])
   {
   unsigned int i;

   FILE *file,*out;

   unsigned char data[2];

   unsigned int width,height,depth,bits;

   if (argc!=3)
      {
      printf("usage: %s <input.rek> <output.raw>\n",argv[0]);
      printf(" input: 8bit or 16bit fraunhofer volume file format (with 2048 byte header)\n");
      printf(" output: 8bit or 16bit MSB raw volume\n");
      exit(1);
      }

   if ((file=fopen(argv[1],"rb"))==NULL) exit(1);

   if (fread(&data,1,2,file)!=2) exit(1);
   width=data[0]+256*data[1];

   if (fread(&data,1,2,file)!=2) exit(1);
   height=data[0]+256*data[1];

   if (fread(&data,1,2,file)!=2) exit(1);
   bits=data[0]+256*data[1];

   if (fread(&data,1,2,file)!=2) exit(1);
   depth=data[0]+256*data[1];

   if (width<1 || height<1 || depth<1) exit(1);
   if (bits!=8 && bits!=16) exit(1);

   printf("found volume with dimensions %dx%dx%d and %d bits\n",width,height,depth,bits);

   if (fseek(file,2048,SEEK_SET)==-1) exit(1);

   if ((out=fopen(argv[2],"wb"))==NULL) exit(1);

   for (i=0; i<width*height*depth; i++)
      if (bits==8)
         {
         if (fread(data,1,1,file)!=1) exit(1);
         fwrite(data,1,1,out);
         }
      else
         {
         if (fread(data,1,2,file)!=2) exit(1);
         fwrite(data+1,1,1,out);
         fwrite(data,1,1,out);
         }

   fclose(file);
   fclose(out);

   return(0);
   }
