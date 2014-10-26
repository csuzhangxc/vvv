// (c) by Stefan Roettger, licensed under GPL 3.0

#include "swipeSlider.h"

SwipeSlider::SwipeSlider(Qt::Orientation orientation, QWidget *parent)
   : QWidget(parent),
     minimum_(0.0),
     maximum_(1.0),
     enabled(false),
     orientation_(orientation),
     value_(0.0)
{
   filter = new SwipeFilter(this);

   connect(filter, SIGNAL(move(SwipeDirection, int)),
           this, SLOT(move(SwipeDirection, int)));

   connect(filter, SIGNAL(kinetic(SwipeDirection, int)),
           this, SLOT(kinetic(SwipeDirection, int)));

   enableSwipe();

   filter->set_delta(5);
   filter->set_damping(0.001);
}

SwipeSlider::~SwipeSlider()
{
   disableSwipe();

   delete filter;
}

void SwipeSlider::setRange(double minimum, double maximum)
{
   minimum_ = minimum;
   maximum_ = maximum;
}

void SwipeSlider::enableSwipe()
{
   if (!enabled)
      qApp->installEventFilter(filter);

   enabled = true;
}

void SwipeSlider::disableSwipe()
{
   if (enabled)
      qApp->removeEventFilter(filter);

   enabled = false;
}

double SwipeSlider::getValue()
{
   return(value_*(maximum_-minimum_)+minimum_);
}

double SwipeSlider::getNormalizedValue()
{
   return(value_);
}

void SwipeSlider::setValue(double value)
{
   setNormalizedValue((value-minimum_)/(maximum_-minimum_));
}

void SwipeSlider::setNormalizedValue(double value)
{
   value_ = value;

   if (value_ < 0.0) value_ = 0.0;
   else if (value_ > 1.0) value_ = 1.0;

   repaint();
}

void SwipeSlider::scrollSlider(double offset)
{
   double size = (orientation_ == Qt::Horizontal)? width() : height();
   setNormalizedValue(getNormalizedValue() + offset/size);

   emit valueChanged(getValue());
}

void SwipeSlider::move(SwipeDirection direction, int offset)
{
   if (orientation_ == Qt::Vertical)
   {
      if (direction == SwipeDown)
      {
         scrollSlider(-offset);
      }
      else if (direction == SwipeUp)
      {
         scrollSlider(offset);
      }
   }
   else
   {
      if (direction == SwipeLeft)
      {
         scrollSlider(-offset);
      }
      else if (direction == SwipeRight)
      {
         scrollSlider(offset);
      }
   }
}

void SwipeSlider::kinetic(SwipeDirection direction, int offset)
{
   if (orientation_ == Qt::Vertical)
   {
      if (direction == SwipeDown)
      {
         scrollSlider(-offset);
      }
      else if (direction == SwipeUp)
      {
         scrollSlider(offset);
      }
   }
   else
   {
      if (direction == SwipeLeft)
      {
         scrollSlider(-offset);
      }
      else if (direction == SwipeRight)
      {
         scrollSlider(offset);
      }
   }
}

void SwipeSlider::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);

   QPointF a(0, height()-1);
   QPointF b((orientation_ == Qt::Horizontal)? width()-1 : 0,
             (orientation_ == Qt::Vertical)? 0 : height()-1);

   QLinearGradient linGrad(a, b);
   linGrad.setColorAt(0, Qt::gray);
   linGrad.setColorAt(1, Qt::white);

   painter.setPen(Qt::NoPen);
   painter.setBrush(linGrad);
   painter.drawRect(rect());

   painter.setPen(QPen(Qt::blue, 3));

   if (orientation_ == Qt::Vertical)
   {
      painter.drawLine(0, (1.0-value_)*(height()-1),
                       width()-1, (1.0-value_)*(height()-1));
   }
   else
   {
      painter.drawLine(value_*(width()-1), 0,
                       value_*(width()-1), height()-1);
   }
}
