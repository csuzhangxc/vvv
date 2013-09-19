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

#define VOLREN_DEFAULT_BRICKSIZE 128

#define VOLREN_DEFAULT_WIDTH 1536
#define VOLREN_DEFAULT_HEIGHT 768

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
      altpath_ = NULL;
      loading_ = false;

      fps_ = 30.0;
      angle_ = 0.0;
      omega_ = 0.0;
      tiltXY_ = tiltYZ_ = 0.0;
      tilt_ = 0.0;
      zoom_ = 0.0;
      dist_ = 1.0;
      oversampling_ = 1.0;
      red_ = VOLREN_DEFAULT_RED;
      green_ = VOLREN_DEFAULT_GREEN;
      blue_ = VOLREN_DEFAULT_BLUE;
      emi_ = 0.25;
      att_ = 0.25;
      emi_gm_ = 0.25;
      att_gm_ = 0.25;
      inv_ = false;
      gm_ = false;
      tf_ = false;
      tf_center_ = 0.5f;
      tf_size_ = 1.0f;
      tf_inverse_ = false;

      rendercount_ = 0;

      bLeftButtonDown = false;
      bRightButtonDown = false;

      startTimer((int)(1000.0/fps_)); // ms=1000/fps
   }

   //! dtor
   virtual ~QGLVolRenWidget()
   {
      if (vr_)
         delete vr_;

      if (toload_)
         free(toload_);

      if (altpath_)
         free(altpath_);
   }

   //! load a volume
   void loadVolume(const char *filename,
                   const char *altpath=NULL)
   {
      if (loading_) return;

      if (toload_) free(toload_);
      toload_ = strdup(filename);

      if (altpath_)
      {
         free(altpath_);
         altpath_ = NULL;
      }

      if (altpath)
         altpath_ = strdup(altpath);
   }

   //! load a DICOM series
   void loadSeries(const std::vector<std::string> list)
   {
      if (loading_) return;

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
   void setTiltXY(double tiltXY=0.0)
   {
      tiltXY_=tiltXY;
   }

   //! set volume rotation angle
   void setTiltYZ(double tiltYZ=0.0)
   {
      tiltYZ_=tiltYZ;
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

   //! set oversampling rate
   void setOversampling(double rate=1.0)
      {oversampling_=1.0/rate;}

   //! set default color
   void setColor(float r,float g,float b)
   {
      red_=r;
      green_=g;
      blue_=b;
   }

   //! set emission
   void setEmission(double emi=0.0)
   {
      if (gm_)
         emi_gm_=emi;
      else
         emi_=emi;
   }

   //! get emission
   double getEmission()
      {return(gm_?emi_gm_:emi_);}

   //! set absorption
   void setAbsorption(double att=0.0)
   {
      if (gm_)
         att_gm_=att;
      else
         att_=att;
   }

   //! get absorption
   double getAbsorption()
      {return(gm_?att_gm_:att_);}

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
         vr_->get_tfunc()->set_mode(7);
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

      tf_center_=center;
      tf_size_=size;
      tf_inverse_=inverse;
   }

   //! get eye point
   void getEyePoint(double &ex,double &ey,double &ez,
                    double &dx,double &dy,double &dz,
                    double &ux,double &uy,double &uz)
   {
      vr_->get_eye(ex,ey,ez, dx,dy,dz, ux,uy,uz);
   }

   //! get near plane
   void getNearPlane(double &px,double &py,double &pz,
                     double &nx,double &ny,double &nz)
   {
      vr_->get_near(px,py,pz, nx,ny,nz);
   }

   //! define clip plane via point on plane and normal
   void setClipPlane(int n,
                     double px,double py,double pz,
                     double nx,double ny,double nz)
   {
      vr_->define_clip(n, px,py,pz, nx,ny,nz);
   }

   //! enable clip plane
   void enableClipPlane(int n,int on)
   {
      vr_->enable_clip(n,on);
   }

   //! disable all clip planes
   void disableClipPlanes()
   {
      vr_->disable_clip();
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
      return(QSize(VOLREN_DEFAULT_WIDTH, VOLREN_DEFAULT_HEIGHT));
   }

protected:

   volren *vr_;

   char *toload_,*altpath_;
   std::vector<std::string> series_;
   bool loading_;

   double fps_; // animated frames per second
   double omega_; // rotation speed in degrees/s
   double angle_; // rotation angle in degrees
   double tiltXY_,tiltYZ_; // rotation angle in degrees
   double tilt_; // tilt angle in degrees
   double zoom_; // zoom into volume
   double dist_; // clipping distance
   double oversampling_; // oversampling rate
   double red_,green_,blue_; // default color
   double emi_; // volume emission
   double att_; // volume absorption
   double emi_gm_; // volume emission for gradmag
   double att_gm_; // volume absorption for gradmag
   bool inv_; // inverse mode?
   bool gm_; // gradient magnitude?
   bool tf_; // tfunc given?
   float tf_center_; // tfunc center
   float tf_size_; // tfunc size
   float tf_inverse_; // inverse tfunc

   unsigned int rendercount_;

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
         vr_ = new volren();

      double eye_x=0,eye_y=0,eye_z=2;
      double eye_dx=0,eye_dy=0,eye_dz=-1;
      double eye_ux=0,eye_uy=1,eye_uz=0;

      double gfx_fovy=60.0;
      double gfx_aspect=(double)width()/height();
      double gfx_near=0.01;
      double gfx_far=10.0;

      bool gfx_fbo=true;

#ifdef MACOSX
      // fbo bugfix for macos x 10.5
      if (rendercount_<5) gfx_fbo=false;
#endif

      double vol_emission=1000.0;
      double vol_density=1000.0;

      double tf_re_scale,tf_ge_scale,tf_be_scale;
      double tf_ra_scale,tf_ga_scale,tf_ba_scale;

      if (gm_)
         {
         tf_re_scale=tf_ge_scale=tf_be_scale=1.5*emi_gm_;
         tf_ra_scale=tf_ga_scale=tf_ba_scale=att_gm_;
         }
      else
         {
         tf_re_scale=tf_ge_scale=tf_be_scale=emi_;
         tf_ra_scale=tf_ga_scale=tf_ba_scale=att_;
         }

      double vol_over=oversampling_;

      // zoom
      eye_z=(1.0-zoom_)*eye_z+zoom_*gfx_near;

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
                  tiltXY_,tiltYZ_, // volume rotation in degrees
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
                  dist_); // clipping distance relative to origin

      // show histogram and tfunc
      if (vr_->has_data() && bLeftButtonDown)
         {
         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();
         glLoadIdentity();
         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
         glLoadIdentity();
         gluOrtho2D(0.0f,1.0f,0.0f,1.0f);
         glMatrixMode(GL_MODELVIEW);

         glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
         glEnable(GL_BLEND);

         if (!gm_)
         {
            // show 1D histogram
            for (int i=0; i<256; i++)
               qgl_drawquad((float)i/256,0.0f,1.0f/256,vr_->get_hist()[i],
                            vr_->get_histRGBA()[4*i],vr_->get_histRGBA()[4*i+1],vr_->get_histRGBA()[4*i+2],
                            0.5f*vr_->get_histRGBA()[4*i+3]);
         }
         else
         {
            // show 2D histogram
            unsigned int texid;
            texid=qgl_buildtexmap2DRGBA(vr_->get_hist2DQRGBA(),256,256);
            qgl_drawtexture(0.0f,0.0f,1.0f,1.0f,texid,256,256,0.5f);
            qgl_deletetexmap(texid);
         }

         // show windowing
         qgl_drawquad(tf_center_-0.5f*tf_size_,0.0f,tf_size_,1.0f,0.5f,0.5f,0.5f,0.25f);

         glDisable(GL_BLEND);

         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();
         glMatrixMode(GL_PROJECTION);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);
         }

      angle_+=omega_/fps_;

      if (vr_->has_data()) rendercount_++;
   }

   void timerEvent(QTimerEvent *)
   {
      repaint();
      updateVolume();
   }

   void updateVolume()
   {
      if (vr_)
      {
         if (toload_)
         {
            loading_ = true;

            delete vr_;
            vr_ = new volren();

            volren *vr = new volren();

            // try to load from regular file path
            if (!loadFile(vr, toload_))
               if (altpath_!=NULL)
                  {
                  char *toload=strdup2(altpath_,toload_);

                  // load from alternate file path
                  loadFile(vr, toload);

                  free(toload);
                  }

            free(toload_);
            toload_=NULL;

            delete vr_;
            vr_ = vr;

            vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);
            setGradMag(gm_);

            loading_ = false;
         }

         if (series_.size()>0)
         {
            loading_ = true;

            delete vr_;
            vr_ = new volren();

            volren *vr = new volren();

            vr->loadseries(series_,
                           0.0f,0.0f,0.0f,
                           1.0f,1.0f,1.0f,
                           128,8.0f,
                           FALSE,FALSE,FALSE,
                           FALSE,FALSE,
                           TRUE,
                           5,5.0f,1,1.0f,
                           feedback,this);

            series_.clear();

            delete vr_;
            vr_ = vr;

            vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);
            setGradMag(gm_);

            loading_ = false;
         }
      }
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
         {
            tf_center_ = x;
            tf_size_ = 1.0f-y;
            tf_inverse_ = !shift;

            vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);
         }
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

   virtual void update(const char *info,float percent)
   {
      int act;
      static int last=0;

      if (percent>0.0f)
      {
         act=(int)(100.0f*percent+0.5f);
         if (act>last) printf("%s: %d%%\n",info,act);
         last=act;
      }
      else
      {
         printf("%s\n",info);
         last=0;
      }
   }

   static void feedback(const char *info,float percent,void *obj)
   {
      QGLVolRenWidget *w=(QGLVolRenWidget *)obj;
      w->update(info,percent);
   }

