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
      {VOL=new mipmap(base);}

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
                      int histmin=5,float histfreq=5.0f,int kneigh=1,float histstep=1.0f)
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
                             histmin,histfreq,kneigh,histstep));
      }

   // save the volume data as PVM
   void savePVMvolume(const char *filename)
      {VOL->savePVMvolume(filename);}

   // render the volume mipmap pyramid
   void render(float eye_x,float eye_y,float eye_z, // eye point
               float eye_dx,float eye_dy,float eye_dz, // viewing direction
               float eye_ux,float eye_uy,float eye_uz, // up vector
               float gfx_fovy,float gfx_aspect,float gfx_near,float gfx_far, // opengl perspective
               BOOLINT gfx_fbo, // use frame buffer object
               float vol_rot, // volume rotation in degrees
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
               BOOLINT vol_histo=FALSE) // spatialized histogram
      {
      float ex,ey,ez,
            dx,dy,dz,
            ux,uy,uz;

      vol_rot*=PI/180.0f;

      ex=fcos(vol_rot)*eye_x+fsin(vol_rot)*eye_z+vol_dx;
      ey=eye_y+vol_dy;
      ez=-fsin(vol_rot)*eye_x+fcos(vol_rot)*eye_z+vol_dz;

      dx=fcos(vol_rot)*eye_dx+fsin(vol_rot)*eye_dz;
      dy=eye_dy;
      dz=-fsin(vol_rot)*eye_dx+fcos(vol_rot)*eye_dz;

      ux=fcos(vol_rot)*eye_ux+fsin(vol_rot)*eye_uz;
      uy=eye_uy;
      uz=-fsin(vol_rot)*eye_ux+fcos(vol_rot)*eye_uz;

      VOL->get_tfunc()->set_escale(fsqr(tf_re_scale),fsqr(tf_ge_scale),fsqr(tf_be_scale));
      VOL->get_tfunc()->set_ascale(fsqr(tf_ra_scale),fsqr(tf_ga_scale),fsqr(tf_ba_scale));

      VOL->get_tfunc()->set_invmode(vol_inv);

      VOL->get_tfunc()->refresh(vol_emi,vol_att,VOL->get_slab()*vol_over,
                                tf_premult,tf_preint,vol_light);

      VOL->set_light(0.01f,0.3f,0.5f,0.2f,10.0f);

      initogl();

      volume::usefbo(gfx_fbo);

      if (vol_white)
         if (!vol_inv) setbackground(0.85f,0.85f,0.85f);
         else setbackground(1.0f,1.0f,1.0f);
      else setbackground(0.0f,0.0f,0.0f);

      clearbuffer();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(gfx_fovy,gfx_aspect,gfx_near,gfx_far);
      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(ex,ey,ez,ex+dx,ey+dy,ez+dz,ux,uy,uz);

      if (vol_wire) VOL->drawwireframe();

      if (VOL->has_data())
         {
         if (vol_histo)
            VOL->get_histo()->render2DQ(VOL->getcenterx(),
                                        VOL->getcentery(),
                                        VOL->getcenterz(),
                                        VOL->getsizex(),
                                        VOL->getsizey(),
                                        VOL->getsizez());

         if (!vol_clip)
            VOL->render(ex,ey,ez,
                        dx,dy,dz,
                        ux,uy,uz,
                        gfx_near,VOL->get_slab()*vol_over,
                        vol_light);
         else
            VOL->render(ex,ey,ez,
                        dx,dy,dz,
                        ux,uy,uz,
                        sqrt(ex*ex+ey*ey+ez*ez)-vol_clip_dist,VOL->get_slab()*vol_over,
                        vol_light);
         }

      glPopMatrix();
      }

   protected:

   mipmap *VOL;
   };

#endif
