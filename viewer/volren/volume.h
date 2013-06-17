// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLUME_H
#define VOLUME_H

#include <vector>
#include <string>

#include "codebase.h" // universal code base
#include "ddsbase.h" // volume file reader
#include "dicombase.h" // dicom file reader
#include "rekbase.h" // rek file reader
#include "rawbase.h" // raw file reader
#include "oglbase.h" // OpenGL base and window handling
#include "tfbase.h" // transfer functions
#include "tilebase.h" // volume tiles and bricks

// the volume
class volume
   {
   public:

   // default constructor
   volume(tfunc2D *tf,char *base=NULL);

   // destructor
   ~volume();

   // check brick size
   static BOOLINT check(int bricksize,float overmax);

   // set the volume data
   void set_data(unsigned char *data,
                 unsigned char *extra,
                 long long width,long long height,long long depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int bricksize,float overmax);

   float get_slab() {return(SLAB);} // return the slab thickness
   tfunc2D *get_tfunc() {return(TFUNC);} // return the transfer function

   // set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   // render the volume
   BOOLINT render(float ex,float ey,float ez,
                  float dx,float dy,float dz,
                  float ux,float uy,float uz,
                  float nearp,float slab,float rslab,
                  BOOLINT lighting=FALSE,
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL);

   // render the wire frame
   static void drawwireframe(float mx=0.0f,float my=0.0f,float mz=0.0f,
                             float sx=1.0f,float sy=1.0f,float sz=1.0f);

   // return center of bounding box
   float getcenterx() {return(MX);}
   float getcentery() {return(MY);}
   float getcenterz() {return(MZ);}

   // return size of bounding box
   float getsizex() {return(BX);}
   float getsizey() {return(BY);}
   float getsizez() {return(BZ);}

   // use 16-bit fbo
   static void usefbo(BOOLINT yes=FALSE);

   protected:

   float MX,MY,MZ,
         SX,SY,SZ,
         BX,BY,BZ;

   float SLAB;

   tileptr *TILE;
   int TILEMAX,TILECNT;

   int TILEX,TILEY,TILEZ;

   tfunc2D *TFUNC;

   private:

   char BASE[MAXSTR];

   BOOLINT sort(int x,int y,int z,
                int sx,int sy,int sz,
                float ex,float ey,float ez,
                float dx,float dy,float dz,
                float ux,float uy,float uz,
                float nearp,float slab,float rslab,
                BOOLINT lighting=FALSE,
                BOOLINT (*abort)(void *abortdata)=NULL,
                void *abortdata=NULL);

   static void updatefbo();

   // frame buffer object:

   static void setup(int width,int heigth);
   static void destroy();

   static BOOLINT HASFBO;
   static BOOLINT USEFBO;
   static int fboWidth,fboHeight;
   static GLuint textureId;
   static GLuint rboId;
   static GLuint fboId;
   };

typedef volume *volumeptr;

// the volume hierarchy:

