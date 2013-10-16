// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef PREFWINDOW_H
#define PREFWINDOW_H

#include <QtGui>

class QTV3PrefWindow: public QDockWidget
{
   Q_OBJECT; // Qt Metacall object for signal/slot connections

public:

   //! default ctor
   QTV3PrefWindow(QWidget *parent = 0);

   //! dtor
   ~QTV3PrefWindow();

   //! return preferred minimum window size
   QSize minimumSizeHint() const;

   //! return preferred window size
   QSize sizeHint() const;
};

#endif
