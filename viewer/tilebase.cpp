// (c) by Stefan Roettger, licensed under GPL 2+

#define SOBEL
#define MULTILEVEL

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
GLuint tile::PROGID[PROGNUM];

BOOLINT tile::HASFBO=FALSE;

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
      printf("warning: necessary rendering extensions not fully supported\n");

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
         glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,&isNative);

         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);

         if (errorPos==0) printf("warning: fragment programs unavailable\n");
         else
            {
            if (errorPos!=-1)
               {
               printf("fragment program error at position=%d in program #%d\n",errorPos,i+1);
               ERRORMSG();
               }

            if (isNative!=1) printf("warning: fragment program #%d is not native\n",i+1);
            }

#endif

         free(prog[i]);
         }

      HASFBO=FALSE;

      if (strstr(GL_EXTs,"ARB_framebuffer_object")!=NULL)
         {
#ifdef GL_ARB_framebuffer_object

         HASFBO=TRUE;

         // create a texture object
         glGenTextures(1, &textureId);
         glBindTexture(GL_TEXTURE_2D, textureId);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap off
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, 0);
         glBindTexture(GL_TEXTURE_2D, 0);

         // create a renderbuffer object to store depth info
         glGenRenderbuffersARB(1, &rboId);
         glBindRenderbufferARB(GL_RENDERBUFFER, rboId);
         glRenderbufferStorageARB(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                  TEXTURE_WIDTH, TEXTURE_HEIGHT);
         glBindRenderbuffer(GL_RENDERBUFFER, 0);

         // create a framebuffer object
         glGenFramebuffersARB(1, &fboId);
         glBindFramebufferARB(GL_FRAMEBUFFER, fboId);

         // attach the texture to FBO color attachment point
         glFramebufferTexture2DARB(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, textureId, 0);

         // attach the renderbuffer to depth attachment point
         glFramebufferRenderbufferARB(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, rboId);

         // check FBO status
         GLenum status = glCheckFramebufferStatusARB(GL_FRAMEBUFFER);
         if (status != GL_FRAMEBUFFER_COMPLETE) HASFBO=FALSE;

         // switch back to window-system-provided framebuffer
         glBindFramebufferARB(GL_FRAMEBUFFER, 0);

#endif
         }

      LOADED=TRUE;
      }
   }

// destroy fragment programs
void tile::destroy()
   {
#ifdef GL_ARB_fragment_program

   int i;

   if (LOADED)
      {
      for (i=0; i<PROGNUM; i++)
         glDeleteProgramsARB(1,&PROGID[i]);

      LOADED=FALSE;

#ifdef GL_ARB_framebuffer_object

      if (HASFBO)
         {
         glDeleteTextures(1, &textureId);
         glDeleteRenderBuffers(1, rboId);
         glDeleteFrameBuffers(1, fboId);
         }

      HASFBO=FALSE;

#endif
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
                  BOOLINT lighting)
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

   glDepthMask(GL_FALSE);

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

   glDepthMask(GL_TRUE);

   glEnable(GL_CULL_FACE);

   glDisable(GL_BLEND);
   }

// the volume:

volume::volume(tfunc2D *tf,char *base)
   {
   TILEMAX=TILEINC;
   TILE=new tileptr[TILEMAX];
   TILECNT=0;

   TFUNC=tf;

   if (base==NULL) strncpy(BASE,"",MAXSTR);
   else strncpy(BASE,base,MAXSTR);
   }

volume::~volume()
   {
   int i;

   for (i=0; i<TILECNT; i++) delete TILE[i];
   delete TILE;
   }

// check brick size
BOOLINT volume::check(int bricksize,float overmax)
   {
   int border=(int)fceil(overmax);
   return(bricksize>2*border);
   }

// set the volume data
void volume::set_data(unsigned char *data,
                      unsigned char *extra,
                      int width,int height,int depth,
                      float mx,float my,float mz,
                      float sx,float sy,float sz,
                      int bricksize,float overmax)
   {
   int i;

   tileptr *tiles;

   int px,py,pz;

   int border=(int)fceil(overmax);

   float mx2,my2,mz2,
         sx2,sy2,sz2;

   float newsize;

   if (bricksize<=2*border) ERRORMSG();

   for (TILEZ=0,pz=-2*border; pz<depth-1+border; pz+=bricksize-1-2*border,TILEZ++)
      for (TILEY=0,py=-2*border; py<height-1+border; py+=bricksize-1-2*border,TILEY++)
         for (TILEX=0,px=-2*border; px<width-1+border; px+=bricksize-1-2*border,TILEX++)
            {
            sx2=(bricksize-1-2*border)*sx/(width-1);
            sy2=(bricksize-1-2*border)*sy/(height-1);
            sz2=(bricksize-1-2*border)*sz/(depth-1);

            mx2=mx-sx/2.0f+(px+border)*sx/(width-1)+sx2/2.0f;
            my2=my-sy/2.0f+(py+border)*sy/(height-1)+sy2/2.0f;
            mz2=mz-sz/2.0f+(pz+border)*sz/(depth-1)+sz2/2.0f;

            if (TILECNT>=TILEMAX)
               {
               TILEMAX+=TILEINC;
               tiles=new tileptr[TILEMAX];
               for (i=0; i<TILECNT; i++) tiles[i]=TILE[i];
               delete TILE;
               TILE=tiles;
               }

            TILE[TILECNT]=new tile(TFUNC,BASE);

            TILE[TILECNT]->set_data(data,
                                    width,height,depth,
                                    mx2,my2,mz2,
                                    sx2,sy2,sz2,
                                    px,py,pz,
                                    bricksize,border);

            if (extra!=NULL)
               TILE[TILECNT]->set_extra(extra,
                                        width,height,depth,
                                        px,py,pz,
                                        bricksize);

            if (px+bricksize>width+2*border)
               {
               newsize=sx2*(float)(width-1-px)/(bricksize-1-2*border);
               mx2-=(sx2-newsize)/2.0f;
               sx2=newsize;
               }

            if (py+bricksize>height+2*border)
               {
               newsize=sy2*(float)(height-1-py)/(bricksize-1-2*border);
               my2-=(sy2-newsize)/2.0f;
               sy2=newsize;
               }

            if (pz+bricksize>depth+2*border)
               {
               newsize=sz2*(float)(depth-1-pz)/(bricksize-1-2*border);
               mz2-=(sz2-newsize)/2.0f;
               sz2=newsize;
               }

            TILE[TILECNT++]->set_size(mx2,my2,mz2,
                                      sx2,sy2,sz2);
            }

   MX=mx;
   MY=my;
   MZ=mz;

   SX=sx*(width-1+2*border)/(width-1);
   SY=sy*(height-1+2*border)/(height-1);
   SZ=sz*(depth-1+2*border)/(depth-1);

   SLAB=fmin(sx/(width-1),fmin(sy/(height-1),sz/(depth-1)));
   }

// set ambient/diffuse/specular lighting coefficients
void volume::set_light(float noise,float ambnt,float difus,float specl,float specx)
   {
   int i;

   for (i=0; i<TILECNT; i++) TILE[i]->set_light(noise,ambnt,difus,specl,specx);
   }

