// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"

#include "dirbase.h"

#include "dicombase.h"

#ifdef HAVE_DCMTK
#include <dcmtk/dcmjpeg/djdecode.h>
#endif

DicomVolume::ImageDesc::~ImageDesc()
   {
#ifdef HAVE_DCMTK
   delete m_Image;
#endif
   }

DicomVolume::DicomVolume():m_Voxels(0) {}

DicomVolume::~DicomVolume()
   {
   delete[] m_Voxels;
   deleteImages();
   }

void DicomVolume::deleteImages()
   {
#ifdef HAVE_DCMTK
   int s=m_Images.size();
   for (int i=0; i<s; i++) delete m_Images[i];
   m_Images.clear();
#endif
   }

bool DicomVolume::loadImages(const char *filenamepattern,
                             void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   if (m_Images.size()!=0) deleteImages();

   if (!dicomLoad(filenamepattern,feedback,obj))
      {
      deleteImages();
      return(false);
      }

   return(true);
   }

bool DicomVolume::loadImages(const std::vector<std::string> list,
                             void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   if (m_Images.size()!=0) deleteImages();

   if (!dicomLoad(list,feedback,obj))
      {
      deleteImages();
      return(false);
      }

   return(true);
   }

bool DicomVolume::dicomLoad(const char *filenamepattern,
                            void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
#ifdef HAVE_DCMTK

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

      if (feedback!=NULL)
         {
         char *info=strdup2("loading DICOM file ",fname);
         feedback(info,0,obj);
         free(info);
         }

      if (desc->m_Image->loadFile(fname).bad()) return(false);

      desc->m_Image->getDataset()->loadAllDataIntoMemory();

      // add to image vector
      m_Images.push_back(desc);

      fname=findfile();
      }
   while (fname);

   return(dicomProcess());

#else

   return(false);

#endif
   }

bool DicomVolume::dicomLoad(const std::vector<std::string> list,
                            void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
#ifdef HAVE_DCMTK

   unsigned int i;

   const char *fname;

   for (i=0; i<list.size(); i++)
      {
      ImageDesc *desc=new ImageDesc();
      desc->m_Image=new DcmFileFormat();

      fname=list.at(i).c_str();

      if (feedback!=NULL)
         {
         char *info=strdup2("loading DICOM file ",fname);
         feedback(info,0,obj);
         free(info);
         }

      if (desc->m_Image->loadFile(fname).bad()) return(false);

      desc->m_Image->getDataset()->loadAllDataIntoMemory();

      // add to image vector
      m_Images.push_back(desc);
      }

   return(dicomProcess());

#else

   return(false);

#endif
   }

bool check_intel()
   {
   static unsigned short int RAW_INTEL=1;
   return(*((unsigned char *)(&RAW_INTEL)+1)==0);
   }

bool DicomVolume::dicomProcess()
   {
#ifdef HAVE_DCMTK

   unsigned int i,j;

   if (!check_intel()) return(false);
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
   sscanf(tmp.c_str(),"%lld",&m_Cols);
   if (firstImage->findAndGetOFString(DCM_Rows,tmp).bad()) return(false);
   sscanf(tmp.c_str(),"%lld",&m_Rows);

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
      long long cols,rows;
      unsigned int smallestPixVal,largestPixVal;

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
      sscanf(tmp.c_str(),"%lld",&cols);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_Rows,tmp).bad()) return(false);
      sscanf(tmp.c_str(),"%lld",&rows);

      // compare number of columns and rows
      if (cols!=m_Cols || rows!=m_Rows) return(false);

      // retrieve smallest and largest pixel value
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_SmallestImagePixelValue,tmp).bad()) smallestPixVal=0;
      else sscanf(tmp.c_str(),"%u",&smallestPixVal);
      if (desc->m_Image->getDataset()->findAndGetOFString(DCM_LargestImagePixelValue,tmp).bad()) largestPixVal=65535;
      else sscanf(tmp.c_str(),"%u",&largestPixVal);

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

   long long totalSize=m_Cols*m_Rows*m_Images.size();

   unsigned short *voxels=m_Voxels=new unsigned short[totalSize];
   if (voxels==0) return(false);

   // calculate the scaling factor from the pixel value range
   if (m_LargestPixVal==m_SmallestPixVal) m_LargestPixVal++;
   float factor=65535.0f/(m_LargestPixVal-m_SmallestPixVal);

   const Uint16 *data=NULL;
   unsigned long length=0;

   DJDecoderRegistration::registerCodecs(); // register JPEG codecs

   // now we copy the pixel data into the volume slice by slice
   for (i=0; i<=last; i++)
      {
      DcmDataset *dataset=m_Images[i]->m_Image->getDataset();

      dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL); // decompress
      if (!dataset->canWriteXfer(EXS_LittleEndianExplicit)) return(false);

      if (dataset->findAndGetUint16Array(DCM_PixelData,data,&length).bad()) return(false);
      if (data==NULL || length==0) return(false);

      unsigned short *usdata=(unsigned short *)data;

      // scale and copy each voxel
      for (j=0; j<length; j++, voxels++)
         *voxels=(unsigned short)((usdata[j]-m_SmallestPixVal)*factor+0.5f);
      }

   DJDecoderRegistration::cleanup(); // deregister JPEG codecs

   return(true);

#else

   return(false);

#endif
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

// read a DICOM series identified by the * in the filename pattern
unsigned char *readDICOMvolume(const char *filename,
                               long long *width,long long *height,long long *depth,unsigned int *components,
                               float *scalex,float *scaley,float *scalez,
                               void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   DicomVolume data;
   unsigned char *chunk;

   if (!data.loadImages(filename,feedback,obj)) return(NULL);

   if ((chunk=(unsigned char *)malloc(data.getByteCount()))==NULL) ERRORMSG();
   memcpy(chunk,data.getVoxelData(),data.getByteCount());

   *width=data.getCols();
   *height=data.getRows();
   *depth=data.getSlis();

   *components=2;

   if (scalex!=NULL) *scalex=data.getBound(0)/data.getCols();
   if (scaley!=NULL) *scaley=data.getBound(1)/data.getRows();
   if (scalez!=NULL) *scalez=data.getBound(2)/data.getSlis();

   return(chunk);
   }

// read a DICOM series from a file name list
unsigned char *readDICOMvolume(const std::vector<std::string> list,
                               long long *width,long long *height,long long *depth,unsigned int *components,
                               float *scalex,float *scaley,float *scalez,
                               void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   DicomVolume data;
   unsigned char *chunk;

   if (!data.loadImages(list,feedback,obj)) return(NULL);

   if ((chunk=(unsigned char *)malloc(data.getByteCount()))==NULL) ERRORMSG();
   memcpy(chunk,data.getVoxelData(),data.getByteCount());

   *width=data.getCols();
   *height=data.getRows();
   *depth=data.getSlis();

   *components=2;

   if (scalex!=NULL) *scalex=data.getBound(0)/data.getCols();
   if (scaley!=NULL) *scaley=data.getBound(1)/data.getRows();
   if (scalez!=NULL) *scalez=data.getBound(2)/data.getSlis();

   return(chunk);
   }

// check for intel lsb representation
bool DicomVolume::check_intel()
   {
   static unsigned short int INTEL=1;
   return(*((unsigned char *)(&INTEL)+1)==0);
   }