protected:

   BOOLINT loadFile(volren *vr, const char *filename)
   {
      int histmin = 5;
      float histfreq = 7.0f;
      int kneigh = 2;
      float histstep = 2.0f;

      return(vr->loadvolume(filename,NULL,
                            0.0f,0.0f,0.0f,
                            1.0f,1.0f,1.0f,
                            VOLREN_DEFAULT_BRICKSIZE,8.0f,
                            FALSE,FALSE,FALSE,
                            FALSE,FALSE,
                            TRUE,
                            NULL,
                            histmin,histfreq,kneigh,histstep,
                            feedback,this));
   }

   void qgl_drawquad(float x,float y,float width,float height,
                     float r,float g,float b,float alpha)
   {
      glColor4f(r,g,b,alpha);
      glBegin(GL_QUADS);
      glVertex2f(x,y);
      glVertex2f(x+width,y);
      glVertex2f(x+width,y+height);
      glVertex2f(x,y+height);
      glEnd();
   }

   unsigned int qgl_buildtexmap2DRGBA(float *image,int width,int height)
   {
      GLuint texid;

      glGenTextures(1,&texid);
      glBindTexture(GL_TEXTURE_2D,texid);

      glPixelStorei(GL_UNPACK_ALIGNMENT,1);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,
                   GL_RGBA,GL_FLOAT,image);

      glBindTexture(GL_TEXTURE_2D,0);

      return(texid);
   }

   void qgl_deletetexmap(unsigned int texid)
   {
      GLuint GLtexid=texid;
      if (texid>0) glDeleteTextures(1,&GLtexid);
   }

   void qgl_drawtexture(float x,float y,float width,float height,
                        int texid,int sizex,int sizey,float alpha=1.0f)
   {
      glEnable(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D,texid);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.5f/sizex,0.5f/sizey,0.0f);
      glScalef((float)(sizex-1)/sizex,(float)(sizey-1)/sizey,1.0f);
      glMatrixMode(GL_MODELVIEW);

      glColor4f(1.0f,1.0f,1.0f,alpha);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f,0.0f);
      glVertex2f(x,y);
      glTexCoord2f(1.0f,0.0f);
      glVertex2f(x+width,y);
      glTexCoord2f(1.0f,1.0f);
      glVertex2f(x+width,y+height);
      glTexCoord2f(0.0f,1.0f);
      glVertex2f(x,y+height);
      glEnd();

      glMatrixMode(GL_TEXTURE);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);

      glBindTexture(GL_TEXTURE_2D,0);

      glDisable(GL_TEXTURE_2D);
   }

};

#endif