// sort tiles
void volume::sort(int x,int y,int z,
                  int sx,int sy,int sz,
                  float ex,float ey,float ez,
                  float dx,float dy,float dz,
                  float ux,float uy,float uz,
                  float nearp,float slab,float rslab,
                  BOOLINT lighting)
   {
   tileptr t1,t2;

   if (sx>1)
      {
      t1=TILE[(x+sx/2)+(y+z*TILEY)*TILEX];
      t2=TILE[(x+sx/2-1)+(y+z*TILEY)*TILEX];

      if ((t1->get_mx()+t2->get_mx())/2.0f>ex)
         {
         sort(x+sx/2,y,z,sx-sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x,y,z,sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      else
         {
         sort(x,y,z,sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x+sx/2,y,z,sx-sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      }
   else if (sy>1)
      {
      t1=TILE[x+((y+sy/2)+z*TILEY)*TILEX];
      t2=TILE[x+((y+sy/2-1)+z*TILEY)*TILEX];

      if ((t1->get_my()+t2->get_my())/2.0f>ey)
         {
         sort(x,y+sy/2,z,sx,sy-sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x,y,z,sx,sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      else
         {
         sort(x,y,z,sx,sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x,y+sy/2,z,sx,sy-sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      }
   else if (sz>1)
      {
      t1=TILE[x+(y+(z+sz/2)*TILEY)*TILEX];
      t2=TILE[x+(y+(z+sz/2-1)*TILEY)*TILEX];

      if ((t1->get_mz()+t2->get_mz())/2.0f>ez)
         {
         sort(x,y,z+sz/2,sx,sy,sz-sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x,y,z,sx,sy,sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      else
         {
         sort(x,y,z,sx,sy,sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         sort(x,y,z+sz/2,sx,sy,sz-sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting);
         }
      }
   else
      TILE[x+(y+z*TILEY)*TILEX]->render(ex,ey,ez,
                                        dx,dy,dz,
                                        ux,uy,uz,
                                        nearp,slab,rslab,
                                        lighting);
   }

// render the volume
void volume::render(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float ux,float uy,float uz,
                    float nearp,float slab,float rslab,
                    BOOLINT lighting)
   {
   sort(0,0,0,TILEX,TILEY,TILEZ,
        ex,ey,ez,dx,dy,dz,ux,uy,uz,
        nearp,slab,rslab,
        lighting);
   }

// draw the surrounding wire frame box
void volume::drawwireframe()
   {
   glColor3f(0.5f,0.5f,0.5f);
   glBegin(GL_LINES);
   glVertex3f(-0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(-0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(-0.5f*SX,0.5f*SY,0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,0.5f*SZ);
   glVertex3f(-0.5f*SX,0.5f*SY,0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(-0.5f*SX,-0.5f*SY,-0.5f*SZ);
   glVertex3f(-0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,-0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,-0.5f*SY,0.5f*SZ);
   glVertex3f(0.5f*SX,0.5f*SY,0.5f*SZ);
   glVertex3f(-0.5f*SX,-0.5f*SY,0.5f*SZ);
   glVertex3f(-0.5f*SX,0.5f*SY,0.5f*SZ);
   glEnd();

   glDisable(GL_CULL_FACE);
   glColor3f(0.25f,0.25f,0.25f);
   glBegin(GL_TRIANGLE_FAN);
   glVertex3f(-0.5f*SX,-0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,-0.5f*SY,-0.5f*SZ);
   glVertex3f(0.5f*SX,-0.5f*SY,0.5f*SZ);
   glVertex3f(-0.5f*SX,-0.5f*SY,0.5f*SZ);
   glVertex3f(-0.5f*SX,-0.5f*SY,-0.5f*SZ);
   glEnd();
   glEnable(GL_CULL_FACE);
   }

// the volume hierarchy:

mipmap::mipmap(char *base,int res)
   {
   if (res==0) res=256;

   VOLCNT=0;

   TFUNC=new tfunc2D(res);
   HISTO=new histo;

   VOLUME=GRAD=NULL;

   DSX=DSY=DSZ=1.0f;

   GRADMAX=1.0f;

   if (base==NULL) strncpy(BASE,"",MAXSTR);
   else strncpy(BASE,base,MAXSTR);

   strncpy(filestr,"",MAXSTR);
   strncpy(gradstr,"",MAXSTR);
   strncpy(commstr,"",MAXSTR);
   strncpy(zerostr,"",MAXSTR);

   xsflag=FALSE; ysflag=FALSE; zsflag=FALSE;
   xrflag=FALSE; zrflag=FALSE;

   hmvalue=0.0f; hfvalue=0.0f; hsvalue=0.0f;
   knvalue=0;

   CACHE=NULL;

   CSIZEX=0;
   CSIZEY=0;
   CSLICE=0;
   CSLICES=0;

   QUEUEMAX=QUEUEINC;

   QUEUEX=new int[QUEUEMAX];
   QUEUEY=new int[QUEUEMAX];
   QUEUEZ=new int[QUEUEMAX];
   }

mipmap::~mipmap()
   {
   int i;

   for (i=0; i<VOLCNT; i++) delete VOL[i];
   if (VOLCNT>0) delete VOL;

   delete TFUNC;
   delete HISTO;

   if (VOLUME!=NULL) free(VOLUME);
   if (GRAD!=NULL) free(GRAD);

   if (CACHE!=NULL) delete CACHE;

   delete QUEUEX;
   delete QUEUEY;
   delete QUEUEZ;
   }

// reduce a volume to half its size
unsigned char *mipmap::reduce(unsigned char *data,
                              unsigned int width,unsigned int height,unsigned int depth)
   {
   unsigned int i,j,k;

   unsigned char *data2,*ptr;

   if (data==NULL) return(NULL);

   if ((data2=(unsigned char *)malloc((width/2)*(height/2)*(depth/2)))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<depth-1; k+=2)
      for (j=0; j<height-1; j+=2)
         for (i=0; i<width-1; i+=2)
            *ptr++=((int)data[i+(j+k*height)*width]+
                    (int)data[i+1+(j+k*height)*width]+
                    (int)data[i+(j+1+k*height)*width]+
                    (int)data[i+1+(j+1+k*height)*width]+
                    (int)data[i+(j+(k+1)*height)*width]+
                    (int)data[i+1+(j+(k+1)*height)*width]+
                    (int)data[i+(j+1+(k+1)*height)*width]+
                    (int)data[i+1+(j+1+(k+1)*height)*width]+4)/8;

   return(data2);
   }

// set the volume data
void mipmap::set_data(unsigned char *data,
                      unsigned char *extra,
                      int width,int height,int depth,
                      float mx,float my,float mz,
                      float sx,float sy,float sz,
                      int bricksize,float overmax)
   {
   int i;

   float o;

   unsigned char *data2,*extra2;

   if (VOLCNT!=0)
      {
      for (i=0; i<VOLCNT; i++) delete VOL[i];
      if (VOLCNT>0) delete VOL;
      }

   for (VOLCNT=0,i=bricksize,o=overmax; volume::check(i,o); i/=2,o/=2.0f,VOLCNT++)
      if ((width>>VOLCNT)<=2 || (height>>VOLCNT)<=2 || (depth>>VOLCNT)<=2) break;

   if (VOLCNT==0) ERRORMSG();

   VOL=new volumeptr[VOLCNT];

   VOL[0]=new volume(TFUNC,BASE);

   VOL[0]->set_data(data,
                    extra,
                    width,height,depth,
                    mx,my,mz,
                    sx,sy,sz,
                    bricksize,overmax);

   for (i=1; i<VOLCNT; i++)
      {
      VOL[i]=new volume(TFUNC,BASE);

      data2=reduce(data,width,height,depth);
      extra2=reduce(extra,width,height,depth);

      width/=2;
      height/=2;
      depth/=2;

      bricksize/=2;
      overmax/=2.0f;

      VOL[i]->set_data(data2,
                       extra2,
                       width,height,depth,
                       mx,my,mz,
                       sx,sy,sz,
                       bricksize,overmax);

      if (i>1)
         {
         free(data);
         if (extra!=NULL) free(extra);
         }

      data=data2;
      extra=extra2;
      }

   if (VOLCNT>1)
      {
      free(data);
      if (extra!=NULL) free(extra);
      }
   }

// swap the volume along the axis
unsigned char *mipmap::swap(unsigned char *data,
                            unsigned int *width,unsigned int *height,unsigned int *depth,
                            float *dsx,float *dsy,float *dsz,
                            BOOLINT xswap,BOOLINT yswap,BOOLINT zswap,
                            BOOLINT xrotate,BOOLINT zrotate)
   {
   unsigned int i,j,k;

   unsigned char *data2,*ptr;

   unsigned int size;
   float dim;

   if (xrotate)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*width; j++)
            for (i=0; i<*height; i++)
               *ptr++=data[j+(i+k*(*height))*(*width)];

      size=*width;
      *width=*height;
      *height=size;

      if (dsx!=NULL && dsy!=NULL && dsz!=NULL)
         {
         dim=*dsx;
         *dsx=*dsy;
         *dsy=dim;
         }

      free(data);
      data=data2;
      }

   if (zrotate)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*height; k++)
         for (j=0; j<*depth; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[i+(k+j*(*height))*(*width)];

      size=*height;
      *height=*depth;
      *depth=size;

      if (dsx!=NULL && dsy!=NULL && dsz!=NULL)
         {
         dim=*dsy;
         *dsy=*dsz;
         *dsz=dim;
         }

      free(data);
      data=data2;
      }

   if (xswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*height; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[*width-1-i+(j+k*(*height))*(*width)];

      free(data);
      data=data2;
      }

   if (yswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*height; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[i+(*height-1-j+k*(*height))*(*width)];

      free(data);
      data=data2;
      }

   if (zswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<(*depth); k++)
         for (j=0; j<(*height); j++)
            for (i=0; i<(*width); i++)
               *ptr++=data[i+(j+(*depth-1-k)*(*height))*(*width)];

      free(data);
      data=data2;
      }

   return(data);
   }

// cache a row of slices
void mipmap::cache(const unsigned char *data,
                   unsigned int width,unsigned int height,unsigned int depth,
                   int slice,int slices)
   {
   int i;

   if (slices!=CSLICES)
      {
      if (CACHE!=NULL) delete CACHE;
      CACHE=NULL;

      if (slices>0)
         {
         CACHE=new unsigned char[width*height*(unsigned int)slices];

         for (i=0; i<slices; i++)
            if (slice+i>=0 && slice+i<(int)depth)
               memcpy(&CACHE[width*height*i],&data[width*height*(unsigned int)(slice+i)],width*height);
         }

      CSIZEX=width;
      CSIZEY=height;
      CSLICE=slice;
      CSLICES=slices;
      }

   if (CSLICES>0)
      {
      if (slice>CSLICE)
         for (i=0; i<CSLICES; i++)
            if (slice+i>=0 && slice+i<(int)depth)
               if (slice+i>=CSLICE && slice+i<CSLICE+CSLICES)
                  memcpy(&CACHE[CSIZEX*CSIZEY*i],&CACHE[CSIZEX*CSIZEY*(i+slice-CSLICE)],CSIZEX*CSIZEY);

      if (slice<CSLICE)
         for (i=CSLICES-1; i>=0; i--)
            if (slice+i>=0 && slice+i<(int)depth)
               if (slice+i>=CSLICE && slice+i<CSLICE+CSLICES)
                  memcpy(&CACHE[CSIZEX*CSIZEY*i],&CACHE[CSIZEX*CSIZEY*(i+CSLICE-slice)],CSIZEX*CSIZEY);

      for (i=0; i<CSLICES; i++)
         if (slice+i>=0 && slice+i<(int)depth)
            if (slice+i<CSLICE || slice+i>=CSLICE+CSLICES)
               memcpy(&CACHE[CSIZEX*CSIZEY*i],&data[CSIZEX*CSIZEY*(unsigned int)(slice+i)],CSIZEX*CSIZEY);

      CSLICE=slice;
      }
   }

inline unsigned char mipmap::get(const unsigned char *data,
                                 const unsigned int width,const unsigned int height,const unsigned int depth,
                                 const unsigned int x,const unsigned int y,const unsigned int z)
   {
   if (CACHE!=NULL)
      if ((int)z>=CSLICE && (int)z<CSLICE+CSLICES)
         return(CACHE[x+(y+(z-CSLICE)*height)*width]);

   return(data[x+(y+z*height)*width]);
   }

inline void mipmap::set(unsigned char *data,
                        const unsigned int width,const unsigned int height,const unsigned int depth,
                        const unsigned int x,const unsigned int y,const unsigned int z,unsigned char v)
   {data[x+(y+z*height)*width]=v;}

// calculate the gradient magnitude
unsigned char *mipmap::gradmag(unsigned char *data,
                               unsigned int width,unsigned int height,unsigned int depth,
                               float dsx,float dsy,float dsz,
                               float *gradmax)
   {
   static const float mingrad=0.1f;

   int i,j,k;

   unsigned char *data2,*ptr;

   float gx,gy,gz;
   float gm,gmax;

   float minds;

   minds=fmin(dsx,fmin(dsy,dsz));

   dsx/=minds;
   dsy/=minds;
   dsz/=minds;

   if (dsx>4.0f) dsx=4.0f;
   else if (dsx>2.0f) dsx=2.0f;
   else if (dsx>1.0f) dsx=1.0f;

   if (dsy>4.0f) dsy=4.0f;
   else if (dsy>2.0f) dsy=2.0f;
   else if (dsy>1.0f) dsy=1.0f;

   if (dsz>4.0f) dsz=4.0f;
   else if (dsz>2.0f) dsz=2.0f;
   else if (dsz>1.0f) dsz=1.0f;

   dsx=1.0f/dsx;
   dsy=1.0f/dsy;
   dsz=1.0f/dsz;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (gmax=1.0f,k=0; k<(int)depth; k+=2)
      for (j=0; j<(int)height; j+=2)
         for (i=0; i<(int)width; i+=2)
            {
            if (i>0)
               if (i<(int)width-1) gx=(get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i-1,j,k))/2.0f;
               else gx=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i-1,j,k);
            else gx=get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i,j,k);

            if (j>0)
               if (j<(int)height-1) gy=(get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j-1,k))/2.0f;
               else gy=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j-1,k);
            else gy=get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j,k);

            if (k>0)
               if (k<(int)depth-1) gz=(get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k-1))/2.0f;
               else gz=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j,k-1);
            else gz=get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k);

            gm=fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz);
            if (gm>gmax) gmax=gm;
            }

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
            if (i>0)
               if (i<(int)width-1) gx=(get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i-1,j,k))/2.0f;
               else gx=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i-1,j,k);
            else gx=get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i,j,k);

            if (j>0)
               if (j<(int)height-1) gy=(get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j-1,k))/2.0f;
               else gy=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j-1,k);
            else gy=get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j,k);

            if (k>0)
               if (k<(int)depth-1) gz=(get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k-1))/2.0f;
               else gz=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j,k-1);
            else gz=get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k);

            *ptr++=ftrc(255.0f*threshold(fsqrt(fmin((fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz))/gmax,1.0f)),mingrad)+0.5f);
            }

   if (gradmax!=NULL) *gradmax=fsqrt(gmax)/255.0f;

   return(data2);
   }

