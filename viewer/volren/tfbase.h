// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef TFBASE_H
#define TFBASE_H

#include "codebase.h" // universal code base
#include "ddsbase.h" // volume file reader
#include "oglbase.h" // OpenGL base and window handling

// a transfer function:

class tfunc
   {
   public:

   // default constructor
   tfunc(int res);

   // destructor
   ~tfunc();

   // refresh the actual transfer function
   BOOLINT refresh1D(const float emission, // global emission
                     const float density, // global density
                     const float slab, // slab thickness
                     BOOLINT premult=FALSE, // pre-multiplied optical model on/off
                     BOOLINT RGBA=TRUE); // use RGBA or RGB mode

   // pre-integrate the actual transfer function
   BOOLINT refresh2D(const float emission, // global emission
                     const float density, // global density
                     const float slab, // slab thickness
                     BOOLINT premult=FALSE, // pre-multiplied optical model on/off
                     BOOLINT preint=TRUE, // pre-integration on/off
                     BOOLINT RGBA=TRUE); // use RGBA or RGB mode

   // check visibility via ZOT (Zero Opacity Test)
   BOOLINT zot(float mindata,float maxdata);

   // preintegrate transfer function (needed by ZOT)
   void preint(BOOLINT premult=FALSE);

   // check transparency via MOT (Minimum Opacity Test)
   BOOLINT mot(float mindata,float maxdata);

   // precompute minimum opacity (needed by MOT)
   void premin();

   // get resolution of transfer function
   int get_res() {return(RES);}

   // set scaling of emission/absorption
   void set_escale(float re,float ge,float be);
   void set_ascale(float ra,float ga,float ba);

   // get scaling of emission/absorption
   void get_escale(float *re,float *ge,float *be);
   void get_ascale(float *ra,float *ga,float *ba);

   // set importance of transfer function
   void set_imp(float imp);

   // get importance of transfer function
   float get_imp() {return(IMPORTANCE);}

   // set inverse mode
   void set_invmode(BOOLINT invmode=FALSE);

   // get one emission component of the transfer function
   float *get_re() {return(RE);}
   float *get_ge() {return(GE);}
   float *get_be() {return(BE);}

   // get one absorption component of the transfer function
   float *get_ra() {return(RA);}
   float *get_ga() {return(GA);}
   float *get_ba() {return(BA);}

   // set one emission component of the transfer function
   void set_re(float *re);
   void set_ge(float *ge);
   void set_be(float *be);

   // set one absorption component of the transfer function
   void set_ra(float *ra);
   void set_ga(float *ga);
   void set_ba(float *ba);

   // copy one component of the transfer function
   void copy_tf(float *src,float *tf);

   // set a line segment of the transfer function
   void set_line(float x1,float y1,float x2,float y2,float *tf);

   // 1D perlin noise
   float *pn_perlin(int size,
                    int start,float *carrier,
                    float persist);

   // hsv to rgb conversion
   static void hsv2rgb(float hue,float sat,float val,float *rgb);

   // rgb to hsv conversion
   static void rgb2hsv(float r,float g,float b,float *hsv);

   // randomize transfer function
   void randomize();

   // get minimum scalar value with non-zero opacity
   float get_nonzero_min();

   // get maximum scalar value with non-zero opacity
   float get_nonzero_max();

   unsigned char *get_pre_e() {return(EDATA);} // get pre-integrated emission table
   unsigned char *get_pre_a() {return(ADATA);} // get pre-integrated absorption table

   // check whether or not the absorption is equal for all channels
   BOOLINT checkRGBA();

   // get pre-multiplication of tables
   BOOLINT get_premult() {return(LAST_MLT);}

   // get dimension of tables (FALSE=1D/TRUE=2D)
   BOOLINT get_dim() {return(LAST_DIM);}

   // save2file
   void save(FILE *file);

   // load
   void load(FILE *file);

   protected:

   int RES; // resolution of the transfer function table

   float *RE,*GE,*BE; // rgb emission
   float *RA,*GA,*BA; // rgb absorption

   float RE_SCALE,GE_SCALE,BE_SCALE; // rgb scaling of emission
   float RA_SCALE,GA_SCALE,BA_SCALE; // rgb scaling of absorption

   float IMPORTANCE; // importance of transfer function

   BOOLINT INVMODE; // inverse mode flag

   unsigned char *EDATA,*ADATA; // pre-integration tables

   float LAST_EMS,LAST_DNS,LAST_SLB;
   BOOLINT LAST_MLT,LAST_INT,LAST_RGBA,LAST_DIM;

   BOOLINT CHANGED;

   private:

   float *PRE,*PGE,*PBE;
   float *PRA,*PGA,*PBA;

   float *PMIN;

   int PPSIZE;
   float *TABLE1,*TABLE2,*TABLE3;
   float LAST1,LAST2,LAST3;

   float prepow1(float x,float p);
   float prepow2(float x,float p);
   float prepow3(float x,float p);

   inline unsigned char quant(float x);

   void invert1D(BOOLINT RGBA);
   void invert2D(BOOLINT RGBA);

   float pn_interpolate(float *octave,int size,float c);
   inline float pn_interpolateC(float v0,float v1,float v2,float v3,float x);
   };

