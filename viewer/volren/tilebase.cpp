// (c) by Stefan Roettger, licensed under GPL 2+

#include "tilebase.h"

#include "progs.h"

// a texture brick:

brick::brick()
   {TEXID=0;}

brick::~brick()
   {deletetexmap3D();}

// generate 3D texture map
void brick::buildtexmap3D(unsigned char *volume,
                          int width,int height,int depth)
   {
   if (width<2 || height<2 || depth<2) ERRORMSG();

   deletetexmap3D();

   glGenTextures(1,&TEXID);
   glBindTexture(GL_TEXTURE_3D,TEXID);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#ifndef WINOS
   glTexImage3D(GL_TEXTURE_3D,0,GL_LUMINANCE,width,height,depth,0,
                GL_LUMINANCE,GL_UNSIGNED_BYTE,volume);
#else
   glTexImage3DEXT(GL_TEXTURE_3D,0,GL_LUMINANCE,width,height,depth,0,
                   GL_LUMINANCE,GL_UNSIGNED_BYTE,volume);
#endif

   glBindTexture(GL_TEXTURE_3D,0);
   }

// delete 3D texture map
void brick::deletetexmap3D()
   {if (TEXID>0) glDeleteTextures(1,&TEXID);}

// a tile of the volume:

BOOLINT tile::LOADED=FALSE;
unsigned int tile::INSTANCES=0;
GLuint tile::PROGID[PROGNUM];

tile::tile(tfunc2D *tf,char *base)
   {
   BRICK=new brick();
   EXTRA=NULL;

   TFUNC=tf;

   NOISE=0.01f;
   AMBNT=0.3f;
   DIFUS=0.5f;
   SPECL=0.2f;
   SPECX=10.0f;

   setup(base);
   }

tile::~tile()
   {
   delete BRICK;
   if (EXTRA!=NULL) delete EXTRA;

   destroy();
   }

// load fragment programs
void tile::setup(char *base)
   {
   int i;

   char progname[PROGNUM][MAXSTR];

   unsigned char *prog[PROGNUM];
   unsigned int len[PROGNUM];

   char *GL_EXTs;

   if ((GL_EXTs=(char *)glGetString(GL_EXTENSIONS))==NULL) ERRORMSG();

   if (strstr(GL_EXTs,"ARB_multitexture")==NULL ||
       strstr(GL_EXTs,"ARB_fragment_program")==NULL)
      WARNMSG("necessary rendering extensions not fully supported");

   if (!LOADED)
      {
      if (base==NULL)
         for (i=0; i<PROGNUM; i++)
            snprintf(progname[i],MAXSTR,"prog%d.arb",i+1);
      else if (strlen(base)==0)
         for (i=0; i<PROGNUM; i++)
            snprintf(progname[i],MAXSTR,"prog%d.arb",i+1);
      else
         for (i=0; i<PROGNUM; i++)
            snprintf(progname[i],MAXSTR,"%s/prog%d.arb",base,i+1);

      for (i=0; i<PROGNUM; i++)
         if ((prog[i]=readRAWfile(progname[i],&len[i]))==NULL)
            {
            prog[i]=(unsigned char *)strdup(inline_prog[i]);
            len[i]=strlen(inline_prog[i]);
            }

      for (i=0; i<PROGNUM; i++)
         {
#ifdef GL_ARB_fragment_program

         GLint errorPos,isNative;

         glGenProgramsARB(1,&PROGID[i]);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[i]);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,len[i],prog[i]);

         glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB,&errorPos);

         if (errorPos==0)
            {
            WARNMSG("shader program unavailable");
            WARNMSG((char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
            }
         else
            {
            if (errorPos!=-1)
               {
               WARNMSG((char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
               ERRORMSG();
               }
            if (isNative!=1) WARNMSG("shader program non-native");
            }

         glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,&isNative);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);

#endif

         free(prog[i]);
         }

      LOADED=TRUE;
      }

   INSTANCES++;
   }

// destroy fragment programs
void tile::destroy()
   {
#ifdef GL_ARB_fragment_program

   int i;

   INSTANCES--;

   if (INSTANCES==0)
      if (LOADED)
         {
         for (i=0; i<PROGNUM; i++)
            glDeleteProgramsARB(1,&PROGID[i]);

         LOADED=FALSE;
         }

#endif
   }

