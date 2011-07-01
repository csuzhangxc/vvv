// (c) by Stefan Roettger

#include "codebase.h"

#include "dirbase.h"

#include "dicombase.h"

DicomVolume::ImageDesc::~ImageDesc()
   {delete m_Image;}

DicomVolume::DicomVolume():m_Voxels(0) {}

DicomVolume::~DicomVolume()
   {
   delete[] m_Voxels;
   deleteImages();
   }

void DicomVolume::deleteImages()
   {
   int s=m_Images.size();
   for (int i=0; i<s; i++) delete m_Images[i];
   m_Images.clear();
   }

bool DicomVolume::loadImages(const char *filenamepattern)
   {
   if (m_Images.size()!=0) deleteImages();

   if (!dicomLoad(filenamepattern))
      {
      deleteImages();
      return(false);
      }

   return(true);
   }

bool DicomVolume::dicomLoad(const char *filenamepattern)
   {
   unsigned int i,j;

   const char *fname;

   if (filenamepattern==NULL) return(false);

   // read DICOM instances:

   filesearch(filenamepattern);
   fname=findfile();

   if (!fname) return(false);

   do
      {
      ImageDesc *desc=new ImageDesc();
      desc->m_Image=new DcmFileFormat();

      printf("load DICOM file %s\n",fname);

      if (desc->m_Image->loadFile(fname).bad()) return(false);

      desc->m_Image->getDataset()->loadAllDataIntoMemory();

      // add to image vector
      m_Images.push_back(desc);

      fname=findfile();
      }
   while (fname);

   if (m_Images.size()<2) return(false);

   // check and sort the images by their position:

   OFString tmp;

   float position0[3];
   float position1[3];

   unsigned int last = m_Images.size()-1;

   DcmDataset *firstImage=m_Images[0]->m_Image->getDataset();
   DcmDataset *lastImage=m_Images[last]->m_Image->getDataset();

   // get position of first image
   if (firstImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,0).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position0[0]);
   if (firstImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,1).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position0[1]);
   if (firstImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,2).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position0[2]);

   // get position of last image
   if (lastImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,0).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position1[0]);
   if (lastImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,1).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position1[1]);
   if (lastImage->findAndGetOFString(DCM_ImagePositionPatient,tmp,2).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&position1[2]);

   // calculate direction vector
   m_VolDir[0]=position1[0]-position0[0];
   m_VolDir[1]=position1[1]-position0[1];
   m_VolDir[2]=position1[2]-position0[2];

   // calculate first and last slice position along direction vector
   float minPos=m_Images[0]->m_pos=0.0;
   float maxPos=sqrt(m_VolDir[0]*m_VolDir[0]+m_VolDir[1]*m_VolDir[1]+m_VolDir[2]*m_VolDir[2]);

   // direction vector should be normalized
   m_VolDir[0]/=maxPos;
   m_VolDir[1]/=maxPos;
   m_VolDir[2]/=maxPos;

   // read columns and rows
   if (firstImage->findAndGetOFString(DCM_Columns,tmp).bad()) return(false);
   sscanf(tmp.c_str(),"%lu",&m_Cols);
   if (firstImage->findAndGetOFString(DCM_Rows,tmp).bad()) return(false);
   sscanf(tmp.c_str(),"%lu",&m_Rows);

   // read pixel spacing
   if (firstImage->findAndGetOFString(DCM_PixelSpacing,tmp,0).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&m_PixSpaceRow);
   if (firstImage->findAndGetOFString(DCM_PixelSpacing,tmp,1).bad()) return(false);
   sscanf(tmp.c_str(),"%g",&m_PixSpaceCol);

   // read pixel value range
   if (firstImage->findAndGetOFString(DCM_SmallestImagePixelValue,tmp).bad()) m_SmallestPixVal=0;
   else sscanf(tmp.c_str(),"%lu",&m_SmallestPixVal);
   if (firstImage->findAndGetOFString(DCM_LargestImagePixelValue,tmp).bad()) m_LargestPixVal=4095;
   else sscanf(tmp.c_str(),"%lu",&m_LargestPixVal);

   // now calculate the position of the slices along the direction vector
   for (i=1; i<=last; i++)
      {
      ImageDesc *desc=m_Images[i];

      float position[3];
      unsigned long cols,rows;
      unsigned long smallestPixVal,largestPixVal;

      // get position of actual slice
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_ImagePositionPatient,tmp,0).bad()) return(false);
      sscanf(tmp.c_str(),"%g",&position[0]);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_ImagePositionPatient,tmp,1).bad()) return(false);
      sscanf(tmp.c_str(),"%g",&position[1]);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_ImagePositionPatient,tmp,2).bad()) return(false);
      sscanf(tmp.c_str(),"%g",&position[2]);

      // the slice position is the dot product between the direction and the position offset
      float pos=desc->m_pos=m_VolDir[0]*(position[0]-position0[0])+m_VolDir[1]*(position[1]-position0[1])+m_VolDir[2]*(position[2]-position0[2]);

      // update position range
      if (pos<minPos) minPos=pos;
      if (pos>maxPos) maxPos=pos;

      // retrieve number of columns and rows
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_Columns,tmp).bad()) return(false);
      sscanf(tmp.c_str(),"%lu",&cols);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_Rows,tmp).bad()) return(false);
      sscanf(tmp.c_str(),"%lu",&rows);

      // compare number of columns and rows
      if (cols!=m_Cols || rows!=m_Rows) return(false);

      // retrieve smallest and largest pixel value
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_SmallestImagePixelValue,tmp).bad()) smallestPixVal=0;
      else sscanf(tmp.c_str(),"%lu",&smallestPixVal);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_LargestImagePixelValue,tmp).bad()) largestPixVal=4095;
      else sscanf(tmp.c_str(),"%lu",&largestPixVal);

      if (smallestPixVal<m_SmallestPixVal) m_SmallestPixVal=smallestPixVal;
      if (largestPixVal>m_LargestPixVal) m_LargestPixVal=largestPixVal;
      }

   // calculate bounds
   m_Bounds[0]=m_PixSpaceCol*(m_Cols-1);
   m_Bounds[1]=m_PixSpaceRow*(m_Rows-1);
   m_Bounds[2]=maxPos-minPos;

   // sort images
   sortImages();

   // create the volume:

   unsigned int totalSize=m_Cols*m_Rows*m_Images.size();

   unsigned char *voxels=m_Voxels=new unsigned char[totalSize];
   if (voxels==0) return(false);

   // calculate the scaling factor from the pixel value range
   if (m_LargestPixVal==m_SmallestPixVal) m_LargestPixVal++;
   float factor=255.0f/(m_LargestPixVal-m_SmallestPixVal);

   const Uint16 *data=NULL;
   unsigned long length=0;

   // now we copy the pixel data into the volume slice by slice
   for (i=0; i<=last; i++)
      {
      if (m_Images[i]->m_Image->getDataset()->findAndGetUint16Array(DCM_PixelData,data,&length).bad()) return(false);
      if (data==NULL || length==0) return(false);

      unsigned short *usdata=(unsigned short *)data;

      // scale each voxel
      for (j=0; j<length; j++, voxels++)
         *voxels=(unsigned char)((usdata[j]-m_SmallestPixVal)*factor+0.5f);
      }

   return(true);
   }

void DicomVolume::sortImages()
   {
   int i;

   int s=m_Images.size();
   if (s==0) return;

   ImageDesc **descArray=new ImageDesc*[s];
   for (i=0; i<s; i++) descArray[i]=m_Images[i];

   qsort(descArray,s,sizeof(ImageDesc*),compareFunc);

   for (i=0; i<s; i++) m_Images[i]=descArray[i];
   delete[] descArray;
   }

int DicomVolume::compareFunc(const void* elem1,const void* elem2)
   {
   const ImageDesc** ppid1=(const ImageDesc**)elem1;
   const ImageDesc** ppid2=(const ImageDesc**)elem2;

   if ((*ppid1)->m_pos<(*ppid2)->m_pos) return(-1);

   return(1);
   }
