// (c) by Stefan Roettger, licensed under GPL 3.0

#include <typeinfo>

#include "swipeFilter.h"
#include "swipeSlider.h"

static const int mouse_delta = 16;
static const double mouse_threshold = 20.0;

static const double motion_fps = 30;
static const double motion_damping = 0.1;

SwipeFilter::SwipeFilter(QWidget *parent)
   : QObject(),
     parent_(parent),
     leftButtonDown_(false),
     widgetHit_(NULL),
     delta_(mouse_delta),
     damping_(motion_damping)
{
   connect(&motion_, SIGNAL(timeout()), this, SLOT(motion()));
}

SwipeFilter::~SwipeFilter()
{}

void SwipeFilter::set_delta(int delta)
{
   delta_ = delta;
}

void SwipeFilter::set_damping(double damping)
{
   damping_ = damping;
}

bool SwipeFilter::eventFilter(QObject *obj, QEvent *event)
{
   QMouseEvent *mouseEvent;

   if (parent_->isVisible())
      if (mouseEvent = dynamic_cast<QMouseEvent*>(event))
      {
         QPoint pos = mouseEvent->globalPos();

         if (mouseEvent->type() == QEvent::MouseButtonPress)
         {
            if (mouseEvent->button() == Qt::LeftButton)
               if (!leftButtonDown_)
               {
                  QPoint loc = parent_->mapFromGlobal(pos);
                  QRect area = parent_->rect();

                  if (area.contains(loc))
                  {
                     QWidget *widget = qApp->widgetAt(pos);

                     if (widget)
                     {
                        if (dynamic_cast<QGroupBox*>(widget) ||
                            dynamic_cast<QLabel*>(widget))
                        {
                           widgetHit_ = widget;

                           return(true);
                        }

                        if (!dynamic_cast<QScrollBar*>(widget) &&
                            (!dynamic_cast<QSlider*>(widget) || dynamic_cast<SwipeSlider*>(widget)) &&
                            !dynamic_cast<QDial*>(widget))
                        {
                           leftButtonDown_ = true;
                           direction_ = SwipeNone;
                           lastPos_ = pos;
                           offset_ = 0;
                           time_.start();
                           elapsed_ = 0.0;
                           kinetic_ = 0.0;
                           motion_.stop();

                           return(true);
                        }
                     }
                  }
               }
         }
         else if (mouseEvent->type() == QEvent::MouseMove)
         {
            if (leftButtonDown_)
            {
               int dx = pos.x()-lastPos_.x();
               int dy = pos.y()-lastPos_.y();

               switch (direction_)
               {
                  case SwipeNone:
                     if (abs(dx)>abs(dy))
                     {
                        if (dx<-delta_) direction_ = SwipeLeft;
                        else if (dx>delta_) direction_ = SwipeRight;
                     }
                     else
                     {
                        if (dy>delta_) direction_ = SwipeDown;
                        else if (dy<-delta_) direction_ = SwipeUp;
                     }
                     break;
                  case SwipeLeft:
                     if (dx != 0)
                     {
                        offset_ = -dx;
                        if (offset_ < 0) { offset_ = -offset_; direction_ = SwipeRight; }
                        emit move(direction_, offset_);
                        elapsed_ = 0.001*time_.elapsed();
                        lastPos_ = pos;
                        time_.start();
                     }
                     break;
                  case SwipeRight:
                     if (dx != 0)
                     {
                        offset_ = dx;
                        if (offset_ < 0) { offset_ = -offset_; direction_ = SwipeLeft; }
                        emit move(direction_, offset_);
                        elapsed_ = 0.001*time_.elapsed();
                        lastPos_ = pos;
                        time_.start();
                     }
                     break;
                  case SwipeDown:
                     if (dy != 0)
                     {
                        offset_ = dy;
                        if (offset_ < 0) { offset_ = -offset_; direction_ = SwipeUp; }
                        emit move(direction_, offset_);
                        elapsed_ = 0.001*time_.elapsed();
                        lastPos_ = pos;
                        time_.start();
                     }
                     break;
                  case SwipeUp:
                     if (dy != 0)
                     {
                        offset_ = -dy;
                        if (offset_ < 0) { offset_ = -offset_; direction_ = SwipeDown; }
                        emit move(direction_, offset_);
                        elapsed_ = 0.001*time_.elapsed();
                        lastPos_ = pos;
                        time_.start();
                     }
                     break;
               }
            }
         }
         else if (mouseEvent->type() == QEvent::MouseButtonRelease)
         {
            if (mouseEvent->button() == Qt::LeftButton)
            {
               if (widgetHit_)
               {
                  emit click(widgetHit_);

                  widgetHit_ = NULL;
               }

               if (leftButtonDown_)
               {
                  if (direction_ != SwipeNone)
                  {
                     leftButtonDown_ = false;

                     elapsed_ += 0.001*time_.elapsed();

                     if (elapsed_ > 0.0)
                     {
                        double speed = offset_/elapsed_;

                        emit swipe(direction_, speed);

                        if (speed > mouse_threshold)
                        {
                           kinetic_ = speed;
                           motion_.start(1000/motion_fps); // ms
                        }
                     }
                     else
                        emit swipe(direction_, 0.0);

                     return(true);
                  }
                  else
                  {
                     QMouseEvent press(QEvent::MouseButtonPress, mouseEvent->pos(),
                                       Qt::LeftButton,  Qt::MouseButtons(Qt::LeftButton), mouseEvent->modifiers());

                     QApplication::sendEvent(obj, &press);

                     leftButtonDown_ = false;
                  }
               }
            }
         }
      }

   // unhandled events are passed to the base class
   return(QObject::eventFilter(obj, event));
}

void SwipeFilter::motion()
{
   if (direction_ != SwipeNone && kinetic_ > 0.0)
   {
      emit kinetic(direction_, kinetic_/motion_fps);

      kinetic_ *= pow(damping_, 1.0/motion_fps);
      if (kinetic_ < 0.5*motion_fps) kinetic_ = 0.0;
   }
   else
      motion_.stop();
}