// set the tile data
void tile::set_data(unsigned char *data,
                    unsigned int width,unsigned int height,unsigned int depth,
                    float mx,float my,float mz,
                    float sx,float sy,float sz,
                    int px,int py,int pz,
                    int bricksize,int border)
   {
   int i,j,k;

   unsigned char *volume,*ptr;

   if ((volume=(unsigned char *)malloc(bricksize*bricksize*bricksize))==NULL) ERRORMSG();

   for (ptr=volume,k=pz; k<pz+bricksize; k++)
      for (j=py; j<py+bricksize; j++)
         for (i=px; i<px+bricksize; i++)
            if (i<0 || i>=(int)width ||
                j<0 || j>=(int)height ||
                k<0 || k>=(int)depth) *ptr++=0;
            else *ptr++=data[((unsigned int)i)+(((unsigned int)j)+((unsigned int)k)*height)*width];

   BRICK->buildtexmap3D(volume,bricksize,bricksize,bricksize);

   BSIZE=bricksize;

   MX=MX2=mx;
   MY=MY2=my;
   MZ=MZ2=mz;

   SX=SX2=sx;
   SY=SY2=sy;
   SZ=SZ2=sz;

   BORDER=border;

   MINDATA=255;
   MAXDATA=0;

   for (ptr=volume,k=0; k<bricksize; k++)
      for (j=0; j<bricksize; j++)
         for (i=0; i<bricksize; i++,ptr++)
            {
            if (*ptr<MINDATA) MINDATA=*ptr;
            if (*ptr>MAXDATA) MAXDATA=*ptr;
            }

   free(volume);

   if (EXTRA!=NULL)
      {
      delete EXTRA;
      EXTRA=NULL;
      }
   }

// set the extra tile data
void tile::set_extra(unsigned char *extra,
                     unsigned int width,unsigned int height,unsigned int depth,
                     int px,int py,int pz,
                     int bricksize)
   {
   int i,j,k;

   unsigned char *volume,*ptr;

   if (bricksize!=BSIZE) ERRORMSG();

   if ((volume=(unsigned char *)malloc(bricksize*bricksize*bricksize))==NULL) ERRORMSG();

   for (ptr=volume,k=pz; k<pz+bricksize; k++)
      for (j=py; j<py+bricksize; j++)
         for (i=px; i<px+bricksize; i++)
            if (i<0 || i>=(int)width ||
                j<0 || j>=(int)height ||
                k<0 || k>=(int)depth) *ptr++=0;
            else *ptr++=extra[((unsigned int)i)+(((unsigned int)j)+((unsigned int)k)*height)*width];

   if (EXTRA==NULL) EXTRA=new brick();

   EXTRA->buildtexmap3D(volume,bricksize,bricksize,bricksize);

   MINEXTRA=255;
   MAXEXTRA=0;

   for (ptr=volume,k=0; k<bricksize; k++)
      for (j=0; j<bricksize; j++)
         for (i=0; i<bricksize; i++,ptr++)
            {
            if (*ptr<MINEXTRA) MINEXTRA=*ptr;
            if (*ptr>MAXEXTRA) MAXEXTRA=*ptr;
            }

   free(volume);
   }

// set the tile size
void tile::set_size(float mx,float my,float mz,
                    float sx,float sy,float sz)
   {
   MX2=mx;
   MY2=my;
   MZ2=mz;

   SX2=sx;
   SY2=sy;
   SZ2=sz;
   }

// set ambient/diffuse/specular lighting coefficients
void tile::set_light(float noise,float ambnt,float difus,float specl,float specx)
   {
   NOISE=noise;
   AMBNT=ambnt;
   DIFUS=difus;
   SPECL=specl;
   SPECX=specx;
   }

// get ambient/diffuse/specular lighting coefficients
void tile::get_light(float *noise,float *ambnt,float *difus,float *specl,float *specx)
   {
   *noise=NOISE;
   *ambnt=AMBNT;
   *difus=DIFUS;
   *specl=SPECL;
   *specx=SPECX;
   }

// intersect a line with a plane
inline void tile::intersect(const float px,const float py,const float pz,
                            const float dx,const float dy,const float dz,
                            const float ox,const float oy,const float oz,
                            const float nx,const float ny,const float nz,
                            float *mx,float *my,float *mz)
   {
   float lambda;

   lambda=nx*dx+ny*dy+nz*dz;
   if (lambda!=0.0f) lambda=(nx*(ox-px)+ny*(oy-py)+nz*(oz-pz))/lambda;

   *mx=px+lambda*dx;
   *my=py+lambda*dy;
   *mz=pz+lambda*dz;
   }

