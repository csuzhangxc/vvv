// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_H
#define VOLREN_H

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base and window handling
#include "volume.h" // volume mipmap pyramid

#include "v3d.h" // vector class

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

   //! check for volume
   BOOLINT hasvolume()
      {return(has_data());}

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

   //! show the surface data
   void showsurface(BOOLINT yes)
      {volscene::showsurface(yes);}

   //! check for surface
   BOOLINT hassurface()
      {return(has_geo());}

   //! begin rendering
   void begin(float gfx_fovy,float gfx_aspect,float gfx_near,float gfx_far, // opengl perspective
              BOOLINT vol_white=TRUE, // white background
              BOOLINT vol_inv=FALSE) // inverse mode
      {
      // volren setup:

      if (vol_white)
         if (!vol_inv) setbackground(0.85f,0.85f,0.85f);
         else setbackground(1.0f,1.0f,1.0f);
      else setbackground(0.0f,0.0f,0.0f);

      clearbuffer();

      set_light(0.01f,0.3f,0.5f,0.2f,10.0f);

      // perspective:

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(gfx_fovy,gfx_aspect,gfx_near,gfx_far);
      glMatrixMode(GL_MODELVIEW);
      }

   //! render the volume mipmap pyramid
   BOOLINT render(float eye_x,float eye_y,float eye_z, // eye point
                  float eye_dx,float eye_dy,float eye_dz, // viewing direction
                  float eye_ux,float eye_uy,float eye_uz, // up vector
                  float gfx_near, // near plane
                  BOOLINT gfx_fbo, // use frame buffer object
                  float vol_rot, // volume rotation in degrees
                  float vol_tltXY, // volume tilt in degrees
                  float vol_tltYZ, // volume tilt in degrees
                  float vol_dx,float vol_dy,float vol_dz, // volume translation
                  float vol_emi,float vol_att, // global volume emi and att
                  float tf_re_scale,float tf_ge_scale,float tf_be_scale, // emi scale
                  float tf_ra_scale,float tf_ga_scale,float tf_ba_scale, // att scale
                  BOOLINT tf_premult=TRUE,BOOLINT tf_preint=TRUE, // pre-multiplication and pre-integration
                  BOOLINT vol_inv=FALSE, // inverse mode
                  float vol_over=1.0f, // oversampling
                  BOOLINT vol_light=FALSE, // lighting
                  BOOLINT vol_clip=FALSE, // view-aligned clipping
                  float vol_clip_dist=0.0f, // clipping distance relative to origin
                  BOOLINT vol_clip_near=FALSE, // clip at near plane
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL)
      {
      BOOLINT aborted=FALSE;

      transform(eye_x,eye_y,eye_z,
                eye_dx,eye_dy,eye_dz,
                eye_ux,eye_uy,eye_uz,
                vol_rot,vol_tltXY,vol_tltYZ,
                vol_dx,vol_dy,vol_dz);

      // model view:

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(eye_x,eye_y,eye_z,eye_x+eye_dx,eye_y+eye_dy,eye_z+eye_dz,eye_ux,eye_uy,eye_uz);

      // tf setup:

      if (tf_re_scale<0.0f) tf_re_scale=0.0f;
      if (tf_ge_scale<0.0f) tf_ge_scale=0.0f;
      if (tf_be_scale<0.0f) tf_be_scale=0.0f;

      if (tf_ra_scale<0.0f) tf_ra_scale=0.0f;
      if (tf_ga_scale<0.0f) tf_ga_scale=0.0f;
      if (tf_ba_scale<0.0f) tf_ba_scale=0.0f;

      get_tfunc()->set_escale(fsqr(tf_re_scale),fsqr(tf_ge_scale),fsqr(tf_be_scale));
      get_tfunc()->set_ascale(fsqr(tf_ra_scale),fsqr(tf_ga_scale),fsqr(tf_ba_scale));

      get_tfunc()->set_invmode(vol_inv);

      get_tfunc()->refresh(vol_emi,vol_att,get_slab()*vol_over,
                           tf_premult,tf_preint,vol_light);

      // render:

      if (!vol_clip)
         aborted=volscene::render(eye_x,eye_y,eye_z,
                                  eye_dx,eye_dy,eye_dz,
                                  eye_ux,eye_uy,eye_uz,
                                  gfx_near,get_slab()*vol_over,
                                  vol_clip_near,
                                  vol_light,
                                  gfx_fbo,
                                  abort,abortdata);
      else
         aborted=volscene::render(eye_x,eye_y,eye_z,
                                  eye_dx,eye_dy,eye_dz,
                                  eye_ux,eye_uy,eye_uz,
                                  -eye_x*eye_dx-eye_y*eye_dy-eye_z*eye_dz-vol_clip_dist,
                                  get_slab()*vol_over,
                                  vol_clip_near,
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
                    float slice_alpha=1.0f, // volume slice opacity
                    float slice_alpha2=0.1f) // outer slice opacity
      {
      if (slice_alpha==0.0f && slice_alpha2==0.0f) return;

      transform(eye_x,eye_y,eye_z,
                eye_dx,eye_dy,eye_dz,
                eye_ux,eye_uy,eye_uz,
                vol_rot,vol_tltXY,vol_tltYZ,
                vol_dx,vol_dy,vol_dz);

      // model view:

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(eye_x,eye_y,eye_z,eye_x+eye_dx,eye_y+eye_dy,eye_z+eye_dz,eye_ux,eye_uy,eye_uz);

      // render:

      volscene::renderslice(eye_x,eye_y,eye_z,
                            eye_dx,eye_dy,eye_dz,
                            eye_ux,eye_uy,eye_uz,
                            -eye_x*eye_dx-eye_y*eye_dy-eye_z*eye_dz-slice_dist,
                            slice_alpha,slice_alpha2);

      glPopMatrix();
      }

   //! render a volume slice
   void renderslice(float eye_x,float eye_y,float eye_z, // eye point
                    float eye_dx,float eye_dy,float eye_dz, // viewing direction
                    float eye_ux,float eye_uy,float eye_uz, // up vector
                    float vol_rot, // volume rotation in degrees
                    float vol_tltXY, // volume tilt in degrees
                    float vol_tltYZ, // volume tilt in degrees
                    float vol_dx,float vol_dy,float vol_dz, // volume translation
                    float slice_x,float slice_y,float slice_z, // volume slice point
                    float slice_nx,float slice_ny,float slice_nz, // volume slice normal
                    float slice_alpha=1.0f, // volume slice opacity
                    float slice_alpha2=0.1f) // outer slice opacity
      {
      if (slice_alpha==0.0f && slice_alpha2==0.0f) return;

      transform(eye_x,eye_y,eye_z,
                eye_dx,eye_dy,eye_dz,
                eye_ux,eye_uy,eye_uz,
                vol_rot,vol_tltXY,vol_tltYZ,
                vol_dx,vol_dy,vol_dz);

      // model view:

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(eye_x,eye_y,eye_z,eye_x+eye_dx,eye_y+eye_dy,eye_z+eye_dz,eye_ux,eye_uy,eye_uz);

      // render:

      volscene::renderslice(slice_x,slice_y,slice_z,
                            slice_nx,slice_ny,slice_nz,
                            slice_alpha,slice_alpha2);

      glPopMatrix();
      }

   //! render volume scene (volume, iso surface, reconstruction plane, clip planes, metric geometry)
   BOOLINT renderscene(float eye_x,float eye_y,float eye_z, // eye point
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
                       BOOLINT vol_clip_near=FALSE, // clip at near plane
                       BOOLINT vol_clip_opaque=FALSE, // opaque clipping plane
                       float vol_clip_opacity=1.0f, // clipping plane opacity
                       float vol_outer_opacity=0.1f, // outer clipping plane opacity
                       BOOLINT geo_show=TRUE, // show surface geometry
                       BOOLINT (*abort)(void *abortdata)=NULL,
                       void *abortdata=NULL)
      {
      BOOLINT aborted;

      begin(gfx_fovy,gfx_aspect,gfx_near,gfx_far,
            vol_white,vol_inv);

      showsurface(geo_show);

      aborted=render(eye_x,eye_y,eye_z,
                     eye_dx,eye_dy,eye_dz,
                     eye_ux,eye_uy,eye_uz,
                     gfx_near,
                     gfx_fbo,
                     vol_rot,vol_tltXY,vol_tltYZ,
                     vol_dx,vol_dy,vol_dz,
                     vol_emi,vol_att,
                     tf_re_scale,tf_ge_scale,tf_be_scale,
                     tf_ra_scale,tf_ga_scale,tf_ba_scale,
                     tf_premult,tf_preint,
                     vol_inv,vol_over,vol_light,
                     vol_clip,vol_clip_dist,vol_clip_near,
                     abort,abortdata);

      if (!aborted)
         if (vol_clip_opaque)
            renderslice(eye_x,eye_y,eye_z,
                        eye_dx,eye_dy,eye_dz,
                        eye_ux,eye_uy,eye_uz,
                        vol_rot,vol_tltXY,vol_tltYZ,
                        vol_dx,vol_dy,vol_dz,
                        vol_clip_dist,vol_clip_opacity,vol_outer_opacity);

      return(aborted);
      }

   //! transform point into rendering coordinate system
   void transform(float &x,float &y,float &z, // point
                  float vol_rot, // volume rotation in degrees
                  float vol_tltXY, // volume tilt in degrees
                  float vol_tltYZ, // volume tilt in degrees
                  float vol_dx,float vol_dy,float vol_dz) // volume translation
      {
      float x0,y0,z0;

      vol_rot*=PI/180.0f;
      vol_tltXY*=PI/180.0f;
      vol_tltYZ*=PI/180.0f;

      // move:

      x0=x+vol_dx;
      y0=y+vol_dy;
      z0=z+vol_dz;

      // rotate:

      x=fcos(vol_rot)*x0+fsin(vol_rot)*z0;
      y=y0;
      z=-fsin(vol_rot)*x0+fcos(vol_rot)*z0;

      // tiltXY:

      x0=fcos(vol_tltXY)*x-fsin(vol_tltXY)*y;
      y0=fsin(vol_tltXY)*x+fcos(vol_tltXY)*y;
      z0=z;

      // tiltYZ:

      x=x0;
      y=fcos(vol_tltYZ)*y0-fsin(vol_tltYZ)*z0;
      z=fsin(vol_tltYZ)*y0+fcos(vol_tltYZ)*z0;

      x*=getscale();
      y*=getscale();
      z*=getscale();
      }

   //! rotate about anchor point
   void rotate(float ax,float ay,float az,
               float angle1,float angle2,
               float &eye_x,float &eye_y,float &eye_z,
               float &eye_dx,float &eye_dy,float &eye_dz,
               float &eye_ux,float &eye_uy,float &eye_uz)
      {
      float ex,ey,ez;
      float dx,dy,dz;
      float ux,uy,uz;

      float arx,ary,arz;
      float aux,auy,auz;
      float adx,ady,adz;

      float tx,ty,tz;
      float l;

      angle1*=PI/180.0f;
      angle2*=PI/180.0f;

      // compute anchor coords:

      arx=eye_dy*eye_uz-eye_dz*eye_uy;
      ary=eye_dz*eye_ux-eye_dx*eye_uz;
      arz=eye_dx*eye_uy-eye_dy*eye_ux;

      if ((l=fsqrt(arx*arx+ary*ary+arz*arz))>0.0f)
         {
         arx/=l;
         ary/=l;
         arz/=l;
         }

      aux=eye_ux;
      auy=eye_uy;
      auz=eye_uz;

      if ((l=fsqrt(aux*aux+auy*auy+auz*auz))>0.0f)
         {
         aux/=l;
         auy/=l;
         auz/=l;
         }

      adx=eye_dx;
      ady=eye_dy;
      adz=eye_dz;

      if ((l=fsqrt(adx*adx+ady*ady+adz*adz))>0.0f)
         {
         adx/=l;
         ady/=l;
         adz/=l;
         }

      // translate to anchor coords:

      eye_x-=ax;
      eye_y-=ay;
      eye_z-=az;

      // rotate to anchor coords:

      ex=eye_x;
      ey=eye_y;
      ez=eye_z;

      eye_x=ex*arx+ey*ary+ez*arz;
      eye_y=ex*aux+ey*auy+ez*auz;
      eye_z=ex*adx+ey*ady+ez*adz;

      tx=eye_dx*arx+eye_dy*ary+eye_dz*arz;
      ty=eye_dx*aux+eye_dy*auy+eye_dz*auz;
      tz=eye_dx*adx+eye_dy*ady+eye_dz*adz;

      eye_dx=tx;
      eye_dy=ty;
      eye_dz=tz;

      tx=eye_ux*arx+eye_uy*ary+eye_uz*arz;
      ty=eye_ux*aux+eye_uy*auy+eye_uz*auz;
      tz=eye_ux*adx+eye_uy*ady+eye_uz*adz;

      eye_ux=tx;
      eye_uy=ty;
      eye_uz=tz;

      // tilt:

      ex=eye_x;
      ey=fcos(angle2)*eye_y-fsin(angle2)*eye_z;
      ez=fsin(angle2)*eye_y+fcos(angle2)*eye_z;

      dx=eye_dx;
      dy=fcos(angle2)*eye_dy-fsin(angle2)*eye_dz;
      dz=fsin(angle2)*eye_dy+fcos(angle2)*eye_dz;

      ux=eye_ux;
      uy=fcos(angle2)*eye_uy-fsin(angle2)*eye_uz;
      uz=fsin(angle2)*eye_uy+fcos(angle2)*eye_uz;

      eye_x=ex;
      eye_y=ey;
      eye_z=ez;

      eye_dx=dx;
      eye_dy=dy;
      eye_dz=dz;

      eye_ux=ux;
      eye_uy=uy;
      eye_uz=uz;

      // rotate:

      ex=fcos(angle1)*eye_x+fsin(angle1)*eye_z;
      ey=eye_y;
      ez=-fsin(angle1)*eye_x+fcos(angle1)*eye_z;

      dx=fcos(angle1)*eye_dx+fsin(angle1)*eye_dz;
      dy=eye_dy;
      dz=-fsin(angle1)*eye_dx+fcos(angle1)*eye_dz;

      ux=fcos(angle1)*eye_ux+fsin(angle1)*eye_uz;
      uy=eye_uy;
      uz=-fsin(angle1)*eye_ux+fcos(angle1)*eye_uz;

      eye_x=ex;
      eye_y=ey;
      eye_z=ez;

      eye_dx=dx;
      eye_dy=dy;
      eye_dz=dz;

      eye_ux=ux;
      eye_uy=uy;
      eye_uz=uz;

      // rotate to world coords:

      ex=eye_x;
      ey=eye_y;
      ez=eye_z;

      eye_x=ex*arx+ey*aux+ez*adx;
      eye_y=ex*ary+ey*auy+ez*ady;
      eye_z=ex*arz+ey*auz+ez*adz;

      tx=eye_dx*arx+eye_dy*aux+eye_dz*adx;
      ty=eye_dx*ary+eye_dy*auy+eye_dz*ady;
      tz=eye_dx*arz+eye_dy*auz+eye_dz*adz;

      eye_dx=tx;
      eye_dy=ty;
      eye_dz=tz;

      tx=eye_ux*arx+eye_uy*aux+eye_uz*adx;
      ty=eye_ux*ary+eye_uy*auy+eye_uz*ady;
      tz=eye_ux*arz+eye_uy*auz+eye_uz*adz;

      eye_ux=tx;
      eye_uy=ty;
      eye_uz=tz;

      // translate to world coords:

      eye_x+=ax;
      eye_y+=ay;
      eye_z+=az;

      // normalize:

      if ((l=fsqrt(eye_dx*eye_dx+eye_dy*eye_dy+eye_dz*eye_dz))>0.0f)
         {
         eye_dx/=l;
         eye_dy/=l;
         eye_dz/=l;
         }

      if ((l=fsqrt(eye_ux*eye_ux+eye_uy*eye_uy+eye_uz*eye_uz))>0.0f)
         {
         eye_ux/=l;
         eye_uy/=l;
         eye_uz/=l;
         }

      // orthogonalize:

      arx=eye_dy*eye_uz-eye_dz*eye_uy;
      ary=eye_dz*eye_ux-eye_dx*eye_uz;
      arz=eye_dx*eye_uy-eye_dy*eye_ux;

      eye_ux=ary*eye_dz-arz*eye_dy;
      eye_uy=arz*eye_dx-arx*eye_dz;
      eye_uz=arx*eye_dy-ary*eye_dx;
      }

   //! project screen coordinates onto anchor plane
   //!  returns absolute position in meters
   v3d project(double sx,double sy,
               double fovy,double aspect)
      {
      double eye_x,eye_y,eye_z;
      double eye_dx,eye_dy,eye_dz;
      double eye_ux,eye_uy,eye_uz;
      double eye_rx,eye_ry,eye_rz;

      double ax,ay,az;
      double dx,dy,dz;

      double d,t;

      get_eye(eye_x,eye_y,eye_z,
              eye_dx,eye_dy,eye_dz,
              eye_ux,eye_uy,eye_uz,
              eye_rx,eye_ry,eye_rz);

      get_near(ax,ay,az,
               dx,dy,dz);

      sx=2.0f*(sx-0.5);
      sy=2.0f*(0.5-sy);

      d=(ax-eye_x)*eye_dx+
        (ay-eye_y)*eye_dy+
        (az-eye_z)*eye_dz;

      t=d*tan(fovy/2.0*PI/180.0);

      eye_x+=d*eye_dx+
             eye_rx*sx*t*aspect+
             eye_ux*sy*t;

      eye_y+=d*eye_dy+
             eye_ry*sx*t*aspect+
             eye_uy*sy*t;

      eye_z+=d*eye_dz+
             eye_rz*sx*t*aspect+
             eye_uz*sy*t;

      return(v3d(eye_x,eye_y,eye_z)*getscale());
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

      eye_x+=vol_dx;
      eye_y+=vol_dy;
      eye_z+=vol_dz;

      // rotate:

      ex=fcos(vol_rot)*eye_x+fsin(vol_rot)*eye_z;
      ey=eye_y;
      ez=-fsin(vol_rot)*eye_x+fcos(vol_rot)*eye_z;

      dx=fcos(vol_rot)*eye_dx+fsin(vol_rot)*eye_dz;
      dy=eye_dy;
      dz=-fsin(vol_rot)*eye_dx+fcos(vol_rot)*eye_dz;

      ux=fcos(vol_rot)*eye_ux+fsin(vol_rot)*eye_uz;
      uy=eye_uy;
      uz=-fsin(vol_rot)*eye_ux+fcos(vol_rot)*eye_uz;

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

   protected:

   virtual void rendergeometry()
      {
      float scale=1.0f/getscale();

      // apply scaling
      glPushMatrix();
      glScalef(scale,scale,scale);

      // render metric geometry
      rendermetric();

      // restore scaling
      glPopMatrix();

      // call base class
      volscene::rendergeometry();
      }

   //! virtual function that can be overloaded to render metric geometry
   virtual void rendermetric() {}
   };

#endif
