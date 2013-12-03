// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "volren_qgl.h"

#include "prefwindow.h"

QTV3PrefWindow::QTV3PrefWindow(QWidget *parent, QGLVolRenWidget *vrw)
   : QDockWidget(parent)
{
   setWindowTitle("QTV3 Volume Rendering Preferences");

   vrw_ = vrw;

   vol_maxsize_ = 512;
   iso_maxsize_ = 256;

   border_ratio_ = 0.25f;

   slice_opacity_ = 0.75f;

   QSettings settings("www.open-terrain.org", "qtv3");

   if (settings.contains("vol_maxsize"))
      vol_maxsize_ = settings.value("vol_maxsize").toInt();
   if (settings.contains("iso_maxsize"))
      iso_maxsize_ = settings.value("iso_maxsize").toInt();

   if (settings.contains("slice_opacity"))
      slice_opacity_ = settings.value("slice_opacity").toFloat();

   QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

   if (env.contains("QTV3_VOL_LIMIT"))
   {
      vol_maxsize_ = env.value("QTV3_VOL_LIMIT").toUInt();
   }

   if (env.contains("QTV3_ISO_LIMIT"))
   {
      iso_maxsize_ = env.value("QTV3_ISO_LIMIT").toUInt();
   }

   vrw_->set_vol_maxsize(vol_maxsize_, border_ratio_);
   vrw_->set_iso_maxsize(iso_maxsize_, border_ratio_);

   vrw_->setSliceOpacity(slice_opacity_);

   createWidgets();
}

QTV3PrefWindow::~QTV3PrefWindow()
{
   QSettings settings("www.open-terrain.org", "qtv3");

   settings.setValue("vol_maxsize", vol_maxsize_);
   settings.setValue("iso_maxsize", iso_maxsize_);

   settings.setValue("slice_opacity", slice_opacity_);
}

QSize QTV3PrefWindow::minimumSizeHint() const
{
   return(QSize(256, 100));
}

QSize QTV3PrefWindow::sizeHint() const
{
   return(QSize(256, 768));
}

void QTV3PrefWindow::createWidgets()
{
   QGroupBox *group = new QGroupBox;
   QVBoxLayout *layout = new QVBoxLayout;

   QLineEdit *lineEdit_vol_maxsize = new QLineEdit;
   QGroupBox *vol_maxsize_group = createEdit("Maximum Volume Size for RAW/REK processing", QString::number(vol_maxsize_), &lineEdit_vol_maxsize);
   connect(lineEdit_vol_maxsize,SIGNAL(textChanged(QString)),this,SLOT(volMaxSizeChange(QString)));
   layout->addWidget(vol_maxsize_group);

   QLineEdit *lineEdit_iso_maxsize = new QLineEdit;
   QGroupBox *iso_maxsize_group = createEdit("Maximum Volume Size for RAW/REK Iso Surface Extraction", QString::number(iso_maxsize_), &lineEdit_iso_maxsize);
   connect(lineEdit_iso_maxsize,SIGNAL(textChanged(QString)),this,SLOT(isoMaxSizeChange(QString)));
   layout->addWidget(iso_maxsize_group);

   QFrame* line = new QFrame();
   line->setFrameShape(QFrame::HLine);
   line->setFrameShadow(QFrame::Raised);
   layout->addWidget(line);

   lineEdit_slice_opacity_ = new QLineEdit;
   QGroupBox *slice_opacity_group = createEdit("Opacity of Clip Plane", QString::number(slice_opacity_), &lineEdit_slice_opacity_);
   connect(lineEdit_slice_opacity_,SIGNAL(textChanged(QString)),this,SLOT(sliceOpacityChange(QString)));
   slice_opacity_slider_=createSlider(0,100,100*slice_opacity_);
   slice_opacity_group->layout()->addWidget(slice_opacity_slider_);
   connect(slice_opacity_slider_,SIGNAL(valueChanged(int)), this, SLOT(sliceOpacityChange(int)));
   layout->addWidget(slice_opacity_group);

   layout->addStretch(1000);

   group->setLayout(layout);
   setWidget(group);
}

QGroupBox *QTV3PrefWindow::createEdit(QString name, QString value,
                                      QLineEdit **lineEdit)
{
   QGroupBox *lineEditGroup = new QGroupBox(name);
   QVBoxLayout *lineEditLayout = new QVBoxLayout;
   lineEditGroup->setLayout(lineEditLayout);
   *lineEdit = new QLineEdit(value);
   lineEditLayout->addWidget(*lineEdit);
   return(lineEditGroup);
}

QSlider *QTV3PrefWindow::createSlider(int minimum, int maximum, int value)
{
   QSlider *slider = new QSlider(Qt::Horizontal);
   slider->setRange(minimum * 16, maximum * 16);
   slider->setSingleStep(16);
   slider->setPageStep((maximum - minimum) / 10 * 16);
   slider->setTickInterval((maximum - minimum) / 10 * 16);
   slider->setTickPosition(QSlider::TicksBelow);
   slider->setValue(value * 16);
   return(slider);
}

void QTV3PrefWindow::volMaxSizeChange(QString maxsize)
{
   vol_maxsize_ = maxsize.toUInt();
   vrw_->set_vol_maxsize(vol_maxsize_, border_ratio_);
}

void QTV3PrefWindow::isoMaxSizeChange(QString maxsize)
{
   iso_maxsize_ = maxsize.toUInt();
   vrw_->set_iso_maxsize(iso_maxsize_, border_ratio_);
}

void QTV3PrefWindow::sliceOpacityChange(QString opacity)
{
   slice_opacity_ = opacity.toFloat();
   vrw_->setSliceOpacity(slice_opacity_);
   slice_opacity_slider_->setValue(slice_opacity_ * 100 * 16);
}

void QTV3PrefWindow::sliceOpacityChange(int opacity)
{
   slice_opacity_ = opacity / 100.0f / 16.0f;
   vrw_->setSliceOpacity(slice_opacity_);
   lineEdit_slice_opacity_->setText(QString::number(slice_opacity_));
}
