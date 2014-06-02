// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_QGL_H
#define VOLREN_QGL_H

#ifdef HAVE_QT5
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QtOpenGL/qgl.h>

#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

#include <vector>

#include "volren.h"

#ifdef MACOSX
#include <AvailabilityMacros.h>
#endif

#define VOLREN_DEFAULT_HUE 120.0f
#define VOLREN_DEFAULT_SAT 0.75f
#define VOLREN_DEFAULT_VAL 1.0f

#define VOLREN_DEFAULT_BRICKSIZE 128

#define VOLREN_DEFAULT_WIDTH 800
#define VOLREN_DEFAULT_HEIGHT 600

class volren_qgl: public volren
{
public:

   volren_qgl() : volren() {}
   virtual ~volren_qgl() {}

   void showCross(bool on,
                  float cx=0.0f,float cy=0.0f,float cz=0.0f)
   {
      cross_=on;

      cx_=cx;
      cy_=cy;
      cz_=cz;
   }

   void appendLine(const v3d &p)
   {
      line_.push_back(p);
   }

   void clearLine()
   {
      line_.clear();
   }

   double getLength()
   {
      double l=0.0;

      for (unsigned int i=1; i<line_.size(); i++)
         l+=(line_[i]-line_[i-1]).getlength();

      return(l);
   }

   double getEndLength()
   {
      double l=0.0;

      if (line_.size()>1)
         l=(line_[line_.size()-1]-line_[0]).getlength();

      return(l);
   }

protected:

   bool cross_;
   float cx_,cy_,cz_;

   std::vector<v3d> line_;

   virtual void rendermetric()
   {
      float vx,vy,vz;

      vx=getvoxelx();
      vy=getvoxely();
      vz=getvoxelz();

      // show rotation cross
      if (cross_==TRUE)
      {
         static const float s=3.0f; // size of cross in voxels

         glBegin(GL_LINES);
         glColor3f(1.0f,0.0f,0.0f);
         glVertex3f(cx_-s*vx,cy_,cz_);
         glVertex3f(cx_+s*vx,cy_,cz_);
         glColor3f(0.0f,1.0f,0.0f);
         glVertex3f(cx_,cy_-s*vy,cz_);
         glVertex3f(cx_,cy_+s*vy,cz_);
         glColor3f(0.0f,0.0f,1.0f);
         glVertex3f(cx_,cy_,cz_-s*vz);
         glVertex3f(cx_,cy_,cz_+s*vz);
         glEnd();
      }

      // render measurement line (with starting cross)
      if (line_.size()>0)
      {
         static const float s=1.0f; // size of cross in voxels
         static const float scale=0.99f; // projection scale factor

         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
         glScalef(scale,scale,scale);
         glMatrixMode(GL_MODELVIEW);

         glBegin(GL_LINES);
         glColor3f(1.0f,0.0f,0.0f);
         glVertex3f(line_[0].x-s*vx,line_[0].y,line_[0].z);
         glVertex3f(line_[0].x+s*vx,line_[0].y,line_[0].z);
         glColor3f(0.0f,1.0f,0.0f);
         glVertex3f(line_[0].x,line_[0].y-s*vy,line_[0].z);
         glVertex3f(line_[0].x,line_[0].y+s*vy,line_[0].z);
         glColor3f(0.0f,0.0f,1.0f);
         glVertex3f(line_[0].x,line_[0].y,line_[0].z-s*vz);
         glVertex3f(line_[0].x,line_[0].y,line_[0].z+s*vz);
         glEnd();

         glColor3f(1.0f,0.0f,1.0f);
         glBegin(GL_LINE_STRIP);
         for (unsigned int i=0; i<line_.size(); i++)
            glVertex3d(line_[i].x,line_[i].y,line_[i].z);
         glEnd();

         glMatrixMode(GL_PROJECTION);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);
      }
   }

};

class QGLVolRenWidget: public QGLWidget
{
public:

   enum InteractionMode {
      InteractionMode_Window,
      InteractionMode_Move,
      InteractionMode_RotateAxis,
      InteractionMode_RotateCenter,
      InteractionMode_RotateAnchor,
      InteractionMode_Zoom,
      InteractionMode_Clip,
      InteractionMode_Opacity,
      InteractionMode_Measure
   };

