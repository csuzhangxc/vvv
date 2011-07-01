// (c) by Stefan Roettger

#ifndef TILEBASE_H
#define TILEBASE_H

#include "codebase.h" // universal code base
#include "ddsbase.h" // volume file reader
#include "dicombase.h" // dicom file reader
#include "oglbase.h" // OpenGL base and window handling
#include "tfbase.h" // transfer functions

#define MAXSTR 256

#define PROGNUM 5

#define TILEINC 1000
#define QUEUEINC 1000

// a texture brick:

class brick
   {
   public:

   // default constructor
   brick();

   // destructor
   ~brick();

   // generate 3D texture map
   void buildtexmap3D(unsigned char *volume,
                      int width,int height,int depth);

   // return texture id
   int get_id() {return(TEXID);}

   protected:

   GLuint TEXID;

   // delete 3D texture map
   void deletetexmap3D();

   private:
   };

// a tile of the volume:

class tile
   {
   public:

   // default constructor
   tile(tfunc2D *tf,char *base=NULL);

   // destructor
   ~tile();

   // set the tile data
   void set_data(unsigned char *data,
                 unsigned int width,unsigned int height,unsigned int depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int px,int py,int pz,
                 int bricksize,int border);

   // set the extra tile data
   void set_extra(unsigned char *extra,
                  unsigned int width,unsigned int height,unsigned int depth,
                  int px,int py,int pz,
                  int bricksize);

   // set the tile size
   void set_size(float mx,float my,float mz,
                 float sx,float sy,float sz);

   // set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   // get ambient/diffuse/specular lighting coefficients
   void get_light(float *noise,float *ambnt,float *difus,float *specl,float *specx);

   // get functions for tile position and size
   float get_mx() {return(MX);}
   float get_my() {return(MY);}
   float get_mz() {return(MZ);}
   float get_sx() {return(SX);}
   float get_sy() {return(SY);}
   float get_sz() {return(SZ);}

   // get functions for position and size of visible tile
   float get_mx2() {return(MX2);}
   float get_my2() {return(MY2);}
   float get_mz2() {return(MZ2);}
   float get_sx2() {return(SX2);}
   float get_sy2() {return(SY2);}
   float get_sz2() {return(SZ2);}

   // render the tile
   void render(float ex,float ey,float ez,
               float dx,float dy,float dz,
               float ux,float uy,float uz,
               float nearp,float slab,float rslab,
               BOOLINT lighting=FALSE);

   protected:

   int BSIZE; // block size in voxels

   float MX,MY,MZ, // midpoint of tile
         SX,SY,SZ; // size of tile

   float MX2,MY2,MZ2, // midpoint of visible tile
         SX2,SY2,SZ2; // size of visible tile

   int BORDER; // tile overlap in voxels

   unsigned char MINDATA,MAXDATA; // range of primary data
   unsigned char MINEXTRA,MAXEXTRA; // range of extra data

   brick *BRICK,*EXTRA; // primary and extra data
   tfunc2D *TFUNC; // applied transfer function

   // ambient/diffuse/specular lighting coefficients
   float NOISE,AMBNT,DIFUS,SPECL,SPECX;
   BOOLINT LIGHTING;

   // computation of slice planes:

   inline void intersect(const float px,const float py,const float pz,
                         const float dx,const float dy,const float dz,
                         const float ox,const float oy,const float oz,
                         const float nx,const float ny,const float nz,
                         float *mx,float *my,float *mz);

   inline void projtexcoords(const float x,const float y,const float z,const float slab2);

   inline void slicetetra1(const float p1x,const float p1y,const float p1z,const float d1,
                           const float p2x,const float p2y,const float p2z,const float d2,
                           const float p3x,const float p3y,const float p3z,const float d3,
                           const float p4x,const float p4y,const float p4z,const float d4,
                           const float slab);

   inline void slicetetra2(const float p1x,const float p1y,const float p1z,const float d1,
                           const float p2x,const float p2y,const float p2z,const float d2,
                           const float p3x,const float p3y,const float p3z,const float d3,
                           const float p4x,const float p4y,const float p4z,const float d4,
                           const float slab);

   inline void slicetetra(const float p1x,const float p1y,const float p1z,
                          const float p2x,const float p2y,const float p2z,
                          const float p3x,const float p3y,const float p3z,
                          const float p4x,const float p4y,const float p4z,
                          const float slab);

   inline BOOLINT isfront(const float p1x,const float p1y,const float p1z,
                          const float p2x,const float p2y,const float p2z,
                          const float p3x,const float p3y,const float p3z,
                          const float p4x,const float p4y,const float p4z);

   void drawhexa(const float p1x,const float p1y,const float p1z,
                 const float p2x,const float p2y,const float p2z,
                 const float p3x,const float p3y,const float p3z,
                 const float p4x,const float p4y,const float p4z,
                 const float p5x,const float p5y,const float p5z,
                 const float p6x,const float p6y,const float p6z,
                 const float p7x,const float p7y,const float p7z,
                 const float p8x,const float p8y,const float p8z,
                 const float slab);

   private:

   // eye parameters:

   float EX,EY,EZ,
         DX,DY,DZ,
         UX,UY,UZ;

   float NEARP;

   // texture shader and fragment program setup
   void bindtexmaps(int texid3D,int texid2DE,int texid2DA);
   void bindtexmaps1D(int texid3D,int texid1DE,int texid1DA);
   void bindtexmaps2D(int texid3D,int texid3DG,int texid2DE,int texid2DA);
   void bindtexmaps3D(int texid3D,int texid3DG,int texid3DE,int texid3DA,float rslab);

   // fragment program loading:

   static BOOLINT LOADED;
   static GLuint PROGID[PROGNUM];

   static void setup(char *base=NULL);
   static void destroy();
   };

