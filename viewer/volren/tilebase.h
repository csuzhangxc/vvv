// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef TILEBASE_H
#define TILEBASE_H

#include "codebase.h" // universal code base
#include "ddsbase.h" // volume file reader
#include "dicombase.h" // dicom file reader
#include "oglbase.h" // OpenGL base and window handling
#include "tfbase.h" // transfer functions

#define MAXSTR 256

#define PROGNUM 10

// a texture brick
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

// a tile of the volume
class tile
   {
   public:

   // default constructor
   tile(tfunc2D *tf,char *base=NULL);

   // destructor
   ~tile();

   // set stereo interlacing mode
   static void setSFXmode(int sfxmode);

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
               BOOLINT lighting=FALSE,
               BOOLINT depth=TRUE);

   // render a tile slice
   void renderslice(float ox,float oy,float oz,
                    float nx,float ny,float nz);

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

   inline void intersecttetra1(const float p1x,const float p1y,const float p1z,const float d1,
                               const float p2x,const float p2y,const float p2z,const float d2,
                               const float p3x,const float p3y,const float p3z,const float d3,
                               const float p4x,const float p4y,const float p4z,const float d4);

   inline void intersecttetra2(const float p1x,const float p1y,const float p1z,const float d1,
                               const float p2x,const float p2y,const float p2z,const float d2,
                               const float p3x,const float p3y,const float p3z,const float d3,
                               const float p4x,const float p4y,const float p4z,const float d4);

   inline void intersecttetra(const float p1x,const float p1y,const float p1z,
                              const float p2x,const float p2y,const float p2z,
                              const float p3x,const float p3y,const float p3z,
                              const float p4x,const float p4y,const float p4z,
                              const float ox,const float oy,const float oz,
                              const float nx,const float ny,const float nz);

   void intersecthexa(const float p1x,const float p1y,const float p1z,
                      const float p2x,const float p2y,const float p2z,
                      const float p3x,const float p3y,const float p3z,
                      const float p4x,const float p4y,const float p4z,
                      const float p5x,const float p5y,const float p5z,
                      const float p6x,const float p6y,const float p6z,
                      const float p7x,const float p7y,const float p7z,
                      const float p8x,const float p8y,const float p8z,
                      const float ox,const float oy,const float oz,
                      const float nx,const float ny,const float nz);

   private:

   // eye parameters:

   float EX,EY,EZ,
         DX,DY,DZ,
         UX,UY,UZ;

   float NEARP;

   // texture shader and fragment program setup
   void bindtexmap(int texid3D);
   void bindtexmaps(int texid3D,int texid2DE,int texid2DA,int sfxmode=0);
   void bindtexmaps1D(int texid3D,int texid1DE,int texid1DA,int sfxmode=0);
   void bindtexmaps2D(int texid3D,int texid3DG,int texid2DE,int texid2DA,int sfxmode=0);
   void bindtexmaps3D(int texid3D,int texid3DG,int texid3DE,int texid3DA,float rslab,int sfxmode=0);
   void setprogparSFX(int sfxmode=0);

   // fragment program loading:

   static BOOLINT LOADED;
   static unsigned int INSTANCES;
   static GLuint PROGID[PROGNUM];

   static void setup(char *base=NULL);
   static void destroy();

   // stereo interlacing mode:

   static int SFXMODE;
   };

typedef tile *tileptr;

#endif
