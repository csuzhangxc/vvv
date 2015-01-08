// (c) by Stefan Roettger, licensed under GPL 3.0

#ifndef SWIPEFILTER_H
#define SWIPEFILTER_H

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

//! swiping direction
enum SwipeDirection
{
   SwipeNone,
   SwipeLeft,
   SwipeRight,
   SwipeDown,
   SwipeUp
};

//! swiping event filter
class SwipeFilter: public QObject
{
   Q_OBJECT

public:

   SwipeFilter(QWidget *parent);
   virtual ~SwipeFilter();

   void set_delta(int delta);
   void set_damping(double damping = 0.1);

protected:

   bool eventFilter(QObject *obj, QEvent *event);

   QWidget *parent_;

   bool leftButtonDown_;
   QWidget *widgetHit_;
   QPoint lastPos_;
   int delta_;

   SwipeDirection direction_;
   int offset_;
   QTime time_;
   double elapsed_;
   double kinetic_;

   QTimer motion_;
   double damping_;

protected slots:

   void motion();

signals:

   void click(QWidget *widget);
   void doubleClick(QWidget *widget);
   void swipe(SwipeDirection direction, double speed);
   void move(SwipeDirection direction, int offset);
   void kinetic(SwipeDirection direction, int offset);
};

#endif