typedef tile *tileptr;

// the volume:

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
                 int width,int height,int depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int bricksize,float overmax);

   float get_slab() {return(SLAB);} // return the slab thickness
   tfunc2D *get_tfunc() {return(TFUNC);} // return the transfer function

   // set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   // render the volume
   void render(float ex,float ey,float ez,
               float dx,float dy,float dz,
               float ux,float uy,float uz,
               float nearp,float slab,float rslab,
               BOOLINT lighting=FALSE);

   // render the wire frame
   void drawwireframe();

   // return center of bounding box
   float getcenterx() {return(MX);}
   float getcentery() {return(MY);}
   float getcenterz() {return(MZ);}

   // return size of bounding box
   float getsizex() {return(SX);}
   float getsizey() {return(SY);}
   float getsizez() {return(SZ);}

   protected:

   float MX,MY,MZ,
         SX,SY,SZ;

   float SLAB;

   tileptr *TILE;
   int TILEMAX,TILECNT;

   int TILEX,TILEY,TILEZ;

   tfunc2D *TFUNC;

   private:

   char BASE[MAXSTR];

   void sort(int x,int y,int z,
             int sx,int sy,int sz,
             float ex,float ey,float ez,
             float dx,float dy,float dz,
             float ux,float uy,float uz,
             float nearp,float slab,float rslab,
             BOOLINT lighting=FALSE);
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
                 int width,int height,int depth,
                 float mx,float my,float mz,
                 float sx,float sy,float sz,
                 int bricksize,float overmax);

   // load the volume data
   void loadvolume(char *filename,
                   char *gradname=NULL,
                   float mx=0.0f,float my=0.0f,float mz=0.0f,
                   float sx=1.0f,float sy=1.0f,float sz=1.0f,
                   int bricksize=128,float overmax=8.0f,
                   BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                   BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                   BOOLINT usegrad=FALSE,
                   char *commands=NULL,
                   int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f);

   // save the volume data as PVM
   void savePVMvolume(char *filename);

   tfunc2D *get_tfunc() {return(TFUNC);} // return the transfer function
   histo *get_histo() {return(HISTO);} // return the histogram

   float *get_hist(); // return the histogram
   float *get_histRGBA(); // return the colored histogram
   float *get_hist2D(); // return the scatter plot
   float *get_hist2DRGBA(); // return the colored scatter plot
   float *get_hist2DQRGBA(); // return the quantized scatter plot
   float *get_hist2DTFRGBA(); // return the quantized transfer function

   float get_slab(); // return the slab thickness

   // set ambient/diffuse/specular lighting coefficients
   void set_light(float noise,float ambnt,float difus,float specl,float specx);

   // render the volume
   void render(float ex,float ey,float ez,
               float dx,float dy,float dz,
               float ux,float uy,float uz,
               float nearp,float slab,
               BOOLINT lighting=FALSE);

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
   unsigned int WIDTH,HEIGHT,DEPTH,COMPONENTS;
   unsigned int GWIDTH,GHEIGHT,GDEPTH,GCOMPONENTS;
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
   int CSIZEX,CSIZEY,CSLICE,CSLICES;

   int *QUEUEX,*QUEUEY,*QUEUEZ;
   unsigned int QUEUEMAX,QUEUECNT,QUEUESTART,QUEUEEND;

   unsigned char *readANYvolume(char *filename,
                                unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components=NULL,
                                float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

   unsigned char *reduce(unsigned char *data,
                         unsigned int width,unsigned int height,unsigned int depth);

   unsigned char *swap(unsigned char *data,
                       unsigned int *width,unsigned int *height,unsigned int *depth,
                       float *dsx,float *dsy,float *dsz,
                       BOOLINT xswap,BOOLINT yswap,BOOLINT zswap,
                       BOOLINT xrotate,BOOLINT zrotate);

   void cache(const unsigned char *data=NULL,
              unsigned int width=0,unsigned int height=0,unsigned int depth=0,
              int slice=0,int slices=0);

   inline unsigned char get(const unsigned char *data,
                            const unsigned int width,const unsigned int height,const unsigned int depth,
                            const unsigned int x,const unsigned int y,const unsigned int z);

   inline void set(unsigned char *data,
                   const unsigned int width,const unsigned int height,const unsigned int depth,
                   const unsigned int x,const unsigned int y,const unsigned int z,unsigned char v);

   unsigned char *gradmag(unsigned char *data,
                          unsigned int width,unsigned int height,unsigned int depth,
                          float dsx=1.0f,float dsy=1.0f,float dsz=1.0f,
                          float *gradmax=NULL);

   inline float getgrad(unsigned char *data,
                        int width,int height,int depth,
                        int i,int j,int k,
                        float dsx,float dsy,float dsz);

   inline float getgrad2(unsigned char *data,
                         int width,int height,int depth,
                         int i,int j,int k,
                         float dsx,float dsy,float dsz);

   inline float getsobel(unsigned char *data,
                         int width,int height,int depth,
                         int i,int j,int k,
                         float dsx,float dsy,float dsz);

   inline float threshold(float x,float thres);

   unsigned char *gradmagML(unsigned char *data,
                            unsigned int width,unsigned int height,unsigned int depth,
                            float dsx=1.0f,float dsy=1.0f,float dsz=1.0f,
                            float *gradmax=NULL);

   unsigned char *variance(unsigned char *data,
                           unsigned int width,unsigned int height,unsigned int depth);

   void blur(unsigned char *data,
             unsigned int width,unsigned int height,unsigned int depth);

   void usetf(unsigned char *data,unsigned char *grad,
              unsigned int width,unsigned int height,unsigned int depth);

   void useop(unsigned char *data,unsigned char *grad,
              unsigned int width,unsigned int height,unsigned int depth);

   void remove(unsigned char *grad,
               unsigned int width,unsigned int height,unsigned int depth);

   void tangle(unsigned char *grad,
               unsigned int width,unsigned int height,unsigned int depth);

   unsigned int grow(unsigned char *grad,
                     unsigned int width,unsigned int height,unsigned int depth);

   unsigned int floodfill(const unsigned char *data,unsigned char *mark,
                          const unsigned int width,const unsigned int height,const unsigned int depth,
                          const unsigned int x,const unsigned int y,const unsigned int z,
                          const int value,const int maxdev,
                          const int token);

   float countfill(const unsigned char *data,unsigned char *mark,
                   const unsigned int width,const unsigned int height,const unsigned int depth,
                   const unsigned int x,const unsigned int y,const unsigned int z,
                   const int value,const int maxdev,
                   const int token);

   unsigned int gradfill(const unsigned char *grad,unsigned char *mark,
                         const unsigned int width,const unsigned int height,const unsigned int depth,
                         const unsigned int x,const unsigned int y,const unsigned int z,
                         const int token,const int maxgrad);

   unsigned char *sizify(unsigned char *data,
                         unsigned int width,unsigned int height,unsigned int depth,
                         float maxdev);

   unsigned char *classify(unsigned char *grad,
                           unsigned int width,unsigned int height,unsigned int depth,
                           float maxgrad,
                           unsigned int *classes=NULL);

   void zero(unsigned char *data,unsigned char *grad,
             unsigned int width,unsigned int height,unsigned int depth,
             float maxdev);

   void parsecommands(unsigned char *volume,
                      unsigned int width,unsigned int height,unsigned int depth,
                      char *commands);

   void parsegradcommands(unsigned char *volume,unsigned char *grad,
                          unsigned int width,unsigned int height,unsigned int depth,
                          char *commands);

   inline unsigned char getscalar(unsigned char *volume,
                                  unsigned int width,unsigned int height,unsigned int depth,
                                  float x,float y,float z);

   inline float getscalar(float *volume,
                          unsigned int width,unsigned int height,unsigned int depth,
                          float x,float y,float z);

   unsigned char *scale(unsigned char *volume,
                        unsigned int width,unsigned int height,unsigned int depth,
                        unsigned int nwidth,unsigned int nheight,unsigned int ndepth);
   };

#endif