   //! default ctor
   QGLVolRenWidget(QWidget *parent = 0, bool stereo = false)
      : QGLWidget(parent)
   {
      if (!stereo) setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));
      else setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::StereoBuffers));

      vr_ = NULL;

      toload_ = NULL;
      altpath_ = NULL;
      geotoload_ = NULL;
      loading_ = false;

      set_vol_maxsize(512);
      set_iso_maxsize(256);

      setSliceOpacity(0.75f);
      setOuterOpacity(0.1f);

      eye_x_ = 0;
      eye_y_ = 0;
      eye_z_ = 2;

      eye_dx_ = 0;
      eye_dy_ = 0;
      eye_dz_ = -1;

      eye_ux_ = 0;
      eye_uy_ = 1;
      eye_uz_ = 0;

      fps_ = 30.0;
      fovy_ = 60.0;
      angle_ = 0.0;
      omega_ = 0.0;
      tiltXY_ = tiltYZ_ = 0.0;
      tilt_ = 0.0;
      zoom_ = 0.0;
      vol_dx_ = vol_dy_ = vol_dz_ = 0.0;
      clipdist_ = 1.0;
      clipgeo_ = TRUE;
      opaque_ = FALSE;
      opacity_ = 0.75f;
      opacity2_ = 0.1f;
      oversampling_ = 1.0;
      emi_ = 0.25;
      att_ = 0.25;
      emi_gm_ = 0.25;
      att_gm_ = 0.25;
      inv_ = false;
      gm_ = false;
      sfx_ = false;
      sfx_ana_ = true;
      tf_ = false;
      tf_center_ = 0.5f;
      tf_size_ = 1.0f;
      tf_inverse_ = false;
      geo_show_ = true;

      setColorHue(VOLREN_DEFAULT_HUE,VOLREN_DEFAULT_SAT,VOLREN_DEFAULT_VAL);

      rendercount_ = 0;

      mode_ = InteractionMode_Window;

      bLeftButtonDown = false;
      bMiddleButtonDown = false;
      bRightButtonDown = false;

      bMouseMove = false;
      mouseLastX = mouseLastY = 0.5f;

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

      if (geotoload_)
         free(geotoload_);
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

   //! check for volume
   bool hasVolume()
      {return(vr_->hasvolume());}

   //! extract a surface
   char *extractSurface()
   {
      char *surface;

      if (loading_) return(NULL);

      vr_->set_iso_maxsize(iso_maxsize_,iso_ratio_);
      surface=vr_->extractTFsurface(feedback,this);

      tf_center_ = 0.5f;
      tf_size_ = 1.0f;
      tf_inverse_ = false;

      vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);

      return(surface);
   }

   //! load a surface
   void loadSurface(const char *filename)
   {
      if (loading_) return;

      if (geotoload_) free(geotoload_);
      geotoload_ = strdup(filename);
   }

   //! check for surface
   bool hasSurface()
      {return(vr_->hassurface());}

   //! show the surface
   void showSurface(int on)
   {
      geo_show_ = on;
   }

   //! clear surface
   void clearSurface()
   {
      if (loading_) return;

      if (vr_)
         vr_->clearsurface();
   }

   //! clear the volume
   void clearVolume()
   {
      if (loading_) return;

      if (vr_)
         delete vr_;

      vr_ = new volren_qgl();

      if (toload_) free(toload_);
      toload_ = NULL;

      if (altpath_) free(altpath_);
      altpath_ = NULL;

      series_.clear();

      if (geotoload_) free(geotoload_);
      geotoload_ = NULL;
   }

   //! append line segment
   void appendLine(const v3d &p)
   {
      if (vr_)
         vr_->appendLine(p);
   }

   //! clear line segments
   void clearLine()
   {
   if (vr_)
      vr_->clearLine();
   }

   //! set limit for maximum displayable volume size
   void set_vol_maxsize(long long maxsize,
                        float ratio=0.25f)
   {
      vol_maxsize_=maxsize;
      vol_ratio_=ratio;
   }

   //! set limit for maximum volume size for iso surface extraction
   void set_iso_maxsize(long long maxsize,
                        float ratio=0.25f)
   {
      iso_maxsize_=maxsize;
      iso_ratio_=ratio;
   }

   //! set volume rotation speed
   void setRotation(double omega=30.0)
   {
      omega_=omega;
   }

   //! get volume rotation speed
   double getRotation()
   {
      return(omega_);
   }

   //! set volume rotation angle
   void setAngle(double angle=0.0)
   {
      omega_=0.0;
      angle_=angle;

      updated_rotation();
   }

   //! get volume rotation angle
   double getAngle()
   {
      return(angle_);
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
      {
         tilt_=tilt;

         updated_rotation();
      }
   }

   //! get volume tilt angle
   double getTilt()
   {
      return(tilt_);
   }

   //! set zoom factor
   void setZoom(double zoom=0.0)
   {
      if (zoom>=0.0 && zoom<=1.0)
      {
         zoom_=zoom;

         updated_zoom();
      }
   }

   //! get zoom factor
   double getZoom()
   {
      return(zoom_);
   }

   //! set clipping distance
   void setClipDist(double dist=0.0)
   {
      clipdist_=dist;

      updated_clipping();
   }

   //! get clipping distance
   double getClipDist()
   {
      return(clipdist_);
   }

   //! enable clipping of geometry at clipping distance
   void enableGeoClip(bool geo=FALSE)
   {
      clipgeo_=geo;
   }

   //! set clip plane opacity
   void setClipOpacity(bool opaque=FALSE)
   {
      opaque_=opaque;
   }

   //! set clip plane slice opacity
   void setSliceOpacity(float opacity=0.75f)
   {
      opacity_=opacity;
   }

   //! set outer plane slice opacity
   void setOuterOpacity(float opacity=0.1f)
   {
      opacity2_=opacity;
   }

   //! set oversampling rate
   void setOversampling(double rate=1.0)
      {oversampling_=1.0/rate;}

   //! set default color
   void setColor(float r,float g,float b)
   {
      red_=r;
      green_=g;
      blue_=b;

      if (vr_)
         vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);
   }

   //! set default color hue
   void setColorHue(float hue=120.0f,float sat=0.75f,float val=1.0f)
   {
      float rgb[3];

      tfunc::hsv2rgb(hue,sat,val,rgb);
      setColor(rgb[0],rgb[1],rgb[2]);
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
   {
      return(gm_?emi_gm_:emi_);
   }

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
   {
      return(gm_?att_gm_:att_);
   }

   //! set inverse mode
   void setInvMode(bool on=false)
   {
      inv_=on;
   }

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

   //! get gradient magnitude mode
   bool getGradMag()
   {
      return(gm_);
   }

   //! set stereo mode
   void setSFX(bool on=false)
   {
      sfx_=on;
   }

   //! get stereo mode
   bool getSFX()
   {
      return(sfx_);
   }

   //! set anaglyph mode
   void setAnaglyph(bool on=false)
   {
      sfx_ana_=on;
   }

   //! get anaglyph mode
   bool getAnaglyph()
   {
      return(sfx_ana_);
   }

   //! use linear transfer function
   void set_tfunc(float center=0.5f,float size=1.0f,
                  bool inverse=FALSE)
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
                    double &ux,double &uy,double &uz,
                    double &rx,double &ry,double &rz)
   {
      vr_->get_eye(ex,ey,ez, dx,dy,dz, ux,uy,uz, rx,ry,rz);
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
      if (vr_)
         vr_->define_clip(n, px,py,pz, nx,ny,nz);
   }

   //! enable clip plane
   void enableClipPlane(int n,int on)
   {
      if (vr_)
         vr_->enable_clip(n,on);
   }

   //! disable all clip planes
   void disableClipPlanes()
   {
      if (vr_)
         vr_->disable_clip();
   }

   //! return volume renderer
   volren_qgl *getVR()
   {
      return(vr_);
   }

   //! set interaction mode
   void setInteractionMode(int mode)
   {
      mode_=mode;
   }

   //! reset interactions
   void resetInteractions()
   {
      eye_x_ = 0;
      eye_y_ = 0;
      eye_z_ = 2;

      eye_dx_ = 0;
      eye_dy_ = 0;
      eye_dz_ = -1;

      eye_ux_ = 0;
      eye_uy_ = 1;
      eye_uz_ = 0;

      angle_ = 0.0;
      omega_ = 0.0;

      tilt_ = 0.0;

      updated_rotation();

      zoom_ = 0.0;

      updated_zoom();

      clipdist_ = 1.0;

      updated_clipping();
   }

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

   volren_qgl *vr_;

   char *toload_,*altpath_;
   std::vector<std::string> series_;
   char *geotoload_;
   bool loading_;

   long long vol_maxsize_;
   float vol_ratio_;
   long long iso_maxsize_;
   float iso_ratio_;

   float eye_x_,eye_y_,eye_z_;
   float eye_dx_,eye_dy_,eye_dz_;
   float eye_ux_,eye_uy_,eye_uz_;

   double fps_; // animated frames per second
   double fovy_; // vertical field of view
   double omega_; // rotation speed in degrees/s
   double angle_; // rotation angle in degrees
   double tiltXY_,tiltYZ_; // rotation angle in degrees
   double tilt_; // tilt angle in degrees
   double vol_dx_,vol_dy_,vol_dz_; // volume translation
   double zoom_; // zoom into volume
   double clipdist_; // clipping distance
   bool clipgeo_; // geometry clipping enabled?
   bool opaque_; // opaque clipping plane enabled?
   float opacity_; // clipping plane opacity
   float opacity2_; // outer clipping plane opacity
   double oversampling_; // oversampling rate
   double red_,green_,blue_; // default color
   double emi_; // volume emission
   double att_; // volume absorption
   double emi_gm_; // volume emission for gradmag
   double att_gm_; // volume absorption for gradmag
   bool inv_; // inverse mode?
   bool gm_; // gradient magnitude?
   bool sfx_; // stereo mode?
   bool sfx_ana_; // anaglyph mode?
   bool tf_; // tfunc given?
   float tf_center_; // tfunc center
   float tf_size_; // tfunc size
   float tf_inverse_; // inverse tfunc
   bool geo_show_; // show geometry?

   unsigned int rendercount_;

   int mode_;

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
         vr_ = new volren_qgl();

      double eye_x=eye_x_,eye_y=eye_y_,eye_z=eye_z_;
      double eye_dx=eye_dx_,eye_dy=eye_dy_,eye_dz=eye_dz_;
      double eye_ux=eye_ux_,eye_uy=eye_uy_,eye_uz=eye_uz_;

      double gfx_fovy=fovy_;
      double gfx_aspect=(double)width()/height();
      double gfx_near=0.01;
      double gfx_far=10.0;

      double sfx_base=0.0;
      double sfx_focus=0.2*gfx_far;
      bool sfx_ana=true;

      if (sfx_)
      {
         sfx_base=0.005;
         sfx_ana=sfx_ana_;
      }

      bool gfx_fbo=true;

