// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_H
#define VOLREN_H

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base and window handling
#include "volume.h" // volume mipmap pyramid

// the volume renderer
class volren
   {
   public:

   // default constructor
   volren(char *base=NULL)
      {
      initogl();
      VOL=new mipmap(base);
      }

   // destructor
   ~volren()
      {delete VOL;}

   mipmap *get_volume() {return(VOL);} // return the volume
   tfunc2D *get_tfunc() {return(VOL->get_tfunc());} // return the transfer function
   histo *get_histo() {return(VOL->get_histo());} // return the histogram

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
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f,
                      void (*feedback)(const char *info,float percent)=NULL)
      {
      return(VOL->loadvolume(filename,
                             gradname,
                             mx,my,mz,
                             sx,sy,sz,
                             bricksize,overmax,
                             xswap,yswap,zswap,
                             xrotate,zrotate,
                             usegrad,
                             commands,
                             histmin,histfreq,kneigh,histstep,
                             feedback));
      }

   // load a DICOM series
   BOOLINT loadseries(const std::vector<std::string> list,
                      float mx=0.0f,float my=0.0f,float mz=0.0f,
                      float sx=1.0f,float sy=1.0f,float sz=1.0f,
                      int bricksize=128,float overmax=8.0f,
                      BOOLINT xswap=FALSE,BOOLINT yswap=FALSE,BOOLINT zswap=FALSE,
                      BOOLINT xrotate=FALSE,BOOLINT zrotate=FALSE,
                      BOOLINT usegrad=FALSE,
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f,
                      void (*feedback)(const char *info,float percent)=NULL)
      {
      return(VOL->loadseries(list,
                             mx,my,mz,
                             sx,sy,sz,
                             bricksize,overmax,
                             xswap,yswap,zswap,
                             xrotate,zrotate,
                             usegrad,
                             histmin,histfreq,kneigh,histstep,
                             feedback));
      }

   // save the volume data as PVM
   void savePVMvolume(const char *filename)
      {VOL->savePVMvolume(filename);}

   // check whether or not the hierarchy has volume data
   BOOLINT has_data()
      {return(VOL->has_data());}

   // check whether or not the hierarchy has gradient data
   BOOLINT has_grad()
      {return(VOL->has_grad());}

   // return the slab thickness
   float get_slab()
      {return(VOL->get_slab());}

   // use linear transfer function
   void set_tfunc(float center=0.5f,float size=1.0f,
                  float r=1.0f,float g=1.0f,float b=1.0f,
                  BOOLINT inverse=FALSE)
      {
      float x1=center-0.5f*size;
      float x2=center+0.5f*size;

      // tf emission (emi)
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_re());
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_ge());
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_be());
      VOL->get_tfunc()->set_line(x1,0.0f,x2,r,VOL->get_tfunc()->get_re());
      VOL->get_tfunc()->set_line(x1,0.0f,x2,g,VOL->get_tfunc()->get_ge());
      VOL->get_tfunc()->set_line(x1,0.0f,x2,b,VOL->get_tfunc()->get_be());

      // tf absorption (att)
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_ra());
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_ga());
      VOL->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOL->get_tfunc()->get_ba());
      if (inverse)
         {
         VOL->get_tfunc()->set_line(x1,1.0f,x2,0.0f,VOL->get_tfunc()->get_ra());
         VOL->get_tfunc()->set_line(x1,1.0f,x2,0.0f,VOL->get_tfunc()->get_ga());
         VOL->get_tfunc()->set_line(x1,1.0f,x2,0.0f,VOL->get_tfunc()->get_ba());
         }
      else
         {
         VOL->get_tfunc()->set_line(x1,0.0f,x2,1.0f,VOL->get_tfunc()->get_ra());
         VOL->get_tfunc()->set_line(x1,0.0f,x2,1.0f,VOL->get_tfunc()->get_ga());
         VOL->get_tfunc()->set_line(x1,0.0f,x2,1.0f,VOL->get_tfunc()->get_ba());
         }
      }

   // render the volume mipmap pyramid
   BOOLINT render(float eye_x,float eye_y,float eye_z, // eye point
                  float eye_dx,float eye_dy,float eye_dz, // viewing direction
                  float eye_ux,float eye_uy,float eye_uz, // up vector
                  float gfx_fovy,float gfx_aspect,float gfx_near,float gfx_far, // opengl perspective
                  BOOLINT gfx_fbo, // use frame buffer object
                  float vol_rot, // volume rotation in degrees
                  float vol_tlt1, // volume tilt in degrees
                  float vol_tlt2, // volume tilt in degrees
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
                  BOOLINT vol_wire=FALSE, // wire frame box
                  BOOLINT vol_histo=FALSE, // spatialized histogram
                  BOOLINT (*abort)(void *abortdata)=NULL,
                  void *abortdata=NULL)
      {
      BOOLINT aborted=FALSE;

      float ex0,ey0,ez0,
            dx0,dy0,dz0,
            ux0,uy0,uz0;

      float ex,ey,ez,
            dx,dy,dz,
            ux,uy,uz;

      vol_rot*=PI/180.0f;
      vol_tlt1*=PI/180.0f;
      vol_tlt2*=PI/180.0f;

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

      // tilt1:

      ex0=fcos(vol_tlt1)*ex-fsin(vol_tlt1)*ey;
      ey0=fsin(vol_tlt1)*ex+fcos(vol_tlt1)*ey;
      ez0=ez;

      dx0=fcos(vol_tlt1)*dx-fsin(vol_tlt1)*dy;
      dy0=fsin(vol_tlt1)*dx+fcos(vol_tlt1)*dy;
      dz0=dz;

      ux0=fcos(vol_tlt1)*ux-fsin(vol_tlt1)*uy;
      uy0=fsin(vol_tlt1)*ux+fcos(vol_tlt1)*uy;
      uz0=uz;

      // tilt2:

      ex=ex0;
      ey=fcos(vol_tlt2)*ey0-fsin(vol_tlt2)*ez0;
      ez=fsin(vol_tlt2)*ey0+fcos(vol_tlt2)*ez0;

      dx=dx0;
      dy=fcos(vol_tlt2)*dy0-fsin(vol_tlt2)*dz0;
      dz=fsin(vol_tlt2)*dy0+fcos(vol_tlt2)*dz0;

      ux=ux0;
      uy=fcos(vol_tlt2)*uy0-fsin(vol_tlt2)*uz0;
      uz=fsin(vol_tlt2)*uy0+fcos(vol_tlt2)*uz0;

      // tf setup:

      VOL->get_tfunc()->set_escale(fsqr(tf_re_scale),fsqr(tf_ge_scale),fsqr(tf_be_scale));
      VOL->get_tfunc()->set_ascale(fsqr(tf_ra_scale),fsqr(tf_ga_scale),fsqr(tf_ba_scale));

      VOL->get_tfunc()->set_invmode(vol_inv);

      VOL->get_tfunc()->refresh(vol_emi,vol_att,VOL->get_slab()*vol_over,
                                tf_premult,tf_preint,vol_light);

      // volren setup:

      VOL->set_light(0.01f,0.3f,0.5f,0.2f,10.0f);

      volume::usefbo(gfx_fbo);

      if (vol_white)
         if (!vol_inv) setbackground(0.85f,0.85f,0.85f);
         else setbackground(1.0f,1.0f,1.0f);
      else setbackground(0.0f,0.0f,0.0f);

      // render:

      clearbuffer();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(gfx_fovy,gfx_aspect,gfx_near,gfx_far);
      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(ex,ey,ez,ex+dx,ey+dy,ez+dz,ux,uy,uz);

      if (vol_wire) VOL->drawwireframe();

      if (vol_histo)
         if (VOL->has_data())
            VOL->get_histo()->render2DQ(VOL->getcenterx(),
                                        VOL->getcentery(),
                                        VOL->getcenterz(),
                                        VOL->getsizex(),
                                        VOL->getsizey(),
                                        VOL->getsizez());

      if (!vol_clip)
         aborted=VOL->render(ex,ey,ez,
                             dx,dy,dz,
                             ux,uy,uz,
                             gfx_near,VOL->get_slab()*vol_over,
                             vol_light,
                             abort,abortdata);
      else
         aborted=VOL->render(ex,ey,ez,
                             dx,dy,dz,
                             ux,uy,uz,
                             sqrt(ex*ex+ey*ey+ez*ez)-vol_clip_dist,VOL->get_slab()*vol_over,
                             vol_light,
                             abort,abortdata);

      glPopMatrix();

      return(aborted);
      }

   protected:

   mipmap *VOL;
   };

#endif