inline float mipmap::getgrad(unsigned char *data,
                             int width,int height,int depth,
                             int i,int j,int k,
                             float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v;

   float gx,gy,gz;

   ptr=&data[i+(j+k*height)*width];
   v=*ptr;

   if (i>0) gx=v-ptr[-1];
   else gx=ptr[1]-v;

   if (j>0) gy=v-ptr[-width];
   else gy=ptr[width]-v;

   if (k>0) gz=v-ptr[-width*height];
   else gz=ptr[width*height]-v;

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::getgrad2(unsigned char *data,
                              int width,int height,int depth,
                              int i,int j,int k,
                              float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v;

   float gx,gy,gz;

   ptr=&data[i+(j+k*height)*width];
   v=*ptr;

   if (i>0)
      if (i<width-1) gx=0.5f*(ptr[1]-ptr[-1]);
      else gx=v-ptr[-1];
   else gx=ptr[1]-v;

   if (j>0)
      if (j<height-1) gy=0.5f*(ptr[width]-ptr[-width]);
      else gy=v-ptr[-width];
   else gy=ptr[width]-v;

   if (k>0)
      if (k<depth-1) gz=0.5f*(ptr[width*height]-ptr[-width*height]);
      else gz=v-ptr[-width*height];
   else gz=ptr[width*height]-v;

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::getsobel(unsigned char *data,
                              int width,int height,int depth,
                              int i,int j,int k,
                              float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v0,v[27];

   float gx,gy,gz;

   if (i>0 && i<width-1 && j>0 && j<height-1 && k>0 && k<depth-1)
      {
      ptr=&data[i-1+(j-1+(k-1)*height)*width];

      v[0]=*ptr++;
      v[1]=3*(*ptr++);
      v[2]=*ptr;
      ptr+=width-2;
      v[3]=3*(*ptr++);
      v[4]=6*(*ptr++);
      v[5]=3*(*ptr);
      ptr+=width-2;
      v[6]=*ptr++;
      v[7]=3*(*ptr++);
      v[8]=*ptr;
      ptr+=width*height-2*width-2;
      v[9]=3*(*ptr++);
      v[10]=6*(*ptr++);
      v[11]=3*(*ptr);
      ptr+=width-2;
      v[12]=6*(*ptr++);
      v[13]=*ptr++;
      v[14]=6*(*ptr);
      ptr+=width-2;
      v[15]=3*(*ptr++);
      v[16]=6*(*ptr++);
      v[17]=3*(*ptr);
      ptr+=width*height-2*width-2;
      v[18]=*ptr++;
      v[19]=3*(*ptr++);
      v[20]=*ptr;
      ptr+=width-2;
      v[21]=3*(*ptr++);
      v[22]=6*(*ptr++);
      v[23]=3*(*ptr);
      ptr+=width-2;
      v[24]=*ptr++;
      v[25]=3*(*ptr++);
      v[26]=*ptr;

      gx=(-v[0]-v[3]-v[6]-v[9]-v[12]-v[15]-v[18]-v[21]-v[24]+v[2]+v[5]+v[8]+v[11]+v[14]+v[17]+v[20]+v[23]+v[26])/44.0f;
      gy=(-v[0]-v[1]-v[2]-v[9]-v[10]-v[11]-v[18]-v[19]-v[20]+v[6]+v[7]+v[8]+v[15]+v[16]+v[17]+v[24]+v[25]+v[26])/44.0f;
      gz=(-v[0]-v[1]-v[2]-v[3]-v[4]-v[5]-v[6]-v[7]-v[8]+v[18]+v[19]+v[20]+v[21]+v[22]+v[23]+v[24]+v[25]+v[26])/44.0f;
      }
   else
      {
      ptr=&data[i+(j+k*height)*width];
      v0=*ptr;

      if (i>0)
         if (i<width-1) gx=0.5f*(ptr[1]-ptr[-1]);
         else gx=v0-ptr[-1];
      else gx=ptr[1]-v0;

      if (j>0)
         if (j<height-1) gy=0.5f*(ptr[width]-ptr[-width]);
         else gy=v0-ptr[-width];
      else gy=ptr[width]-v0;

      if (k>0)
         if (k<depth-1) gz=0.5f*(ptr[width*height]-ptr[-width*height]);
         else gz=v0-ptr[-width*height];
      else gz=ptr[width*height]-v0;
      }

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::threshold(float x,float thres)
   {
   if (x>=thres) return(x);
   return(x*fexp(-3.0f*fsqr((thres-x)/thres)));
   }

// calculate the gradient magnitude with multi-level averaging
unsigned char *mipmap::gradmagML(unsigned char *data,
                                 unsigned int width,unsigned int height,unsigned int depth,
                                 float dsx,float dsy,float dsz,
                                 float *gradmax)
   {
   static const int maxlevel=2;
   static const float mingrad=0.1f;

   int i,j,k;

   float *data2,*ptr2;
   float *data3,*ptr3;
   unsigned char *data4,*ptr4;
   unsigned char *data5,*ptr5;

   int width2,height2,depth2;

   float gm,gmax,gmax2;

   int level;
   float weight;

   float minds;

   minds=fmin(dsx,fmin(dsy,dsz));

   dsx/=minds;
   dsy/=minds;
   dsz/=minds;

   if (dsx>4.0f) dsx=4.0f;
   else if (dsx>2.0f) dsx=2.0f;
   else if (dsx>1.0f) dsx=1.0f;

   if (dsy>4.0f) dsy=4.0f;
   else if (dsy>2.0f) dsy=2.0f;
   else if (dsy>1.0f) dsy=1.0f;

   if (dsz>4.0f) dsz=4.0f;
   else if (dsz>2.0f) dsz=2.0f;
   else if (dsz>1.0f) dsz=1.0f;

   dsx=1.0f/dsx;
   dsy=1.0f/dsy;
   dsz=1.0f/dsz;

   if ((data2=(float *)malloc(width*height*depth*sizeof(float)))==NULL) ERRORMSG();

   gmax=0.0f;

   for (ptr2=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
#ifndef SOBEL
            gm=getgrad(data,width,height,depth,i,j,k,dsx,dsy,dsz);
#else
            gm=getsobel(data,width,height,depth,i,j,k,dsx,dsy,dsz);
#endif
            if (gm>gmax) gmax=gm;

            *ptr2++=gm;
            }

   if (gmax==0.0f) gmax=1.0f;

   for (ptr2=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
            gm=*ptr2;
            *ptr2++=threshold(gm/gmax,mingrad);
            }

   width2=width;
   height2=height;
   depth2=depth;

   data4=ptr4=data;

   level=0;
   weight=1.0f;

   gmax2=0.0f;

   while (width2>5 && height2>5 && depth2>5 && level<maxlevel)
      {
      data5=reduce(data4,width2,height2,depth2);
      if (data4!=data) free(data4);
      data4=data5;

      width2=width2/2;
      height2=height2/2;
      depth2=depth2/2;

      level++;
      weight*=0.5f;

      if ((data3=(float *)malloc(width2*height2*depth2*sizeof(float)))==NULL) ERRORMSG();

      for (ptr3=data3,k=0; k<depth2; k++)
         for (j=0; j<height2; j++)
            for (i=0; i<width2; i++)
            {
#ifndef SOBEL
            gm=getgrad(data4,width2,height2,depth2,i,j,k,dsx,dsy,dsz);
#else
            gm=getsobel(data4,width2,height2,depth2,i,j,k,dsx,dsy,dsz);
#endif
            *ptr3++=gm/gmax;
            }

      for (ptr2=data2,k=0; k<(int)depth; k++)
         for (j=0; j<(int)height; j++)
            for (i=0; i<(int)width; i++)
               {
               gm=*ptr2;
               gm+=weight*threshold(getscalar(data3,width2,height2,depth2,(float)i/(width-1),(float)j/(height-1),(float)k/(depth-1)),mingrad);
               if (gm>gmax2) gmax2=gm;

               *ptr2++=gm;
               }

      free(data3);
      }

   if (gmax2==0.0f) gmax2=1.0f;

   if (data4!=data) free(data4);

   if ((data5=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr2=data2,ptr5=data5,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
            gm=*ptr2++;
            *ptr5++=ftrc(255.0f*gm/gmax2+0.5f);
            }

   free(data2);

   if (gradmax!=NULL) *gradmax=gmax2/255.0f;

   return(data5);
   }

// calculate the variance
unsigned char *mipmap::variance(unsigned char *data,
                                unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *data2,*ptr;

   int val,cnt;

   int dev,dmax;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (dmax=1,k=0; k<(int)depth; k+=2)
      for (j=0; j<(int)height; j+=2)
         for (i=0; i<(int)width; i+=2)
            {
            // central voxel
            val=get(data,width,height,depth,i,j,k);
            dev=cnt=0;

            // x-axis
            if (i>0) {dev+=abs(get(data,width,height,depth,i-1,j,k)-val); cnt++;}
            if (i<(int)width-1) {dev+=abs(get(data,width,height,depth,i+1,j,k)-val); cnt++;}

            // y-axis
            if (j>0) {dev+=abs(get(data,width,height,depth,i,j-1,k)-val); cnt++;}
            if (j<(int)height-1) {dev+=abs(get(data,width,height,depth,i,j+1,k)-val); cnt++;}

            // z-axis
            if (k>0) {dev+=abs(get(data,width,height,depth,i,j,k-1)-val); cnt++;}
            if (k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j,k+1)-val); cnt++;}

            // xy-plane
            if (i>0 && j>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k)-val); cnt++;}
            if (i<(int)width-1 && j>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k)-val); cnt++;}
            if (i>0 && j<(int)height-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k)-val); cnt++;}

            // xz-plane
            if (i>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j,k-1)-val); cnt++;}
            if (i<(int)width-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j,k-1)-val); cnt++;}
            if (i>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j,k+1)-val); cnt++;}
            if (i<(int)width-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j,k+1)-val); cnt++;}

            // yz-plane
            if (j>0 && k>0) {dev+=abs(get(data,width,height,depth,i,j-1,k-1)-val); cnt++;}
            if (j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i,j+1,k-1)-val); cnt++;}
            if (j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j-1,k+1)-val); cnt++;}
            if (j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j+1,k+1)-val); cnt++;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k-1)-val); cnt++;}
            if (i<(int)width-1 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k-1)-val); cnt++;}
            if (i>0 && j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j+1,k-1)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j+1,k-1)-val); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j-1,k+1)-val); cnt++;}
            if (i<(int)width-1 && j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j-1,k+1)-val); cnt++;}
            if (i>0 && j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k+1)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k+1)-val); cnt++;}

            dev=(dev+cnt/2)/cnt;
            if (dev>dmax) dmax=dev;
            }

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
            // central voxel
            val=get(data,width,height,depth,i,j,k);
            dev=cnt=0;

            // x-axis
            if (i>0) {dev+=abs(get(data,width,height,depth,i-1,j,k)-val); cnt++;}
            if (i<(int)width-1) {dev+=abs(get(data,width,height,depth,i+1,j,k)-val); cnt++;}

            // y-axis
            if (j>0) {dev+=abs(get(data,width,height,depth,i,j-1,k)-val); cnt++;}
            if (j<(int)height-1) {dev+=abs(get(data,width,height,depth,i,j+1,k)-val); cnt++;}

            // z-axis
            if (k>0) {dev+=abs(get(data,width,height,depth,i,j,k-1)-val); cnt++;}
            if (k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j,k+1)-val); cnt++;}

            // xy-plane
            if (i>0 && j>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k)-val); cnt++;}
            if (i<(int)width-1 && j>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k)-val); cnt++;}
            if (i>0 && j<(int)height-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k)-val); cnt++;}

            // xz-plane
            if (i>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j,k-1)-val); cnt++;}
            if (i<(int)width-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j,k-1)-val); cnt++;}
            if (i>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j,k+1)-val); cnt++;}
            if (i<(int)width-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j,k+1)-val); cnt++;}

            // yz-plane
            if (j>0 && k>0) {dev+=abs(get(data,width,height,depth,i,j-1,k-1)-val); cnt++;}
            if (j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i,j+1,k-1)-val); cnt++;}
            if (j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j-1,k+1)-val); cnt++;}
            if (j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i,j+1,k+1)-val); cnt++;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k-1)-val); cnt++;}
            if (i<(int)width-1 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k-1)-val); cnt++;}
            if (i>0 && j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j+1,k-1)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j+1,k-1)-val); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j-1,k+1)-val); cnt++;}
            if (i<(int)width-1 && j>0 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j-1,k+1)-val); cnt++;}
            if (i>0 && j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k+1)-val); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k+1)-val); cnt++;}

            dev=(dev+cnt/2)/cnt;
            *ptr++=ftrc(255.0f*fmin((float)dev/dmax,1.0f)+0.5f);
            }

   return(data2);
   }