#ifdef HAVE_NO_FBO
       gfx_fbo=false;
#endif

#ifdef MACOSX
#ifndef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
       // fbo bugfix for macos x 10.5 and prior
       gfx_fbo=false;
#endif
#endif

      float vol_dx=vol_dx_;
      float vol_dy=vol_dy_;
      float vol_dz=vol_dz_;

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
      eye_x=(1.0-zoom_)*eye_x;
      eye_y=(1.0-zoom_)*eye_y;
      eye_z=(1.0-zoom_)*eye_z;

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

      // show rotation center as cross
      if (mode_==InteractionMode_Move ||
          mode_==InteractionMode_RotateCenter ||
          mode_==InteractionMode_RotateAnchor)
      {
         float cx,cy,cz;
         getRotationCenter(cx,cy,cz);
         float tx=cx;
         float ty=cos(tilt_*PI/180)*cy+sin(tilt_*PI/180)*cz;
         float tz=-sin(tilt_*PI/180)*cy+cos(tilt_*PI/180)*cz;
         vr_->transform(tx,ty,tz,angle_,tiltXY_,tiltYZ_,vol_dx_,vol_dy_,vol_dz_);
         vr_->showCross(TRUE,tx,ty,tz);
      }
      else vr_->showCross(FALSE);

      // call volume renderer
      if (sfx_base==0.0)
         vr_->renderscene(eye_tx,eye_ty,eye_tz, // view point
                          eye_tdx,eye_tdy,eye_tdz, // viewing direction
                          eye_tux,eye_tuy,eye_tuz, // up vector
                          gfx_fovy,gfx_aspect,gfx_near,gfx_far, // frustum
                          gfx_fbo, // use fbo
                          angle_, // volume rotation in degrees
                          tiltXY_,tiltYZ_, // volume rotation in degrees
                          vol_dx,vol_dy,vol_dz, // volume translation
                          vol_emission,vol_density, // global emi and att
                          tf_re_scale,tf_ge_scale,tf_be_scale, // emi scale
                          tf_ra_scale,tf_ga_scale,tf_ba_scale, // att scale
                          TRUE,TRUE, // pre-multiplication and pre-integration
                          TRUE, // white background
                          inv_, // inverse mode
                          vol_over, // oversampling
                          TRUE, // lighting
                          TRUE, // view-aligned clipping
                          clipdist_, // clipping distance relative to origin
                          clipgeo_, // clip geometry at clipping distance
                          opaque_, // opaque clipping plane
                          opacity_, // clipping plane opacity
                          opacity2_, // outer clipping plane opacity
                          geo_show_); // show surface geometry
      else
      {
         double eye_rx,eye_ry,eye_rz;

         eye_rx=eye_dy*eye_uz-eye_dz*eye_uy;
         eye_ry=eye_dz*eye_ux-eye_dx*eye_uz;
         eye_rz=eye_dx*eye_uy-eye_dy*eye_ux;

         double eye_tlx,eye_tly,eye_tlz;
         double eye_trx,eye_try,eye_trz;

         eye_tlx=eye_tx-sfx_base*eye_rx;
         eye_tly=eye_ty-sfx_base*eye_ry;
         eye_tlz=eye_tz-sfx_base*eye_rz;

         eye_trx=eye_tx+sfx_base*eye_rx;
         eye_try=eye_ty+sfx_base*eye_ry;
         eye_trz=eye_tz+sfx_base*eye_rz;

         double eye_tdlx,eye_tdly,eye_tdlz;
         double eye_tdrx,eye_tdry,eye_tdrz;

         eye_tdlx=eye_tdx+sfx_base/sfx_focus*eye_rx;
         eye_tdly=eye_tdy+sfx_base/sfx_focus*eye_ry;
         eye_tdlz=eye_tdz+sfx_base/sfx_focus*eye_rz;

         eye_tdrx=eye_tdx-sfx_base/sfx_focus*eye_rx;
         eye_tdry=eye_tdy-sfx_base/sfx_focus*eye_ry;
         eye_tdrz=eye_tdz-sfx_base/sfx_focus*eye_rz;

         if (sfx_ana)
            if (!inv_)
               glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);
            else
               glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
         else
            glDrawBuffer(GL_BACK_LEFT);

         vr_->renderscene(eye_tlx,eye_tly,eye_tlz, // left view point
                          eye_tdlx,eye_tdly,eye_tdlz, // viewing direction
                          eye_tux,eye_tuy,eye_tuz, // up vector
                          gfx_fovy,gfx_aspect,gfx_near,gfx_far, // frustum
                          gfx_fbo, // use fbo
                          angle_, // volume rotation in degrees
                          tiltXY_,tiltYZ_, // volume rotation in degrees
                          vol_dx,vol_dy,vol_dz, // volume translation
                          vol_emission,vol_density, // global emi and att
                          tf_re_scale,tf_ge_scale,tf_be_scale, // emi scale
                          tf_ra_scale,tf_ga_scale,tf_ba_scale, // att scale
                          TRUE,TRUE, // pre-multiplication and pre-integration
                          TRUE, // white background
                          inv_, // inverse mode
                          vol_over, // oversampling
                          TRUE, // lighting
                          TRUE, // view-aligned clipping
                          clipdist_, // clipping distance relative to origin
                          clipgeo_, // clip geometry at clipping distance
                          opaque_, // opaque clipping plane
                          opacity_, // clipping plane opacity
                          opacity2_, // outer clipping plane opacity
                          geo_show_); // show surface geometry

         if (sfx_ana)
            if (!inv_)
               glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
            else
               glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);
         else
            glDrawBuffer(GL_BACK_RIGHT);

         vr_->renderscene(eye_trx,eye_try,eye_trz, // right view point
                          eye_tdrx,eye_tdry,eye_tdrz, // viewing direction
                          eye_tux,eye_tuy,eye_tuz, // up vector
                          gfx_fovy,gfx_aspect,gfx_near,gfx_far, // frustum
                          gfx_fbo, // use fbo
                          angle_, // volume rotation in degrees
                          tiltXY_,tiltYZ_, // volume rotation in degrees
                          vol_dx,vol_dy,vol_dz, // volume translation
                          vol_emission,vol_density, // global emi and att
                          tf_re_scale,tf_ge_scale,tf_be_scale, // emi scale
                          tf_ra_scale,tf_ga_scale,tf_ba_scale, // att scale
                          TRUE,TRUE, // pre-multiplication and pre-integration
                          TRUE, // white background
                          inv_, // inverse mode
                          vol_over, // oversampling
                          TRUE, // lighting
                          TRUE, // view-aligned clipping
                          clipdist_, // clipping distance relative to origin
                          clipgeo_, // clip geometry at clipping distance
                          opaque_, // opaque clipping plane
                          opacity_, // clipping plane opacity
                          opacity2_, // outer clipping plane opacity
                          geo_show_); // show surface geometry

         if (sfx_ana)
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
         else
            glDrawBuffer(GL_BACK);
      }

      // show histogram and tfunc
      if (vr_->has_data() && bLeftButtonDown && mode_==InteractionMode_Window)
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

      if (angle_>180.0) angle_-=360.0;
      else if (angle_<-180.0) angle_+=360.0;

      if (omega_!=0.0) updated_rotation();

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
            updating();

            loading_ = true;

            delete vr_;
            vr_ = new volren_qgl();

            volren_qgl *vr = new volren_qgl();

            // set maximum volume size
            vr->set_vol_maxsize(vol_maxsize_,vol_ratio_);

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

            updated();
         }

         if (series_.size()>0)
         {
            updating();

            loading_ = true;

            delete vr_;
            vr_ = new volren_qgl();

            volren_qgl *vr = new volren_qgl();

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

            updated();
         }

         if (geotoload_)
         {
            loading_ = true;

            vr_->loadsurface(geotoload_);

            free(geotoload_);
            geotoload_=NULL;

            loading_ = false;
         }
      }
   }

