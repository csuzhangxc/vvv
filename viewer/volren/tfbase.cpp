// (c) by Stefan Roettger, licensed under GPL 2+

#include "tfbase.h"

// a transfer function:

tfunc::tfunc(int res)
   {
   if (res<2) ERRORMSG();

   RES=res;

   RE=new float[res];
   GE=new float[res];
   BE=new float[res];

   RA=new float[res];
   GA=new float[res];
   BA=new float[res];

   set_line(0.0f,0.0f,1.0f,0.0f,RE);
   set_line(0.0f,0.0f,1.0f,0.0f,GE);
   set_line(0.0f,0.0f,1.0f,0.0f,BE);

   set_line(0.0f,0.0f,1.0f,0.0f,RA);
   set_line(0.0f,0.0f,1.0f,0.0f,GA);
   set_line(0.0f,0.0f,1.0f,0.0f,BA);

   RE_SCALE=GE_SCALE=BE_SCALE=1.0f;
   RA_SCALE=GA_SCALE=BA_SCALE=1.0f;

   IMPORTANCE=1.0f;

   INVMODE=FALSE;

   PRE=new float[res];
   PGE=new float[res];
   PBE=new float[res];

   PRA=new float[res];
   PGA=new float[res];
   PBA=new float[res];

   PMIN=new float[res*res];

   PPSIZE=256;

   TABLE1=new float[PPSIZE];
   TABLE2=new float[PPSIZE];
   TABLE3=new float[PPSIZE];

   LAST1=LAST2=LAST3=-1.0f;

   EDATA=ADATA=NULL;

   LAST_EMS=0.0f;
   LAST_DNS=0.0f;
   LAST_SLB=0.0f;

   LAST_MLT=FALSE;
   LAST_INT=FALSE;
   LAST_RGBA=FALSE;
   LAST_DIM=FALSE;

   CHANGED=TRUE;
   }

tfunc::~tfunc()
   {
   delete RE;
   delete GE;
   delete BE;

   delete RA;
   delete GA;
   delete BA;

   delete PRE;
   delete PGE;
   delete PBE;

   delete PRA;
   delete PGA;
   delete PBA;

   delete PMIN;

   delete TABLE1;
   delete TABLE2;
   delete TABLE3;

   if (EDATA!=NULL) delete EDATA;
   if (ADATA!=NULL) delete ADATA;
   }

// check whether or not the absorption is equal for all channels
BOOLINT tfunc::checkRGBA()
   {
   const float tolerance=1.0E-6f;

   int c;

   if (fabs(RA_SCALE-GA_SCALE)>tolerance ||
       fabs(RA_SCALE-BA_SCALE)>tolerance) return(FALSE);

   for (c=0; c<RES; c++)
      if (fabs(RA[c]-GA[c])>tolerance ||
          fabs(RA[c]-BA[c])>tolerance) return(FALSE);

   return(TRUE);
   }

// pre-computed power functions:

float tfunc::prepow1(float x,float p)
   {
   int i;

   if (x!=LAST1)
      {
      for (i=0; i<PPSIZE; i++) TABLE1[i]=fpow(x,(float)i/(PPSIZE-1));
      LAST1=x;
      }

   return(TABLE1[ftrc(p*(PPSIZE-1)+0.5f)]);
   }

float tfunc::prepow2(float x,float p)
   {
   int i;

   if (x!=LAST2)
      {
      for (i=0; i<PPSIZE; i++) TABLE2[i]=fpow(x,(float)i/(PPSIZE-1));
      LAST2=x;
      }

   return(TABLE2[ftrc(p*(PPSIZE-1)+0.5f)]);
   }

float tfunc::prepow3(float x,float p)
   {
   int i;

   if (x!=LAST3)
      {
      for (i=0; i<PPSIZE; i++) TABLE3[i]=fpow(x,(float)i/(PPSIZE-1));
      LAST3=x;
      }

   return(TABLE3[ftrc(p*(PPSIZE-1)+0.5f)]);
   }

// quantize float to 8 bit
unsigned char tfunc::quant(float x)
   {return((x<1.0f)?ftrc(255.0f*x+0.5f):255);}

// invert the actual transfer function
void tfunc::invert1D(BOOLINT RGBA)
   {
   int c;

   unsigned char *ptr;
   int R,G,B;

   if (!INVMODE) return;

   for (ptr=EDATA,c=0; c<RES; c++)
      {
      R=ptr[0];
      G=ptr[1];
      B=ptr[2];

      *ptr++=max(max(G-R,B-R),0);
      *ptr++=max(max(R-G,B-G),0);
      *ptr++=max(max(R-B,G-B),0);

      if (RGBA) ptr++;
      }
   }

// invert the actual transfer function
void tfunc::invert2D(BOOLINT RGBA)
   {
   int c,r;

   unsigned char *ptr;
   int R,G,B;

   if (!INVMODE) return;

   for (ptr=EDATA,r=0; r<RES; r++)
      for (c=0; c<RES; c++)
         {
         R=ptr[0];
         G=ptr[1];
         B=ptr[2];

         *ptr++=max(max(G-R,B-R),0);
         *ptr++=max(max(R-G,B-G),0);
         *ptr++=max(max(R-B,G-B),0);

         if (RGBA) ptr++;
         }
   }

// refresh the actual transfer function
BOOLINT tfunc::refresh1D(const float emission,
                         const float density,
                         const float slab,
                         BOOLINT premult,
                         BOOLINT RGBA)
   {
   int c;

   unsigned char *ptr;

   float scale_re,scale_ge,scale_be;
   float exp_ra,exp_ga,exp_ba;

   if (emission!=LAST_EMS || density!=LAST_DNS || slab!=LAST_SLB ||
       premult!=LAST_MLT || RGBA!=LAST_RGBA || LAST_DIM)
      {
      LAST_EMS=emission;
      LAST_DNS=density;
      LAST_SLB=slab;

      LAST_MLT=premult;
      LAST_RGBA=RGBA;
      LAST_DIM=FALSE;

      CHANGED=TRUE;
      }

   if (!CHANGED) return(FALSE);

   // delete old transfer function tables
   if (EDATA!=NULL) delete EDATA;
   if (ADATA!=NULL) delete ADATA;

   // pre-integrate emission for RGB channels:

   scale_re=emission*slab*RE_SCALE*IMPORTANCE;
   scale_ge=emission*slab*GE_SCALE*IMPORTANCE;
   scale_be=emission*slab*BE_SCALE*IMPORTANCE;

   PRE[0]=RE[0];
   PGE[0]=GE[0];
   PBE[0]=BE[0];

   for (c=1; c<RES; c++)
      if (premult)
         {
         PRE[c]=PRE[c-1]+RE[c]*RA[c];
         PGE[c]=PGE[c-1]+GE[c]*GA[c];
         PBE[c]=PBE[c-1]+BE[c]*BA[c];
         }
      else
         {
         PRE[c]=PRE[c-1]+RE[c];
         PGE[c]=PGE[c-1]+GE[c];
         PBE[c]=PBE[c-1]+BE[c];
         }

   if (RGBA)
      {
      // pre-integrate absorption for a single channel:

      exp_ra=fexp(-density*slab*RA_SCALE*IMPORTANCE);

      PRA[0]=PGA[0]=PBA[0]=RA[0];

      for (c=1; c<RES; c++) PRA[c]=PGA[c]=PBA[c]=PRA[c-1]+RA[c];

      // create new RGBA emission/absorption table
      EDATA=new unsigned char[4*RES];
      ADATA=NULL;

      // calculate emission/absorption
      for (ptr=EDATA,c=0; c<RES; c++)
         if (premult)
            {
            *ptr++=quant(scale_re*RE[c]*RA[c]);
            *ptr++=quant(scale_ge*GE[c]*GA[c]);
            *ptr++=quant(scale_be*BE[c]*BA[c]);
            *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
            }
         else
            {
            *ptr++=quant(scale_re*RE[c]);
            *ptr++=quant(scale_ge*GE[c]);
            *ptr++=quant(scale_be*BE[c]);
            *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
            }
      }
   else
      {
      // create new RGB emission table
      EDATA=new unsigned char[3*RES];

      // calculate emission
      for (ptr=EDATA,c=0; c<RES; c++)
         if (premult)
            {
            *ptr++=quant(scale_re*RE[c]*RA[c]);
            *ptr++=quant(scale_ge*GE[c]*GA[c]);
            *ptr++=quant(scale_be*BE[c]*BA[c]);
            }
         else
            {
            *ptr++=quant(scale_re*RE[c]);
            *ptr++=quant(scale_ge*GE[c]);
            *ptr++=quant(scale_be*BE[c]);
            }

      // pre-integrate absorption for RGB channels:

      exp_ra=fexp(-density*slab*RA_SCALE*IMPORTANCE);
      exp_ga=fexp(-density*slab*GA_SCALE*IMPORTANCE);
      exp_ba=fexp(-density*slab*BA_SCALE*IMPORTANCE);

      PRA[0]=RA[0];
      PGA[0]=GA[0];
      PBA[0]=BA[0];

      for (c=1; c<RES; c++)
         {
         PRA[c]=PRA[c-1]+RA[c];
         PGA[c]=PGA[c-1]+GA[c];
         PBA[c]=PBA[c-1]+BA[c];
         }

      // create new RGB absorption table
      ADATA=new unsigned char[3*RES];

      // calculate absorption
      for (ptr=ADATA,c=0; c<RES; c++)
         {
         *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
         *ptr++=quant(1.0f-prepow2(exp_ga,GA[c]));
         *ptr++=quant(1.0f-prepow3(exp_ba,BA[c]));
         }
      }

   invert1D(RGBA);

   CHANGED=FALSE;

   return(TRUE);
   }

