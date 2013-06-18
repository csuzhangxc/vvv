// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_QGL_H
#define VOLREN_QGL_H

#include "volren.h"

#include <QtOpenGL/qgl.h>

#include <QMouseEvent>
#include <QWheelEvent>

#define VOLREN_DEFAULT_RED 0.5f
#define VOLREN_DEFAULT_GREEN 1.0f
#define VOLREN_DEFAULT_BLUE 0.5f

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
      angle_=0.0;
      omega_=0.0;
      tilt1_=tilt2_=0.0;
      tilt_=0.0;
      zoom_=0.0;
      dist_=1.0;
      red_=VOLREN_DEFAULT_RED;
      green_=VOLREN_DEFAULT_GREEN;
      blue_=VOLREN_DEFAULT_BLUE;
      emi_=0.25;
      att_=0.25;
      inv_=false;
      gm_=false;
      tf_=false;

      bLeftButtonDown = false;
      bRightButtonDown = false;

      startTimer((int)(1000.0/fps_)); // ms=1000/fps
   }

   //! dtor
   virtual ~QGLVolRenWidget()
   {
      if (vr_)
         delete vr_;
   }

   //! load a volume
   void loadVolume(const char *filename)
   {
      if (toload_) free(toload_);
      toload_ = strdup(filename);
   }

   //! load a DICOM series
   void loadSeries(const std::vector<std::string> list)
   {
      series_ = list;
   }

   //! set volume rotation speed
   void setRotation(double omega=30.0)
      {omega_=omega;}

   //! get volume rotation speed
   double getRotation()
      {return(omega_);}

   //! set volume rotation angle
   void setAngle(double angle=0.0)
   {
      omega_=0.0;
      angle_=angle;
   }

   //! set volume rotation angle
   void setTilt1(double tilt1=0.0)
   {
      tilt1_=tilt1;
   }

   //! set volume rotation angle
   void setTilt2(double tilt2=0.0)
   {
      tilt2_=tilt2;
   }

   //! set volume tilt angle
   void setTilt(double tilt=0.0)
   {
      if (tilt>=-90 && tilt<=90)
         tilt_=tilt;
   }

   //! set zoom factor
   void setZoom(double zoom=0.0)
   {
      if (zoom>=0.0 && zoom<=1.0)
         zoom_=zoom;
   }

   //! set clipping distance
   void setClipDist(double dist=0.0)
      {dist_=dist;}

   //! set default color
   void setColor(float r,float g,float b)
   {
      red_=r;
      green_=g;
      blue_=b;
   }

   //! set emission
   void setEmission(double emi=0.0)
      {emi_=emi;}

   //! set absorption
   void setAbsorption(double att=0.0)
      {att_=att;}

   //! set inverse mode
   void setInvMode(bool on=false)
      {inv_=on;}

   //! set gradient magnitude mode
   void setGradMag(bool on=false)
      {
      gm_=on;

      if (gm_ && vr_->has_grad())
         {
         vr_->get_tfunc()->set_num(32);
         vr_->get_tfunc()->set_mode(3);
         }
      else
         {
         vr_->get_tfunc()->set_num(1);
         vr_->get_tfunc()->set_mode(0);
         }
      }

   //! use linear transfer function
   void set_tfunc(float center=0.5f,float size=1.0f,
                  BOOLINT inverse=FALSE)
      {
      if (vr_)
         vr_->set_tfunc(center,size, red_,green_,blue_, inverse);

      tf_=true;
      }

   //! return volume renderer
   volren *getVR()
      {return(vr_);}

   //! return preferred minimum window size
   QSize minimumSizeHint() const
   {
      return(QSize(100, 100));
   }

   //! return preferred window size
   QSize sizeHint() const
   {
      return(QSize(768, 768));
   }

