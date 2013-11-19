// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_H
#define VOLREN_H

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base and window handling
#include "volume.h" // volume mipmap pyramid

//! the volume renderer
class volren: public volscene
   {
   public:

   //! default constructor
   volren(char *base=NULL)
      : volscene(base)
      {initogl();}

   //! destructor
   ~volren() {}

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
                      void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL)
      {
      return(volscene::loadvolume(filename,
                                  gradname,
                                  mx,my,mz,
                                  sx,sy,sz,
                                  bricksize,overmax,
                                  xswap,yswap,zswap,
                                  xrotate,zrotate,
                                  usegrad,
                                  commands,
                                  histmin,histfreq,kneigh,histstep,
                                  feedback,obj));
      }

   //! load a DICOM series
   BOOLINT loadseries(const std::vector<std::string> list,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f,
                      void (*feedback)(const char *info,float percent,void *obj)=NULL,void *obj=NULL)
      {
      return(volscene::loadseries(list,
                                  mx,my,mz,
                                  sx,sy,sz,
                                  bricksize,overmax,
                                  xswap,yswap,zswap,
                                  xrotate,zrotate,
                                  usegrad,
                                  histmin,histfreq,kneigh,histstep,
                                  feedback,obj));
      }

   //! use linear transfer function
   void set_tfunc(float center=0.5f,float size=1.0f,
                  float r=1.0f,float g=1.0f,float b=1.0f,
                  BOOLINT inverse=FALSE)
      {
      float x1=center-0.5f*size;
      float x2=center+0.5f*size;

      // tf emission (emi)
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_re());
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_ge());
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_be());
      get_tfunc()->set_line(x1,0.0f,x2,r,get_tfunc()->get_re());
      get_tfunc()->set_line(x1,0.0f,x2,g,get_tfunc()->get_ge());
      get_tfunc()->set_line(x1,0.0f,x2,b,get_tfunc()->get_be());

      // tf absorption (att)
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_ra());
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_ga());
      get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,get_tfunc()->get_ba());
      if (inverse)
         {
         get_tfunc()->set_line(x1,1.0f,x2,0.0f,get_tfunc()->get_ra());
         get_tfunc()->set_line(x1,1.0f,x2,0.0f,get_tfunc()->get_ga());
         get_tfunc()->set_line(x1,1.0f,x2,0.0f,get_tfunc()->get_ba());
         }
      else
         {
         get_tfunc()->set_line(x1,0.0f,x2,1.0f,get_tfunc()->get_ra());
         get_tfunc()->set_line(x1,0.0f,x2,1.0f,get_tfunc()->get_ga());
         get_tfunc()->set_line(x1,0.0f,x2,1.0f,get_tfunc()->get_ba());
         }
      }

   //! extract the surface data
   char *extractsurface()
      {return(volscene::extractTFsurface());}

   //! load the surface data
   BOOLINT loadsurface(const char *filename)
      {return(volscene::loadsurface(filename));}

   //! render the volume mipmap pyramid
   BOOLINT render(float eye_x,float eye_y,float eye_z, // eye point
                  float eye_dx,float eye_dy,float eye_dz, // viewing direction
                  float eye_ux,float eye_uy,float eye_uz, // up vector
                  float gfx_fovy,float gfx_aspect,float gfx_near,float gfx_far, // opengl perspective
                  BOOLINT gfx_fbo, // use frame buffer object
                  float vol_rot, // volume rotation in degrees
                  float vol_tltXY, // volume tilt in degrees
                  float vol_tltYZ, // volume tilt in degrees
                  float vol_dx,float vol_dy,float vol_dz, // volume translation
                  float vol_emi,float vol_att, // global volume emi and att
                  float tf_re_scale,float tf_ge_scale,float tf_be_scale, // emi scale
                  float tf_ra_scale,float tf_ga_scale,float tf_ba_scale, // att scale
                  BOOLINT tf_premult=TRUE,BOOLINT tf_preint=TRUE, // pre-multiplication and pre-integration
                  BOOLINT vol_white=TRUE, // white background
                  BOOLINT vol_inv=FALSE, // inverse mode
                  float vol_over=1.0f, // oversampling
                  BOOLINT vol_light=FALSE, // lighting
                  BOOLINT vol_clip=FALSE, // view-aligned clipping
                  float vol_clip_dist=0.0f, // clipping distance relative to origin
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL)
      {
      BOOLINT aborted=FALSE;

      transform(eye_x,eye_y,eye_z,
                eye_dx,eye_dy,eye_dz,
                eye_ux,eye_uy,eye_uz,
                vol_rot,vol_tltXY,vol_tltYZ,
                vol_dx,vol_dy,vol_dz);

      // tf setup:

      get_tfunc()->set_escale(fsqr(tf_re_scale),fsqr(tf_ge_scale),fsqr(tf_be_scale));
      get_tfunc()->set_ascale(fsqr(tf_ra_scale),fsqr(tf_ga_scale),fsqr(tf_ba_scale));

      get_tfunc()->set_invmode(vol_inv);

      get_tfunc()->refresh(vol_emi,vol_att,get_slab()*vol_over,
                           tf_premult,tf_preint,vol_light);

      // volren setup:

      set_light(0.01f,0.3f,0.5f,0.2f,10.0f);

      if (vol_white)
         if (!vol_inv) setbackground(0.85f,0.85f,0.85f);
         else setbackground(1.0f,1.0f,1.0f);
      else setbackground(0.0f,0.0f,0.0f);

      clearbuffer();

      // model view:

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(gfx_fovy,gfx_aspect,gfx_near,gfx_far);
      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(eye_x,eye_y,eye_z,eye_x+eye_dx,eye_y+eye_dy,eye_z+eye_dz,eye_ux,eye_uy,eye_uz);

      // render:

      if (!vol_clip)
         aborted=volscene::render(eye_x,eye_y,eye_z,
                                  eye_dx,eye_dy,eye_dz,
                                  eye_ux,eye_uy,eye_uz,
                                  gfx_near,get_slab()*vol_over,
                                  vol_light,
                                  gfx_fbo,
                                  abort,abortdata);
      else
         aborted=volscene::render(eye_x,eye_y,eye_z,
                                  eye_dx,eye_dy,eye_dz,
                                  eye_ux,eye_uy,eye_uz,
                                  sqrt(eye_x*eye_x+eye_y*eye_y+eye_z*eye_z)-vol_clip_dist,get_slab()*vol_over,
                                  vol_light,
                                  gfx_fbo,
                                  abort,abortdata);

      glPopMatrix();

      return(aborted);
      }

   //! render a volume slice
   void renderslice(float eye_x,float eye_y,float eye_z, // eye point
                    float eye_dx,float eye_dy,float eye_dz, // viewing direction
                    float eye_ux,float eye_uy,float eye_uz, // up vector
                    float vol_rot, // volume rotation in degrees
                    float vol_tltXY, // volume tilt in degrees
                    float vol_tltYZ, // volume tilt in degrees
                    float vol_dx,float vol_dy,float vol_dz, // volume translation
                    float slice_dist, // volume slice distance
                    float slice_alpha=1.0f) // volume slice opacity
      {
      if (alpha==0.0f) return;

      transform(eye_x,eye_y,eye_z,
                eye_dx,eye_dy,eye_dz,
                eye_ux,eye_uy,eye_uz,
                vol_rot,vol_tltXY,vol_tltYZ,
                vol_dx,vol_dy,vol_dz);

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(eye_x,eye_y,eye_z,eye_x+eye_dx,eye_y+eye_dy,eye_z+eye_dz,eye_ux,eye_uy,eye_uz);

      volscene::renderslice(eye_x,eye_y,eye_z,
                            eye_dx,eye_dy,eye_dz,
                            eye_ux,eye_uy,eye_uz,
                            sqrt(eye_x*eye_x+eye_y*eye_y+eye_z*eye_z)-slice_dist,
                            slice_alpha);

      glPopMatrix();
      }

   private:

   void transform(float &eye_x,float &eye_y,float &eye_z, // eye point
                  float &eye_dx,float &eye_dy,float &eye_dz, // viewing direction
                  float &eye_ux,float &eye_uy,float &eye_uz, // up vector
                  float vol_rot, // volume rotation in degrees
                  float vol_tltXY, // volume tilt in degrees
                  float vol_tltYZ, // volume tilt in degrees
                  float vol_dx,float vol_dy,float vol_dz) // volume translation
      {
      float ex0,ey0,ez0,
            dx0,dy0,dz0,
            ux0,uy0,uz0;

      float ex,ey,ez,
            dx,dy,dz,
            ux,uy,uz;

      vol_rot*=PI/180.0f;
      vol_tltXY*=PI/180.0f;
      vol_tltYZ*=PI/180.0f;

      // move:

      ex0=eye_x+vol_dx;
      ey0=eye_y+vol_dy;
      ez0=eye_z+vol_dz;

      dx0=eye_dx;
      dy0=eye_dy;
      dz0=eye_dz;

      ux0=eye_ux;
      uy0=eye_uy;
      uz0=eye_uz;

      // rotate:

      ex=fcos(vol_rot)*ex0+fsin(vol_rot)*ez0;
      ey=ey0;
      ez=-fsin(vol_rot)*ex0+fcos(vol_rot)*ez0;

      dx=fcos(vol_rot)*dx0+fsin(vol_rot)*dz0;
      dy=dy0;
      dz=-fsin(vol_rot)*dx0+fcos(vol_rot)*dz0;

      ux=fcos(vol_rot)*ux0+fsin(vol_rot)*uz0;
      uy=uy0;
      uz=-fsin(vol_rot)*ux0+fcos(vol_rot)*uz0;

      // tiltXY:

      ex0=fcos(vol_tltXY)*ex-fsin(vol_tltXY)*ey;
      ey0=fsin(vol_tltXY)*ex+fcos(vol_tltXY)*ey;
      ez0=ez;

      dx0=fcos(vol_tltXY)*dx-fsin(vol_tltXY)*dy;
      dy0=fsin(vol_tltXY)*dx+fcos(vol_tltXY)*dy;
      dz0=dz;

      ux0=fcos(vol_tltXY)*ux-fsin(vol_tltXY)*uy;
      uy0=fsin(vol_tltXY)*ux+fcos(vol_tltXY)*uy;
      uz0=uz;

      // tiltYZ:

      eye_x=ex0;
      eye_y=fcos(vol_tltYZ)*ey0-fsin(vol_tltYZ)*ez0;
      eye_z=fsin(vol_tltYZ)*ey0+fcos(vol_tltYZ)*ez0;

      eye_dx=dx0;
      eye_dy=fcos(vol_tltYZ)*dy0-fsin(vol_tltYZ)*dz0;
      eye_dz=fsin(vol_tltYZ)*dy0+fcos(vol_tltYZ)*dz0;

      eye_ux=ux0;
      eye_uy=fcos(vol_tltYZ)*uy0-fsin(vol_tltYZ)*uz0;
      eye_uz=fsin(vol_tltYZ)*uy0+fcos(vol_tltYZ)*uz0;
      }

   };

#endif