// pre-integrate the actual transfer function
BOOLINT tfunc::refresh2D(const float emission,
                         const float density,
                         const float slab,
                         BOOLINT premult,
                         BOOLINT preint,
                         BOOLINT RGBA)
   {
   int c,r;

   unsigned char *ptr;

   float scale_re,scale_ge,scale_be;
   float exp_ra,exp_ga,exp_ba;

   if (emission!=LAST_EMS || density!=LAST_DNS || slab!=LAST_SLB ||
       premult!=LAST_MLT || preint!=LAST_INT || RGBA!=LAST_RGBA || !LAST_DIM)
      {
      LAST_EMS=emission;
      LAST_DNS=density;
      LAST_SLB=slab;

      LAST_MLT=premult;
      LAST_INT=preint;
      LAST_RGBA=RGBA;
      LAST_DIM=TRUE;

      CHANGED=TRUE;
      }

   if (!CHANGED) return(FALSE);

   // delete old pre-integration tables
   if (EDATA!=NULL) delete EDATA;
   if (ADATA!=NULL) delete ADATA;

   // pre-integrate emission for RGB channels:

   scale_re=emission*slab*RE_SCALE*IMPORTANCE;
   scale_ge=emission*slab*GE_SCALE*IMPORTANCE;
   scale_be=emission*slab*BE_SCALE*IMPORTANCE;

   PRE[0]=RE[0];
   PGE[0]=GE[0];
   PBE[0]=BE[0];

   for (c=1; c<RES; c++)
      if (premult)
         {
         PRE[c]=PRE[c-1]+RE[c]*RA[c];
         PGE[c]=PGE[c-1]+GE[c]*GA[c];
         PBE[c]=PBE[c-1]+BE[c]*BA[c];
         }
      else
         {
         PRE[c]=PRE[c-1]+RE[c];
         PGE[c]=PGE[c-1]+GE[c];
         PBE[c]=PBE[c-1]+BE[c];
         }

   if (RGBA)
      {
      // pre-integrate absorption for a single channel:

      exp_ra=fexp(-density*slab*RA_SCALE*IMPORTANCE);

      PRA[0]=PGA[0]=PBA[0]=RA[0];

      for (c=1; c<RES; c++) PRA[c]=PGA[c]=PBA[c]=PRA[c-1]+RA[c];

      // create new RGBA emission/absorption table
      EDATA=new unsigned char[4*RES*RES];
      ADATA=NULL;

      // calculate emission/absorption
      for (ptr=EDATA,r=0; r<RES; r++)
         for (c=0; c<RES; c++)
            if (c==r || !preint)
               if (premult)
                  {
                  *ptr++=quant(scale_re*RE[c]*RA[c]);
                  *ptr++=quant(scale_ge*GE[c]*GA[c]);
                  *ptr++=quant(scale_be*BE[c]*BA[c]);
                  *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
                  }
               else
                  {
                  *ptr++=quant(scale_re*RE[c]);
                  *ptr++=quant(scale_ge*GE[c]);
                  *ptr++=quant(scale_be*BE[c]);
                  *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
                  }
            else
               {
               *ptr++=quant(scale_re*fabs((PRE[c]-PRE[r])/(c-r)));
               *ptr++=quant(scale_ge*fabs((PGE[c]-PGE[r])/(c-r)));
               *ptr++=quant(scale_be*fabs((PBE[c]-PBE[r])/(c-r)));
               *ptr++=quant(1.0f-prepow1(exp_ra,fabs((PRA[c]-PRA[r])/(c-r))));
               }
      }
   else
      {
      // create new RGB emission table
      EDATA=new unsigned char[3*RES*RES];

      // calculate emission
      for (ptr=EDATA,r=0; r<RES; r++)
         for (c=0; c<RES; c++)
            if (c==r || !preint)
               if (premult)
                  {
                  *ptr++=quant(scale_re*RE[c]*RA[c]);
                  *ptr++=quant(scale_ge*GE[c]*GA[c]);
                  *ptr++=quant(scale_be*BE[c]*BA[c]);
                  }
               else
                  {
                  *ptr++=quant(scale_re*RE[c]);
                  *ptr++=quant(scale_ge*GE[c]);
                  *ptr++=quant(scale_be*BE[c]);
                  }
            else
               {
               *ptr++=quant(scale_re*fabs((PRE[c]-PRE[r])/(c-r)));
               *ptr++=quant(scale_ge*fabs((PGE[c]-PGE[r])/(c-r)));
               *ptr++=quant(scale_be*fabs((PBE[c]-PBE[r])/(c-r)));
               }

      // pre-integrate absorption for RGB channels:

      exp_ra=fexp(-density*slab*RA_SCALE*IMPORTANCE);
      exp_ga=fexp(-density*slab*GA_SCALE*IMPORTANCE);
      exp_ba=fexp(-density*slab*BA_SCALE*IMPORTANCE);

      PRA[0]=RA[0];
      PGA[0]=GA[0];
      PBA[0]=BA[0];

      for (c=1; c<RES; c++)
         {
         PRA[c]=PRA[c-1]+RA[c];
         PGA[c]=PGA[c-1]+GA[c];
         PBA[c]=PBA[c-1]+BA[c];
         }

      // create new RGB absorption table
      ADATA=new unsigned char[3*RES*RES];

      // calculate absorption
      for (ptr=ADATA,r=0; r<RES; r++)
         for (c=0; c<RES; c++)
            if (c==r || !preint)
               {
               *ptr++=quant(1.0f-prepow1(exp_ra,RA[c]));
               *ptr++=quant(1.0f-prepow2(exp_ga,GA[c]));
               *ptr++=quant(1.0f-prepow3(exp_ba,BA[c]));
               }
            else
               {
               *ptr++=quant(1.0f-prepow1(exp_ra,fabs((PRA[c]-PRA[r])/(c-r))));
               *ptr++=quant(1.0f-prepow2(exp_ga,fabs((PGA[c]-PGA[r])/(c-r))));
               *ptr++=quant(1.0f-prepow3(exp_ba,fabs((PBA[c]-PBA[r])/(c-r))));
               }
      }

   invert2D(RGBA);

   CHANGED=FALSE;

   return(TRUE);
   }

// check visibility via ZOT (Zero Opacity Test)
BOOLINT tfunc::zot(float mindata,float maxdata)
   {
   const float tolerance=1.0E-3f;

   int minpos=ftrc(ffloor((RES-1)*mindata));
   int maxpos=ftrc(fceil((RES-1)*maxdata));

   if (minpos==maxpos)
      if (LAST_MLT)
         return(RE[minpos]*RA[minpos]<tolerance &&
                GE[minpos]*GA[minpos]<tolerance &&
                BE[minpos]*BA[minpos]<tolerance &&
                RA[minpos]<tolerance &&
                GA[minpos]<tolerance &&
                BA[minpos]<tolerance);
      else
         return(RE[minpos]<tolerance &&
                GE[minpos]<tolerance &&
                BE[minpos]<tolerance &&
                RA[minpos]<tolerance &&
                GA[minpos]<tolerance &&
                BA[minpos]<tolerance);

   return(PRE[maxpos]-PRE[minpos]<tolerance &&
          PGE[maxpos]-PGE[minpos]<tolerance &&
          PBE[maxpos]-PBE[minpos]<tolerance &&
          PRA[maxpos]-PRA[minpos]<tolerance &&
          PGA[maxpos]-PGA[minpos]<tolerance &&
          PBA[maxpos]-PBA[minpos]<tolerance);
   }

// preintegrate transfer function
void tfunc::preint(BOOLINT premult)
   {
   int c;

   PRE[0]=RE[0];
   PGE[0]=GE[0];
   PBE[0]=BE[0];

   PRA[0]=RA[0];
   PGA[0]=GA[0];
   PBA[0]=BA[0];

   for (c=1; c<RES; c++)
      {
      if (premult)
         {
         PRE[c]=PRE[c-1]+RE[c]*RA[c];
         PGE[c]=PGE[c-1]+GE[c]*GA[c];
         PBE[c]=PBE[c-1]+BE[c]*BA[c];
         }
      else
         {
         PRE[c]=PRE[c-1]+RE[c];
         PGE[c]=PGE[c-1]+GE[c];
         PBE[c]=PBE[c-1]+BE[c];
         }

      PRA[c]=PRA[c-1]+RA[c];
      PGA[c]=PGA[c-1]+GA[c];
      PBA[c]=PBA[c-1]+BA[c];
      }

   LAST_MLT=premult;
   }

// check transparency via MOT (Minimum Opacity Test)
BOOLINT tfunc::mot(float mindata,float maxdata)
   {
   const float tolerance=1.0E-3f;

   int minpos=ftrc(ffloor((RES-1)*mindata));
   int maxpos=ftrc(fceil((RES-1)*maxdata));

   return(PMIN[minpos+maxpos*RES]<tolerance);
   }

// precompute minimum opacity
void tfunc::premin()
   {
   int c,r,s;

   float val;

   for (r=0; r<RES; r++)
      for (c=0; c<=r; c++)
         {
         val=1.0f;

         for (s=c; s<=r; s++) val=fmin(val,fmax(RA[s],fmax(GA[s],BA[s])));

         PMIN[c+r*RES]=val;
         }

   for (r=0; r<RES; r++)
      for (c=r+1; c<RES; c++)
         PMIN[c+r*RES]=PMIN[r+c*RES];
   }

// set scaling of emission
void tfunc::set_escale(float re,float ge,float be)
   {
   const float tolerance=1.0E-6f;

   if (fabs(re-RE_SCALE)>tolerance ||
       fabs(ge-GE_SCALE)>tolerance ||
       fabs(be-BE_SCALE)>tolerance) CHANGED=TRUE;

   RE_SCALE=re;
   GE_SCALE=ge;
   BE_SCALE=be;
   }

// set scaling of absorption
void tfunc::set_ascale(float ra,float ga,float ba)
   {
   const float tolerance=1.0E-6f;

   if (fabs(ra-RA_SCALE)>tolerance ||
       fabs(ga-GA_SCALE)>tolerance ||
       fabs(ba-BA_SCALE)>tolerance) CHANGED=TRUE;

   RA_SCALE=ra;
   GA_SCALE=ga;
   BA_SCALE=ba;
   }

// get scaling of emission
void tfunc::get_escale(float *re,float *ge,float *be)
   {
   *re=RE_SCALE;
   *ge=GE_SCALE;
   *be=BE_SCALE;
   }

// get scaling of absorption
void tfunc::get_ascale(float *ra,float *ga,float *ba)
   {
   *ra=RA_SCALE;
   *ga=GA_SCALE;
   *ba=BA_SCALE;
   }

// set importance of transfer function
void tfunc::set_imp(float imp)
   {
   const float tolerance=1.0E-6f;

   if (fabs(imp-IMPORTANCE)>tolerance) CHANGED=TRUE;

   IMPORTANCE=imp;
   }

// set inverse mode
void tfunc::set_invmode(BOOLINT invmode)
   {
   if (INVMODE!=invmode)
      {
      INVMODE=invmode;
      CHANGED=TRUE;
      }
   }

// set red emission component of the transfer function
void tfunc::set_re(float *re)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(re[i]-RE[i])>tolerance)
         {
         CHANGED=TRUE;
         RE[i]=re[i];
         }
   }

// set green emission component of the transfer function
void tfunc::set_ge(float *ge)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(ge[i]-GE[i])>tolerance)
         {
         CHANGED=TRUE;
         GE[i]=ge[i];
         }
   }

// set blue emission component of the transfer function
void tfunc::set_be(float *be)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(be[i]-BE[i])>tolerance)
         {
         CHANGED=TRUE;
         BE[i]=be[i];
         }
   }

// set red absorption component of the transfer function
void tfunc::set_ra(float *ra)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(ra[i]-RA[i])>tolerance)
         {
         CHANGED=TRUE;
         RA[i]=ra[i];
         }
   }

// set green absorption component of the transfer function
void tfunc::set_ga(float *ga)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(ga[i]-GA[i])>tolerance)
         {
         CHANGED=TRUE;
         GA[i]=ga[i];
         }
   }

// set blue absorption component of the transfer function
void tfunc::set_ba(float *ba)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(ba[i]-BA[i])>tolerance)
         {
         CHANGED=TRUE;
         BA[i]=ba[i];
         }
   }

// copy one component of the transfer function
void tfunc::copy_tf(float *src,float *tf)
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (fabs(src[i]-tf[i])>tolerance)
         {
         CHANGED=TRUE;
         tf[i]=src[i];
         }
   }

