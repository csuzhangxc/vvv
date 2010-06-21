// (c) by Stefan Roettger

#ifndef DICOMBASE_H
#define DICOMBASE_H

#include <vector>

#include "codebase.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

class DicomVolume
   {
   class ImageDesc
      {
      public:

      ImageDesc() : m_Image(0) {}
      virtual ~ImageDesc();

      DcmFileFormat *m_Image;
      float m_pos;

      private:

      ImageDesc(const ImageDesc&);
      ImageDesc& operator=(const ImageDesc&);
      };

   public:

   DicomVolume();
   virtual ~DicomVolume();

   bool loadImages(const char *filenamepattern);

   unsigned char *getVoxelData() {return(m_Voxels);}
   int getVoxelNum() {return(getCols()*getRows()*getSlis());}

   unsigned long getCols() {return m_Cols;}
   unsigned long getRows() {return m_Rows;}
   unsigned long getSlis() {return m_Images.size();}

   float getBound(int c) {return(m_Bounds[c]);}

   private:

   bool dicomLoad(const char *filenamepattern);

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