typedef tfunc *tfuncptr;

// a 2D transfer function:

class tfunc2D
   {
   public:

   // default constructor
   tfunc2D(int res);

   // destructor
   ~tfunc2D();

   // set number of transfer functions
   void set_num(int num);

   // get number of transfer functions
   int get_num() {return(NUM);}

   // set mode of 2D transfer function setup
   void set_mode(int mode);

   // get mode of 2D transfer function setup
   int get_mode() {return(MODE);}

   // get interpolation mode of 2D transfer function setup
   BOOLINT get_imode() {return(MODE<=7 || MODE>=10);}

   // set most important transfer function
   void set_imp(float which,float range=0.0f);

   // unset most important transfer function
   void unset_imp();

   // get importance of transfer function
   float get_imp(int num);

   // set inverse mode
   void set_invmode(BOOLINT invmode=FALSE);

   // get inverse mode
   BOOLINT get_invmode() {return(INVMODE);}

   // pre-integrate the transfer functions
   void refresh(const float emission=1000.0f, // global emission
                const float density=1000.0f, // global density
                const float slab=1.0f, // slab thickness
                BOOLINT premult=FALSE, // pre-multiplied optical model on/off
                BOOLINT preint=TRUE, // pre-integration on/off
                BOOLINT light=FALSE); // lighting on/off

   // check visibility via ZOT (Zero Opacity Test)
   BOOLINT zot(float mindata,float maxdata);

   // check visibility via ZOT (Zero Opacity Test)
   BOOLINT zot(float mindata,float maxdata,
               float minextra,float maxextra);

   // preintegrate transfer function (needed by ZOT)
   void preint(BOOLINT premult=FALSE);

   // check transparency via MOT (Minimum Opacity Test)
   BOOLINT mot(float mindata,float maxdata);

   // check transparency via MOT (Minimum Opacity Test)
   BOOLINT mot(float mindata,float maxdata,
               float minextra,float maxextra);

   // precompute minimum opacity (needed by MOT)
   void premin();

   // get resolution of transfer functions
   int get_res() {return(RES);}

   // set scaling of emission/absorption
   void set_escale(float re,float ge,float be);
   void set_ascale(float ra,float ga,float ba);

   // get scaling of emission/absorption
   void get_escale(float *re,float *ge,float *be);
   void get_ascale(float *ra,float *ga,float *ba);

   // get one emission component of the transfer function
   float *get_re() {return(MODE>=10?NULL:TF[0]->get_re());}
   float *get_ge() {return(MODE>=10?NULL:TF[0]->get_ge());}
   float *get_be() {return(MODE>=10?NULL:TF[0]->get_be());}

   // get one absorption component of the transfer function
   float *get_ra() {return(MODE==14?NULL:TF[0]->get_ra());}
   float *get_ga() {return(MODE==14?NULL:TF[0]->get_ga());}
   float *get_ba() {return(MODE==14?NULL:TF[0]->get_ba());}

   // copy one component of the transfer function
   void copy_tf(float *src,float *tf);

   // copy RGB components of the transfer function
   void copy_tfRGB(float *rgb,int res,int skip=0);

   // copy RGBA components of the transfer function
   void copy_tfRGBA(float *rgba,int res,int skip=0);

   // copy one component of the 2D transfer function
   void copy_2dtf(float *src,float *tf,float level);

   // copy RGB components of the 2D transfer function
   void copy_2dtfRGB(float *rgb,float level,int skip=0);

   // copy RGBA components of the 2D transfer function
   void copy_2dtfRGBA(float *rgba,float level,int skip=0);

   // copy RGB components of the 2D transfer function
   void copy_2DTFRGB(float *rgb,int res,int num,int skip=0);

   // copy RGBA components of the 2D transfer function
   void copy_2DTFRGBA(float *rgba,int res,int num,int skip=0);

   // return RGB components of the 2D transfer function
   void get_2DTFRGB(float *rgb,int num);

   // return RGBA components of the 2D transfer function
   void get_2DTFRGBA(float *rgba,int num);

   // set a line segment of the transfer function
   void set_line(float x1,float y1,float x2,float y2,float *tf);

   // randomize transfer function
   void randomize();

   // get minimum scalar value with non-zero opacity
   float get_nonzero_min();

   // get maximum scalar value with non-zero opacity
   float get_nonzero_max();

   int get_eid() {return(EID);} // get texture id of pre-integrated emission
   int get_aid() {return(AID);} // get texture id of pre-integrated absorption

   // check whether or not the absorption is equal for all channels
   BOOLINT checkRGBA();

   // get pre-multiplication of tables
   BOOLINT get_premult() {return(TF[0]->get_premult());}

   // get dimension of textures (FALSE=1D/TRUE=2D)
   BOOLINT get_dim() {return(TF[0]->get_dim());}

   // save2file
   void save(FILE *file);

   // load
   void load(FILE *file);

   protected:

   int RES; // resolution of transfer functions

   int NUM; // number of transfer functions
   tfuncptr *TF; // transfer function array

   float RE_SCALE,GE_SCALE,BE_SCALE; // rgb scaling of emission
   float RA_SCALE,GA_SCALE,BA_SCALE; // rgb scaling of absorption

   float WHICH,RANGE; // most important transfer function and its range
   BOOLINT IMPORTANT; // importance on/off

   BOOLINT INVMODE; // inverse mode flag

   int MODE; // transfer function setup mode

   int EID,AID; // texture ids of pre-integrated tables

   private:

   // update the transfer functions
   void update();

   // return id of 1D RGB texture map
   int buildtexmap1DRGB(unsigned char *table,int size);

   // return id of 1D RGBA texture map
   int buildtexmap1DRGBA(unsigned char *table,int size);

   // return id of 2D RGB texture map
   int buildtexmap2DRGB(unsigned char *image,int width,int height);

   // return id of 2D RGBA texture map
   int buildtexmap2DRGBA(unsigned char *image,int width,int height);

   // return id of 3D RGB texture map
   int buildtexmap3DRGB(unsigned char *volume,
                        int width,int height,int depth);

   // return id of 3D RGBA texture map
   int buildtexmap3DRGBA(unsigned char *volume,
                         int width,int height,int depth);

   // delete texture map
   void deletetexmap(int texid);
   };