// set a line segment of the transfer function
void tfunc::set_line(float x1,float y1,float x2,float y2,float *tf)
   {
   const float tolerance=1.0E-6f;

   int i;

   int c1,c2;

   float y;

   c1=ftrc((RES-1)*x1+0.5f);
   c2=ftrc((RES-1)*x2+0.5f);

   y=y1;

   if (c1<=c2)
      {
      for (i=c1; i<=c2; i++)
         if (i>=0 && i<=RES-1)
            {
            if (fabs(y-tf[i])>tolerance) CHANGED=TRUE;

            tf[i]=y;
            y+=(y2-y1)/(c2-c1+1);
            }
      }
   else
      {
      for (i=c1; i>=c2; i--)
         if (i>=0 && i<=RES-1)
            {
            if (fabs(y-tf[i])>tolerance) CHANGED=TRUE;

            tf[i]=y;
            y+=(y2-y1)/(c1-c2+1);
            }
      }
   }

// cubic interpolation
inline float tfunc::pn_interpolateC(float v0,float v1,float v2,float v3,float x)
   {
   float p,q,r;

   p=v3-v2+v1-v0;
   q=v0-v1-p;
   r=v2-v0;

   return(((p*x+q)*x+r)*x+v1);
   }

// cubic perlin noise interpolation
float tfunc::pn_interpolate(float *octave,int size,float c)
   {
   int k;
   float w;

   if (c<=0.0f) return(octave[0]);
   if (c>=1.0f) return(octave[size-1]);

   k=ftrc(c*(size-1));
   w=c*(size-1)-k;

   return(pn_interpolateC((k>0)?octave[k-1]:octave[k],
                          octave[k],octave[k+1],
                          (k<size-2)?octave[k+2]:octave[k+1],w));
   }

// 1D perlin noise
float *tfunc::pn_perlin(int size,
                        int start,float *carrier,
                        float persist)
   {
   int i,j;

   float scaling,maxr;

   float *noise,*octave;

   if ((size&(size-1))!=0 || size<2) ERRORMSG();
   if ((start&(start-1))!=0 || start<2 || start>size) ERRORMSG();

   if ((noise=(float *)malloc(size*sizeof(float)))==NULL) ERRORMSG();
   for (i=0; i<size; i++) noise[i]=0.0f;

   if ((octave=(float *)malloc(size*sizeof(float)))==NULL) ERRORMSG();

   scaling=1.0f;
   for (i=start; i<=size; i*=2)
      {
      for (j=0; j<i; j++)
         if (i==start && carrier!=NULL) octave[j]=carrier[j];
         else octave[j]=GETRANDOM()*scaling;

      for (j=0; j<size; j++)
         noise[j]+=pn_interpolate(octave,i,(float)j/(size-1));

      scaling*=persist;
      }

   free(octave);

   maxr=0.0f;
   for (i=0; i<size; i++)
      if (noise[i]>maxr) maxr=noise[i];

   if (maxr>0.0f)
      for (i=0; i<size; i++) noise[i]=fmax(noise[i]/maxr,0.0f);

   return(noise);
   }

// hsv to rgb conversion
void tfunc::hsv2rgb(float hue,float sat,float val,float *rgb)
   {
   float hue6,r,s,t;

   if (hue<0.0f || sat<0.0f || sat>1.0f || val<0.0f || val>1.0f) ERRORMSG();

   hue/=60.0f;
   hue=hue-6.0f*ftrc(hue/6.0f);
   hue6=hue-ftrc(hue);

   r=val*(1.0f-sat);
   s=val*(1.0f-sat*hue6);
   t=val*(1.0f-sat*(1.0f-hue6));

   switch (ftrc(hue))
        {
        case 0: // red -> yellow
           rgb[0] = val;
           rgb[1] = t;
           rgb[2] = r;
           break;
        case 1: // yellow -> green
           rgb[0] = s;
           rgb[1] = val;
           rgb[2] = r;
           break;
        case 2: // green -> cyan
           rgb[0] = r;
           rgb[1] = val;
           rgb[2] = t;
           break;
        case 3: // cyan -> blue
           rgb[0] = r;
           rgb[1] = s;
           rgb[2] = val;
           break;
        case 4: // blue -> magenta
           rgb[0] = t;
           rgb[1] = r;
           rgb[2] = val;
           break;
        case 5: // magenta -> red
           rgb[0] = val;
           rgb[1] = r;
           rgb[2] = s;
           break;
        }
   }

// rgb to hsv conversion
void tfunc::rgb2hsv(float r,float g,float b,float *hsv)
   {
   float h,s,v;
   float maxv,minv,diff,rdist,gdist,bdist;

   maxv=fmax(r,fmax(g,b));
   minv=fmin(r,fmin(g,b));
   diff=maxv-minv;

   v=maxv;

   if (maxv!=0.0f) s=diff/maxv;
   else s=0.0f;

   if (s==0.0f) h=0.0f;
   else
      {
      rdist=(maxv-r)/diff;
      gdist=(maxv-g)/diff;
      bdist=(maxv-b)/diff;

      if (r==maxv) h=bdist-gdist;
      else if (g==maxv) h=2.0f+rdist-bdist;
      else h=4.0f+gdist-rdist;

      h*=60.0f;
      if (h<0.0f) h+=360.0f;
      }

   hsv[0]=h;
   hsv[1]=s;
   hsv[2]=v;
   }

// randomize transfer function
void tfunc::randomize()
   {
   int i;

   float *hue,*sat,*val;

   float rgb[3]={0};

   hue=pn_perlin(RES,16,NULL,0.5f);
   sat=pn_perlin(RES,16,NULL,0.5f);
   val=pn_perlin(RES,16,NULL,0.5f);

   for (i=0; i<RES; i++)
      {
      hsv2rgb(360.0f*hue[i],1.0f,val[i],rgb);

      RE[i]=rgb[0];
      GE[i]=rgb[1];
      BE[i]=rgb[2];
      }

   CHANGED=TRUE;

   free(hue);
   free(sat);
   free(val);
   }

// get minimum scalar value with non-zero opacity
float tfunc::get_nonzero_min()
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=0; i<RES; i++)
      if (RA[i]>tolerance || GA[i]>tolerance || BA[i]>tolerance) break;

   if (i>0) i--;

   return((float)i/(RES-1));
   }

// get maximum scalar value with non-zero opacity
float tfunc::get_nonzero_max()
   {
   const float tolerance=1.0E-6f;

   int i;

   for (i=RES-1; i>=0; i--)
      if (RA[i]>tolerance || GA[i]>tolerance || BA[i]>tolerance) break;

   if (i<RES-1) i++;

   return((float)i/(RES-1));
   }

// save2file
void tfunc::save(FILE *file)
   {
   int i;

   fprintf(file,"TF:\n");

   fprintf(file,"res=%d\n",RES);

   fprintf(file,"rescale=%g\n",RE_SCALE);
   fprintf(file,"gescale=%g\n",GE_SCALE);
   fprintf(file,"bescale=%g\n",BE_SCALE);
   fprintf(file,"rascale=%g\n",RA_SCALE);
   fprintf(file,"gascale=%g\n",GA_SCALE);
   fprintf(file,"bascale=%g\n",BA_SCALE);

   for (i=0; i<RES; i++)
      {
      fprintf(file,"re=%g\n",RE[i]);
      fprintf(file,"ge=%g\n",GE[i]);
      fprintf(file,"be=%g\n",BE[i]);
      fprintf(file,"ra=%g\n",RA[i]);
      fprintf(file,"ga=%g\n",GA[i]);
      fprintf(file,"ba=%g\n",BA[i]);
      }
   }

// load
void tfunc::load(FILE *file)
   {
   int i;

   int res;

   if (fscanf(file,"TF:\n")!=0) return;

   fscanf(file,"res=%d\n",&res);
   if (res!=RES) ERRORMSG();

   fscanf(file,"rescale=%g\n",&RE_SCALE);
   fscanf(file,"gescale=%g\n",&GE_SCALE);
   fscanf(file,"bescale=%g\n",&BE_SCALE);
   fscanf(file,"rascale=%g\n",&RA_SCALE);
   fscanf(file,"gascale=%g\n",&GA_SCALE);
   fscanf(file,"bascale=%g\n",&BA_SCALE);

   for (i=0; i<RES; i++)
      {
      fscanf(file,"re=%g\n",&RE[i]);
      fscanf(file,"ge=%g\n",&GE[i]);
      fscanf(file,"be=%g\n",&BE[i]);
      fscanf(file,"ra=%g\n",&RA[i]);
      fscanf(file,"ga=%g\n",&GA[i]);
      fscanf(file,"ba=%g\n",&BA[i]);
      }

   IMPORTANCE=1.0f;

   CHANGED=TRUE;
   }

// hsv to rgb
void tfunc::hsv2rgb(float hue,float sat,float val,float *rgb)
   {
   float hue6,r,s,t;

   if (hue<0.0f || sat<0.0f || sat>1.0f || val<0.0f || val>1.0f) ERRORMSG();

   hue/=60.0f;
   hue=hue-6.0f*ftrc(hue/6.0f);
   hue6=hue-ftrc(hue);

   r=val*(1.0f-sat);
   s=val*(1.0f-sat*hue6);
   t=val*(1.0f-sat*(1.0f-hue6));

   switch (ftrc(hue))
        {
        case 0: // red -> yellow
           rgb[0] = val;
           rgb[1] = t;
           rgb[2] = r;
           break;
        case 1: // yellow -> green
           rgb[0] = s;
           rgb[1] = val;
           rgb[2] = r;
           break;
        case 2: // green -> cyan
           rgb[0] = r;
           rgb[1] = val;
           rgb[2] = t;
           break;
        case 3: // cyan -> blue
           rgb[0] = r;
           rgb[1] = s;
           rgb[2] = val;
           break;
        case 4: // blue -> magenta
           rgb[0] = t;
           rgb[1] = r;
           rgb[2] = val;
           break;
        case 5: // magenta -> red
           rgb[0] = val;
           rgb[1] = r;
           rgb[2] = s;
           break;
        }
   }

// a 2D transfer function:

tfunc2D::tfunc2D(int res)
   {
   RES=res;

   NUM=1;

   TF=new tfuncptr[1];
   TF[0]=new tfunc(RES);

   RE_SCALE=GE_SCALE=BE_SCALE=1.0f;
   RA_SCALE=GA_SCALE=BA_SCALE=1.0f;

   WHICH=RANGE=0.0f;
   IMPORTANT=FALSE;

   INVMODE=FALSE;

   MODE=0;

   EID=AID=0;
   }

tfunc2D::~tfunc2D()
   {
   int i;

   for (i=0; i<NUM; i++) delete TF[i];
   delete TF;

   deletetexmap(EID);
   deletetexmap(AID);
   }

// set number of transfer functions
void tfunc2D::set_num(int num)
   {
   int i;

   tfuncptr *tf;

   if (num==NUM) return;

   for (i=num; i<NUM; i++) delete TF[i];

   tf=new tfuncptr[num];

   for (i=0; i<num; i++)
      if (i<NUM) tf[i]=TF[i];
      else tf[i]=new tfunc(RES);

   NUM=num;
   delete TF;
   TF=tf;

   IMPORTANT=FALSE;

   update();

   deletetexmap(EID);
   deletetexmap(AID);

   EID=AID=0;
   }