// project the texcoords onto the back and the front of the slab
inline void tile::projtexcoords(const float x,const float y,const float z,const float slab2)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z;

   if (TFUNC->get_dim())
      {
      intersect(EX,EY,EZ,
                x-EX,y-EY,z-EZ,
                x+slab2*DX,y+slab2*DY,z+slab2*DZ,
                DX,DY,DZ,
                &pp1x,&pp1y,&pp1z);

      intersect(EX,EY,EZ,
                x-EX,y-EY,z-EZ,
                x-slab2*DX,y-slab2*DY,z-slab2*DZ,
                DX,DY,DZ,
                &pp2x,&pp2y,&pp2z);

#ifdef GL_ARB_multitexture
      glMultiTexCoord3fARB(GL_TEXTURE0_ARB,pp1x,pp1y,pp1z);
      glMultiTexCoord3fARB(GL_TEXTURE1_ARB,pp2x,pp2y,pp2z);
      if (TFUNC->get_num()!=1) glMultiTexCoord3fARB(GL_TEXTURE2_ARB,x,y,z);
#endif
      }
   else
      {
#ifdef GL_ARB_multitexture
      glMultiTexCoord3fARB(GL_TEXTURE0_ARB,x,y,z);
      if (TFUNC->get_num()!=1) glMultiTexCoord3fARB(GL_TEXTURE1_ARB,x,y,z);
#endif
      }
   }

// extract triangle from tetrahedron
inline void tile::slicetetra1(const float p1x,const float p1y,const float p1z,const float d1,
                              const float p2x,const float p2y,const float p2z,const float d2,
                              const float p3x,const float p3y,const float p3z,const float d3,
                              const float p4x,const float p4y,const float p4z,const float d4,
                              const float slab)
   {
#ifdef GL_ARB_multitexture

   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);

   if (TFUNC->get_num()==1)
      if (TFUNC->get_aid()!=0)
         {
         glBindTexture(GL_TEXTURE_2D,TFUNC->get_aid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f,0.0f,0.0f,1.0f);
#endif

         glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();

         glBindTexture(GL_TEXTURE_2D,TFUNC->get_eid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,AMBNT,DIFUS,SPECL,SPECX);
#endif

         glBlendFunc(GL_ONE,GL_ONE);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
      else
         {
         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
   else
      if (TFUNC->get_aid()!=0)
         {
         if (TFUNC->get_dim()) glBindTexture(GL_TEXTURE_3D,TFUNC->get_aid());
         else glBindTexture(GL_TEXTURE_2D,TFUNC->get_aid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f,0.0f,0.0f,1.0f);
#endif

         glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();

         if (TFUNC->get_dim()) glBindTexture(GL_TEXTURE_3D,TFUNC->get_eid());
         else glBindTexture(GL_TEXTURE_2D,TFUNC->get_eid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,AMBNT,DIFUS,SPECL,SPECX);
#endif

         glBlendFunc(GL_ONE,GL_ONE);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
      else
         {
         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }

#endif
   }

// extract quad from tetrahedron
inline void tile::slicetetra2(const float p1x,const float p1y,const float p1z,const float d1,
                              const float p2x,const float p2y,const float p2z,const float d2,
                              const float p3x,const float p3y,const float p3z,const float d3,
                              const float p4x,const float p4y,const float p4z,const float d4,
                              const float slab)
   {
#ifdef GL_ARB_multitexture

   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z,
         pp4x,pp4y,pp4z;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);

   if (TFUNC->get_num()==1)
      if (TFUNC->get_aid()!=0)
         {
         glBindTexture(GL_TEXTURE_2D,TFUNC->get_aid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f,0.0f,0.0f,1.0f);
#endif

         glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();

         glBindTexture(GL_TEXTURE_2D,TFUNC->get_eid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,AMBNT,DIFUS,SPECL,SPECX);
#endif

         glBlendFunc(GL_ONE,GL_ONE);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
      else
         {
         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
   else
      if (TFUNC->get_aid()!=0)
         {
         if (TFUNC->get_dim()) glBindTexture(GL_TEXTURE_3D,TFUNC->get_aid());
         else glBindTexture(GL_TEXTURE_2D,TFUNC->get_aid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f,0.0f,0.0f,1.0f);
#endif

         glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();

         if (TFUNC->get_dim()) glBindTexture(GL_TEXTURE_3D,TFUNC->get_eid());
         else glBindTexture(GL_TEXTURE_2D,TFUNC->get_eid());

#ifdef GL_ARB_fragment_program
         if (LIGHTING)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,AMBNT,DIFUS,SPECL,SPECX);
#endif

         glBlendFunc(GL_ONE,GL_ONE);

         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }
      else
         {
         glBegin(GL_TRIANGLE_FAN);
         projtexcoords(pp1x,pp1y,pp1z,slab/2.0f);
         glVertex3f(pp1x,pp1y,pp1z);
         projtexcoords(pp2x,pp2y,pp2z,slab/2.0f);
         glVertex3f(pp2x,pp2y,pp2z);
         projtexcoords(pp4x,pp4y,pp4z,slab/2.0f);
         glVertex3f(pp4x,pp4y,pp4z);
         projtexcoords(pp3x,pp3y,pp3z,slab/2.0f);
         glVertex3f(pp3x,pp3y,pp3z);
         glEnd();
         }

#endif
   }

// slice tetrahedron from back to front at distances given by the slab thickness
inline void tile::slicetetra(const float p1x,const float p1y,const float p1z,
                             const float p2x,const float p2y,const float p2z,
                             const float p3x,const float p3y,const float p3z,
                             const float p4x,const float p4y,const float p4z,
                             const float slab)
   {
   int flag;

   float d,d1,d2,d3,d4,dmin,dmax;

   d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ;
   d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ;
   d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ;
   d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ;

   dmin=fmin(fmin(d1,d2),fmin(d3,d4));
   dmax=fmax(fmax(d1,d2),fmax(d3,d4));

   d=(ffloor((dmax-NEARP)/slab-0.5f)+0.5f)*slab+NEARP;

   d1-=d;
   d2-=d;
   d3-=d;
   d4-=d;

   while (d>dmin && d>NEARP)
      {
      flag=0;

      if (d1<0.0f) flag|=1;
      if (d2<0.0f) flag|=2;
      if (d3<0.0f) flag|=4;
      if (d4<0.0f) flag|=8;

      switch (flag)
         {
         case 1: case 14: slicetetra1(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),slab); break;
         case 2: case 13: slicetetra1(p2x,p2y,p2z,fabs(d2),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),slab); break;
         case 4: case 11: slicetetra1(p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),slab); break;
         case 8: case 7: slicetetra1(p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),slab); break;

         case 3: slicetetra2(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),slab); break;
         case 5: slicetetra2(p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),slab); break;
         case 6: slicetetra2(p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4),slab); break;
         case 9: slicetetra2(p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),slab); break;
         case 10: slicetetra2(p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),slab); break;
         case 12: slicetetra2(p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),slab); break;
         }

      d1+=slab;
      d2+=slab;
      d3+=slab;
      d4+=slab;

      d-=slab;
      }
   }

