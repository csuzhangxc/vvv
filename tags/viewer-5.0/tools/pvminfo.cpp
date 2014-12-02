// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "ddsbase.h"

void readarg(int arg,unsigned char **par,
             int argc,char *argv[])
   {
   unsigned int bytes;

   if (arg>=argc) *par=NULL;
   else if ((*par=readRAWfile(argv[arg],&bytes))==NULL)
      {
      if ((*par=(unsigned char *)strdup(argv[arg]))==NULL) ERRORMSG();
      bytes=strlen(argv[arg])+1;
      }

   if (*par!=NULL)
      {
      if (*(*par+bytes-1)!='\n' && *(*par+bytes-1)!='\0') bytes++;
      if ((*par=(unsigned char *)realloc(*par,bytes))==NULL) ERRORMSG();
      *(*par+bytes-1)='\0';
      }
   }

int main(int argc,char *argv[])
   {
   unsigned char *volume;

   unsigned int width,height,depth,
                components;

   float scalex,scaley,scalez;

   unsigned char *description,*courtesy,*parameters,*comment;

   if (argc<2 || argc>7)
      {
      printf("usage: %s <input.pvm> [<output.pvm>\n",argv[0]);
      printf("       [<object description>\n");
      printf("       [<courtesy information (institution, copyright, contact, etc.)>\n");
      printf("       [<scan parameters (date, time, mode, etc.)>\n");
      printf("       [<additonal comments>]]]]]\n");
      exit(1);
      }

   printf("reading PVM file\n");

   if ((volume=readPVMvolume(argv[1],&width,&height,&depth,&components,&scalex,&scaley,&scalez,&description,&courtesy,&parameters,&comment))==NULL) exit(1);

   printf("found volume with width=%d height=%d depth=%d components=%d\n",
          width,height,depth,components);

   if (scalex!=1.0f || scaley!=1.0f || scalez!=1.0f)
      printf("and edge length %g/%g/%g\n",scalex,scaley,scalez);

   printf("and data checksum=%08X\n",checksum(volume,width*height*depth*components));

   if (description!=NULL)
      printf("object description:\n%s\n",description);

   if (courtesy!=NULL)
      printf("courtesy information:\n%s\n",courtesy);

   if (parameters!=NULL)
      printf("scan parameters:\n%s\n",parameters);

   if (comment!=NULL)
      printf("additonal comments:\n%s\n",comment);

   if (argc>2)
      {
      readarg(3,&description,argc,argv);
      readarg(4,&courtesy,argc,argv);
      readarg(5,&parameters,argc,argv);
      readarg(6,&comment,argc,argv);

      writePVMvolume(argv[2],volume,width,height,depth,components,scalex,scaley,scalez,description,courtesy,parameters,comment);

      printf("wrote annotated PVM file\n");

      if (description!=NULL) free(description);
      if (courtesy!=NULL) free(courtesy);
      if (parameters!=NULL) free(parameters);
      if (comment!=NULL) free(comment);
      }

   free(volume);

   return(0);
   }