class mipmap
   {
   public:

   // default constructor
   mipmap(char *base=NULL,int res=0);

   // destructor
   ~mipmap();

   // set the volume data
   void set_data(unsigned char *data,
                 unsigned char *extra,
                 long long width,long long height,long long depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int bricksize,float overmax);

   // load the volume data
   BOOLINT loadvolume(const char *filename,
                      const char *gradname=NULL,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      char *commands=NULL,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f);

   // load a DICOM series
   BOOLINT loadseries(const std::vector<std::string> list,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f);

   // save the volume data as PVM
   void savePVMvolume(const char *filename);

   tfunc2D *get_tfunc() {return(TFUNC);} // return the transfer function
   histo *get_histo() {return(HISTO);} // return the histogram

   float *get_hist(); // return the histogram
   float *get_histRGBA(); // return the colored histogram
   float *get_hist2D(); // return the scatter plot
   float *get_hist2DRGBA(); // return the colored scatter plot
   float *get_hist2DQRGBA(); // return the quantized scatter plot
   float *get_hist2DTFRGBA(); // return the quantized transfer function

   BOOLINT has_data(); // check whether or not the hierarchy has volume data
   float get_slab(); // return the slab thickness

   // set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   // render the volume
   BOOLINT render(float ex,float ey,float ez,
                  float dx,float dy,float dz,
                  float ux,float uy,float uz,
                  float nearp,float slab,
                  BOOLINT lighting=FALSE,
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL);

   // render the wire frame
   void drawwireframe();

   // return center of bounding box
   float getcenterx() {return(VOL[0]->getcenterx());}
   float getcentery() {return(VOL[0]->getcentery());}
   float getcenterz() {return(VOL[0]->getcenterz());}

   // return size of bounding box
   float getsizex() {return(VOL[0]->getsizex());}
   float getsizey() {return(VOL[0]->getsizey());}
   float getsizez() {return(VOL[0]->getsizez());}

   protected:

   volumeptr *VOL;
   int VOLCNT;

   tfunc2D *TFUNC;
   histo *HISTO;

   unsigned char *VOLUME,*GRAD;
   long long WIDTH,HEIGHT,DEPTH;
   unsigned int COMPONENTS;
   long long GWIDTH,GHEIGHT,GDEPTH;
   unsigned int GCOMPONENTS;
   float DSX,DSY,DSZ,GRADMAX;

   private:

   char BASE[MAXSTR];

   char filestr[MAXSTR];
   char gradstr[MAXSTR];
   char commstr[MAXSTR];
   char zerostr[MAXSTR];

   BOOLINT xsflag,ysflag,zsflag;
   BOOLINT xrflag,zrflag;

   float hmvalue,hfvalue,hsvalue;
   int knvalue;

   unsigned char *CACHE;
   long long CSIZEX,CSIZEY,CSLICE,CSLICES;

   int *QUEUEX,*QUEUEY,*QUEUEZ;
   unsigned int QUEUEMAX,QUEUECNT,QUEUESTART,QUEUEEND;

   unsigned char *readANYvolume(const char *filename,
                                long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                                float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                                BOOLINT *msb=NULL);

   unsigned char *reduce(unsigned char *data,
                         long long width,long long height,long long depth);

   unsigned char *swap(unsigned char *data,
                       long long *width,long long *height,long long *depth,
                       float *dsx,float *dsy,float *dsz,
                       BOOLINT xswap,BOOLINT yswap,BOOLINT zswap,
                       BOOLINT xrotate,BOOLINT zrotate);

   void cache(const unsigned char *data=NULL,
              long long width=0,long long height=0,long long depth=0,
              long long slice=0,long long slices=0);

   inline unsigned char get(const unsigned char *data,
                            const long long width,const long long height,const long long depth,
                            const long long x,const long long y,const long long z);

   inline void set(unsigned char *data,
                   const long long width,const long long height,const long long depth,
                   const long long x,const long long y,const long long z,unsigned char v);

   unsigned char *gradmag(unsigned char *data,
                          long long width,long long height,long long depth,
                          float dsx=1.0f,float dsy=1.0f,float dsz=1.0f,
                          float *gradmax=NULL);

   inline float getgrad(unsigned char *data,
                        long long width,long long height,long long depth,
                        long long i,long long j,long long k,
                        float dsx,float dsy,float dsz);

   inline float getgrad2(unsigned char *data,
                         long long width,long long height,long long depth,
                         long long i,long long j,long long k,
                         float dsx,float dsy,float dsz);

   inline float getsobel(unsigned char *data,
                         long long width,long long height,long long depth,
                         long long i,long long j,long long k,
                         float dsx,float dsy,float dsz);

   inline float threshold(float x,float thres);

   unsigned char *gradmagML(unsigned char *data,
                            long long width,long long height,long long depth,
                            float dsx=1.0f,float dsy=1.0f,float dsz=1.0f,
                            float *gradmax=NULL);

   unsigned char *variance(unsigned char *data,
                           long long width,long long height,long long depth);

   void blur(unsigned char *data,
             long long width,long long height,long long depth);

   void usetf(unsigned char *data,unsigned char *grad,
              long long width,long long height,long long depth);

   void useop(unsigned char *data,unsigned char *grad,
              long long width,long long height,long long depth);

   void remove(unsigned char *grad,
               long long width,long long height,long long depth);

   void tangle(unsigned char *grad,
               long long width,long long height,long long depth);

   long long grow(unsigned char *grad,
                  long long width,long long height,long long depth);

   long long floodfill(const unsigned char *data,unsigned char *mark,
                       const long long width,const long long height,const long long depth,
                       const long long x,const long long y,const long long z,
                       const int value,const int maxdev,
                       const int token);

   double countfill(const unsigned char *data,unsigned char *mark,
                    const long long width,const long long height,const long long depth,
                    const long long x,const long long y,const long long z,
                    const int value,const int maxdev,
                    const int token);

   long long gradfill(const unsigned char *grad,unsigned char *mark,
                      const long long width,const long long height,const long long depth,
                      const long long x,const long long y,const long long z,
                      const int token,const int maxgrad);

   unsigned char *sizify(unsigned char *data,
                         long long width,long long height,long long depth,
                         float maxdev);

   unsigned char *classify(unsigned char *grad,
                           long long width,long long height,long long depth,
                           float maxgrad,
                           unsigned int *classes=NULL);

   void zero(unsigned char *data,unsigned char *grad,
             long long width,long long height,long long depth,
             float maxdev);

   void parsecommands(unsigned char *volume,
                      long long width,long long height,long long depth,
                      char *commands);

   void parsegradcommands(unsigned char *volume,unsigned char *grad,
                          long long width,long long height,long long depth,
                          char *commands);

   inline unsigned char getscalar(unsigned char *volume,
                                  long long width,long long height,long long depth,
                                  float x,float y,float z);

   inline float getscalar(unsigned short int *volume,
                          long long width,long long height,long long depth,
                          float x,float y,float z);

   inline float getscalar(float *volume,
                          long long width,long long height,long long depth,
                          float x,float y,float z);

   unsigned char *scale(unsigned char *volume,
                        long long width,long long height,long long depth,
                        long long nwidth,long long nheight,long long ndepth);
   };

#endif
