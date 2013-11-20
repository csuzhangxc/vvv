// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef PREFWINDOW_H
#define PREFWINDOW_H

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

class QGLVolRenWidget;

class QTV3PrefWindow: public QDockWidget
{
   Q_OBJECT; // Qt Metacall object for signal/slot connections

public:

   //! default ctor
   QTV3PrefWindow(QWidget *parent, QGLVolRenWidget *vrw);

   //! dtor
   ~QTV3PrefWindow();

   //! return preferred minimum window size
   QSize minimumSizeHint() const;

   //! return preferred window size
   QSize sizeHint() const;

   long long vol_maxsize_;
   long long iso_maxsize_;

   float border_ratio_;

   float slice_opacity_;

protected:

   QGLVolRenWidget *vrw_;

   void createWidgets();

   QGroupBox *createEdit(QString name, QString value,
                         QLineEdit **lineEdit);

protected slots:

   void volMaxSizeChange(QString);
   void isoMaxSizeChange(QString);

   void sliceOpacityChange(QString);
};

#endif