// check whether or not p4 is in front of a triangle defined by p1-3
inline BOOLINT tile::isfront(const float p1x,const float p1y,const float p1z,
                             const float p2x,const float p2y,const float p2z,
                             const float p3x,const float p3y,const float p3z,
                             const float p4x,const float p4y,const float p4z)
   {
   float dx1,dy1,dz1,
         dx2,dy2,dz2,
         nx,ny,nz;

   dx1=p2x-p1x;
   dy1=p2y-p1y;
   dz1=p2z-p1z;

   dx2=p3x-p1x;
   dy2=p3y-p1y;
   dz2=p3z-p1z;

   nx=dz1*dy2-dy1*dz2;
   ny=dx1*dz2-dz1*dx2;
   nz=dy1*dx2-dx1*dy2;

   if ((EX-p1x)*nx+(EY-p1y)*ny+(EZ-p1z)*nz>0.0f)
      return((p4x-p1x)*nx+(p4y-p1y)*ny+(p4z-p1z)*nz>0.0f);
   else
      return((p4x-p1x)*nx+(p4y-p1y)*ny+(p4z-p1z)*nz<0.0f);
   }

// slice hexahedron by breaking it up into 5 sorted tetrahedra
void tile::drawhexa(const float p1x,const float p1y,const float p1z,
                    const float p2x,const float p2y,const float p2z,
                    const float p3x,const float p3y,const float p3z,
                    const float p4x,const float p4y,const float p4z,
                    const float p5x,const float p5y,const float p5z,
                    const float p6x,const float p6y,const float p6z,
                    const float p7x,const float p7y,const float p7z,
                    const float p8x,const float p8y,const float p8z,
                    const float slab)
   {
   BOOLINT f1,f2,f3,f4;

   f1=isfront(p1x,p1y,p1z,p8x,p8y,p8z,p6x,p6y,p6z,p5x,p5y,p5z);
   f2=isfront(p3x,p3y,p3z,p6x,p6y,p6z,p8x,p8y,p8z,p7x,p7y,p7z);
   f3=isfront(p6x,p6y,p6z,p3x,p3y,p3z,p1x,p1y,p1z,p2x,p2y,p2z);
   f4=isfront(p8x,p8y,p8z,p1x,p1y,p1z,p3x,p3y,p3z,p4x,p4y,p4z);

   if (!f1) slicetetra(p1x,p1y,p1z,p6x,p6y,p6z,p8x,p8y,p8z,p5x,p5y,p5z,slab);
   if (!f2) slicetetra(p3x,p3y,p3z,p8x,p8y,p8z,p6x,p6y,p6z,p7x,p7y,p7z,slab);
   if (!f3) slicetetra(p6x,p6y,p6z,p1x,p1y,p1z,p3x,p3y,p3z,p2x,p2y,p2z,slab);
   if (!f4) slicetetra(p8x,p8y,p8z,p3x,p3y,p3z,p1x,p1y,p1z,p4x,p4y,p4z,slab);

   slicetetra(p1x,p1y,p1z,p8x,p8y,p8z,p6x,p6y,p6z,p3x,p3y,p3z,slab);

   if (f1) slicetetra(p1x,p1y,p1z,p6x,p6y,p6z,p8x,p8y,p8z,p5x,p5y,p5z,slab);
   if (f2) slicetetra(p3x,p3y,p3z,p8x,p8y,p8z,p6x,p6y,p6z,p7x,p7y,p7z,slab);
   if (f3) slicetetra(p6x,p6y,p6z,p1x,p1y,p1z,p3x,p3y,p3z,p2x,p2y,p2z,slab);
   if (f4) slicetetra(p8x,p8y,p8z,p3x,p3y,p3z,p1x,p1y,p1z,p4x,p4y,p4z,slab);
   }