// blur the volume
void mipmap::blur(unsigned char *data,
                  unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *ptr;

   int val,cnt;

   for (ptr=data,k=0; k<(int)depth; k++)
      {
      cache(data,width,height,depth,k-1,2);

      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            {
            // central voxel
            val=54*get(data,width,height,depth,i,j,k);
            cnt=54;

            // x-axis
            if (i>0) {val+=6*get(data,width,height,depth,i-1,j,k); cnt+=6;}
            if (i<(int)width-1) {val+=6*get(data,width,height,depth,i+1,j,k); cnt+=6;}

            // y-axis
            if (j>0) {val+=6*get(data,width,height,depth,i,j-1,k); cnt+=6;}
            if (j<(int)height-1) {val+=6*get(data,width,height,depth,i,j+1,k); cnt+=6;}

            // z-axis
            if (k>0) {val+=6*get(data,width,height,depth,i,j,k-1); cnt+=6;}
            if (k<(int)depth-1) {val+=6*get(data,width,height,depth,i,j,k+1); cnt+=6;}

            // xy-plane
            if (i>0 && j>0) {val+=2*get(data,width,height,depth,i-1,j-1,k); cnt+=2;}
            if (i<(int)width-1 && j>0) {val+=2*get(data,width,height,depth,i+1,j-1,k); cnt+=2;}
            if (i>0 && j<(int)height-1) {val+=2*get(data,width,height,depth,i-1,j+1,k); cnt+=2;}
            if (i<(int)width-1 && j<(int)height-1) {val+=2*get(data,width,height,depth,i+1,j+1,k); cnt+=2;}

            // xz-plane
            if (i>0 && k>0) {val+=2*get(data,width,height,depth,i-1,j,k-1); cnt+=2;}
            if (i<(int)width-1 && k>0) {val+=2*get(data,width,height,depth,i+1,j,k-1); cnt+=2;}
            if (i>0 && k<(int)depth-1) {val+=2*get(data,width,height,depth,i-1,j,k+1); cnt+=2;}
            if (i<(int)width-1 && k<(int)depth-1) {val+=2*get(data,width,height,depth,i+1,j,k+1); cnt+=2;}

            // yz-plane
            if (j>0 && k>0) {val+=2*get(data,width,height,depth,i,j-1,k-1); cnt+=2;}
            if (j<(int)height-1 && k>0) {val+=2*get(data,width,height,depth,i,j+1,k-1); cnt+=2;}
            if (j>0 && k<(int)depth-1) {val+=2*get(data,width,height,depth,i,j-1,k+1); cnt+=2;}
            if (j<(int)height-1 && k<(int)depth-1) {val+=2*get(data,width,height,depth,i,j+1,k+1); cnt+=2;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {val+=get(data,width,height,depth,i-1,j-1,k-1); cnt++;}
            if (i<(int)width-1 && j>0 && k>0) {val+=get(data,width,height,depth,i+1,j-1,k-1); cnt++;}
            if (i>0 && j<(int)height-1 && k>0) {val+=get(data,width,height,depth,i-1,j+1,k-1); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k>0) {val+=get(data,width,height,depth,i+1,j+1,k-1); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<(int)depth-1) {val+=get(data,width,height,depth,i-1,j-1,k+1); cnt++;}
            if (i<(int)width-1 && j>0 && k<(int)depth-1) {val+=get(data,width,height,depth,i+1,j-1,k+1); cnt++;}
            if (i>0 && j<(int)height-1 && k<(int)depth-1) {val+=get(data,width,height,depth,i-1,j+1,k+1); cnt++;}
            if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) {val+=get(data,width,height,depth,i+1,j+1,k+1); cnt++;}

            *ptr++=(val+cnt/2)/cnt;
            }
      }

   cache();
   }

