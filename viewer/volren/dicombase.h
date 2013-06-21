// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef DICOMBASE_H
#define DICOMBASE_H

#include <vector>
#include <string>

#include "codebase.h"

#ifdef VIEWER_HAVE_DCMTK
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#endif

class DicomVolume
   {
   class ImageDesc
      {
      public:

      ImageDesc()
#ifdef VIEWER_HAVE_DCMTK
         : m_Image(0)
#endif
         {}

      virtual ~ImageDesc();

#ifdef VIEWER_HAVE_DCMTK
      DcmFileFormat *m_Image;
#endif
      float m_pos;

      private:

      ImageDesc(const ImageDesc&);
      ImageDesc& operator=(const ImageDesc&);
      };

   public:

   DicomVolume();
   virtual ~DicomVolume();

   bool loadImages(const char *filenamepattern,
                   void (*feedback)(const char *info,float percent)=NULL);

   bool loadImages(const std::vector<std::string> list,
                   void (*feedback)(const char *info,float percent)=NULL);

   unsigned char *getVoxelData() {return(m_Voxels);}
   long long getVoxelNum() {return(getCols()*getRows()*getSlis());}

   long long getCols() {return m_Cols;}
   long long getRows() {return m_Rows;}
   long long getSlis() {return m_Images.size();}

   float getBound(int c) {return(m_Bounds[c]);}

   private:

   bool dicomLoad(const char *filenamepattern,
                  void (*feedback)(const char *info,float percent)=NULL);

   bool dicomLoad(const std::vector<std::string> list,
                  void (*feedback)(const char *info,float percent)=NULL);

   bool dicomProcess();

   void deleteImages();
   void sortImages();

   std::vector<ImageDesc*> m_Images;

   long long m_Cols;
   long long m_Rows;

   float m_PixSpaceRow;
   float m_PixSpaceCol;

   float m_Bounds[3];
   float m_VolDir[3];

   unsigned long m_SmallestPixVal;
   unsigned long m_LargestPixVal;

   unsigned char *m_Voxels;

   static int compareFunc(const void *elem1,const void *elem2);
   };

// read a DICOM series identified by the * in the filename pattern
unsigned char *readDICOMvolume(const char *filename,
                               long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                               float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                               void (*feedback)(const char *info,float percent)=NULL);

// read a DICOM series from a file name list
unsigned char *readDICOMvolume(const std::vector<std::string> list,
                               long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                               float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                               void (*feedback)(const char *info,float percent)=NULL);

#endif