// bind 3D texture map
void tile::bindtexmap(int texid3D)
   {
   if (texid3D>0)
      {
      glBindTexture(GL_TEXTURE_3D,texid3D);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glEnable(GL_TEXTURE_3D);
      }
   else glDisable(GL_TEXTURE_3D);
   }

// bind 3D texture map using dependent 2D lookup
void tile::bindtexmaps(int texid3D,int texid2DE,int texid2DA)
   {
#ifdef GL_ARB_fragment_program

   if (texid3D>0)
      {
      // activate fragment program
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[0]);

      // lighting is not supported
      LIGHTING=FALSE;

      // texture 0:

      glActiveTextureARB(GL_TEXTURE0_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3D);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glEnable(GL_TEXTURE_3D);

      // texture 3:

      glActiveTextureARB(GL_TEXTURE3_ARB);

      glBindTexture(GL_TEXTURE_2D,texid2DE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      if (texid2DA!=0)
         {
         glBindTexture(GL_TEXTURE_2D,texid2DA);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }

      glEnable(GL_TEXTURE_2D);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   else
      {
      glActiveTextureARB(GL_TEXTURE3_ARB);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glDisable(GL_TEXTURE_3D);

      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      }

#endif
   }

// bind 3D texture map using dependent 1D lookup
void tile::bindtexmaps1D(int texid3D,int texid1DE,int texid1DA)
   {
#ifdef GL_ARB_fragment_program

   if (texid3D>0)
      {
      // activate fragment program
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[1]);

      // lighting is not supported
      LIGHTING=FALSE;

      // texture 0:

      glActiveTextureARB(GL_TEXTURE0_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3D);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glEnable(GL_TEXTURE_3D);

      // texture 3:

      glActiveTextureARB(GL_TEXTURE3_ARB);

      glBindTexture(GL_TEXTURE_2D,texid1DE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      if (texid1DA!=0)
         {
         glBindTexture(GL_TEXTURE_2D,texid1DA);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }

      glEnable(GL_TEXTURE_2D);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   else
      {
      glActiveTextureARB(GL_TEXTURE3_ARB);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glDisable(GL_TEXTURE_3D);

      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      }

#endif
   }

// bind 3D texture map using dependent 2D lookup with gradient magnitude
void tile::bindtexmaps2D(int texid3D,int texid3DG,int texid2DE,int texid2DA)
   {
#ifdef GL_ARB_fragment_program

   if (texid3D>0)
      {
      // activate fragment program
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[2]);

      // lighting is not supported
      LIGHTING=FALSE;

      // texture 0:

      glActiveTextureARB(GL_TEXTURE0_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3D);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glEnable(GL_TEXTURE_3D);

      // texture 1:

      glActiveTextureARB(GL_TEXTURE1_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3DG);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);

      if (TFUNC->get_imode())
         {
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }
      else
         {
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
         }

      glEnable(GL_TEXTURE_3D);

      // texture 3:

      glActiveTextureARB(GL_TEXTURE3_ARB);

      glBindTexture(GL_TEXTURE_2D,texid2DE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      if (texid2DA!=0)
         {
         glBindTexture(GL_TEXTURE_2D,texid2DA);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }

      glEnable(GL_TEXTURE_2D);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   else
      {
      glActiveTextureARB(GL_TEXTURE3_ARB);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_3D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glDisable(GL_TEXTURE_3D);

      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      }

#endif
   }

// bind 3D texture map using dependent 3D lookup with gradient magnitude and lighting
void tile::bindtexmaps3D(int texid3D,int texid3DG,int texid3DE,int texid3DA,float rslab)
   {
#ifdef GL_ARB_fragment_program

   if (texid3D>0)
      {
      // activate fragment program
      glEnable(GL_FRAGMENT_PROGRAM_ARB);

      if (rslab==0.0f)
         {
         // disable lighting:

         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[3]);

         LIGHTING=FALSE;
         }
      else
         {
         // enable lighting:

         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PROGID[4]);

         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,rslab,NOISE,0.0f,0.0f);
         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,AMBNT,DIFUS,SPECL,SPECX);

         LIGHTING=TRUE;
         }

      // texture 0:

      glActiveTextureARB(GL_TEXTURE0_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3D);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glEnable(GL_TEXTURE_3D);

      // texture 1:

      glActiveTextureARB(GL_TEXTURE1_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3DG);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);

      if (TFUNC->get_imode())
         {
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }
      else
         {
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
         }

      glEnable(GL_TEXTURE_3D);

      // texture 3:

      glActiveTextureARB(GL_TEXTURE3_ARB);

      glBindTexture(GL_TEXTURE_3D,texid3DE);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      if (texid3DA!=0)
         {
         glBindTexture(GL_TEXTURE_3D,texid3DA);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         }

      glEnable(GL_TEXTURE_3D);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   else
      {
      glActiveTextureARB(GL_TEXTURE3_ARB);
      glDisable(GL_TEXTURE_3D);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_3D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glDisable(GL_TEXTURE_3D);

      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      }