// set gradient to maximum where transfer function is transparent
void mipmap::usetf(unsigned char *data,unsigned char *grad,
                   unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *ptr1,*ptr2;

   int mindata,maxdata;

   int val,nbg[26];

   int oldmode;

   oldmode=TFUNC->get_mode();

   if (!TFUNC->get_imode()) TFUNC->set_mode(0);

   TFUNC->preint(TRUE);

   for (ptr1=data,ptr2=grad,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr1++,ptr2++)
            {
            // central voxel
            mindata=maxdata=*ptr1;

            // x-axis:

            if (i>0) nbg[0]=get(data,width,height,depth,i-1,j,k); else nbg[0]=*ptr1;
            if (i<(int)width-1) nbg[1]=get(data,width,height,depth,i+1,j,k); else nbg[1]=*ptr1;

            val=(*ptr1)+nbg[0]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[1]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // y-axis:

            if (j>0) nbg[2]=get(data,width,height,depth,i,j-1,k); else nbg[2]=*ptr1;
            if (j<(int)height-1) nbg[3]=get(data,width,height,depth,i,j+1,k); else nbg[3]=*ptr1;

            val=(*ptr1)+nbg[2]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[3]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // z-axis:

            if (k>0) nbg[4]=get(data,width,height,depth,i,j,k-1); else nbg[4]=*ptr1;
            if (k<(int)depth-1) nbg[5]=get(data,width,height,depth,i,j,k+1); else nbg[5]=*ptr1;

            val=(*ptr1)+nbg[4]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[5]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // xy-plane:

            if (i>0 && j>0) nbg[6]=get(data,width,height,depth,i-1,j-1,k); else nbg[6]=*ptr1;
            if (i<(int)width-1 && j>0) nbg[7]=get(data,width,height,depth,i+1,j-1,k); else nbg[7]=*ptr1;
            if (i>0 && j<(int)height-1) nbg[8]=get(data,width,height,depth,i-1,j+1,k); else nbg[8]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1) nbg[9]=get(data,width,height,depth,i+1,j+1,k); else nbg[9]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // xz-plane:

            if (i>0 && k>0) nbg[10]=get(data,width,height,depth,i-1,j,k-1); else nbg[10]=*ptr1;
            if (i<(int)width-1 && k>0) nbg[11]=get(data,width,height,depth,i+1,j,k-1); else nbg[11]=*ptr1;
            if (i>0 && k<(int)depth-1) nbg[12]=get(data,width,height,depth,i-1,j,k+1); else nbg[12]=*ptr1;
            if (i<(int)width-1 && k<(int)depth-1) nbg[13]=get(data,width,height,depth,i+1,j,k+1); else nbg[13]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[4]+nbg[10]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[4]+nbg[11]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[5]+nbg[12]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[5]+nbg[13]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // yz-plane:

            if (j>0 && k>0) nbg[14]=get(data,width,height,depth,i,j-1,k-1); else nbg[14]=*ptr1;
            if (j<(int)height-1 && k>0) nbg[15]=get(data,width,height,depth,i,j+1,k-1); else nbg[15]=*ptr1;
            if (j>0 && k<(int)depth-1) nbg[16]=get(data,width,height,depth,i,j-1,k+1); else nbg[16]=*ptr1;
            if (j<(int)height-1 && k<(int)depth-1) nbg[17]=get(data,width,height,depth,i,j+1,k+1); else nbg[17]=*ptr1;

            val=(*ptr1)+nbg[2]+nbg[4]+nbg[14]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[4]+nbg[15]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[2]+nbg[5]+nbg[16]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[5]+nbg[17]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // bottom xy-plane:

            if (i>0 && j>0 && k>0) nbg[18]=get(data,width,height,depth,i-1,j-1,k-1); else nbg[18]=*ptr1;
            if (i<(int)width-1 && j>0 && k>0) nbg[19]=get(data,width,height,depth,i+1,j-1,k-1); else nbg[19]=*ptr1;
            if (i>0 && j<(int)height-1 && k>0) nbg[20]=get(data,width,height,depth,i-1,j+1,k-1); else nbg[20]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1 && k>0) nbg[21]=get(data,width,height,depth,i+1,j+1,k-1); else nbg[21]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[4]+nbg[10]+nbg[14]+nbg[18]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[4]+nbg[11]+nbg[14]+nbg[19]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[4]+nbg[10]+nbg[15]+nbg[20]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[4]+nbg[11]+nbg[15]+nbg[21]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // top xy-plane:

            if (i>0 && j>0 && k<(int)depth-1) nbg[22]=get(data,width,height,depth,i-1,j-1,k+1); else nbg[22]=*ptr1;
            if (i<(int)width-1 && j>0 && k<(int)depth-1) nbg[23]=get(data,width,height,depth,i+1,j-1,k+1); else nbg[23]=*ptr1;
            if (i>0 && j<(int)height-1 && k<(int)depth-1) nbg[24]=get(data,width,height,depth,i-1,j+1,k+1); else nbg[24]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) nbg[25]=get(data,width,height,depth,i+1,j+1,k+1); else nbg[25]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[5]+nbg[12]+nbg[16]+nbg[22]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[5]+nbg[13]+nbg[16]+nbg[23]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[5]+nbg[12]+nbg[17]+nbg[24]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[5]+nbg[13]+nbg[17]+nbg[25]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // check for transparency
            if (TFUNC->zot(mindata/255.0f,maxdata/255.0f,(*ptr2)/255.0f,(*ptr2)/255.0f)) *ptr2=255;
            else *ptr2/=2;
            }

   TFUNC->set_mode(oldmode);
   }

// set gradient to maximum where opacity is zero
void mipmap::useop(unsigned char *data,unsigned char *grad,
                   unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *ptr1,*ptr2;

   int mindata,maxdata;

   int val,nbg[26];

   int oldmode;

   oldmode=TFUNC->get_mode();

   if (!TFUNC->get_imode()) TFUNC->set_mode(0);

   TFUNC->premin();

   for (ptr1=data,ptr2=grad,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr1++,ptr2++)
            {
            // central voxel
            mindata=maxdata=*ptr1;

            // x-axis:

            if (i>0) nbg[0]=get(data,width,height,depth,i-1,j,k); else nbg[0]=*ptr1;
            if (i<(int)width-1) nbg[1]=get(data,width,height,depth,i+1,j,k); else nbg[1]=*ptr1;

            val=(*ptr1)+nbg[0]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[1]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // y-axis:

            if (j>0) nbg[2]=get(data,width,height,depth,i,j-1,k); else nbg[2]=*ptr1;
            if (j<(int)height-1) nbg[3]=get(data,width,height,depth,i,j+1,k); else nbg[3]=*ptr1;

            val=(*ptr1)+nbg[2]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[3]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // z-axis:

            if (k>0) nbg[4]=get(data,width,height,depth,i,j,k-1); else nbg[4]=*ptr1;
            if (k<(int)depth-1) nbg[5]=get(data,width,height,depth,i,j,k+1); else nbg[5]=*ptr1;

            val=(*ptr1)+nbg[4]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[5]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // xy-plane:

            if (i>0 && j>0) nbg[6]=get(data,width,height,depth,i-1,j-1,k); else nbg[6]=*ptr1;
            if (i<(int)width-1 && j>0) nbg[7]=get(data,width,height,depth,i+1,j-1,k); else nbg[7]=*ptr1;
            if (i>0 && j<(int)height-1) nbg[8]=get(data,width,height,depth,i-1,j+1,k); else nbg[8]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1) nbg[9]=get(data,width,height,depth,i+1,j+1,k); else nbg[9]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // xz-plane:

            if (i>0 && k>0) nbg[10]=get(data,width,height,depth,i-1,j,k-1); else nbg[10]=*ptr1;
            if (i<(int)width-1 && k>0) nbg[11]=get(data,width,height,depth,i+1,j,k-1); else nbg[11]=*ptr1;
            if (i>0 && k<(int)depth-1) nbg[12]=get(data,width,height,depth,i-1,j,k+1); else nbg[12]=*ptr1;
            if (i<(int)width-1 && k<(int)depth-1) nbg[13]=get(data,width,height,depth,i+1,j,k+1); else nbg[13]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[4]+nbg[10]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[4]+nbg[11]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[5]+nbg[12]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[5]+nbg[13]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // yz-plane:

            if (j>0 && k>0) nbg[14]=get(data,width,height,depth,i,j-1,k-1); else nbg[14]=*ptr1;
            if (j<(int)height-1 && k>0) nbg[15]=get(data,width,height,depth,i,j+1,k-1); else nbg[15]=*ptr1;
            if (j>0 && k<(int)depth-1) nbg[16]=get(data,width,height,depth,i,j-1,k+1); else nbg[16]=*ptr1;
            if (j<(int)height-1 && k<(int)depth-1) nbg[17]=get(data,width,height,depth,i,j+1,k+1); else nbg[17]=*ptr1;

            val=(*ptr1)+nbg[2]+nbg[4]+nbg[14]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[4]+nbg[15]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[2]+nbg[5]+nbg[16]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[5]+nbg[17]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // bottom xy-plane:

            if (i>0 && j>0 && k>0) nbg[18]=get(data,width,height,depth,i-1,j-1,k-1); else nbg[18]=*ptr1;
            if (i<(int)width-1 && j>0 && k>0) nbg[19]=get(data,width,height,depth,i+1,j-1,k-1); else nbg[19]=*ptr1;
            if (i>0 && j<(int)height-1 && k>0) nbg[20]=get(data,width,height,depth,i-1,j+1,k-1); else nbg[20]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1 && k>0) nbg[21]=get(data,width,height,depth,i+1,j+1,k-1); else nbg[21]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[4]+nbg[10]+nbg[14]+nbg[18]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[4]+nbg[11]+nbg[14]+nbg[19]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[4]+nbg[10]+nbg[15]+nbg[20]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[4]+nbg[11]+nbg[15]+nbg[21]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // top xy-plane:

            if (i>0 && j>0 && k<(int)depth-1) nbg[22]=get(data,width,height,depth,i-1,j-1,k+1); else nbg[22]=*ptr1;
            if (i<(int)width-1 && j>0 && k<(int)depth-1) nbg[23]=get(data,width,height,depth,i+1,j-1,k+1); else nbg[23]=*ptr1;
            if (i>0 && j<(int)height-1 && k<(int)depth-1) nbg[24]=get(data,width,height,depth,i-1,j+1,k+1); else nbg[24]=*ptr1;
            if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) nbg[25]=get(data,width,height,depth,i+1,j+1,k+1); else nbg[25]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[5]+nbg[12]+nbg[16]+nbg[22]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[5]+nbg[13]+nbg[16]+nbg[23]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[5]+nbg[12]+nbg[17]+nbg[24]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[5]+nbg[13]+nbg[17]+nbg[25]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // check for transparency
            if (TFUNC->mot(mindata/255.0f,maxdata/255.0f,(*ptr2)/255.0f,(*ptr2)/255.0f)) *ptr2=255;
            else *ptr2/=2;
            }

   TFUNC->set_mode(oldmode);
   }