// the histogram:

class histo
   {
   public:

   // default constructor
   histo();

   // destructor
   ~histo();

   // update histograms
   void set_histograms(unsigned char *data,
                       unsigned char *extra,
                       int width,int height,int depth,
                       int histmin,float histfreq,
                       int kneigh=1,float histstep=1.0f,
                       void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   // init 1D histogram
   void inithist(unsigned char *volume,
                 unsigned int width,unsigned int height,unsigned int depth,
                 int mincnt,float freq,
                 BOOLINT init=TRUE,
                 void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   // init 2D histogram
   void inithist2D(unsigned char *volume,unsigned char *grad,
                   unsigned int width,unsigned int height,unsigned int depth,
                   int mincnt,float freq,int kneigh,float step,
                   BOOLINT init=TRUE,
                   void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   // init 2D histogram using vector quantization
   void inithist2DQ(unsigned char *volume,unsigned char *grad,
                    unsigned int width,unsigned int height,unsigned int depth,
                    int mincnt,float freq,int kneigh,float step,
                    BOOLINT init=TRUE,
                    void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   // clear regions
   BOOLINT clear(BOOLINT full=FALSE);

   // select a region
   void click(float x,float y,float rad);
   void click(int s,int t);

   // render the histogram points
   void render2DQ(float mx,float my,float mz,
                  float sx,float sy,float sz);

   float *get_hist(); // return the histogram
   float *get_histRGBA(); // return the colored histogram
   float *get_hist2D(); // return the scatter plot
   float *get_hist2DRGBA(); // return the colored scatter plot
   float *get_hist2DQRGBA(); // return the quantized scatter plot
   float *get_hist2DTFRGBA(); // return the quantized transfer function

   // save2file
   void save(FILE *file);

   // load
   void load(FILE *file);

   static void hsv2rgb(float hue,float sat,float val,float *rgb);

   protected:

   BOOLINT MULTI;

   float centroid1D[3*256];

   float *centroid2D;
   float *variance2D;

   double HIST[256],
          *HIST2D;

   float HISTL[256],HISTRGBA[4*256],
         *HIST2DL,*HIST2DRGBA,
         *HIST2DQRGBA,*HIST2DTFRGBA;

   unsigned char *STATE;
   float *EMIT,*ABSORB;

   int MINCNT;
   float FREQ;

   private:

   BOOLINT CLICKED;
   int CLICKs,CLICKt;

   inline unsigned char getscalar(unsigned char *volume,
                                  unsigned int width,unsigned int height,unsigned int depth,
                                  float x,float y,float z);

   inline void getrgb(float x,float y,float z,
                      float freq,float val,
                      float *rgb);

   inline void getrgb(float x,float y,float z,float v,
                      float freq,float val,
                      float *rgb);

   void initcentroids(unsigned char *volume,
                      unsigned int width,unsigned int height,unsigned int depth,
                      void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   void initcentroids2D(unsigned char *volume,unsigned char *grad,
                        unsigned int width,unsigned int height,unsigned int depth,
                        int kneigh,float step,
                        void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL);

   int detect(const int s,const int t,const int v,
              const float r,const float g,const float b,
              int *mins,int *maxs,int *mint,int *maxt,
              BOOLINT flag=TRUE);

   void mark(const int s,const int t,const int v1,const int v2);

   void mark(const int s,const int t,const int v1,const int v2,
             const int mins,const int maxs,const int mint,const int maxt);

   void markall();

   void clear(const int s,const int t,const int v1,const int v2);
   };

#endif