#endif
   }

// render the tile
void tile::render(float ex,float ey,float ez,
                  float dx,float dy,float dz,
                  float ux,float uy,float uz,
                  float nearp,float slab,float rslab,
                  BOOLINT lighting,
                  BOOLINT depth)
   {
   EX=ex; EY=ey; EZ=ez;
   DX=dx; DY=dy; DZ=dz;
   UX=ux; UY=uy; UZ=uz;

   NEARP=nearp;

   // try the ZOT
   if (EXTRA==NULL || TFUNC->get_num()==1)
      {if (TFUNC->zot(MINDATA/255.0f,MAXDATA/255.0f)) return;}
   else
      {if (TFUNC->zot(MINDATA/255.0f,MAXDATA/255.0f,MINEXTRA/255.0f,MAXEXTRA/255.0f)) return;}

   glEnable(GL_BLEND);

   glDisable(GL_CULL_FACE);

   if (!depth) glDepthMask(GL_FALSE);

   if (EXTRA==NULL || TFUNC->get_num()==1)
      if (TFUNC->get_dim())
         {
         // 1D transfer functions:

         // standard version with pre-integration
         bindtexmaps(BRICK->get_id(),TFUNC->get_eid(),TFUNC->get_aid());

#ifdef GL_ARB_multitexture

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glColor4f(1.0f,1.0f,1.0f,1.0f);

         glActiveTextureARB(GL_TEXTURE3_ARB);

         glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

         drawhexa(MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  slab);

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

#endif

         bindtexmaps(0,0,0);
         }
      else
         {
         // 1D transfer functions:

         // fall-back version without pre-integration
         bindtexmaps1D(BRICK->get_id(),TFUNC->get_eid(),TFUNC->get_aid());

#ifdef GL_ARB_multitexture

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glColor4f(1.0f,1.0f,1.0f,1.0f);

         glActiveTextureARB(GL_TEXTURE3_ARB);

         glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

         drawhexa(MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  slab);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

#endif

         bindtexmaps1D(0,0,0);
         }
   else
      if (!TFUNC->get_dim())
         {
         // 2D transfer functions:

         // fall-back version without pre-integration
         bindtexmaps2D(BRICK->get_id(),EXTRA->get_id(),TFUNC->get_eid(),TFUNC->get_aid());

#ifdef GL_ARB_multitexture

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glColor4f(1.0f,1.0f,1.0f,1.0f);

         glActiveTextureARB(GL_TEXTURE3_ARB);

         glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

         drawhexa(MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  slab);

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

#endif

         bindtexmaps2D(0,0,0,0);
         }
      else
         {
         // 2D transfer functions:

         if (!lighting || !TFUNC->get_imode())
            // high-quality version with pre-integration
            bindtexmaps3D(BRICK->get_id(),EXTRA->get_id(),TFUNC->get_eid(),TFUNC->get_aid(),0.0f);
         else
            // high-quality version with pre-integration and lighting
            bindtexmaps3D(BRICK->get_id(),EXTRA->get_id(),TFUNC->get_eid(),TFUNC->get_aid(),1.0f/(slab*rslab));

#ifdef GL_ARB_multitexture

         glActiveTextureARB(GL_TEXTURE2_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);

         glPushMatrix();
         glLoadIdentity();

         glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
         glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE,
                  (float)(BSIZE-1-2*BORDER)/BSIZE);

         glTranslatef(0.5f,0.5f,0.5f);
         glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
         glTranslatef(-MX,-MY,-MZ);

         glMatrixMode(GL_MODELVIEW);

         glColor4f(1.0f,1.0f,1.0f,1.0f);

         glActiveTextureARB(GL_TEXTURE3_ARB);

         glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

         drawhexa(MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                  MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                  slab);

         glActiveTextureARB(GL_TEXTURE2_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE1_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

         glActiveTextureARB(GL_TEXTURE0_ARB);

         glMatrixMode(GL_TEXTURE);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

#endif

         bindtexmaps3D(0,0,0,0,0.0f);
         }

   if (!depth) glDepthMask(GL_TRUE);

   glEnable(GL_CULL_FACE);

   glDisable(GL_BLEND);
   }