// set mode of 2D transfer function setup:
// mode==0: TF[0] is reduplicated (same as a 1D transfer function)
// mode==1: absorption/emission is attenuated linearily (aka. gradient magnitude)
// mode==2: absorption/emission is attenuated quadratically
// mode==3: absorption/emission is attenuated with square root
// mode==4: pseudo random hue shift of TF[0]
// mode==5: pseudo random hue shift of TF[0] with gradient magnitude
// mode==6: pseudo random hue shift of TF[0] with quadratic gradient magnitude
// mode==7: pseudo random hue shift of TF[0] with square root gradient magnitude
// mode==8: pseudo random hue shift of TF[0] for material based rendering with border
// mode==9: pseudo random hue shift of TF[0] for material based rendering
// mode==10: full 2D emission function with gradient magnitude
// mode==11: full 2D emission function with quadratic gradient magnitude
// mode==12: full 2D emission function with square root gradient magnitude
// mode==13: full 2D emission function without gradient magnitude
// mode==14: full 2D transfer function
void tfunc2D::set_mode(int mode)
   {
   if (mode<0 || mode>14) ERRORMSG();

   if (mode==MODE) return;

   MODE=mode;
   IMPORTANT=FALSE;
   update();
   }

// check whether or not the absorption is equal for all channels
BOOLINT tfunc2D::checkRGBA()
   {
   int i;

   for (i=0; i<NUM; i++)
      if (!TF[i]->checkRGBA()) return(FALSE);

   return(TRUE);
   }

// refresh transfer functions
void tfunc2D::refresh(const float emission,
                      const float density,
                      const float slab,
                      BOOLINT premult,
                      BOOLINT preint,
                      BOOLINT light)
   {
   int i;

   unsigned char *data;

   BOOLINT useRGBA;
   BOOLINT changed=FALSE;

   useRGBA=checkRGBA();
   if (EID==0) changed=TRUE;

   if (NUM==1)
      if (!preint)
         {
         // refresh the transfer function
         if (TF[0]->refresh1D(emission,density,slab,premult,useRGBA)) changed=TRUE;

         if (!changed) return;

         // delete old textures
         deletetexmap(EID);
         deletetexmap(AID);

         if (useRGBA)
            {
            // generate new 1D RGBA emission/absorption texture
            EID=buildtexmap1DRGBA(TF[0]->get_pre_e(),RES);

            // separate absorption texture is unused
            AID=0;
            }
         else
            {
            // generate new 1D RGB emission texture
            EID=buildtexmap1DRGB(TF[0]->get_pre_e(),RES);

            // generate new 1D RGB absorption texture
            AID=buildtexmap1DRGB(TF[0]->get_pre_a(),RES);
            }
         }
      else
         {
         // refresh the transfer function
         if (TF[0]->refresh2D(emission,density,slab,premult,preint,useRGBA)) changed=TRUE;

         if (!changed) return;

         // delete old textures
         deletetexmap(EID);
         deletetexmap(AID);

         if (useRGBA)
            {
            // generate new 2D RGBA emission/absorption texture
            EID=buildtexmap2DRGBA(TF[0]->get_pre_e(),RES,RES);

            // separate absorption texture is unused
            AID=0;
            }
         else
            {
            // generate new 2D RGB emission texture
            EID=buildtexmap2DRGB(TF[0]->get_pre_e(),RES,RES);

            // generate new 2D RGB absorption texture
            AID=buildtexmap2DRGB(TF[0]->get_pre_a(),RES,RES);
            }
         }
   else
      if (!preint && !light)
         {
         // refresh all transfer functions
         for (i=0; i<NUM; i++)
            if (TF[i]->refresh1D(emission,density,slab,premult,useRGBA)) changed=TRUE;

         if (!changed) return;

         // delete old textures
         deletetexmap(EID);
         deletetexmap(AID);

         if (useRGBA)
            {
            data=new unsigned char[4*RES*NUM];

            // memcopy transfer functions
            for (i=0; i<NUM; i++)
               memcpy(&data[4*RES*i],TF[i]->get_pre_e(),4*RES);

            // generate new 2D RGBA emission/absorption texture
            EID=buildtexmap2DRGBA(data,RES,NUM);

            // separate absorption texture is unused
            AID=0;

            delete data;
            }
         else
            {
            data=new unsigned char[3*RES*NUM];

            // memcopy transfer functions
            for (i=0; i<NUM; i++)
               memcpy(&data[3*RES*i],TF[i]->get_pre_e(),3*RES);

            // generate new 2D RGB emission texture
            EID=buildtexmap2DRGB(data,RES,NUM);

            // memcopy transfer functions
            for (i=0; i<NUM; i++)
               memcpy(&data[3*RES*i],TF[i]->get_pre_a(),3*RES);

            // generate new 2D RGB absorption texture
            AID=buildtexmap2DRGB(data,RES,NUM);

            delete data;
            }
         }
      else
         {
         // refresh all transfer functions
         for (i=0; i<NUM; i++)
            if (TF[i]->refresh2D(emission,density,slab,premult,preint,useRGBA)) changed=TRUE;

         if (!changed) return;

         // delete old textures
         deletetexmap(EID);
         deletetexmap(AID);

         if (useRGBA)
            {
            data=new unsigned char[4*RES*RES*NUM];

            // memcopy pre-integrated slices
            for (i=0; i<NUM; i++)
               memcpy(&data[4*RES*RES*i],TF[i]->get_pre_e(),4*RES*RES);

            // generate new 3D RGBA emission/absorption texture
            EID=buildtexmap3DRGBA(data,RES,RES,NUM);

            // separate absorption texture is unused
            AID=0;

            delete data;
            }
         else
            {
            data=new unsigned char[3*RES*RES*NUM];

            // memcopy pre-integrated slices
            for (i=0; i<NUM; i++)
               memcpy(&data[3*RES*RES*i],TF[i]->get_pre_e(),3*RES*RES);

            // generate new 3D RGB emission texture
            EID=buildtexmap3DRGB(data,RES,RES,NUM);

            // memcopy pre-integrated slices
            for (i=0; i<NUM; i++)
               memcpy(&data[3*RES*RES*i],TF[i]->get_pre_a(),3*RES*RES);

            // generate new 3D RGB absorption texture
            AID=buildtexmap3DRGB(data,RES,RES,NUM);

            delete data;
            }
         }
   }

// check visibility via ZOT (Zero Opacity Test)
BOOLINT tfunc2D::zot(float mindata,float maxdata)
   {
   int i;

   if (MODE==0) return(TF[0]->zot(mindata,maxdata));
   else if (MODE>=1 && MODE<=9) return(TF[NUM-1]->zot(mindata,maxdata));

   for (i=0; i<NUM; i++)
      if (!TF[i]->zot(mindata,maxdata)) return(FALSE);

   return(TRUE);
   }

// check visibility via ZOT (Zero Opacity Test)
BOOLINT tfunc2D::zot(float mindata,float maxdata,
                     float minextra,float maxextra)
   {
   int i;

   int minpos=ftrc(ffloor((NUM-1)*minextra));
   int maxpos=ftrc(fceil((NUM-1)*maxextra));

   if (MODE==0) return(TF[0]->zot(mindata,maxdata));
   else if (MODE>=1 && MODE<=9) return(TF[maxpos]->zot(mindata,maxdata));

   for (i=minpos; i<=maxpos; i++)
      if (!TF[i]->zot(mindata,maxdata)) return(FALSE);

   return(TRUE);
   }

// preintegrate transfer function
void tfunc2D::preint(BOOLINT premult)
   {
   int i;

   if (MODE==0) TF[0]->preint(premult);
   else if (MODE>=1 && MODE<=9) TF[NUM-1]->preint(premult);
   else for (i=0; i<NUM; i++) TF[i]->preint(premult);
   }

// check transparency via MOT (Minimum Opacity Test)
BOOLINT tfunc2D::mot(float mindata,float maxdata)
   {
   int i;

   if (MODE==0) return(TF[0]->mot(mindata,maxdata));
   else if (MODE>=1 && MODE<=9) return(TF[NUM-1]->mot(mindata,maxdata));

   for (i=0; i<NUM; i++)
      if (!TF[i]->mot(mindata,maxdata)) return(FALSE);

   return(TRUE);
   }

// check visibility via MOT (Minimum Opacity Test)
BOOLINT tfunc2D::mot(float mindata,float maxdata,
                     float minextra,float maxextra)
   {
   int i;

   int minpos=ftrc(ffloor((NUM-1)*minextra));
   int maxpos=ftrc(fceil((NUM-1)*maxextra));

   if (MODE==0) return(TF[0]->mot(mindata,maxdata));
   else if (MODE>=1 && MODE<=9) return(TF[maxpos]->mot(mindata,maxdata));

   for (i=minpos; i<=maxpos; i++)
      if (!TF[i]->mot(mindata,maxdata)) return(FALSE);

   return(TRUE);
   }

// precompute minimum opacity
void tfunc2D::premin()
   {
   int i;

   if (MODE==0) TF[0]->premin();
   else if (MODE>=1 && MODE<=9) TF[NUM-1]->premin();
   else for (i=0; i<NUM; i++) TF[i]->premin();
   }

// set scaling of emission
void tfunc2D::set_escale(float re,float ge,float be)
   {
   int i;

   float att;

   RE_SCALE=re;
   GE_SCALE=ge;
   BE_SCALE=be;

   if ((MODE==1 || MODE==5 || MODE==10) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=(float)i/(NUM-1);
         TF[i]->set_escale(att*re,att*ge,att*be);
         }
   else if ((MODE==2 || MODE==6 || MODE==11) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=fsqr((float)i/(NUM-1));
         TF[i]->set_escale(att*re,att*ge,att*be);
         }
   else if ((MODE==3 || MODE==7 || MODE==12) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=fsqrt((float)i/(NUM-1));
         TF[i]->set_escale(att*re,att*ge,att*be);
         }
   else if (MODE==8 && NUM>1)
      for (i=0; i<NUM; i++)
         {
         if (i==0) att=0.0f;
         else att=1.0f;
         TF[i]->set_escale(att*re,att*ge,att*be);
         }
   else
      for (i=0; i<NUM; i++) TF[i]->set_escale(re,ge,be);
   }

// get scaling of emission
void tfunc2D::get_escale(float *re,float *ge,float *be)
   {
   *re=RE_SCALE;
   *ge=GE_SCALE;
   *be=BE_SCALE;
   }

// get scaling of absorption
void tfunc2D::get_ascale(float *ra,float *ga,float *ba)
   {
   *ra=RA_SCALE;
   *ga=GA_SCALE;
   *ba=BA_SCALE;
   }

// set scaling of absorption
void tfunc2D::set_ascale(float ra,float ga,float ba)
   {
   int i;

   float att;

   RA_SCALE=ra;
   GA_SCALE=ga;
   BA_SCALE=ba;

   if ((MODE==1 || MODE==5 || MODE==10) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=(float)i/(NUM-1);
         TF[i]->set_ascale(att*ra,att*ga,att*ba);
         }
   else if ((MODE==2 || MODE==6 || MODE==11) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=fsqr((float)i/(NUM-1));
         TF[i]->set_ascale(att*ra,att*ga,att*ba);
         }
   else if ((MODE==3 || MODE==7 || MODE==12) && NUM>1)
      for (i=0; i<NUM; i++)
         {
         att=fsqrt((float)i/(NUM-1));
         TF[i]->set_ascale(att*ra,att*ga,att*ba);
         }
   else if (MODE==8 && NUM>1)
      for (i=0; i<NUM; i++)
         {
         if (i==0) att=0.0f;
         else att=1.0f;
         TF[i]->set_ascale(att*ra,att*ga,att*ba);
         }
   else
      for (i=0; i<NUM; i++) TF[i]->set_ascale(ra,ga,ba);
   }