// remove bubbles
void mipmap::remove(unsigned char *grad,
                    unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *ptr;

   int nbg,cnt;

   for (ptr=grad,k=0; k<(int)depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr++)
            if (*ptr==255)
               {
               nbg=cnt=0;

               if (i>0) {if (get(grad,width,height,depth,i-1,j,k)==255) nbg++; cnt++;}
               if (i<(int)width-1) {if (get(grad,width,height,depth,i+1,j,k)==255) nbg++; cnt++;}

               if (j>0) {if (get(grad,width,height,depth,i,j-1,k)==255) nbg++; cnt++;}
               if (j<(int)height-1) {if (get(grad,width,height,depth,i,j+1,k)==255) nbg++; cnt++;}

               if (k>0) {if (get(grad,width,height,depth,i,j,k-1)==255) nbg++; cnt++;}
               if (k<(int)depth-1) {if (get(grad,width,height,depth,i,j,k+1)==255) nbg++; cnt++;}

               if (nbg==cnt) *ptr=0;
               }
      }

   cache();
   }

// tangle material
void mipmap::tangle(unsigned char *grad,
                    unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;

   unsigned char *ptr;

   int cnt;

   for (ptr=grad,k=0; k<(int)depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr++)
            if (*ptr==255)
               {
               cnt=0;

               // x-axis
               if (i>0) if (get(grad,width,height,depth,i-1,j,k)<255) cnt++;
               if (i<(int)width-1) if (get(grad,width,height,depth,i+1,j,k)<255) cnt++;

               // y-axis
               if (j>0) if (get(grad,width,height,depth,i,j-1,k)<255) cnt++;
               if (j<(int)height-1) if (get(grad,width,height,depth,i,j+1,k)<255) cnt++;

               // z-axis
               if (k>0) if (get(grad,width,height,depth,i,j,k-1)<255) cnt++;
               if (k<(int)depth-1) if (get(grad,width,height,depth,i,j,k+1)<255) cnt++;

               // xy-plane
               if (i>0 && j>0) if (get(grad,width,height,depth,i-1,j-1,k)<255) cnt++;
               if (i<(int)width-1 && j>0) if (get(grad,width,height,depth,i+1,j-1,k)<255) cnt++;
               if (i>0 && j<(int)height-1) if (get(grad,width,height,depth,i-1,j+1,k)<255) cnt++;
               if (i<(int)width-1 && j<(int)height-1) if (get(grad,width,height,depth,i+1,j+1,k)<255) cnt++;

               // xz-plane
               if (i>0 && k>0) if (get(grad,width,height,depth,i-1,j,k-1)<255) cnt++;
               if (i<(int)width-1 && k>0) if (get(grad,width,height,depth,i+1,j,k-1)<255) cnt++;
               if (i>0 && k<(int)depth-1) if (get(grad,width,height,depth,i-1,j,k+1)<255) cnt++;
               if (i<(int)width-1 && k<(int)depth-1) if (get(grad,width,height,depth,i+1,j,k+1)<255) cnt++;

               // yz-plane
               if (j>0 && k>0) if (get(grad,width,height,depth,i,j-1,k-1)<255) cnt++;
               if (j<(int)height-1 && k>0) if (get(grad,width,height,depth,i,j+1,k-1)<255) cnt++;
               if (j>0 && k<(int)depth-1) if (get(grad,width,height,depth,i,j-1,k+1)<255) cnt++;
               if (j<(int)height-1 && k<(int)depth-1) if (get(grad,width,height,depth,i,j+1,k+1)<255) cnt++;

               // bottom xy-plane
               if (i>0 && j>0 && k>0) if (get(grad,width,height,depth,i-1,j-1,k-1)<255) cnt++;
               if (i<(int)width-1 && j>0 && k>0) if (get(grad,width,height,depth,i+1,j-1,k-1)<255) cnt++;
               if (i>0 && j<(int)height-1 && k>0) if (get(grad,width,height,depth,i-1,j+1,k-1)<255) cnt++;
               if (i<(int)width-1 && j<(int)height-1 && k>0) if (get(grad,width,height,depth,i+1,j+1,k-1)<255) cnt++;

               // top xy-plane
               if (i>0 && j>0 && k<(int)depth-1) if (get(grad,width,height,depth,i-1,j-1,k+1)<255) cnt++;
               if (i<(int)width-1 && j>0 && k<(int)depth-1) if (get(grad,width,height,depth,i+1,j-1,k+1)<255) cnt++;
               if (i>0 && j<(int)height-1 && k<(int)depth-1) if (get(grad,width,height,depth,i-1,j+1,k+1)<255) cnt++;
               if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) if (get(grad,width,height,depth,i+1,j+1,k+1)<255) cnt++;

               if (cnt>0) *ptr=0;
               }
      }

   cache();
   }

// grow material
unsigned int mipmap::grow(unsigned char *grad,
                          unsigned int width,unsigned int height,unsigned int depth)
   {
   int i,j,k;
   int v,c;

   unsigned char *ptr;

   int cnt,maxcnt;

   int val,nbg[26];

   unsigned int found;

   found=0;

   for (ptr=grad,k=0; k<(int)depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++,ptr++)
            if (*ptr==0)
               {
               cnt=0;

               // x-axis
               if (i>0) {val=get(grad,width,height,depth,i-1,j,k); if (val>0) cnt++; nbg[0]=val;} else nbg[0]=0;
               if (i<(int)width-1) {val=get(grad,width,height,depth,i+1,j,k); if (val>0) cnt++; nbg[1]=val;} else nbg[1]=0;

               // y-axis
               if (j>0) {val=get(grad,width,height,depth,i,j-1,k); if (val>0) cnt++; nbg[2]=val;} else nbg[2]=0;
               if (j<(int)height-1) {val=get(grad,width,height,depth,i,j+1,k); if (val>0) cnt++; nbg[3]=val;} else nbg[3]=0;

               // z-axis
               if (k>0) {val=get(grad,width,height,depth,i,j,k-1); if (val>0) cnt++; nbg[4]=val;} else nbg[4]=0;
               if (k<(int)depth-1) {val=get(grad,width,height,depth,i,j,k+1); if (val>0) cnt++; nbg[5]=val;} else nbg[5]=0;

               // xy-plane
               if (i>0 && j>0) {val=get(grad,width,height,depth,i-1,j-1,k); if (val>0) cnt++; nbg[6]=val;} else nbg[6]=0;
               if (i<(int)width-1 && j>0) {val=get(grad,width,height,depth,i+1,j-1,k); if (val>0) cnt++; nbg[7]=val;} else nbg[7]=0;
               if (i>0 && j<(int)height-1) {val=get(grad,width,height,depth,i-1,j+1,k); if (val>0) cnt++; nbg[8]=val;} else nbg[8]=0;
               if (i<(int)width-1 && j<(int)height-1) {val=get(grad,width,height,depth,i+1,j+1,k); if (val>0) cnt++; nbg[9]=val;} else nbg[9]=0;

               // xz-plane
               if (i>0 && k>0) {val=get(grad,width,height,depth,i-1,j,k-1); if (val>0) cnt++; nbg[10]=val;} else nbg[10]=0;
               if (i<(int)width-1 && k>0) {val=get(grad,width,height,depth,i+1,j,k-1); if (val>0) cnt++; nbg[11]=val;} else nbg[11]=0;
               if (i>0 && k<(int)depth-1) {val=get(grad,width,height,depth,i-1,j,k+1); if (val>0) cnt++; nbg[12]=val;} else nbg[12]=0;
               if (i<(int)width-1 && k<(int)depth-1) {val=get(grad,width,height,depth,i+1,j,k+1); if (val>0) cnt++; nbg[13]=val;} else nbg[13]=0;

               // yz-plane
               if (j>0 && k>0) {val=get(grad,width,height,depth,i,j-1,k-1); if (val>0) cnt++; nbg[14]=val;} else nbg[14]=0;
               if (j<(int)height-1 && k>0) {val=get(grad,width,height,depth,i,j+1,k-1); if (val>0) cnt++; nbg[15]=val;} else nbg[15]=0;
               if (j>0 && k<(int)depth-1) {val=get(grad,width,height,depth,i,j-1,k+1); if (val>0) cnt++; nbg[16]=val;} else nbg[16]=0;
               if (j<(int)height-1 && k<(int)depth-1) {val=get(grad,width,height,depth,i,j+1,k+1); if (val>0) cnt++; nbg[17]=val;} else nbg[17]=0;

               // bottom xy-plane
               if (i>0 && j>0 && k>0) {val=get(grad,width,height,depth,i-1,j-1,k-1); if (val>0) cnt++; nbg[18]=val;} else nbg[18]=0;
               if (i<(int)width-1 && j>0 && k>0) {val=get(grad,width,height,depth,i+1,j-1,k-1); if (val>0) cnt++; nbg[19]=val;} else nbg[19]=0;
               if (i>0 && j<(int)height-1 && k>0) {val=get(grad,width,height,depth,i-1,j+1,k-1); if (val>0) cnt++; nbg[20]=val;} else nbg[20]=0;
               if (i<(int)width-1 && j<(int)height-1 && k>0) {val=get(grad,width,height,depth,i+1,j+1,k-1); if (val>0) cnt++; nbg[21]=val;} else nbg[21]=0;

               // top xy-plane
               if (i>0 && j>0 && k<(int)depth-1) {val=get(grad,width,height,depth,i-1,j-1,k+1); if (val>0) cnt++; nbg[22]=val;} else nbg[22]=0;
               if (i<(int)width-1 && j>0 && k<(int)depth-1) {val=get(grad,width,height,depth,i+1,j-1,k+1); if (val>0) cnt++; nbg[23]=val;} else nbg[23]=0;
               if (i>0 && j<(int)height-1 && k<(int)depth-1) {val=get(grad,width,height,depth,i-1,j+1,k+1); if (val>0) cnt++; nbg[24]=val;} else nbg[24]=0;
               if (i<(int)width-1 && j<(int)height-1 && k<(int)depth-1) {val=get(grad,width,height,depth,i+1,j+1,k+1); if (val>0) cnt++; nbg[25]=val;} else nbg[25]=0;

               if (cnt>0)
                  {
                  val=0;
                  maxcnt=0;

                  for (v=0; v<26; v++)
                     if (nbg[v]>0)
                        {
                        cnt=1;

                        for (c=v+1; c<26; c++)
                           if (nbg[c]==nbg[v])
                              {
                              cnt++;
                              nbg[c]=0;
                              }

                        if (cnt>maxcnt)
                           {
                           maxcnt=cnt;
                           val=nbg[v];
                           }
                        }

                  *ptr=val;
                  found++;
                  }
               }
      }

   cache();

   return(found);
   }