// extract triangle from tetrahedron
inline void tile::intersecttetra1(const float p1x,const float p1y,const float p1z,const float d1,
                                  const float p2x,const float p2y,const float p2z,const float d2,
                                  const float p3x,const float p3y,const float p3z,const float d3,
                                  const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);

   glBegin(GL_TRIANGLE_FAN);
   glTexCoord3f(pp1x,pp1y,pp1z);
   glVertex3f(pp1x,pp1y,pp1z);
   glTexCoord3f(pp2x,pp2y,pp2z);
   glVertex3f(pp2x,pp2y,pp2z);
   glTexCoord3f(pp3x,pp3y,pp3z);
   glVertex3f(pp3x,pp3y,pp3z);
   glEnd();
   }

// extract quad from tetrahedron
inline void tile::intersecttetra2(const float p1x,const float p1y,const float p1z,const float d1,
                                  const float p2x,const float p2y,const float p2z,const float d2,
                                  const float p3x,const float p3y,const float p3z,const float d3,
                                  const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z,
         pp4x,pp4y,pp4z;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);

   glBegin(GL_TRIANGLE_FAN);
   glTexCoord3f(pp1x,pp1y,pp1z);
   glVertex3f(pp1x,pp1y,pp1z);
   glTexCoord3f(pp2x,pp2y,pp2z);
   glVertex3f(pp2x,pp2y,pp2z);
   glTexCoord3f(pp4x,pp4y,pp4z);
   glVertex3f(pp4x,pp4y,pp4z);
   glTexCoord3f(pp3x,pp3y,pp3z);
   glVertex3f(pp3x,pp3y,pp3z);
   glEnd();
   }

