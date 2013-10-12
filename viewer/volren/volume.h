// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLUME_H
#define VOLUME_H

#include <vector>
#include <string>

#include "codebase.h" // universal code base
#include "ddsbase.h" // volume file reader
#include "dicombase.h" // dicom file reader
#ifdef HAVE_MINI
#include <mini/rekbase.h> // rek file reader
#include <mini/rawbase.h> // raw file reader
#endif
#include "oglbase.h" // OpenGL base and window handling
#include "tfbase.h" // transfer functions
#include "tilebase.h" // volume tiles and bricks
#include "geobase.h" // surface wrapper

extern float ISO_TARGET_RATIO;
extern long long ISO_TARGET_CELLS;

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
                 int bricksize,float overmax,
                 void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

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
   void usefbo(BOOLINT yes=FALSE);

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
   };

typedef volume *volumeptr;

//! the volume hierarchy
class mipmap
   {
   public:

   //! default constructor
   mipmap(char *base=NULL,int res=0);

   //! destructor
   virtual ~mipmap();

   //! set the volume data
   void set_data(unsigned char *data,
                 unsigned char *extra,
                 long long width,long long height,long long depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int bricksize,float overmax,
                 void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   //! load the volume data
   BOOLINT loadvolume(const char *filename,
                      const char *gradname=NULL,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      char *commands=NULL,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f,
                      void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   //! load a DICOM series
   BOOLINT loadseries(const std::vector<std::string> list,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f,
                      void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   //! save the volume data as PVM
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
   BOOLINT has_grad(); // check whether or not the hierarchy has gradient data
   float get_slab(); // return the slab thickness

   //! set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   //! extract iso surface
   char *extractsurface(double isovalue,
                        float ratio=ISO_TARGET_RATIO,
                        long long cell_limit=ISO_TARGET_CELLS,
                        void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   //! extract iso surface for the smallest non-zero tfunc entry
   char *extractTFsurface(float ratio=ISO_TARGET_RATIO,
                          long long cell_limit=ISO_TARGET_CELLS,
                          void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   //! load the surface data
   BOOLINT loadsurface(const char *filename);

   BOOLINT has_geo(); // check whether or not a surface is present

   //! render the volume
   BOOLINT render(float ex,float ey,float ez,
                  float dx,float dy,float dz,
                  float ux,float uy,float uz,
                  float nearp,float slab,
                  BOOLINT lighting=FALSE,
                  BOOLINT usefbo=FALSE,
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL);

   //! return center of bounding box
   float getcenterx() {return(VOL[0]->getcenterx());}
   float getcentery() {return(VOL[0]->getcentery());}
   float getcenterz() {return(VOL[0]->getcenterz());}

   //! return size of bounding box
   float getsizex() {return(VOL[0]->getsizex());}
   float getsizey() {return(VOL[0]->getsizey());}
   float getsizez() {return(VOL[0]->getsizez());}

   // return scaling factor
   float getscale()
      {return(fmax(DSX*(WIDTH-1),fmax(DSY*(HEIGHT-1),DSZ*(DEPTH-1))));}

   //! get eye point
   void get_eye(double &ex,double &ey,double &ez,
                double &dx,double &dy,double &dz,
                double &ux,double &uy,double &uz)
      {
      ex=ex_;
      ey=ey_;
      ez=ez_;

      dx=dx_;
      dy=dy_;
      dz=dz_;

      ux=ux_;
      uy=uy_;
      uz=uz_;
      }

   //! get near plane
   void get_near(double &px,double &py,double &pz,
                 double &nx,double &ny,double &nz)
      {
      px=px_;
      py=py_;
      pz=pz_;

      nx=nx_;
      ny=ny_;
      nz=nz_;
      }

   //! define clip plane
   void define_clip(int n,
                    double a,double b,double c,double d)
      {
      if (n<0 || n>=6) return;

      clip_a[n]=a;
      clip_b[n]=b;
      clip_c[n]=c;
      clip_d[n]=d;
      }

   //! define clip plane via point on plane and normal
   void define_clip(int n,
                    double px,double py,double pz,
                    double nx,double ny,double nz)
      {
      double l;

      l=sqrt(nx*nx+ny*ny+nz*nz);

      nx/=l;
      ny/=l;
      nz/=l;

      l=nx*px+ny*py+nz*pz;

      define_clip(n,nx,ny,nz,-l);
      }

   //! enable clip plane
   void enable_clip(int n,int on)
      {
      if (n<0 || n>=6) return;

      clip_on[n]=on;
      }

   //! disable all clip planes
   void disable_clip()
      {
      int i;

      for (i=0; i<6; i++)
         clip_on[i]=0;
      }

   protected:

   volumeptr *VOL;
   int VOLCNT;

   tfunc2D *TFUNC;
   histo *HISTO;

   unsigned char *VOLUME;
   long long WIDTH,HEIGHT,DEPTH;
   unsigned int COMPONENTS;
   float DSX,DSY,DSZ;

   unsigned char *GRAD;
   long long GWIDTH,GHEIGHT,GDEPTH;
   unsigned int GCOMPONENTS;
   float GDSX,GDSY,GDSZ,GRADMAX;

   Surface SURFACE;

   double ex_,ey_,ez_;
   double dx_,dy_,dz_;
   double ux_,uy_,uz_;

   double px_,py_,pz_;
   double nx_,ny_,nz_;

   int clip_on[6];
   double clip_a[6];
   double clip_b[6];
   double clip_c[6];
   double clip_d[6];

   // render opaque geometry
   virtual void rendergeometry() = 0;

   // render the wire frame
   void drawwireframe();

   private:

   // volume options:

   char BASE[MAXSTR];

   char filestr[MAXSTR];
   char gradstr[MAXSTR];
   char commstr[MAXSTR];
   char zerostr[MAXSTR];

   BOOLINT xsflag,ysflag,zsflag;
   BOOLINT xrflag,zrflag;

   float hmvalue,hfvalue,hsvalue;
   int knvalue;

   // preprocessing cache:

   unsigned char *CACHE;
   long long CSIZEX,CSIZEY,CSLICE,CSLICES;

   int *QUEUEX,*QUEUEY,*QUEUEZ;
   unsigned int QUEUEMAX,QUEUECNT,QUEUESTART,QUEUEEND;

   // frame buffer object:

   BOOLINT HASFBO;
   int fboWidth,fboHeight;
   GLuint textureId;
   GLuint rboId;
   GLuint fboId;

   void setup(int width,int heigth);
   void destroy();

   void updatefbo();

   // volume loading and preprocessing:

   unsigned char *readANYvolume(const char *filename,
                                long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                                float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                                BOOLINT *msb=NULL,
                                void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   unsigned char *reduce(unsigned char *data,
                         long long width,long long height,long long depth,
                         void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

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

   unsigned char *calc_gradmag(unsigned char *data,
                               long long width,long long height,long long depth,
                               float dsx,float dsy,float dsz,
                               float *gradmax=NULL,
                               void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   unsigned char *gradmag(unsigned char *data,
                          long long width,long long height,long long depth,
                          float dsx=1.0f,float dsy=1.0f,float dsz=1.0f,
                          float *gradmax=NULL,
                          void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

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
                            float *gradmax=NULL,
                            void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

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

//! the volume scene
class volscene: public mipmap
   {
   public:

   //! default constructor
   volscene(char *base=NULL,int res=0)
      : mipmap(base,res)
      {
      wireframe_=FALSE;
      histogram_=FALSE;
      }

   //! destructor
   virtual ~volscene()
      {}

   //! enable wire frame mode
   void enablewireframe(BOOLINT on=FALSE)
      {wireframe_=on;}

   //! enable display of histogram
   void enablehistogram(BOOLINT on=FALSE)
      {histogram_=on;}

   protected:

   BOOLINT wireframe_;
   BOOLINT histogram_;

   // render opaque geometry
   virtual void rendergeometry()
      {
      // wire frame box
      if (wireframe_ || (!has_data() && !has_geo())) drawwireframe();

      // quantized histogram
      if (histogram_)
         if (has_data())
            get_histo()->render2DQ(getcenterx(),getcentery(),getcenterz(),
                                   getsizex(),getsizey(),getsizez());
      }

   };

#endif
