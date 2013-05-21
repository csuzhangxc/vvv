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

   bool loadImages(const char *filenamepattern);
   bool loadImages(const std::vector<std::string> list);

   unsigned char *getVoxelData() {return(m_Voxels);}
   int getVoxelNum() {return(getCols()*getRows()*getSlis());}

   unsigned long getCols() {return m_Cols;}
   unsigned long getRows() {return m_Rows;}
   unsigned long getSlis() {return m_Images.size();}

   float getBound(int c) {return(m_Bounds[c]);}

   private:

   bool dicomLoad(const char *filenamepattern);
   bool dicomLoad(const std::vector<std::string> list);

   bool dicomProcess();

   void deleteImages();
   void sortImages();

   std::vector<ImageDesc*> m_Images;

   unsigned long m_Cols;
   unsigned long m_Rows;

   float m_PixSpaceRow;
   float m_PixSpaceCol;

   float m_Bounds[3];
   float m_VolDir[3];

   unsigned long m_SmallestPixVal;
   unsigned long m_LargestPixVal;

   unsigned char *m_Voxels;

   static int compareFunc(const void *elem1,const void *elem2);
   };

#endif
