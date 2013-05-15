#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include <QtOpenGL/qgl.h>

#include "volren.h"

static const double fps=30.0; // animated frames per second

class MyQGLWidget: public QGLWidget
{
public:

   //! default ctor
   MyQGLWidget(QWidget *parent = 0)
      : QGLWidget(parent)
   {
      setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::StencilBuffer));

      vr_ = NULL;

      startTimer((int)(1000.0/fps)); // ms=1000/fps
   }

   //! dtor
   ~MyQGLWidget()
   {
      if (vr_)
         delete vr_;
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

   void initializeGL()
   {
      if (!vr_)
      {
         vr_ = new volren();
         vr_->loadvolume("../Bucky.pvm");
      }

      qglClearColor(Qt::black);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
   }

   void resizeGL(int, int)
   {
      glViewport(0, 0, width(), height());
   }

   void paintGL()
   {
      double EYE_X=0,EYE_Y=0,EYE_Z=3;
      double EYE_DX=0,EYE_DY=0,EYE_DZ=-1;
      double EYE_UX=0,EYE_UY=1,EYE_UZ=0;

      double EYE_FOVY=60.0;
      double EYE_NEAR=0.01;
      double EYE_FAR=10.0;

      double VOL_EMISSION=1000.0;
      double VOL_DENSITY=1000.0;

      double re_scale=0.25,
             ge_scale=0.25,
             be_scale=0.25;
      double ra_scale=0.25,
             ga_scale=0.25,
             ba_scale=0.25;

      double aspect=(double)width()/height();
      bool resized=false;

      vr_->render(EYE_X,EYE_Y,EYE_Z,
                  EYE_DX,EYE_DY,EYE_DZ,
                  EYE_UX,EYE_UY,EYE_UZ,
                  EYE_FOVY,aspect,EYE_NEAR,EYE_FAR,
                  TRUE,resized,
                  0.0, // rotation
                  0.0f,0.0,0.0f, // translation
                  VOL_EMISSION,VOL_DENSITY,
                  re_scale,ge_scale,be_scale,
                  ra_scale,ga_scale,ba_scale);
   }

   void timerEvent(QTimerEvent *)
   {
      repaint();
   }

};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   if (!QGLFormat::hasOpenGL()) return(1);

   MyQGLWidget main;
   main.show();

   return(app.exec());
}
