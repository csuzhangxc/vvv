// (c) by Stefan Roettger, licensed under GPL 3.0

#include "swipeSlider.h"

SwipeSlider::SwipeSlider(Qt::Orientation orientation, QString text, QWidget *parent)
   : QWidget(parent),
     value_(0.0),
     minimum_(0.0),
     maximum_(1.0),
     enabled(false),
     orientation_(orientation),
     text_(text),
     outline_(0),
     white_(true)
{
   filter = new SwipeFilter(this);

   connect(filter, SIGNAL(move(SwipeDirection, int)),
           this, SLOT(move(SwipeDirection, int)));

   connect(filter, SIGNAL(kinetic(SwipeDirection, int)),
           this, SLOT(kinetic(SwipeDirection, int)));

   enableSwipe();

   filter->set_delta(5);
   filter->set_damping(0.001);

   QSizePolicy qsp(QSizePolicy::Expanding, QSizePolicy::Expanding);
   qsp.setHeightForWidth(true);

   setSizePolicy(qsp);
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

void SwipeSlider::setOutline(int width)
{
   outline_ = width;
}

void SwipeSlider::setBlackOnWhite(bool white)
{
   white_ = white;
}

void SwipeSlider::paintEvent(QPaintEvent *event)
{
   int size2 = 4;

   QPainter painter(this);

   QPointF a(0, height()-1);

   QPointF b((orientation_ == Qt::Horizontal)? width()-1 : 0,
             (orientation_ == Qt::Vertical)? 0 : height()-1);

   if (outline_ == 0)
   {
      QLinearGradient linGrad(a, b);

      double u = 255*(0.3);
      double v = 255*(0.9);

      linGrad.setColorAt(0, QColor(u, u, u));
      linGrad.setColorAt(1, QColor(v, v, v));

      painter.setPen(Qt::NoPen);
      painter.setBrush(linGrad);
   }
   else
   {
      QPen pen;

      pen.setWidth(outline_);
      pen.setJoinStyle(Qt::MiterJoin);

      if (white_)
      {
         pen.setColor(QColor(0x88, 0x88, 0x88));

         QBrush brush(QColor(0xf8, 0xf8, 0xf8));

         painter.setPen(pen);
         painter.setBrush(brush);
      }
      else
      {
         pen.setColor(QColor(0x33, 0x33, 0x33));

         QLinearGradient linGrad(a, b);

         double u = 255*(0.1);
         double v = 255*(0.3);

         linGrad.setColorAt(0, QColor(u, u, u));
         linGrad.setColorAt(1, QColor(v, v, v));

         painter.setPen(pen);
         painter.setBrush(linGrad);
      }

      size2 = 2;
   }

   painter.drawRect(QRect(outline_/2, outline_/2,
                          width()-outline_, height()-outline_));

   if (orientation_ == Qt::Vertical)
   {
      if (text_ != "")
      {
         painter.save();
         if (outline_ == 0)
            painter.setPen(QColor(160,176,224));
         else
            painter.setPen(QColor(192,192,192));
         painter.setFont(QFont("Arial", width()*2/5));
         painter.translate(width()/4, width()/6);
         painter.rotate(90);
         painter.drawText(0, 0, text_);
         painter.restore();
      }

      double h = value_*(size2+outline_) + (1.0-value_)*(height()-1-(size2+outline_));
      painter.setPen(QPen(QColor(0,32,255,160), 2*size2+1));
      painter.drawLine(outline_+1, h, width()-1-outline_-1, h);
   }
   else
   {
      if (text_ != "")
      {
         painter.save();
         if (outline_ == 0)
            painter.setPen(QColor(160,176,224));
         else
            painter.setPen(QColor(192,192,192));
         painter.setFont(QFont("Arial", height()*2/5));
         painter.translate(height()/4, -height()/6);
         painter.drawText(0, height(), text_);
         painter.restore();
      }

      double w = (1.0-value_)*(size2+outline_) + value_*(width()-1-(size2+outline_));
      painter.setPen(QPen(QColor(0,32,255,160), 2*size2+1));
      painter.drawLine(w, outline_+1, w, height()-1-outline_-1);
   }
}
