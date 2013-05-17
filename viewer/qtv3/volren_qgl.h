// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_QGL_H
#define VOLREN_QGL_H

#include <QtOpenGL/qgl.h>

#include "volren.h"

class QGLVolRenWidget: public QGLWidget
{
public:

   //! default ctor
   QGLVolRenWidget(QWidget *parent = 0)
      : QGLWidget(parent)
   {
      setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));

      vr_ = NULL;
      toload_ = NULL;

      fps_=30.0;
      startTimer((int)(1000.0/fps_)); // ms=1000/fps

      resized_=false;
   }

   //! dtor
   ~QGLVolRenWidget()
   {
      if (vr_)
         delete vr_;
   }

   //! load a volume
   void loadvolume(const char *filename)
   {
      if (toload_) free(toload_);
      toload_ = strdup(filename);
   }

   //! return preferred minimum window size
   QSize minimumSizeHint() const
   {
      return(QSize(100, 100));
   }

   //! return preferred window size
   QSize sizeHint() const
   {
      return(QSize(512, 512));
   }

protected:

   volren *vr_;
   char *toload_;

   double fps_; // animated frames per second
   bool resized_; // viewport resized?

   void initializeGL()
   {
      qglClearColor(Qt::white);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
   }

   void resizeGL(int, int)
   {
      glViewport(0, 0, width(), height());
      resized_=true;
   }

   void paintGL()
   {
      if (!vr_)
         vr_ = new volren();

      if (toload_)
         {
         vr_->loadvolume(toload_);
         free(toload_);
         toload_=NULL;
         }

      double eye_x=0,eye_y=0,eye_z=2.5;
      double eye_dx=0,eye_dy=0,eye_dz=-1;
      double eye_ux=0,eye_uy=1,eye_uz=0;

      double gfx_fovy=60.0;
      double gfx_aspect=(double)width()/height();
      double gfx_near=0.01;
      double gfx_far=10.0;

      bool gfx_fbo=true;
      bool gfx_resized=resized_;
      resized_=false;

      double vol_emission=1000.0;
      double vol_density=1000.0;

      double tf_re_scale=0.25,tf_ge_scale=0.25,tf_be_scale=0.25;
      double tf_ra_scale=0.25,tf_ga_scale=0.25,tf_ba_scale=0.25;

      double vol_over=1.0;

      static double angle=0.0; // rotation angle in degrees
      static const double omega=30.0; // rotation speed in degrees/s

      // tf emission (emi)
      vr_->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,vr_->get_tfunc()->get_be());

      // tf absorption (att)
      vr_->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,vr_->get_tfunc()->get_ra());
      vr_->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,vr_->get_tfunc()->get_ga());
      vr_->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,vr_->get_tfunc()->get_ba());

      vr_->render(eye_x,eye_y,eye_z, // view point
                  eye_dx,eye_dy,eye_dz, // viewing direction
                  eye_ux,eye_uy,eye_uz, // up vector
                  gfx_fovy,gfx_aspect,gfx_near,gfx_far, // frustum
                  gfx_fbo,gfx_resized, // use fbo
                  angle, // volume rotation in degrees
                  0.0f,0.0,0.0f, // volume translation
                  vol_emission,vol_density, // global emi and att
                  tf_re_scale,tf_ge_scale,tf_be_scale, // emi scale
                  tf_ra_scale,tf_ga_scale,tf_ba_scale, // att scale
                  TRUE,TRUE, // pre-multiplication and pre-integration
                  TRUE, // white background
                  FALSE, // inverse mode
                  vol_over, // oversampling
                  FALSE, // lighting
                  FALSE, // view-aligned clipping
                  0.0, // clipping distance relative to origin
                  TRUE); // wire frame box

      angle+=omega/fps_;
   }

   void timerEvent(QTimerEvent *)
   {
      repaint();
   }

};

#endif