// set most important transfer function
void tfunc2D::set_imp(float which,float range)
   {
   IMPORTANT=TRUE;
   WHICH=which;
   RANGE=range;
   update();
   }

// unset most important transfer function
void tfunc2D::unset_imp()
   {
   IMPORTANT=FALSE;
   update();
   }

// get importance of transfer function
float tfunc2D::get_imp(int num)
   {
   if (num<0 || num>=NUM) ERRORMSG();
   return(TF[num]->get_imp());
   }

// set inverse mode
void tfunc2D::set_invmode(BOOLINT invmode)
   {
   int i;

   INVMODE=invmode;

   for (i=0; i<NUM; i++) TF[i]->set_invmode(invmode);
   }

// copy one component of the transfer function
void tfunc2D::copy_tf(float *src,float *tf)
   {
   TF[0]->copy_tf(src,tf);
   update();
   }

// copy RGB components of the transfer function
void tfunc2D::copy_tfRGB(float *rgb,int res,int skip)
   {
   int i;

   float *tf;

   if (res!=RES || MODE>=10) return;

   tf=new float[RES];

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i];
   TF[0]->copy_tf(tf,get_re());

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i+1];
   TF[0]->copy_tf(tf,get_ge());

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i+2];
   TF[0]->copy_tf(tf,get_be());

   delete tf;

   update();
   }

// copy RGBA components of the transfer function
void tfunc2D::copy_tfRGBA(float *rgba,int res,int skip)
   {
   int i;

   float *tf;

   if (res!=RES || MODE>=10) return;

   tf=new float[RES];

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i];
   TF[0]->copy_tf(tf,get_re());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+1];
   TF[0]->copy_tf(tf,get_ge());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+2];
   TF[0]->copy_tf(tf,get_be());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+3];
   TF[0]->copy_tf(tf,get_ra());
   TF[0]->copy_tf(tf,get_ga());
   TF[0]->copy_tf(tf,get_ba());

   delete tf;

   update();
   }

// copy one component of the 2D transfer function
void tfunc2D::copy_2dtf(float *src,float *tf,float level)
   {
   int num=ftrc(level*(NUM-1)+0.5f);

   TF[num]->copy_tf(src,tf);
   update();
   }

// copy RGB components of the 2D transfer function
void tfunc2D::copy_2dtfRGB(float *rgb,float level,int skip)
   {
   int i;

   float *tf;

   int num=ftrc(level*(NUM-1)+0.5f);

   if (MODE<10) return;

   tf=new float[RES];

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i];
   TF[num]->copy_tf(tf,TF[num]->get_re());

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i+1];
   TF[num]->copy_tf(tf,TF[num]->get_ge());

   for (i=0; i<RES; i++) tf[i]=rgb[(3+skip)*i+2];
   TF[num]->copy_tf(tf,TF[num]->get_be());

   delete tf;

   update();
   }

// copy RGBA components of the 2D transfer function
void tfunc2D::copy_2dtfRGBA(float *rgba,float level,int skip)
   {
   int i;

   float *tf;

   int num=ftrc(level*(NUM-1)+0.5f);

   if (MODE!=14) return;

   tf=new float[RES];

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i];
   TF[num]->copy_tf(tf,TF[num]->get_re());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+1];
   TF[num]->copy_tf(tf,TF[num]->get_ge());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+2];
   TF[num]->copy_tf(tf,TF[num]->get_be());

   for (i=0; i<RES; i++) tf[i]=rgba[(4+skip)*i+3];
   TF[num]->copy_tf(tf,TF[num]->get_ra());
   TF[num]->copy_tf(tf,TF[num]->get_ga());
   TF[num]->copy_tf(tf,TF[num]->get_ba());

   delete tf;

   update();
   }

// copy RGB components of the 2D transfer function
void tfunc2D::copy_2DTFRGB(float *rgb,int res,int num,int skip)
   {
   int i,j;

   float *TF2D;

   if (res!=RES || MODE<10) return;

   TF2D=new float[3*res*num];

   for (i=0; i<res*num; i++)
      {
      TF2D[3*i]=rgb[(3+skip)*i];
      TF2D[3*i+1]=rgb[(3+skip)*i+1];
      TF2D[3*i+2]=rgb[(3+skip)*i+2];
      }

   while (num>NUM)
      {
      for (j=0; j<num/2; j++)
         for (i=0; i<res; i++)
            {
            TF2D[3*(i+j*res)]=(TF2D[3*(i+2*j*res)]+TF2D[3*(i+(2*j+1)*res)])/2.0f;
            TF2D[3*(i+j*res)+1]=(TF2D[3*(i+2*j*res)+1]+TF2D[3*(i+(2*j+1)*res)+1])/2.0f;
            TF2D[3*(i+j*res)+2]=(TF2D[3*(i+2*j*res)+2]+TF2D[3*(i+(2*j+1)*res)+2])/2.0f;
            }

      num/=2;
      }

   for (i=0; i<num; i++)
      copy_2dtfRGB(TF2D+3*i*res,(float)i/(num-1));

   delete TF2D;

   update();
   }

// copy RGBA components of the 2D transfer function
void tfunc2D::copy_2DTFRGBA(float *rgba,int res,int num,int skip)
   {
   int i,j;

   float *TF2D;

   if (res!=RES || MODE!=14) return;

   TF2D=new float[4*res*num];

   for (i=0; i<res*num; i++)
      {
      TF2D[4*i]=rgba[(4+skip)*i];
      TF2D[4*i+1]=rgba[(4+skip)*i+1];
      TF2D[4*i+2]=rgba[(4+skip)*i+2];
      TF2D[4*i+3]=rgba[(4+skip)*i+3];
      }

   while (num>NUM)
      {
      for (j=0; j<num/2; j++)
         for (i=0; i<res; i++)
            {
            TF2D[4*(i+j*res)]=(TF2D[4*(i+2*j*res)]+TF2D[4*(i+(2*j+1)*res)])/2.0f;
            TF2D[4*(i+j*res)+1]=(TF2D[4*(i+2*j*res)+1]+TF2D[4*(i+(2*j+1)*res)+1])/2.0f;
            TF2D[4*(i+j*res)+2]=(TF2D[4*(i+2*j*res)+2]+TF2D[4*(i+(2*j+1)*res)+2])/2.0f;
            TF2D[4*(i+j*res)+3]=(TF2D[4*(i+2*j*res)+3]+TF2D[4*(i+(2*j+1)*res)+3])/2.0f;
            }

      num/=2;
      }

   for (i=0; i<num; i++)
      copy_2dtfRGBA(TF2D+4*i*RES,(float)i/(num-1),skip);

   delete TF2D;

   update();
   }

// return RGB components of the 2D transfer function
void tfunc2D::get_2DTFRGB(float *rgb,int num)
   {
   int i,j;

   float *tf;

   for (i=0; i<num; i++)
      {
      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_re();
      for (j=0; j<RES; j++) rgb[3*(j+i*RES)]=tf[j];

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_ge();
      for (j=0; j<RES; j++) rgb[3*(j+i*RES)+1]=tf[j];

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_be();
      for (j=0; j<RES; j++) rgb[3*(j+i*RES)+2]=tf[j];
      }
   }

// return RGBA components of the 2D transfer function
void tfunc2D::get_2DTFRGBA(float *rgba,int num)
   {
   int i,j;

   float *tf;

   for (i=0; i<num; i++)
      {
      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_re();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)]=tf[j];

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_ge();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)+1]=tf[j];

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_be();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)+2]=tf[j];

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_ra();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)+3]=tf[j]/3.0f;

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_ga();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)+3]+=tf[j]/3.0f;

      tf=TF[ftrc((float)i/(num-1)*(NUM-1)+0.5f)]->get_ba();
      for (j=0; j<RES; j++) rgba[4*(j+i*RES)+3]+=tf[j]/3.0f;
      }
   }

// set a line segment of the transfer function
void tfunc2D::set_line(float x1,float y1,float x2,float y2,float *tf)
   {
   TF[0]->set_line(x1,y1,x2,y2,tf);
   update();
   }

// randomize transfer function
void tfunc2D::randomize()
   {
   if (MODE>=10) return;

   TF[0]->randomize();
   update();
   }

// get minimum scalar value with non-zero opacity
float tfunc2D::get_nonzero_min()
   {
   int i;

   float nzmin;

   nzmin=1.0f;

   for (i=0; i<NUM; i++)
      nzmin=fmin(nzmin,TF[i]->get_nonzero_min());

   return(nzmin);
   }

// get maximum scalar value with non-zero opacity
float tfunc2D::get_nonzero_max()
   {
   int i;

   float nzmax;

   nzmax=0.0f;

   for (i=0; i<NUM; i++)
      nzmax=fmax(nzmax,TF[i]->get_nonzero_max());

   return(nzmax);
   }

// save2file
void tfunc2D::save(FILE *file)
   {
   int i;

   fprintf(file,"2DTF:\n");

   fprintf(file,"num=%d\n",NUM);
   fprintf(file,"mode=%d\n",MODE);

   fprintf(file,"rescale=%g\n",RE_SCALE);
   fprintf(file,"gescale=%g\n",GE_SCALE);
   fprintf(file,"bescale=%g\n",BE_SCALE);
   fprintf(file,"rascale=%g\n",RA_SCALE);
   fprintf(file,"gascale=%g\n",GA_SCALE);
   fprintf(file,"bascale=%g\n",BA_SCALE);

   for (i=0; i<NUM; i++) TF[i]->save(file);
   }

// load
void tfunc2D::load(FILE *file)
   {
   int i,num;

   if (fscanf(file,"2DTF:\n")!=0) return;

   fscanf(file,"num=%d\n",&num);
   set_num(num);

   fscanf(file,"mode=%d\n",&MODE);

   fscanf(file,"rescale=%g\n",&RE_SCALE);
   fscanf(file,"gescale=%g\n",&GE_SCALE);
   fscanf(file,"bescale=%g\n",&BE_SCALE);
   fscanf(file,"rascale=%g\n",&RA_SCALE);
   fscanf(file,"gascale=%g\n",&GA_SCALE);
   fscanf(file,"bascale=%g\n",&BA_SCALE);

   for (i=0; i<NUM; i++) TF[i]->load(file);

   IMPORTANT=FALSE;

   update();
   }

