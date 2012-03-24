// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "dirbase.h"

#ifndef WINOS
#include <dirent.h>
#else
#include <windows.h>
#endif

#define STRING_MAX 1024

unsigned int searchstate=0;
char searchpath[STRING_MAX];
char searchpattern[STRING_MAX];
char *searchpre,*searchpost;
char foundfile[STRING_MAX];

// specify file search path and pattern (with '*' as single wildcard)
void filesearch(const char *spec)
   {
   static char defaultpath[]=".";
   static char defaultpattern[]="*";

   char copy[STRING_MAX];
   char *path,*pattern;

   strncpy(copy,spec?spec:defaultpattern,STRING_MAX);

   path=copy;
   pattern=strrchr(copy,'/');

   if (pattern!=NULL) *pattern++='\0';
   else
      {
      pattern=strrchr(copy,'\\');
      if (pattern!=NULL) *pattern++='\0';
      else
         {
         pattern=copy;
         path=defaultpath;
         }
      }

   strncpy(searchpath,path,STRING_MAX);
   strncpy(searchpattern,pattern,STRING_MAX);

   searchpre=searchpattern;
   searchpost=strchr(searchpattern,'*');

   if (searchpost!=NULL) *searchpost++='\0';

   searchstate=1;
   }

// get next file in the search path
const char *nextfile()
   {
   if (searchstate==0) return(NULL);

#ifndef WINOS
   static DIR *searchdir;
   struct dirent *dirp;

   if (searchstate==1)
      {
      searchdir=opendir(searchpath);
      searchstate=2;
      }

   if ((dirp=readdir(searchdir))!=NULL)
      return(dirp->d_name);

   closedir(searchdir);
#else
   char *path2;
   static HANDLE dhandle;
   static WIN32_FIND_DATA fdata;

   if (searchstate==1)
      {
      path2=strdup2(searchpath,"/*");
      dhandle=FindFirstFile(path2,&fdata);
      free(path2);

      if (dhandle==INVALID_HANDLE_VALUE)
         {
         searchstate=0;
         return(NULL);
         }

      searchstate=2;
      return(fdata.cFileName);
      }

   if (FindNextFile(dhandle,&fdata))
      return(fdata.cFileName);

   FindClose(dhandle);
#endif

   searchstate=0;

   return(NULL);
   }

// find next file matching the search pattern
const char *findfile()
   {
   const char *file;

   unsigned int pre,post,len;

   pre=strlen(searchpre);
   post=searchpost?strlen(searchpost):0;

   while ((file=nextfile())!=NULL)
      if (strcasestr(file,searchpre)==file)
         {
         len=strlen(file);
         len=(len>post)?len-post:0;
         if (searchpost==NULL ||
             strcasecmp(file+len,searchpost)==0)
            {
            if (searchpost==NULL && len!=pre) continue;
            if (strcmp(searchpath,".")==0) return(file);
            snprintf(foundfile,STRING_MAX,"%s/%s",searchpath,file);
            return(foundfile);
            }
         }

   return(NULL);
   }
