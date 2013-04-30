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

   void render(float eye_x,float eye_y,float eye_z, // eye point
               float eye_dx,float eye_dy,float eye_dz, // viewing direction
               float eye_ux,float eye_uy,float eye_uz, // up vector
               float gfx_fovy,float gfx_aspect,float gfx_near,float gfx_far, // opengl perspective
               BOOLINT gfx_fbo, // use frame buffer object
               BOOLINT gfx_resized, // resize frame buffer object
               float vol_rot,float vol_height, // volume rotation and elevation
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
               BOOLINT vol_histo=FALSE) // histogram
      {
      float ex,ey,ez,
            dx,dy,dz,
            ux,uy,uz;

      ex=fcos(vol_rot*2.0f*PI)*eye_x+fsin(vol_rot*2.0f*PI)*eye_z;
      ey=eye_y+4.0f*(vol_height-0.5f);
      ez=-fsin(vol_rot*2.0f*PI)*eye_x+fcos(vol_rot*2.0f*PI)*eye_z;

      dx=fcos(vol_rot*2.0f*PI)*eye_dx+fsin(vol_rot*2.0f*PI)*eye_dz;
      dy=eye_dy;
      dz=-fsin(vol_rot*2.0f*PI)*eye_dx+fcos(vol_rot*2.0f*PI)*eye_dz;

      ux=fcos(vol_rot*2.0f*PI)*eye_ux+fsin(vol_rot*2.0f*PI)*eye_uz;
      uy=eye_uy;
      uz=-fsin(vol_rot*2.0f*PI)*eye_ux+fcos(vol_rot*2.0f*PI)*eye_uz;

      VOL->get_tfunc()->set_escale(fsqr(tf_re_scale),fsqr(tf_ge_scale),fsqr(tf_be_scale));
      VOL->get_tfunc()->set_ascale(fsqr(tf_ra_scale),fsqr(tf_ga_scale),fsqr(tf_ba_scale));

      VOL->get_tfunc()->set_invmode(vol_inv);

      VOL->get_tfunc()->refresh(vol_emi,vol_att,VOL->get_slab()*vol_over,
                                tf_premult,tf_preint,vol_light);

      VOL->set_light(0.01f,0.3f,0.5f,0.2f,10.0f);

      if (vol_white)
         if (!vol_inv) setbackground(0.85f,0.85f,0.85f);
         else setbackground(1.0f,1.0f,1.0f);
      else setbackground(0.0f,0.0f,0.0f);

      clearwindow();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(gfx_fovy,gfx_aspect,gfx_near,gfx_far);
      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      gluLookAt(ex,ey,ez,ex+dx,ey+dy,ez+dz,ux,uy,uz);

      volume::usefbo(gfx_fbo);
      if (gfx_fbo)
         if (gfx_resized) volume::updatefbo();

      if (vol_wire) VOL->drawwireframe();

      if (vol_histo) VOL->get_histo()->render2DQ(VOL->getcenterx(),
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

      glPopMatrix();
      }

   protected:

   mipmap *VOL;
   };

#endif
