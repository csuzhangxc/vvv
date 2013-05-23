// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include "volren_qgl.h"

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

private:

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
   void loadvolume(const char *filename);

   //! load a DICOM series
   void loadseries(const std::vector<std::string> list);

   //! set volume rotation speed
   void setrotation(double omega);

private:

   QVBoxLayout *layout_;
   QGLVolRenWidget *vrw_;
   QLabel *label_;

   void createMenus();
   void createWidgets();

   QStringList browse(QString path="",
                      bool newfile=false);

   QTV3Slider *createSlider(int minimum, int maximum, int value,
                            bool vertical=false);

protected:

   void dragEnterEvent(QDragEnterEvent *event);
   void dragMoveEvent(QDragMoveEvent *event);
   void dragLeaveEvent(QDragLeaveEvent *event);

public:

   void dropEvent(QDropEvent *event);

protected slots:

   void open();
   void rotate(int v);
   void about();
};

#endif
