// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include "volren_qgl.h"

class QTV3VolRenWidget: public QGLVolRenWidget
{
   Q_OBJECT;

public:

   QTV3VolRenWidget(QWidget * parent = 0)
      : QGLVolRenWidget(parent)
   {timer_.start();}

protected:

   virtual void update(const char *info,float percent)
   {
      static const float update_fps=10.0f;

      QString text;

      if (percent>0.0f)
         text=QString("%1: %2%").arg(info).arg((int)(100.0f*percent+0.5f));
      else
         text=QString("%1").arg(info);

      emit update_signal(text);

      if (timer_.elapsed()>1000.0f/update_fps)
      {
         repaint();
         QApplication::processEvents();
         timer_.restart();
      }
   }

   QTime timer_;

signals:

   void update_signal(QString text);
};

class QTV3Slider: public QSlider
{
public:

   QTV3Slider(Qt::Orientation orientation, QWidget * parent = 0)
      : QSlider(orientation, parent)
   {o_=orientation;}

   //! return preferred minimum window size
   QSize minimumSizeHint() const
   {
      if (o_==Qt::Horizontal)
         return(QSize(150, 50));
      else
         return(QSize(50, 100));
   }

protected:

   Qt::Orientation o_;
};

class QTV3MainWindow: public QMainWindow
{
   Q_OBJECT; // Qt Metacall object for signal/slot connections

public:

   //! default ctor
   QTV3MainWindow(QWidget *parent = 0);

   //! dtor
   ~QTV3MainWindow();

   //! return preferred minimum window size
   QSize minimumSizeHint() const;

   //! return preferred window size
   QSize sizeHint() const;

   //! load a volume
   void loadVolume(const char *filename);

   //! load a DICOM series
   void loadSeries(const std::vector<std::string> list);

   //! set volume rotation speed
   void setRotation(double omega);

private:

   QVBoxLayout *layout_;
   QTV3VolRenWidget *vrw_;
   QLabel *label_,*update_;

   void createMenus();
   void createWidgets();

   QStringList browse(QString path="",
                      bool newfile=false);

   QTV3Slider *createSlider(int minimum, int maximum, int value,
                            bool vertical=false);

   int flip1_,flip2_;

protected:

   void keyPressEvent(QKeyEvent *event)
   {
      if (event->key() == Qt::Key_Q)
         emit close();

      QMainWindow::keyPressEvent(event);
   }

   void keyReleaseEvent(QKeyEvent *event)
   {
      QMainWindow::keyReleaseEvent(event);
   }

   void dragEnterEvent(QDragEnterEvent *event);
   void dragMoveEvent(QDragMoveEvent *event);
   void dragLeaveEvent(QDragLeaveEvent *event);

public:

   void dropEvent(QDropEvent *event);

protected slots:

   void open();
   void zoom(int v);
   void rotate(int v);
   void tilt(int v);
   void clip(int v);
   void emission(int v);
   void absorption(int v);
   void checkInvMode(int on);
   void checkGradMag(int on);
   void checkFlip1(int on);
   void checkFlip2(int on);
   void about();

   void update_slot(QString text);
};

#endif