// floodfill a segment of the volume based on scalar value
unsigned int mipmap::floodfill(const unsigned char *data,unsigned char *mark,
                               const unsigned int width,const unsigned int height,const unsigned int depth,
                               const unsigned int x,const unsigned int y,const unsigned int z,
                               const int value,const int maxdev,
                               const int token)
   {
   unsigned int i;
   unsigned int cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)==token) continue;
      if (abs(get(data,width,height,depth,xs,ys,zs)-value)>maxdev) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt++;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<(int)width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<(int)height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<(int)depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// floodfill a segment of the volume counting space
float mipmap::countfill(const unsigned char *data,unsigned char *mark,
                        const unsigned int width,const unsigned int height,const unsigned int depth,
                        const unsigned int x,const unsigned int y,const unsigned int z,
                        const int value,const int maxdev,
                        const int token)
   {
   unsigned int i;
   float cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0.0f;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)==token) continue;
      if (abs(get(data,width,height,depth,xs,ys,zs)-value)>maxdev) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt+=1.0f-get(data,width,height,depth,xs,ys,zs)/255.0f;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<(int)width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<(int)height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<(int)depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// floodfill a segment of the volume based on gradient magnitude
unsigned int mipmap::gradfill(const unsigned char *grad,unsigned char *mark,
                              const unsigned int width,const unsigned int height,const unsigned int depth,
                              const unsigned int x,const unsigned int y,const unsigned int z,
                              const int token,const int maxgrad)
   {
   unsigned int i;
   unsigned int cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)!=0) continue;
      if (get(grad,width,height,depth,xs,ys,zs)>=maxgrad) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt++;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<(int)width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<(int)height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<(int)depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// classify the volume by the segment size
unsigned char *mipmap::sizify(unsigned char *data,
                              unsigned int width,unsigned int height,unsigned int depth,
                              float maxdev)
   {
   int i,j,k;

   unsigned char *data2,*ptr2;

   unsigned int size,maxsize;
   int maxd;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr2=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++) *ptr2++=0;

   maxsize=1;
   maxd=ftrc(255.0f*maxdev+0.5f);

   for (k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            if (get(data2,width,height,depth,i,j,k)==0)
               {
               size=floodfill(data,data2,
                              width,height,depth,
                              i,j,k,get(data,width,height,depth,i,j,k),
                              maxd,1);

               if (size>maxsize) maxsize=size;
               }

   for (ptr2=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++) *ptr2++=0;

   for (k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            if (get(data2,width,height,depth,i,j,k)==0)
               {
               size=floodfill(data,data2,
                              width,height,depth,
                              i,j,k,get(data,width,height,depth,i,j,k),
                              maxd,1);

               size=ftrc(255.0f*(1.0f-fpow((float)size/maxsize,1.0f/3))+0.5f);
               if (size==0) size=1;

               floodfill(data,data2,
                         width,height,depth,
                         i,j,k,get(data,width,height,depth,i,j,k),
                         maxd,size);
               }

   return(data2);
   }

// classify the volume by gradient border
unsigned char *mipmap::classify(unsigned char *grad,
                                unsigned int width,unsigned int height,unsigned int depth,
                                float maxgrad,
                                unsigned int *classes)
   {
   const int stepping=71;

   int i,j,k;

   unsigned char *data2,*ptr;

   int token;
   int maxg;

   unsigned int cnt;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++) *ptr++=0;

   token=128;
   maxg=ftrc(255.0f*maxgrad+0.5f);
   cnt=0;

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            if (*ptr++==0)
               if (get(grad,width,height,depth,i,j,k)<maxg)
                  {
                  if (token==0) token=(token+stepping)%256;

                  gradfill(grad,data2,
                           width,height,depth,
                           i,j,k,token,maxg);

                  token=(token+stepping)%256;
                  cnt++;
                  }

   if (classes!=NULL) *classes=cnt;

   return(data2);
   }

// zero space
void mipmap::zero(unsigned char *data,unsigned char *grad,
                  unsigned int width,unsigned int height,unsigned int depth,
                  float maxdev)
   {
   int i,j,k;

   unsigned char *data2,*ptr;

   int maxd;

   float cnt,maxcnt;
   unsigned int mi,mj,mk;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++) *ptr++=0;

   maxd=ftrc(255.0f*maxdev+0.5f);

   maxcnt=0.0f;
   mi=mj=mk=0;

   for (ptr=data2,k=0; k<(int)depth; k++)
      for (j=0; j<(int)height; j++)
         for (i=0; i<(int)width; i++)
            if (*ptr++==0)
               {
               cnt=countfill(grad,data2,
                             width,height,depth,
                             i,j,k,get(grad,width,height,depth,i,j,k),maxd,1);

               if (cnt>maxcnt)
                  {
                  maxcnt=cnt;

                  mi=i;
                  mj=j;
                  mk=k;
                  }
               }

   free(data2);

   floodfill(grad,grad,
             width,height,depth,
             mi,mj,mk,get(grad,width,height,depth,mi,mj,mk),maxd,0);
   }

// parse command string
void mipmap::parsecommands(unsigned char *volume,
                           unsigned int width,unsigned int height,unsigned int depth,
                           char *commands)
   {
   char command;

   if (commands==NULL) return;

   while ((command=*commands++)!='\0')
      switch (command)
         {
         case ' ': // nop
            break;
         case 'b': // blur volume
            blur(volume,width,height,depth);
            break;
         }
   }

// parse gradient command string
void mipmap::parsegradcommands(unsigned char *volume,unsigned char *grad,
                               unsigned int width,unsigned int height,unsigned int depth,
                               char *commands)
   {
   const float maxdev=0.1f;
   const float maxgrad=0.5f;

   char command;

   unsigned char *volume2;

   if (commands==NULL) return;

   while ((command=*commands++)!='\0')
      switch (command)
         {
         case ' ': // nop
         case 'b':
            break;
         case 'd': // derive curvature
            volume2=gradmag(grad,width,height,depth,DSX,DSY,DSZ);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'v': // calculate variance
            volume2=variance(volume,width,height,depth);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'u': // use transfer function
            usetf(volume,grad,width,height,depth);
            break;
         case 'o': // use opacity
            useop(volume,grad,width,height,depth);
            break;
         case 't': // tangle material
            tangle(grad,width,height,depth);
            break;
         case 'r': // remove bubbles
            remove(grad,width,height,depth);
            break;
         case 's': // sizify volume
            volume2=sizify(volume,width,height,depth,maxdev);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'c': // classify volume
            volume2=classify(grad,width,height,depth,maxgrad);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'z': // zero space
            zero(volume,grad,width,height,depth,0.0f);
            break;
         case 'S': // sizify gradient
            volume2=sizify(grad,width,height,depth,0.0f);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'T': // grow material
            grow(grad,width,height,depth);
            break;
         case 'F': // fill border
            grow(grad,width,height,depth);
            break;
         case 'R': // fill space
            while (grow(grad,width,height,depth)>0);
            break;
         default: ERRORMSG();
         }
   }