// update the transfer functions
void tfunc2D::update()
   {
   int i,j;

   set_escale(RE_SCALE,GE_SCALE,BE_SCALE);
   set_ascale(RA_SCALE,GA_SCALE,BA_SCALE);

   if (NUM==1 || !IMPORTANT)
      for (i=0; i<NUM; i++) TF[i]->set_imp(1.0f);
   else
      for (i=0; i<NUM; i++)
         TF[i]->set_imp(0.5f+fcos(PI*fmin(fabs((float)i/(NUM-1)-WHICH)/
                                          (RANGE==0.0f?1.0f/NUM:RANGE),1.0f))/2.0f);

   if (MODE>=4 && MODE<=9)
      {
      float rgb[3],hsv[3];

      float *new_re=new float[RES];
      float *new_ge=new float[RES];
      float *new_be=new float[RES];

      for (i=1; i<NUM; i++)
         {
         for (j=0; j<RES; j++)
            {
            tfunc::rgb2hsv(get_re()[j],get_ge()[j],get_be()[j],hsv);
            tfunc::hsv2rgb(hsv[0]+(float)i/(NUM-1)*240.0f,hsv[1],hsv[2],rgb);

            new_re[j]=rgb[0];
            new_ge[j]=rgb[1];
            new_be[j]=rgb[2];
            }

         TF[i]->set_re(new_re);
         TF[i]->set_ge(new_ge);
         TF[i]->set_be(new_be);

         TF[i]->set_ra(get_ra());
         TF[i]->set_ga(get_ga());
         TF[i]->set_ba(get_ba());
         }

      delete new_re;
      delete new_ge;
      delete new_be;
      }
   else
      for (i=0; i<NUM; i++)
         {
         if (MODE<10)
            {
            TF[i]->set_re(get_re());
            TF[i]->set_ge(get_ge());
            TF[i]->set_be(get_be());
            }

         if (MODE!=14)
            {
            TF[i]->set_ra(get_ra());
            TF[i]->set_ga(get_ga());
            TF[i]->set_ba(get_ba());
            }
         }
   }

// return id of 1D RGB texture map
int tfunc2D::buildtexmap1DRGB(unsigned char *table,int size)
   {
   GLuint texid;

   unsigned char *table2;

   if (size<2) ERRORMSG();

   if ((table2=(unsigned char *)malloc(6*size))==NULL) ERRORMSG();

   memcpy(table2,table,3*size);
   memcpy(table2+3*size,table,3*size);

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,size,2,0,
                GL_RGB,GL_UNSIGNED_BYTE,table2);

   glBindTexture(GL_TEXTURE_2D,0);

   free(table2);

   return(texid);
   }

// return id of 1D RGBA texture map
int tfunc2D::buildtexmap1DRGBA(unsigned char *table,int size)
   {
   GLuint texid;

   unsigned char *table2;

   if (size<2) ERRORMSG();

   if ((table2=(unsigned char *)malloc(8*size))==NULL) ERRORMSG();

   memcpy(table2,table,4*size);
   memcpy(table2+4*size,table,4*size);

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,size,2,0,
                GL_RGBA,GL_UNSIGNED_BYTE,table2);

   glBindTexture(GL_TEXTURE_2D,0);

   free(table2);

   return(texid);
   }