public:

   void rotateCenter(float angle1,float angle2)
   {
      if (vr_)
      {
         vr_->rotate(vol_dx_,vol_dy_,vol_dz_,
                     angle1,angle2,
                     eye_x_,eye_y_,eye_z_,
                     eye_dx_,eye_dy_,eye_dz_,
                     eye_ux_,eye_uy_,eye_uz_);
      }
   }

   void getRotationCenter(float &x,float &y,float &z)
   {
      float dx,dy,dz;

      if (mode_==InteractionMode_RotateAnchor)
         getAnchorPlane(x,y,z,dx,dy,dz);
      else
      {
         x=vol_dx_;
         y=vol_dy_;
         z=vol_dz_;
      }
   }

   void getViewPlane(double &ex,double &ey,double &ez,
                     double &dx,double &dy,double &dz,
                     double &ux,double &uy,double &uz,
                     double &rx,double &ry,double &rz)
   {
      ex=eye_x_;
      ey=eye_y_;
      ez=eye_z_;

      dx=eye_dx_;
      dy=eye_dy_;
      dz=eye_dz_;

      ux=eye_ux_;
      uy=eye_uy_;
      uz=eye_uz_;

      rx=dy*uz-dz*uy;
      ry=dz*ux-dx*uz;
      rz=dx*uy-dy*ux;
   }

   void getAnchorPlane(float &ax,float &ay,float &az,
                       float &dx,float &dy,float &dz)
   {
      float d;

      d=-eye_dx_*(eye_x_-vol_dx_)+
        -eye_dy_*(eye_y_-vol_dy_)+
        -eye_dz_*(eye_z_-vol_dz_);

      d-=clipdist_;

      ax=eye_x_+eye_dx_*d;
      ay=eye_y_+eye_dy_*d;
      az=eye_z_+eye_dz_*d;

      dx=eye_dx_;
      dy=eye_dy_;
      dz=eye_dz_;
   }

   void rotateAnchorPlane(float angle1,float angle2)
   {
      float ax,ay,az;
      float dx,dy,dz;

      if (vr_)
      {
         getAnchorPlane(ax,ay,az,
                        dx,dy,dz);

         vr_->rotate(ax,ay,az,
                     angle1,angle2,
                     eye_x_,eye_y_,eye_z_,
                     eye_dx_,eye_dy_,eye_dz_,
                     eye_ux_,eye_uy_,eye_uz_);

         clipdist_=-eye_dx_*ax+
                   -eye_dy_*ay+
                   -eye_dz_*az;

         updated_clipping();
      }
   }

