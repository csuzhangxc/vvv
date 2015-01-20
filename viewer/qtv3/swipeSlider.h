// (c) by Stefan Roettger, licensed under GPL 3.0

#ifndef SWIPESLIDER_H
#define SWIPESLIDER_H

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "swipeFilter.h"

//! swiping slider widget
class SwipeSlider: public QWidget
{
   Q_OBJECT

public:

   SwipeSlider(Qt::Orientation orientation, QString text = "", QWidget *parent = NULL);
   virtual ~SwipeSlider();

   void setRange(double minimum, double maximum);

   void enableSwipe();
   void disableSwipe();

   void scrollSlider(double offset);

   double getValue();
   double getNormalizedValue();

   void setValue(double value);
   void setNormalizedValue(double value);

   void setOutline(int width=0);
   void setBlackOnWhite(bool white=true);

   //! return preferred minimum window size
   QSize minimumSizeHint() const
   {
      if (orientation_ == Qt::Vertical)
         return(QSize(100, 20));
      else
         return(QSize(20, 100));
   }

protected:

   double value_;

   double minimum_;
   double maximum_;

   SwipeFilter *filter;
   bool enabled;

   Qt::Orientation orientation_;
   QString text_;
   int outline_;
   bool white_;

   virtual void paintEvent(QPaintEvent *event);

protected slots:

   void move(SwipeDirection direction, int offset);
   void kinetic(SwipeDirection direction, int offset);

signals:

   void valueChanged(double value);
};

#endif