// return id of 2D RGB texture map
int tfunc2D::buildtexmap2DRGB(unsigned char *image,
                              int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,
                GL_RGB,GL_UNSIGNED_BYTE,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

// return id of 2D RGBA texture map
int tfunc2D::buildtexmap2DRGBA(unsigned char *image,
                               int width,int height)
   {
   GLuint texid;

   if (width<2 || height<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,
                GL_RGBA,GL_UNSIGNED_BYTE,image);

   glBindTexture(GL_TEXTURE_2D,0);

   return(texid);
   }

// return id of 3D RGB texture map
int tfunc2D::buildtexmap3DRGB(unsigned char *volume,
                              int width,int height,int depth)
   {
   GLuint texid;

   if (width<2 || height<2 || depth<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_3D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#ifndef WINOS
   glTexImage3D(GL_TEXTURE_3D,0,GL_RGB,width,height,depth,0,
                GL_RGB,GL_UNSIGNED_BYTE,volume);
#else
   PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT=(PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT");
   glTexImage3DEXT(GL_TEXTURE_3D,0,GL_RGB,width,height,depth,0,
                   GL_RGB,GL_UNSIGNED_BYTE,volume);
#endif

   glBindTexture(GL_TEXTURE_3D,0);

   return(texid);
   }

// return id of 3D RGBA texture map
int tfunc2D::buildtexmap3DRGBA(unsigned char *volume,
                               int width,int height,int depth)
   {
   GLuint texid;

   if (width<2 || height<2 || depth<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_3D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#ifndef WINOS
   glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,width,height,depth,0,
                GL_RGBA,GL_UNSIGNED_BYTE,volume);
#else
   PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT=(PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT");
   glTexImage3DEXT(GL_TEXTURE_3D,0,GL_RGBA,width,height,depth,0,
                   GL_RGBA,GL_UNSIGNED_BYTE,volume);
#endif

   glBindTexture(GL_TEXTURE_3D,0);

   return(texid);
   }

// delete texture map
void tfunc2D::deletetexmap(int texid)
   {
   GLuint GLtexid=texid;
   if (texid>0) glDeleteTextures(1,&GLtexid);
   }

// the histogram:

histo::histo()
   {
   MULTI=FALSE;

   HIST2D=new double[256*256];
   HIST2DL=new float[256*256];
   HIST2DRGBA=new float[4*256*256];
   HIST2DQRGBA=new float[4*256*256];
   HIST2DTFRGBA=new float[4*256*256];

   centroid2D=new float[3*256*256];
   variance2D=new float[256*256];

   STATE=new unsigned char[256*256];
   EMIT=new float[256*256];
   ABSORB=new float[256*256];

   MINCNT=0;
   FREQ=0.0f;

   CLICKED=FALSE;
   CLICKs=CLICKt=0;
   }

histo::~histo()
   {
   delete HIST2D;
   delete HIST2DL;
   delete HIST2DRGBA;
   delete HIST2DQRGBA;
   delete HIST2DTFRGBA;

   delete centroid2D;
   delete variance2D;

   delete STATE;
   delete EMIT;
   delete ABSORB;
   }

// update histograms
void histo::set_histograms(unsigned char *data,
                           unsigned char *extra,
                           int width,int height,int depth,
                           int histmin,float histfreq,
                           int kneigh,float histstep,
                           void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   inithist(data,width,height,depth,histmin,histfreq,TRUE,feedback,obj);
   inithist2DQ(data,extra,width,height,depth,histmin,histfreq,kneigh,histstep,TRUE,feedback,obj);
   }

// get interpolated scalar value from volume
unsigned char histo::getscalar(unsigned char *volume,
                               unsigned int width,unsigned int height,unsigned int depth,
                               float x,float y,float z)
   {
   int i,j,k;

   unsigned char *ptr1,*ptr2;

   x*=width-1;
   y*=height-1;
   z*=depth-1;

   i=ftrc(x);
   j=ftrc(y);
   k=ftrc(z);

   x-=i;
   y-=j;
   z-=k;

   if (i<0)
      {
      i=0;
      x=0.0f;
      }

   if (j<0)
      {
      j=0;
      y=0.0f;
      }

   if (k<0)
      {
      k=0;
      z=0.0f;
      }

   if (i>=(int)width-1)
      {
      i=width-2;
      x=1.0f;
      }

   if (j>=(int)height-1)
      {
      j=height-2;
      y=1.0f;
      }

   if (k>=(int)depth-1)
      {
      k=depth-2;
      z=1.0f;
      }

   ptr1=&volume[(unsigned int)i+((unsigned int)j+(unsigned int)k*height)*width];
   ptr2=ptr1+width*height;

   return(ftrc((1.0f-z)*((1.0f-y)*((1.0f-x)*ptr1[0]+x*ptr1[1])+
                         y*((1.0f-x)*ptr1[width]+x*ptr1[width+1]))+
               z*((1.0f-y)*((1.0f-x)*ptr2[0]+x*ptr2[1])+
                  y*((1.0f-x)*ptr2[width]+x*ptr2[width+1]))+0.5f));
   }

// get rgb color from barycenter
void histo::getrgb(float x,float y,float z,
                   float freq,float val,
                   float *rgb)
   {getrgb(x,y,z,0.0f,freq,val,rgb);}

// get rgb color from barycenter and variance
void histo::getrgb(float x,float y,float z,float v,
                   float freq,float val,
                   float *rgb)
   {
   float hsv[3];

   rgb[0]=0.5f+fcos(freq*PI*x)/2.0f;
   rgb[1]=0.5f+fcos(freq*PI*y)/2.0f;
   rgb[2]=0.5f+fcos(freq*PI*z)/2.0f;

   tfunc::rgb2hsv(rgb[0],rgb[1],rgb[2],hsv);
   tfunc::hsv2rgb(hsv[0]+360.0f*freq*v,fsqrt(hsv[1])*(x==0.0f && y==0.0f && z==0.0f?0.0f:1.0f),val,rgb);
   }

// compute the centroids
void histo::initcentroids(unsigned char *volume,
                          unsigned int width,unsigned int height,unsigned int depth,
                          void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int i,j,k,p;

   unsigned char *ptr;

   double hmax,havg;

   float alpha;

   for (i=0; i<256; i++) HIST[i]=0.0;

   for (i=0; i<3*256; i++) centroid1D[i]=0.0f;

   for (ptr=volume,k=0; k<(int)depth; k++)
      {
      if (feedback!=NULL) feedback("calculating histogram",(float)(k+1)/depth,obj);

      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr++)
            {
            p=*ptr;
            HIST[p]++;

            centroid1D[3*p]+=(float)i/(width-1)-0.5f;
            centroid1D[3*p+1]+=(float)j/(height-1)-0.5f;
            centroid1D[3*p+2]+=(float)k/(depth-1)-0.5f;
            }
      }

   hmax=2.0;
   havg=0.0;

   for (i=0; i<256; i++)
      {
      if (HIST[i]>hmax) hmax=HIST[i];
      havg+=HIST[i];
      }

   havg/=256.0;
   hmax=fmin(hmax,10.0*havg);
   if (hmax==0.0) hmax++;

   for (i=0; i<256; i++)
      {
      if (HIST[i]>1.0)
         {
         centroid1D[3*i]/=HIST[i];
         centroid1D[3*i+1]/=HIST[i];
         centroid1D[3*i+2]/=HIST[i];
         }

      alpha=fpow(HIST[i]/hmax,1.0f/3);
      if (alpha>1.0f) alpha=1.0f;

      HISTL[i]=fsqrt(alpha);
      }
   }

// compute the histogram
void histo::inithist(unsigned char *volume,
                     unsigned int width,unsigned int height,unsigned int depth,
                     int mincnt,float freq,
                     BOOLINT init,
                     void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int i;

   float cx,cy,cz;

   float rgb[3];

   if (mincnt<1) ERRORMSG();

   if (init) initcentroids(volume,width,height,depth,feedback,obj);

   for (i=0; i<256; i++)
      {
      if (HIST[i]>=mincnt)
         {
         cx=centroid1D[3*i];
         cy=centroid1D[3*i+1];
         cz=centroid1D[3*i+2];
         }
      else
         cx=cy=cz=0.0f;

      getrgb(cx,cy,cz,freq,1.0f,rgb);

      HISTRGBA[4*i]=rgb[0];
      HISTRGBA[4*i+1]=rgb[1];
      HISTRGBA[4*i+2]=rgb[2];
      HISTRGBA[4*i+3]=HISTL[i];
      }
   }

// compute the centroids
void histo::initcentroids2D(unsigned char *volume,unsigned char *grad,
                            unsigned int width,unsigned int height,unsigned int depth,
                            int kneigh,float step,
                            void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int m,n;
   int s,g,p;

   double hmax,havg;

   float alpha;

   if (kneigh<0 || step<=0.0f) ERRORMSG();

   if (grad==NULL)
      {
      MULTI=FALSE;
      return;
      }

   clear();

   for (n=0; n<256*256; n++) HIST2D[n]=0.0;

   for (n=0; n<3*256*256; n++) centroid2D[n]=0.0f;

   if (step==1.0f)
      {
      int i,j,k;

      unsigned char *ptr1,*ptr2;

      for (ptr1=volume,ptr2=grad,k=0; k<(int)depth; k++)
         {
         if (feedback!=NULL) feedback("calculating 2D histogram",0.5f*(k+1)/depth,obj);

         for (j=0; j<(int)height; j++)
            for (i=0; i<(int)width; i++)
               {
               s=*ptr1++;
               g=*ptr2++;

               for (n=g-kneigh; n<=g+kneigh; n++)
                  for (m=s-kneigh; m<=s+kneigh; m++)
                     if (m>=0 && m<256 && n>=0 && n<256)
                        {
                        p=m+n*256;
                        HIST2D[p]++;

                        centroid2D[3*p]+=(float)i/(width-1)-0.5f;
                        centroid2D[3*p+1]+=(float)j/(height-1)-0.5f;
                        centroid2D[3*p+2]+=(float)k/(depth-1)-0.5f;
                        }
               }
         }
      }
   else
      {
      float i,j,k;

      for (k=0.0f; k<=1.0f; k+=step/(depth-1))
         {
         if (feedback!=NULL) feedback("calculating 2D histogram",0.5f*k,obj);

         for (j=0.0f; j<=1.0f; j+=step/(height-1))
            for (i=0.0f; i<=1.0f; i+=step/(width-1))
               {
               s=getscalar(volume,width,height,depth,i,j,k);
               g=getscalar(grad,width,height,depth,i,j,k);

               for (n=g-kneigh; n<=g+kneigh; n++)
                  for (m=s-kneigh; m<=s+kneigh; m++)
                     if (m>=0 && m<256 && n>=0 && n<256)
                        {
                        p=m+n*256;
                        HIST2D[p]++;

                        centroid2D[3*p]+=i-0.5f;
                        centroid2D[3*p+1]+=j-0.5f;
                        centroid2D[3*p+2]+=k-0.5f;
                        }
               }
         }
      }

   hmax=2.0;
   havg=0.0;

   for (n=0; n<256*256; n++)
      {
      if (HIST2D[n]>hmax) hmax=HIST2D[n];
      havg+=HIST2D[n];
      }

   havg/=256.0*256.0;
   hmax=fmin(hmax,10.0*havg);
   if (hmax==0.0) hmax++;

   for (n=0; n<256*256; n++)
      {
      if (HIST2D[n]>1.0)
         {
         centroid2D[3*n]/=HIST2D[n];
         centroid2D[3*n+1]/=HIST2D[n];
         centroid2D[3*n+2]/=HIST2D[n];
         }

      alpha=fpow(HIST2D[n]/hmax,1.0f/3);
      if (alpha>1.0f) alpha=1.0f;

      HIST2DL[n]=fsqrt(alpha);
      }

   for (n=0; n<256*256; n++) variance2D[n]=0.0f;

   if (step==1.0f)
      {
      int i,j,k;

      unsigned char *ptr1,*ptr2;

      for (ptr1=volume,ptr2=grad,k=0; k<(int)depth; k++)
         {
         if (feedback!=NULL) feedback("calculating 2D histogram",0.5f*(k+1)/depth+0.5f,obj);

         for (j=0; j<(int)height; j++)
            for (i=0; i<(int)width; i++)
               {
               s=*ptr1++;
               g=*ptr2++;

               for (n=g-kneigh; n<=g+kneigh; n++)
                  for (m=s-kneigh; m<=s+kneigh; m++)
                     if (m>=0 && m<256 && n>=0 && n<256)
                        {
                        p=m+n*256;
                        variance2D[p]+=fsqr((float)i/(width-1)-0.5f-centroid2D[3*p])+
                                       fsqr((float)j/(height-1)-0.5f-centroid2D[3*p+1])+
                                       fsqr((float)k/(depth-1)-0.5f-centroid2D[3*p+2]);
                        }
               }
         }
      }
   else
      {
      float i,j,k;

      for (k=0.0f; k<=1.0f; k+=step/(depth-1))
         {
         if (feedback!=NULL) feedback("calculating 2D histogram",0.5f*k+0.5f,obj);

         for (j=0.0f; j<=1.0f; j+=step/(height-1))
            for (i=0.0f; i<=1.0f; i+=step/(width-1))
               {
               s=getscalar(volume,width,height,depth,i,j,k);
               g=getscalar(grad,width,height,depth,i,j,k);

               for (n=g-kneigh; n<=g+kneigh; n++)
                  for (m=s-kneigh; m<=s+kneigh; m++)
                     if (m>=0 && m<256 && n>=0 && n<256)
                        {
                        p=m+n*256;
                        variance2D[p]+=fsqr(i-0.5f-centroid2D[3*p])+
                                       fsqr(j-0.5f-centroid2D[3*p+1])+
                                       fsqr(k-0.5f-centroid2D[3*p+2]);
                        }
               }
         }
      }

   for (n=0; n<256*256; n++)
      if (HIST2D[n]>1.0) variance2D[n]/=HIST2D[n];

   MULTI=TRUE;
   }

// compute the scatter plot
void histo::inithist2D(unsigned char *volume,unsigned char *grad,
                       unsigned int width,unsigned int height,unsigned int depth,
                       int mincnt,float freq,int kneigh,float step,
                       BOOLINT init,
                       void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int n;

   float cx,cy,cz,var;

   float rgb[3];

   if (mincnt<1 || kneigh<0 || step<=0.0f) ERRORMSG();

   if (init) initcentroids2D(volume,grad,width,height,depth,kneigh,step,feedback,obj);

   if (!MULTI) return;

   for (n=0; n<256*256; n++)
      {
      if (HIST2D[n]>=mincnt)
         {
         cx=centroid2D[3*n];
         cy=centroid2D[3*n+1];
         cz=centroid2D[3*n+2];
         var=variance2D[n];
         }
      else
         cx=cy=cz=var=0.0f;

      getrgb(cx,cy,cz,var,freq,1.0f,rgb);

      HIST2DRGBA[4*n]=rgb[0];
      HIST2DRGBA[4*n+1]=rgb[1];
      HIST2DRGBA[4*n+2]=rgb[2];
      HIST2DRGBA[4*n+3]=HIST2DL[n];

      HIST2DQRGBA[4*n]=rgb[0];
      HIST2DQRGBA[4*n+1]=rgb[1];
      HIST2DQRGBA[4*n+2]=rgb[2];
      HIST2DQRGBA[4*n+3]=HIST2DL[n];

      HIST2DTFRGBA[4*n]=rgb[0];
      HIST2DTFRGBA[4*n+1]=rgb[1];
      HIST2DTFRGBA[4*n+2]=rgb[2];
      HIST2DTFRGBA[4*n+3]=HIST2DL[n];
      }
   }

// Shellsort as proposed by Robert Sedgewick in "Algorithms"
template <class Item>
void shellsort(Item a[],const int n)
   {
   int i,j,h;

   Item v;

   for (h=1; h<=(n-1)/9; h=3*h+1);

   while (h>0)
      {
      for (i=h; i<n; i++)
         {
         j=i;
         v=a[i];
         while (j>=h && v>a[j-h])
            {
            a[j]=a[j-h];
            j-=h;
            }
         a[j]=v;
         }
      h/=3;
      }
   }

typedef struct{double *ptr; int index; BOOLINT flag;} histo_item;

inline int operator > (const histo_item &A,const histo_item &B)
   {return(*(A.ptr)>*(B.ptr) || (!A.flag && B.flag));}

// compute the scatter plot using vector quantization
void histo::inithist2DQ(unsigned char *volume,unsigned char *grad,
                        unsigned int width,unsigned int height,unsigned int depth,
                        int mincnt,float freq,int kneigh,float step,
                        BOOLINT init,
                        void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int m,n,p,i;

   float rgb[3];

   float cx,cy,cz,
         cx2,cy2,cz2,
         var,var2,d2;

   histo_item *counter;

   if (mincnt<1 || kneigh<0 || step<=0.0f) ERRORMSG();

   if (freq<1.0f) freq=1.0f;

   if (init) initcentroids2D(volume,grad,width,height,depth,kneigh,step,feedback,obj);

   if (!MULTI) return;

   if (mincnt!=MINCNT || freq!=FREQ)
      {
      clear();

      MINCNT=mincnt;
      FREQ=freq;
      }

   counter=new histo_item[256*256];

   for (n=0; n<256*256; n++)
      {
      counter[n].ptr=&HIST2D[n];
      counter[n].index=n;
      counter[n].flag=FALSE;
      }

   shellsort(counter,256*256);

   for (n=0; n<256*256; n++)
      {
      p=counter[n].index;

      if (HIST2D[p]<mincnt)
         {
         HIST2DRGBA[4*p]=0.0f;
         HIST2DRGBA[4*p+1]=0.0f;
         HIST2DRGBA[4*p+2]=0.0f;
         HIST2DRGBA[4*p+3]=0.0f;

         HIST2DQRGBA[4*p]=0.0f;
         HIST2DQRGBA[4*p+1]=0.0f;
         HIST2DQRGBA[4*p+2]=0.0f;
         HIST2DQRGBA[4*p+3]=0.0f;

         HIST2DTFRGBA[4*p]=0.0f;
         HIST2DTFRGBA[4*p+1]=0.0f;
         HIST2DTFRGBA[4*p+2]=0.0f;
         HIST2DTFRGBA[4*p+3]=0.0f;

         counter[n].flag=TRUE;
         }
      }

   shellsort(counter,256*256);

   for (m=256*256-1; m>0 && counter[m].flag; m--);

   for (n=0; n<=m; n++)
      if (!counter[n].flag)
         {
         p=counter[n].index;

         cx=centroid2D[3*p];
         cy=centroid2D[3*p+1];
         cz=centroid2D[3*p+2];
         var=variance2D[p];

         getrgb(cx,cy,cz,var,2.0f*freq,1.0f,rgb);

         for (i=n; i<=m; i++)
            if (!counter[i].flag)
               {
               p=counter[i].index;

               cx2=centroid2D[3*p];
               cy2=centroid2D[3*p+1];
               cz2=centroid2D[3*p+2];
               var2=variance2D[p];

               d2=fsqr(cx-cx2)+fsqr(cy-cy2)+fsqr(cz-cz2)+fsqr(var-var2);

               if (d2*fsqr(freq)<=1.0f)
                  {
                  HIST2DRGBA[4*p]=rgb[0];
                  HIST2DRGBA[4*p+1]=rgb[1];
                  HIST2DRGBA[4*p+2]=rgb[2];
                  HIST2DRGBA[4*p+3]=HIST2DL[p];

                  if (STATE[p]==1)
                     {
                     HIST2DQRGBA[4*p]=rgb[0];
                     HIST2DQRGBA[4*p+1]=rgb[1];
                     HIST2DQRGBA[4*p+2]=rgb[2];
                     HIST2DQRGBA[4*p+3]=HIST2DL[p];
                     }
                  else
                     {
                     HIST2DQRGBA[4*p]=rgb[0]/2.0f;
                     HIST2DQRGBA[4*p+1]=rgb[1]/2.0f;
                     HIST2DQRGBA[4*p+2]=rgb[2]/2.0f;
                     HIST2DQRGBA[4*p+3]=HIST2DL[p];
                     }

                  HIST2DTFRGBA[4*p]=rgb[0]*EMIT[p];
                  HIST2DTFRGBA[4*p+1]=rgb[1]*EMIT[p];
                  HIST2DTFRGBA[4*p+2]=rgb[2]*EMIT[p];
                  HIST2DTFRGBA[4*p+3]=ABSORB[p];

                  counter[i].flag=TRUE;
                  }
               }
         }

   delete counter;
   }

// clear regions
BOOLINT histo::clear(BOOLINT full)
   {
   int i,j;

   BOOLINT empty=TRUE;

   if (full)
      for (i=0; i<256*256; i++)
         {
         if (STATE[i]!=0) empty=FALSE;

         STATE[i]=0;
         EMIT[i]=0.0f;
         ABSORB[i]=0.0f;
         }
   else
      for (i=0; i<256; i++)
         for (j=0; j<256; j++)
            {
            if (STATE[i+j*256]!=0) empty=FALSE;

            STATE[i+j*256]=1;
            EMIT[i+j*256]=i/255.0f;
            ABSORB[i+j*256]=j/255.0f;
            }

   return(empty);
   }

// select a region
void histo::click(float x,float y,float rad)
   {
   int i,j,
       s,t,n;

   float r,g,b;

   int sel,nsel;

   if (!MULTI) return;

   s=ftrc(x*255+0.5f);
   t=ftrc(y*255+0.5f);
   n=ftrc(rad*255+0.5f);

   r=HIST2DRGBA[4*(s+256*t)];
   g=HIST2DRGBA[4*(s+256*t)+1];
   b=HIST2DRGBA[4*(s+256*t)+2];

   if (r>0.0f && g>0.0f && b>0.0f) click(s,t);
   else
      {
      sel=nsel=0;

      for (i=s-n; i<=s+n; i++)
         for (j=t-n; j<=t+n; j++)
            if (i>=0 && i<256 && j>=0 && j<256)
               {
               r=HIST2DRGBA[4*(i+256*j)];
               g=HIST2DRGBA[4*(i+256*j)+1];
               b=HIST2DRGBA[4*(i+256*j)+2];

               if (r>0.0f && g>0.0f && b>0.0f)
                  if (STATE[i+j*256]==1) sel++;
                  else nsel++;
               }

      if (sel==0 && nsel==0)
         {
         if (CLICKED && s==CLICKs && t==CLICKt) markall();

         CLICKED=TRUE;
         CLICKs=s;
         CLICKt=t;
         }
      else
         {
         if (nsel>=sel)
            {
            for (i=s-n; i<=s+n; i++)
               for (j=t-n; j<=t+n; j++)
                  if (i>=0 && i<256 && j>=0 && j<256)
                     if (STATE[i+j*256]==0) click(i,j);
            }
         else
            {
            for (i=s-n; i<=s+n; i++)
               for (j=t-n; j<=t+n; j++)
                  if (i>=0 && i<256 && j>=0 && j<256)
                     if (STATE[i+j*256]==1) click(i,j);
            }

         CLICKED=FALSE;
         }
      }

   inithist2DQ(NULL,NULL,0,0,0,MINCNT,FREQ,0,1.0f,FALSE);
   }

// select a region
void histo::click(int s,int t)
   {
   float r,g,b;

   int mins,maxs,mint,maxt;

   if (!MULTI) return;

   r=HIST2DRGBA[4*(s+256*t)];
   g=HIST2DRGBA[4*(s+256*t)+1];
   b=HIST2DRGBA[4*(s+256*t)+2];

   if (r>0.0f && g>0.0f && b>0.0f)
      if (STATE[s+256*t]==0)
         {
         detect(s,t,2,r,g,b,&mins,&maxs,&mint,&maxt);
         mark(s,t,2,1,mins,maxs,mint,maxt);
         }
      else
         {
         detect(s,t,2,r,g,b,&mins,&maxs,&mint,&maxt);
         clear(s,t,2,0);
         }
   }

// render the histogram points
void histo::render2DQ(float mx,float my,float mz,
                      float sx,float sy,float sz)
   {
   int n;

   if (!MULTI) return;

   glPushMatrix();

   glTranslatef(mx,my,mz);
   glScalef(sx,sy,sz);

   glBegin(GL_POINTS);

   for (n=0; n<256*256; n++)
      if (HIST2DQRGBA[4*n+3]>0.0f)
         {
         glColor3fv(&HIST2DQRGBA[4*n]);
         glVertex3fv(&centroid2D[3*n]);
         }

   glEnd();

   glPopMatrix();
   }

// return the histogram
float *histo::get_hist()
   {return(HISTL);}

// return the colored histogram
float *histo::get_histRGBA()
   {return(HISTRGBA);}

// return the scatter plot
float *histo::get_hist2D()
   {
   if (!MULTI) return(NULL);
   return(HIST2DL);
   }

// return the colored scatter plot
float *histo::get_hist2DRGBA()
   {
   if (!MULTI) return(NULL);
   return(HIST2DRGBA);
   }

// return the quantized scatter plot
float *histo::get_hist2DQRGBA()
   {
   if (!MULTI) return(NULL);
   return(HIST2DQRGBA);
   }

// return the quantized transfer function
float *histo::get_hist2DTFRGBA()
   {
   if (!MULTI) return(NULL);
   return(HIST2DTFRGBA);
   }

// detect a region
int histo::detect(const int s,const int t,const int v,
                  const float r,const float g,const float b,
                  int *mins,int *maxs,int *mint,int *maxt,
                  BOOLINT flag)
   {
   int count=0;

   if (STATE[s+t*256]==v) return(0);

   if (HIST2DRGBA[4*(s+t*256)]!=r) return(0);
   if (HIST2DRGBA[4*(s+t*256)+1]!=g) return(0);
   if (HIST2DRGBA[4*(s+t*256)+2]!=b) return(0);

   STATE[s+t*256]=v;

   if (flag)
      {
      *mins=*maxs=s;
      *mint=*maxt=t;
      }
   else
      {
      if (s<*mins) *mins=s;
      else if (s>*maxs) *maxs=s;

      if (t<*mint) *mint=t;
      else if (t>*maxt) *maxt=t;
      }

   if (s>0) count+=detect(s-1,t,v,r,g,b,mins,maxs,mint,maxt,FALSE);
   if (s<255) count+=detect(s+1,t,v,r,g,b,mins,maxs,mint,maxt,FALSE);
   if (t>0) count+=detect(s,t-1,v,r,g,b,mins,maxs,mint,maxt,FALSE);
   if (t<255) count+=detect(s,t+1,v,r,g,b,mins,maxs,mint,maxt,FALSE);

   return(count+1);
   }

// mark a region
void histo::mark(const int s,const int t,const int v1,const int v2)
   {
   if (STATE[s+t*256]!=v1) return;

   STATE[s+t*256]=v2;

   EMIT[s+t*256]=1.0f;
   ABSORB[s+t*256]=1.0f;

   if (s>0) mark(s-1,t,v1,v2);
   if (s<255) mark(s+1,t,v1,v2);
   if (t>0) mark(s,t-1,v1,v2);
   if (t<255) mark(s,t+1,v1,v2);
   }

// mark a region with gradient
void histo::mark(const int s,const int t,const int v1,const int v2,
                 const int mins,const int maxs,const int mint,const int maxt)
   {
   float x,y;

   if (STATE[s+t*256]!=v1) return;

   STATE[s+t*256]=v2;

   if (mins>=maxs) x=1.0f;
   else x=(float)(s-mins)/(maxs-mins);

   if (mint>=maxt) y=1.0f;
   else y=(float)(t-mint)/(maxt-mint);

   EMIT[s+t*256]=x;
   ABSORB[s+t*256]=(1.0f-x)*y;

   if (s>0) mark(s-1,t,v1,v2,mins,maxs,mint,maxt);
   if (s<255) mark(s+1,t,v1,v2,mins,maxs,mint,maxt);
   if (t>0) mark(s,t-1,v1,v2,mins,maxs,mint,maxt);
   if (t<255) mark(s,t+1,v1,v2,mins,maxs,mint,maxt);
   }

// mark all regions with gradient
void histo::markall()
   {
   int s,t;

   float r,g,b;

   int mins,maxs,mint,maxt;

   if (!MULTI) return;

   if (clear(TRUE))
      for (s=0; s<256; s++)
         for (t=0; t<256; t++)
            if (STATE[s+t*256]==0)
               {
               r=HIST2DRGBA[4*(s+256*t)];
               g=HIST2DRGBA[4*(s+256*t)+1];
               b=HIST2DRGBA[4*(s+256*t)+2];

               if (r>0.0f && g>0.0f && b>0.0f)
                  {
                  detect(s,t,2,r,g,b,&mins,&maxs,&mint,&maxt);
                  mark(s,t,2,1,mins,maxs,mint,maxt);
                  }
               }
   }

// clear a region
void histo::clear(const int s,const int t,const int v1,const int v2)
   {
   if (STATE[s+t*256]!=v1) return;

   STATE[s+t*256]=v2;

   EMIT[s+t*256]=0.0f;
   ABSORB[s+t*256]=0.0f;

   if (s>0) clear(s-1,t,v1,v2);
   if (s<255) clear(s+1,t,v1,v2);
   if (t>0) clear(s,t-1,v1,v2);
   if (t<255) clear(s,t+1,v1,v2);
   }

// save2file
void histo::save(FILE *file)
   {
   int i,j;

   fprintf(file,"HISTO:\n");

   fprintf(file,"multi=%d\n",MULTI);

   if (MULTI)
      {
      fprintf(file,"mincnt=%d\n",MINCNT);
      fprintf(file,"freq=%g\n",FREQ);

      for (i=0; i<256; i++)
         for (j=0; j<256; j++)
            fprintf(file,"%d %g %g\n",STATE[i+j*256],EMIT[i+j*256],ABSORB[i+j*256]);
      }
   }

// load
void histo::load(FILE *file)
   {
   int i,j;

   int multi,state;

   if (fscanf(file,"HISTO:\n")!=0) return;

   fscanf(file,"multi=%d\n",&multi);

   if (multi && MULTI)
      {
      fscanf(file,"mincnt=%d\n",&MINCNT);
      fscanf(file,"freq=%g\n",&FREQ);

      for (i=0; i<256; i++)
         for (j=0; j<256; j++)
            {
            fscanf(file,"%d %g %g\n",&state,&EMIT[i+j*256],&ABSORB[i+j*256]);
            STATE[i+j*256]=state;
            }

      inithist2DQ(NULL,NULL,0,0,0,MINCNT,FREQ,0,1.0f,FALSE);
      }
   }