protected:

   volren *vr_;
   char *toload_;
   std::vector<std::string> series_;

   double fps_; // animated frames per second
   double omega_; // rotation speed in degrees/s
   double angle_; // rotation angle in degrees
   double tilt1_,tilt2_; // rotation angle in degrees
   double tilt_; // tilt angle in degrees
   double zoom_; // zoom into volume
   double dist_; // clipping distance
   double red_,green_,blue_; // default color
   double emi_; // volume emission
   double att_; // volume absorption
   bool inv_; // inverse mode?
   bool gm_; // gradient magnitude?
   bool tf_; // tfunc given?

   void initializeGL()
   {
      qglClearColor(Qt::white);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
   }

   void resizeGL(int, int)
   {
      glViewport(0, 0, width(), height());
   }

   void paintGL()
   {
      if (!vr_)
         {
         vr_ = new volren();

         // linear transfer function
         if (!tf_)
            vr_->set_tfunc(0.5f,1.0f, red_,green_,blue_, FALSE);
         }

      if (toload_)
         {
         vr_->loadvolume(toload_,NULL,
                         0.0f,0.0f,0.0f,
                         1.0f,1.0f,1.0f,
                         128,8.0f,
                         FALSE,FALSE,FALSE,
                         FALSE,FALSE,
                         TRUE);

         free(toload_);
         toload_=NULL;
         }

      if (series_.size()>0)
         {
         vr_->loadseries(series_,
                         0.0f,0.0f,0.0f,
                         1.0f,1.0f,1.0f,
                         128,8.0f,
                         FALSE,FALSE,FALSE,
                         FALSE,FALSE,
                         TRUE);

         series_.clear();
         }

      double eye_x=0,eye_y=0,eye_z=2;
      double eye_dx=0,eye_dy=0,eye_dz=-1;
      double eye_ux=0,eye_uy=1,eye_uz=0;

      double gfx_fovy=60.0;
      double gfx_aspect=(double)width()/height();
      double gfx_near=0.01;
      double gfx_far=10.0;

      bool gfx_fbo=true;

      double vol_emission=1000.0;
      double vol_density=1000.0;

      double tf_re_scale=emi_,tf_ge_scale=emi_,tf_be_scale=emi_;
      double tf_ra_scale=att_,tf_ga_scale=att_,tf_ba_scale=att_;

      double vol_over=1.0;

      // zoom
      eye_z=(1.0-zoom_)*eye_z+zoom_*0.5;

      // tilt
      double eye_tx=eye_x;
      double eye_ty=cos(tilt_*PI/180)*eye_y+sin(tilt_*PI/180)*eye_z;
      double eye_tz=-sin(tilt_*PI/180)*eye_y+cos(tilt_*PI/180)*eye_z;
      double eye_tdx=eye_dx;
      double eye_tdy=cos(tilt_*PI/180)*eye_dy+sin(tilt_*PI/180)*eye_dz;
      double eye_tdz=-sin(tilt_*PI/180)*eye_dy+cos(tilt_*PI/180)*eye_dz;
      double eye_tux=eye_ux;
      double eye_tuy=cos(tilt_*PI/180)*eye_uy+sin(tilt_*PI/180)*eye_uz;
      double eye_tuz=-sin(tilt_*PI/180)*eye_uy+cos(tilt_*PI/180)*eye_uz;

      // call volume renderer
      vr_->render(eye_tx,eye_ty,eye_tz, // view point
                  eye_tdx,eye_tdy,eye_tdz, // viewing direction
                  eye_tux,eye_tuy,eye_tuz, // up vector
                  gfx_fovy,gfx_aspect,gfx_near,gfx_far, // frustum
                  gfx_fbo, // use fbo
                  angle_, // volume rotation in degrees
                  tilt1_,tilt2_, // volume rotation in degrees
                  0.0f,0.0f,0.0f, // volume translation
                  vol_emission,vol_density, // global emi and att
                  tf_re_scale,tf_ge_scale,tf_be_scale, // emi scale
                  tf_ra_scale,tf_ga_scale,tf_ba_scale, // att scale
                  TRUE,TRUE, // pre-multiplication and pre-integration
                  TRUE, // white background
                  inv_, // inverse mode
                  vol_over, // oversampling
                  TRUE, // lighting
                  TRUE, // view-aligned clipping
                  dist_, // clipping distance relative to origin
                  TRUE); // wire frame box

      angle_+=omega_/fps_;
   }

   void timerEvent(QTimerEvent *)
   {
      repaint();
   }

   bool bLeftButtonDown,bRightButtonDown;

   void mousePressEvent(QMouseEvent *event)
   {
      if (event->buttons() & Qt::LeftButton)
         if (QApplication::keyboardModifiers() & Qt::ControlModifier ||
             QApplication::keyboardModifiers() & Qt::AltModifier)
            bRightButtonDown = true;
         else
            bLeftButtonDown = true;
      else if (event->buttons() & Qt::RightButton)
         bRightButtonDown = true;
      else
         event->ignore();
   }

   void mouseReleaseEvent(QMouseEvent *event)
   {
      mouseMoveEvent(event);

      bLeftButtonDown = false;
      bRightButtonDown = false;
   }

   void mouseDoubleClickEvent(QMouseEvent *event)
   {
      mouseMoveEvent(event);

      bLeftButtonDown = false;
      bRightButtonDown = false;
   }

   void mouseMoveEvent(QMouseEvent *event)
   {
      float x = (float)(event->x())/width();
      float y = (float)(event->y())/height();

      bool shift = QApplication::keyboardModifiers() & Qt::ShiftModifier;

      if (!tf_)
         if (bLeftButtonDown)
            vr_->set_tfunc(x,1.0f-y, red_,green_,blue_, !shift);
         else if (bRightButtonDown)
            if (getRotation()==0.0)
               setRotation(shift?-10.0:10.0);
            else
               setRotation(0.0);
         else
            event->ignore();
      else
         event->ignore();
   }

   void wheelEvent(QWheelEvent *event)
   {
      double numDegrees = event->delta()/8.0;

      event->accept();
   }

};

#endif