// intersect tetrahedron with plane
inline void tile::intersecttetra(const float p1x,const float p1y,const float p1z,
                                 const float p2x,const float p2y,const float p2z,
                                 const float p3x,const float p3y,const float p3z,
                                 const float p4x,const float p4y,const float p4z,
                                 const float ox,const float oy,const float oz,
                                 const float nx,const float ny,const float nz)
   {
   int flag;

   float d1,d2,d3,d4,dmin,dmax;

   d1=(p1x-ox)*nx+(p1y-oy)*ny+(p1z-oz)*nz;
   d2=(p2x-ox)*nx+(p2y-oy)*ny+(p2z-oz)*nz;
   d3=(p3x-ox)*nx+(p3y-oy)*ny+(p3z-oz)*nz;
   d4=(p4x-ox)*nx+(p4y-oy)*ny+(p4z-oz)*nz;

   dmin=fmin(fmin(d1,d2),fmin(d3,d4));
   dmax=fmax(fmax(d1,d2),fmax(d3,d4));

   if (dmin*dmax<=0.0f)
      {
      flag=0;

      if (d1<0.0f) flag|=1;
      if (d2<0.0f) flag|=2;
      if (d3<0.0f) flag|=4;
      if (d4<0.0f) flag|=8;

      switch (flag)
         {
         case 1: case 14: intersecttetra1(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
         case 2: case 13: intersecttetra1(p2x,p2y,p2z,fabs(d2),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
         case 4: case 11: intersecttetra1(p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
         case 8: case 7: intersecttetra1(p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;

         case 3: intersecttetra2(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
         case 5: intersecttetra2(p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
         case 6: intersecttetra2(p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4)); break;
         case 9: intersecttetra2(p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;
         case 10: intersecttetra2(p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3)); break;
         case 12: intersecttetra2(p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2)); break;
         }
      }
   }

// intersect hexahedron by breaking it up into 5 tetrahedra
void tile::intersecthexa(const float p1x,const float p1y,const float p1z,
                         const float p2x,const float p2y,const float p2z,
                         const float p3x,const float p3y,const float p3z,
                         const float p4x,const float p4y,const float p4z,
                         const float p5x,const float p5y,const float p5z,
                         const float p6x,const float p6y,const float p6z,
                         const float p7x,const float p7y,const float p7z,
                         const float p8x,const float p8y,const float p8z,
                         const float ox,const float oy,const float oz,
                         const float nx,const float ny,const float nz)
   {
   intersecttetra(p1x,p1y,p1z,p8x,p8y,p8z,p6x,p6y,p6z,p3x,p3y,p3z,ox,oy,oz,nx,ny,nz);
   intersecttetra(p1x,p1y,p1z,p6x,p6y,p6z,p8x,p8y,p8z,p5x,p5y,p5z,ox,oy,oz,nx,ny,nz);
   intersecttetra(p3x,p3y,p3z,p8x,p8y,p8z,p6x,p6y,p6z,p7x,p7y,p7z,ox,oy,oz,nx,ny,nz);
   intersecttetra(p6x,p6y,p6z,p1x,p1y,p1z,p3x,p3y,p3z,p2x,p2y,p2z,ox,oy,oz,nx,ny,nz);
   intersecttetra(p8x,p8y,p8z,p3x,p3y,p3z,p1x,p1y,p1z,p4x,p4y,p4z,ox,oy,oz,nx,ny,nz);
   }

// render a tile slice
void tile::renderslice(float ox,float oy,float oz,
                       float nx,float ny,float nz)
   {
   glDisable(GL_CULL_FACE);

   bindtexmap(BRICK->get_id());

   glMatrixMode(GL_TEXTURE);

   glPushMatrix();
   glLoadIdentity();

   glTranslatef((0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE,(0.5f+BORDER)/BSIZE);
   glScalef((float)(BSIZE-1-2*BORDER)/BSIZE,
            (float)(BSIZE-1-2*BORDER)/BSIZE,
            (float)(BSIZE-1-2*BORDER)/BSIZE);

   glTranslatef(0.5f,0.5f,0.5f);
   glScalef(1.0f/SX,1.0f/SY,1.0f/SZ);
   glTranslatef(-MX,-MY,-MZ);

   glMatrixMode(GL_MODELVIEW);

   intersecthexa(MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                 MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2+0.5f*SZ2,
                 MX2+0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                 MX2-0.5f*SX2,MY2-0.5f*SY2,MZ2-0.5f*SZ2,
                 MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                 MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2+0.5f*SZ2,
                 MX2+0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                 MX2-0.5f*SX2,MY2+0.5f*SY2,MZ2-0.5f*SZ2,
                 ox,oy,oz,nx,ny,nz);

   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   bindtexmap(0);

   glEnable(GL_CULL_FACE);
   }