protected:

   bool bLeftButtonDown;
   bool bMiddleButtonDown;
   bool bRightButtonDown;

   bool bMouseMove;
   float mouseLastX,mouseLastY;

   void mousePressEvent(QMouseEvent *event)
   {
      if (event->buttons() & Qt::LeftButton)
         if (QApplication::keyboardModifiers() & Qt::ControlModifier ||
             QApplication::keyboardModifiers() & Qt::AltModifier)
            bRightButtonDown = true;
         else
            bLeftButtonDown = true;
      else if (event->buttons() & Qt::MiddleButton)
         bMiddleButtonDown = true;
      else if (event->buttons() & Qt::RightButton)
         bRightButtonDown = true;
      else
         event->ignore();

      bMouseMove = false;

      mouseMoveEvent(event);
   }

   void mouseReleaseEvent(QMouseEvent *event)
   {
      mouseMoveEvent(event);

      bLeftButtonDown = false;
      bMiddleButtonDown = false;
      bRightButtonDown = false;

      bMouseMove = false;
   }

   void mouseDoubleClickEvent(QMouseEvent *event)
   {
      mouseMoveEvent(event);

      bLeftButtonDown = false;
      bMiddleButtonDown = false;
      bRightButtonDown = false;

      bMouseMove = false;
   }

   void mouseMoveEvent(QMouseEvent *event)
   {
      double ex,ey,ez;
      double dx,dy,dz;
      double ux,uy,uz;
      double rx,ry,rz;

      float x = (float)(event->x())/width();
      float y = (float)(event->y())/height();

      bool shift = QApplication::keyboardModifiers() & Qt::ShiftModifier;

      if (bLeftButtonDown)
      {
         if (mode_ == InteractionMode_Window && !tf_)
         {
            tf_center_ = x;
            tf_size_ = 1.0f-y;
            tf_inverse_ = !shift;

            vr_->set_tfunc(tf_center_,tf_size_, red_,green_,blue_, tf_inverse_);
         }
         else if (mode_ == InteractionMode_Move)
         {
            if (bMouseMove)
            {
               getViewPlane(ex,ey,ez, dx,dy,dz, ux,uy,uz, rx,ry,rz);

               if (!shift)
               {
                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);

                  eye_x_ += ux*(y-mouseLastY);
                  eye_y_ += uy*(y-mouseLastY);
                  eye_z_ += uz*(y-mouseLastY);
               }
               else
               {
                  eye_x_ += dx*(y-mouseLastY);
                  eye_y_ += dy*(y-mouseLastY);
                  eye_z_ += dz*(y-mouseLastY);

                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);
               }
            }
         }
         else if (mode_ == InteractionMode_RotateAxis)
         {
            if (bMouseMove)
            {
               omega_ = 0.0;

               angle_ -= 180*(x-mouseLastX);
               tilt_ -= 180*(mouseLastY-y);

               updated_rotation();
            }
         }
         else if (mode_ == InteractionMode_RotateCenter)
         {
            if (bMouseMove)
            {
               omega_ = 0.0;

               rotateCenter(180*(x-mouseLastX),
                            180*(y-mouseLastY));
            }
         }
         else if (mode_ == InteractionMode_RotateAnchor)
         {
            if (bMouseMove)
            {
               omega_ = 0.0;

               rotateAnchorPlane(180*(x-mouseLastX),
                                 180*(y-mouseLastY));
            }
         }
         else if (mode_ == InteractionMode_Zoom)
         {
            if (bMouseMove)
            {
               getViewPlane(ex,ey,ez, dx,dy,dz, ux,uy,uz, rx,ry,rz);

               if (!shift)
               {
                  eye_x_ += dx*(y-mouseLastY);
                  eye_y_ += dy*(y-mouseLastY);
                  eye_z_ += dz*(y-mouseLastY);

                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);
               }
               else
               {
                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);

                  eye_x_ += ux*(y-mouseLastY);
                  eye_y_ += uy*(y-mouseLastY);
                  eye_z_ += uz*(y-mouseLastY);
               }
            }
         }
         else if (mode_ == InteractionMode_Clip)
         {
            if (bMouseMove)
            {
               clipdist_ += y-mouseLastY;

               updated_clipping();
            }
         }
         else if (mode_ == InteractionMode_Opacity)
         {
            if (bMouseMove)
            {
               if (!shift)
               {
                  emi_ -= (y-mouseLastY) - (x-mouseLastX);
                  att_ -= (y-mouseLastY);

                  if (emi_<0.0) emi_=0.0;
                  else if (emi_>1.0) emi_=1.0;

                  if (att_<0.0) att_=0.0;
                  else if (att_>1.0) att_=1.0;

                  emi_gm_ -= y-mouseLastY;
                  att_gm_ -= y-mouseLastY;

                  if (emi_gm_<0.0) emi_gm_=0.0;
                  else if (emi_gm_>1.0) emi_gm_=1.0;

                  if (att_gm_<0.0) att_gm_=0.0;
                  else if (att_gm_>1.0) att_gm_=1.0;
               }
               else
               {
                  opacity_ -= (y-mouseLastY);
                  opacity2_ -= (y-mouseLastY) - (x-mouseLastX);

                  if (opacity_<0.0) opacity_=0.0;
                  else if (opacity_>1.0) opacity_=1.0;

                  if (opacity2_<0.0) opacity2_=0.0;
                  else if (opacity2_>1.0) opacity2_=1.0;
               }

               updated_opacity();
            }
         }
         else if (mode_ == InteractionMode_Measure)
         {
            double fovy=fovy_;
            double aspect=(double)width()/height();

            v3d p=vr_->project(x,y, fovy,aspect);
            vr_->appendLine(p);

            updated_measuring(p.x,p.y,p.z,vr_->getLength(),vr_->getEndLength());
         }
      }
      else if (bMiddleButtonDown)
      {
         if (bMouseMove)
         {
            rotateCenter(180*(x-mouseLastX),
                         180*(y-mouseLastY));
         }
      }
      else if (bRightButtonDown)
      {
         if (mode_ == InteractionMode_Opacity)
         {
            if (bMouseMove)
            {
               if (shift)
               {
                  emi_ -= (y-mouseLastY) - (x-mouseLastX);
                  att_ -= (y-mouseLastY);

                  if (emi_<0.0) emi_=0.0;
                  else if (emi_>1.0) emi_=1.0;

                  if (att_<0.0) att_=0.0;
                  else if (att_>1.0) att_=1.0;

                  emi_gm_ -= y-mouseLastY;
                  att_gm_ -= y-mouseLastY;

                  if (emi_gm_<0.0) emi_gm_=0.0;
                  else if (emi_gm_>1.0) emi_gm_=1.0;

                  if (att_gm_<0.0) att_gm_=0.0;
                  else if (att_gm_>1.0) att_gm_=1.0;
               }
               else
               {
                  opacity_ -= (y-mouseLastY);
                  opacity2_ -= (y-mouseLastY) - (x-mouseLastX);

                  if (opacity_<0.0) opacity_=0.0;
                  else if (opacity_>1.0) opacity_=1.0;

                  if (opacity2_<0.0) opacity2_=0.0;
                  else if (opacity2_>1.0) opacity2_=1.0;
               }

               updated_opacity();
            }
         }
         else
            if (bMouseMove)
            {
               getViewPlane(ex,ey,ez, dx,dy,dz, ux,uy,uz, rx,ry,rz);

               if (!shift)
               {
                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);

                  eye_x_ += ux*(y-mouseLastY);
                  eye_y_ += uy*(y-mouseLastY);
                  eye_z_ += uz*(y-mouseLastY);
               }
               else
               {
                  eye_x_ += dx*(y-mouseLastY);
                  eye_y_ += dy*(y-mouseLastY);
                  eye_z_ += dz*(y-mouseLastY);

                  eye_x_ += rx*(mouseLastX-x);
                  eye_y_ += ry*(mouseLastX-x);
                  eye_z_ += rz*(mouseLastX-x);
               }
            }
      }
      else
         event->ignore();

      mouseLastX = x;
      mouseLastY = y;

      bMouseMove = true;
   }

   void wheelEvent(QWheelEvent *event)
   {
      double numDegrees = event->delta()/16.0;

      double ex,ey,ez;
      double dx,dy,dz;
      double ux,uy,uz;
      double rx,ry,rz;

      bool shift = QApplication::keyboardModifiers() & Qt::ShiftModifier;

      if (mode_ == InteractionMode_Clip ||
          mode_ == InteractionMode_RotateAnchor)
      {
         if (event->orientation() == Qt::Vertical)
         {
            clipdist_ -= numDegrees/360.0;

            updated_clipping();
         }
      }
      else if (mode_ == InteractionMode_Opacity)
      {
         if (event->orientation() == Qt::Vertical)
         {
            emi_ += numDegrees/360.0;
            att_ += numDegrees/360.0;

            if (emi_<0.0) emi_=0.0;
            else if (emi_>1.0) emi_=1.0;

            if (att_<0.0) att_=0.0;
            else if (att_>1.0) att_=1.0;

            emi_gm_ += numDegrees/360.0;
            att_gm_ += numDegrees/360.0;

            if (emi_gm_<0.0) emi_gm_=0.0;
            else if (emi_gm_>1.0) emi_gm_=1.0;

            if (att_gm_<0.0) att_gm_=0.0;
            else if (att_gm_>1.0) att_gm_=1.0;
         }
         else
         {
            opacity_ -= numDegrees/360.0;
            opacity2_ -= numDegrees/360.0;

            if (opacity_<0.0) opacity_=0.0;
            else if (opacity_>1.0) opacity_=1.0;

            if (opacity2_<0.0) opacity2_=0.0;
            else if (opacity2_>1.0) opacity2_=1.0;
         }

         updated_opacity();
      }
      else
      {
         getViewPlane(ex,ey,ez, dx,dy,dz, ux,uy,uz, rx,ry,rz);

         if (event->orientation() == Qt::Vertical)
         {
            if (!shift)
            {
               eye_x_ -= dx*numDegrees/360.0;
               eye_y_ -= dy*numDegrees/360.0;
               eye_z_ -= dz*numDegrees/360.0;
            }
            else
            {
               eye_x_ -= ux*numDegrees/360.0;
               eye_y_ -= uy*numDegrees/360.0;
               eye_z_ -= uz*numDegrees/360.0;
            }
         }
         else
         {
            eye_x_ += rx*numDegrees/360.0;
            eye_y_ += ry*numDegrees/360.0;
            eye_z_ += rz*numDegrees/360.0;
         }
      }

      event->accept();
   }

   virtual void updating()
   {
      printf("updating volume\n");
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

   virtual void updated()
   {
      printf("updated volume\n");
   }

   static void feedback(const char *info,float percent,void *obj)
   {
      QGLVolRenWidget *w=(QGLVolRenWidget *)obj;
      w->update(info,percent);
   }

   virtual void updated_rotation()
   {
      printf("updated rotation\n");
   }

   virtual void updated_zoom()
   {
      printf("updated zoom\n");
   }

   virtual void updated_clipping()
   {
      printf("updated clipping\n");
   }

   virtual void updated_opacity()
   {
      printf("updated opacity\n");
   }

   virtual void updated_measuring(double px,double py,double pz,double length,double endlength)
   {
      printf("updated measuring: point(%g,%g,%g) length=%g end2end=%g\n",px,py,pz,length,endlength);
   }

   bool loadFile(volren_qgl *vr, const char *filename)
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