// get interpolated scalar value from volume
unsigned char mipmap::getscalar(unsigned char *volume,
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

// get interpolated scalar value from volume
float mipmap::getscalar(float *volume,
                        unsigned int width,unsigned int height,unsigned int depth,
                        float x,float y,float z)
   {
   int i,j,k;

   float *ptr1,*ptr2;

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

   return((1.0f-z)*((1.0f-y)*((1.0f-x)*ptr1[0]+x*ptr1[1])+
                    y*((1.0f-x)*ptr1[width]+x*ptr1[width+1]))+
          z*((1.0f-y)*((1.0f-x)*ptr2[0]+x*ptr2[1])+
             y*((1.0f-x)*ptr2[width]+x*ptr2[width+1])));
   }

// scale volume
unsigned char *mipmap::scale(unsigned char *volume,
                             unsigned int width,unsigned int height,unsigned int depth,
                             unsigned int nwidth,unsigned int nheight,unsigned int ndepth)
   {
   unsigned int i,j,k;

   unsigned char *volume2;

   if (nwidth==width && nheight==height && ndepth==depth) return(volume);

   if ((volume2=(unsigned char *)malloc(nwidth*nheight*ndepth))==NULL) ERRORMSG();

   for (i=0; i<nwidth; i++)
      for (j=0; j<nheight; j++)
         for (k=0; k<ndepth; k++)
            volume2[i+(j+k*nheight)*nwidth]=getscalar(volume,width,height,depth,(float)i/(nwidth-1),(float)j/(nheight-1),(float)k/(ndepth-1));

   free(volume);
   return(volume2);
   }

// read either PVM or DICOM identified by the * in the filename pattern
unsigned char *mipmap::readANYvolume(const char *filename,
                                     unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                                     float *scalex,float *scaley,float *scalez)
   {
   DicomVolume data;
   unsigned char *chunk;

   if (strchr(filename,'*')==NULL)
      return(readPVMvolume(filename,width,height,depth,components,scalex,scaley,scalez));
   else
      {
      if (!data.loadImages(filename)) ERRORMSG();

      if ((chunk=(unsigned char *)malloc(data.getVoxelNum()))==NULL) ERRORMSG();
      memcpy(chunk,data.getVoxelData(),data.getVoxelNum());

      *width=data.getCols();
      *height=data.getRows();
      *depth=data.getSlis();

      *components=1;

      if (scalex!=NULL) *scalex=data.getBound(0)/data.getCols();
      if (scaley!=NULL) *scaley=data.getBound(1)/data.getRows();
      if (scalez!=NULL) *scalez=data.getBound(2)/data.getSlis();

      return(chunk);
      }
   }

// load the volume and convert it to 8 bit
void mipmap::loadvolume(const char *filename, // filename of PVM to load
                        const char *gradname, // optional filename of gradient volume
                        float mx,float my,float mz, // midpoint of volume (assumed to be fixed)
                        float sx,float sy,float sz, // size of volume (assumed to be fixed)
                        int bricksize,float overmax, // bricksize/overlap of volume (assumed to be fixed)
                        BOOLINT xswap,BOOLINT yswap,BOOLINT zswap, // swap volume flags
                        BOOLINT xrotate,BOOLINT zrotate, // rotate volume flags
                        BOOLINT usegrad, // use gradient volume
                        char *commands, // filter commands
                        int histmin,float histfreq,int kneigh,float histstep) // parameters for histogram computation
   {
   BOOLINT upload=FALSE;

   float maxsize;

   if (gradname==NULL) gradname=zerostr;
   if (commands==NULL) commands=zerostr;

   if (VOLUME==NULL ||
       strncmp(filename,filestr,MAXSTR)!=0 ||
       (usegrad && strlen(gradname)==0 && (GRAD==NULL || strlen(gradstr)>0)) ||
       strncmp(commands,commstr,MAXSTR)!=0 ||
       xswap!=xsflag || yswap!=ysflag || zswap!=zsflag ||
       xrotate!=xrflag || zrotate!=zrflag)
      {
      if (VOLUME!=NULL) free(VOLUME);
      if ((VOLUME=readANYvolume(filename,&WIDTH,&HEIGHT,&DEPTH,&COMPONENTS,&DSX,&DSY,&DSZ))==NULL) exit(1);

      if (COMPONENTS==2) VOLUME=quantize(VOLUME,WIDTH,HEIGHT,DEPTH);
      else if (COMPONENTS!=1) exit(1);

      VOLUME=swap(VOLUME,
                  &WIDTH,&HEIGHT,&DEPTH,
                  &DSX,&DSY,&DSZ,
                  xswap,yswap,zswap,
                  xrotate,zrotate);

      parsecommands(VOLUME,
                    WIDTH,HEIGHT,DEPTH,
                    commands);

      if (GRAD!=NULL)
         {
         free(GRAD);
         GRAD=NULL;
         }

      if (usegrad && strlen(gradname)==0)
         {
#ifdef MULTILEVEL
         GRAD=gradmagML(VOLUME,
                        WIDTH,HEIGHT,DEPTH,
                        DSX,DSY,DSZ,
                        &GRADMAX);
#else
         GRAD=gradmag(VOLUME,
                      WIDTH,HEIGHT,DEPTH,
                      DSX,DSY,DSZ,
                      &GRADMAX);
#endif

         parsegradcommands(VOLUME,GRAD,
                           WIDTH,HEIGHT,DEPTH,
                           commands);
         }

      strncpy(filestr,filename,MAXSTR);
      strncpy(gradstr,"",MAXSTR);
      strncpy(commstr,commands,MAXSTR);

      xsflag=xswap;
      ysflag=yswap;
      zsflag=zswap;

      xrflag=xrotate;
      zrflag=zrotate;

      upload=TRUE;
      }

   if (usegrad && strlen(gradname)>0)
      if (GRAD==NULL ||
          strncmp(gradname,gradstr,MAXSTR)!=0)
         {
         if (GRAD!=NULL) free(GRAD);
         if ((GRAD=readANYvolume(gradname,&GWIDTH,&GHEIGHT,&GDEPTH,&GCOMPONENTS))==NULL) exit(1);
         GRADMAX=1.0f;

         if (GCOMPONENTS==2) GRAD=quantize(GRAD,GWIDTH,GHEIGHT,GDEPTH);
         else if (GCOMPONENTS!=1) exit(1);

         GRAD=swap(GRAD,
                   &GWIDTH,&GHEIGHT,&GDEPTH,
                   NULL,NULL,NULL,
                   xswap,yswap,zswap,
                   xrotate,zrotate);

         GRAD=scale(GRAD,
                    GWIDTH,GHEIGHT,GDEPTH,
                    WIDTH,HEIGHT,DEPTH);

         parsegradcommands(VOLUME,GRAD,
                           WIDTH,HEIGHT,DEPTH,
                           commands);

         strncpy(gradstr,gradname,MAXSTR);

         upload=TRUE;
         }

   if (upload)
      {
      maxsize=fmax(DSX*(WIDTH-1),fmax(DSY*(HEIGHT-1),DSZ*(DEPTH-1)));

      set_data(VOLUME,
               usegrad?GRAD:NULL,
               WIDTH,HEIGHT,DEPTH,
               mx,my,mz,
               sx*DSX*(WIDTH-1)/maxsize,sy*DSY*(HEIGHT-1)/maxsize,sz*DSZ*(DEPTH-1)/maxsize,
               bricksize,overmax);
      }

   if (upload)
      HISTO->set_histograms(VOLUME,GRAD,WIDTH,HEIGHT,DEPTH,histmin,histfreq,kneigh,histstep);

   if (!upload && (hmvalue!=histmin || hfvalue!=histfreq))
      HISTO->inithist(VOLUME,WIDTH,HEIGHT,DEPTH,histmin,histfreq,FALSE);

   if (!upload && (hmvalue!=histmin || hfvalue!=histfreq || kneigh!=knvalue || histstep!=hsvalue))
      HISTO->inithist2DQ(VOLUME,GRAD,WIDTH,HEIGHT,DEPTH,histmin,histfreq,kneigh,histstep,FALSE);

   hmvalue=histmin;
   hfvalue=histfreq;
   knvalue=kneigh;
   hsvalue=histstep;
   }

// save the volume data as PVM
void mipmap::savePVMvolume(const char *filename)
   {
   if (VOLUME==NULL) return;

   writePVMvolume(filename,VOLUME,
                  WIDTH,HEIGHT,DEPTH,COMPONENTS,
                  DSX,DSY,DSZ);
   }

// return the histogram
float *mipmap::get_hist()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_hist());
   }

// return the colored histogram
float *mipmap::get_histRGBA()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_histRGBA());
   }

// return the scatter plot
float *mipmap::get_hist2D()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_hist2D());
   }

// return the colored scatter plot
float *mipmap::get_hist2DRGBA()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_hist2DRGBA());
   }

// return the quantized scatter plot
float *mipmap::get_hist2DQRGBA()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_hist2DQRGBA());
   }

// return the quantized transfer function
float *mipmap::get_hist2DTFRGBA()
   {
   if (VOLCNT==0) ERRORMSG();
   return(HISTO->get_hist2DTFRGBA());
   }

// return the slab thickness
float mipmap::get_slab()
   {
   if (VOLCNT==0) ERRORMSG();
   return(VOL[0]->get_slab());
   }

// set ambient/diffuse/specular lighting coefficients
void mipmap::set_light(float noise,float ambnt,float difus,float specl,float specx)
   {
   int i;

   if (VOLCNT==0) ERRORMSG();

   for (i=0; i<VOLCNT; i++) VOL[i]->set_light(noise,ambnt,difus,specl,specx);
   }

// render the volume
void mipmap::render(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float ux,float uy,float uz,
                    float nearp,float slab,
                    BOOLINT lighting)
   {
   int map=0;

   if (VOLCNT==0) ERRORMSG();

   if (TFUNC->get_imode())
      while (map<VOLCNT-1 && slab/VOL[map]->get_slab()>1.5f) map++;

   VOL[map]->render(ex,ey,ez,
                    dx,dy,dz,
                    ux,uy,uz,
                    nearp,slab,
                    1.0f/get_slab(),
                    lighting);

   if (TFUNC->get_invmode()) invertwindow();
   }

// draw the surrounding wire frame box
void mipmap::drawwireframe()
   {
   if (VOLCNT==0) ERRORMSG();
   VOL[0]->drawwireframe();
   }
