// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "volren_qgl.h"

#include "prefwindow.h"

QTV3PrefWindow::QTV3PrefWindow(QWidget *parent, QGLVolRenWidget *vrw, bool vrw_stereo)
   : QDockWidget(parent)
{
   setWindowTitle("QTV3 Volume Rendering Preferences");

   vrw_ = vrw;
   vrw_stereo_ = vrw_stereo;

   vol_maxsize_ = 512;
   iso_maxsize_ = 256;

   border_ratio_ = 0.25f;

   slice_opacity_ = 0.75f;
   slice_opacity2_ = 0.1f;

   vol_hue_ = 120.0f;

   QSettings settings("www.open-terrain.org", "qtv3");

   if (settings.contains("vol_maxsize"))
      vol_maxsize_ = settings.value("vol_maxsize").toInt();
   if (settings.contains("iso_maxsize"))
      iso_maxsize_ = settings.value("iso_maxsize").toInt();

   if (settings.contains("slice_opacity"))
      slice_opacity_ = settings.value("slice_opacity").toFloat();
   if (settings.contains("slice_opacity2"))
      slice_opacity2_ = settings.value("slice_opacity2").toFloat();

   if (settings.contains("vol_hue"))
      vol_hue_ = settings.value("vol_hue").toFloat();

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
   vrw_->setOuterOpacity(slice_opacity2_);

   vrw_->setColorHue(vol_hue_);

   createWidgets();
}

QTV3PrefWindow::~QTV3PrefWindow()
{
   QSettings settings("www.open-terrain.org", "qtv3");

   settings.setValue("vol_maxsize", vol_maxsize_);
   settings.setValue("iso_maxsize", iso_maxsize_);

   settings.setValue("slice_opacity", slice_opacity_);
   settings.setValue("slice_opacity2", slice_opacity2_);

   settings.setValue("vol_hue", vol_hue_);
}

void QTV3PrefWindow::setLabelFileName(QString fname)
{
   label_filename_->setText(QString("Volume: %1").arg(fname));
}

void QTV3PrefWindow::setLabelDim(long long sx,long long sy,long long sz)
{
   label_dim_->setText(QString("Voxels: %1x%2x%3").arg(sx).arg(sy).arg(sz));
}

void QTV3PrefWindow::setLabelVoxel(float dx,float dy,float dz)
{
   int dxi = int(dx*1E6f+0.5f);
   int dyi = int(dy*1E6f+0.5f);
   int dzi = int(dz*1E6f+0.5f);

   label_voxel_->setText(QString("Resolution in micro meters: %1/%2/%3").arg(dxi).arg(dyi).arg(dzi));
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

   label_filename_ = new QLabel;
   layout->addWidget(label_filename_);
   label_dim_ = new QLabel;
   layout->addWidget(label_dim_);
   label_voxel_ = new QLabel;
   layout->addWidget(label_voxel_);

   QFrame* line1 = new QFrame();
   line1->setFrameShape(QFrame::HLine);
   line1->setFrameShadow(QFrame::Raised);
   layout->addWidget(line1);

   QLineEdit *lineEdit_vol_maxsize = new QLineEdit;
   QGroupBox *vol_maxsize_group = createEdit("Maximum Volume Size for RAW/REK Processing", QString::number(vol_maxsize_), &lineEdit_vol_maxsize);
   connect(lineEdit_vol_maxsize,SIGNAL(textChanged(QString)),this,SLOT(volMaxSizeChange(QString)));
   layout->addWidget(vol_maxsize_group);

   QLineEdit *lineEdit_iso_maxsize = new QLineEdit;
   QGroupBox *iso_maxsize_group = createEdit("Maximum Volume Size for RAW/REK Iso Surface Extraction", QString::number(iso_maxsize_), &lineEdit_iso_maxsize);
   connect(lineEdit_iso_maxsize,SIGNAL(textChanged(QString)),this,SLOT(isoMaxSizeChange(QString)));
   layout->addWidget(iso_maxsize_group);

   QFrame* line2 = new QFrame();
   line2->setFrameShape(QFrame::HLine);
   line2->setFrameShadow(QFrame::Raised);
   layout->addWidget(line2);

   QVBoxLayout *vl = new QVBoxLayout;
   QButtonGroup *gb = new QButtonGroup(this);
   sfxOffCheck_ = new QRadioButton(tr("Normal Rendering"));
   connect(sfxOffCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkSFXoff(bool)));
   vl->addWidget(sfxOffCheck_);
   gb->addButton(sfxOffCheck_);
   anaModeCheck_ = new QRadioButton(tr("Anaglyph Stereo Mode"));
   connect(anaModeCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkAnaMode(bool)));
   vl->addWidget(anaModeCheck_);
   gb->addButton(anaModeCheck_);
   sfxOnCheck_ = new QRadioButton(tr("Dual Buffer Stereo Mode"));
   connect(sfxOnCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkSFXon(bool)));
   vl->addWidget(sfxOnCheck_);
   gb->addButton(sfxOnCheck_);
   if (vrw_stereo_) sfxOnCheck_->setChecked(true);
   else sfxOffCheck_->setChecked(true);
   layout->addLayout(vl);

   QFrame* line3 = new QFrame();
   line3->setFrameShape(QFrame::HLine);
   line3->setFrameShadow(QFrame::Raised);
   layout->addWidget(line3);

   lineEdit_slice_opacity_ = new QLineEdit;
   QGroupBox *slice_opacity_group = createEdit("Opacity of Clip Plane", QString::number(slice_opacity_), &lineEdit_slice_opacity_);
   connect(lineEdit_slice_opacity_,SIGNAL(textChanged(QString)),this,SLOT(sliceOpacityChange(QString)));
   slice_opacity_slider_=createSlider(0,100,100*slice_opacity_);
   slice_opacity_group->layout()->addWidget(slice_opacity_slider_);
   connect(slice_opacity_slider_,SIGNAL(valueChanged(int)), this, SLOT(sliceOpacityChange(int)));
   layout->addWidget(slice_opacity_group);

   lineEdit_slice_opacity2_ = new QLineEdit;
   QGroupBox *slice_opacity_group2 = createEdit("Opacity of Outer Clip Plane", QString::number(slice_opacity2_), &lineEdit_slice_opacity2_);
   connect(lineEdit_slice_opacity2_,SIGNAL(textChanged(QString)),this,SLOT(sliceOpacityChange2(QString)));
   slice_opacity_slider2_=createSlider(0,100,100*slice_opacity2_);
   slice_opacity_group2->layout()->addWidget(slice_opacity_slider2_);
   connect(slice_opacity_slider2_,SIGNAL(valueChanged(int)), this, SLOT(sliceOpacityChange2(int)));
   layout->addWidget(slice_opacity_group2);

   QFrame* line4 = new QFrame();
   line4->setFrameShape(QFrame::HLine);
   line4->setFrameShadow(QFrame::Raised);
   layout->addWidget(line4);

   lineEdit_hue_ = new QLineEdit;
   QGroupBox *hue_group = createEdit("Volume Hue", QString::number(vol_hue_), &lineEdit_hue_);
   connect(lineEdit_hue_,SIGNAL(textChanged(QString)),this,SLOT(hueChange(QString)));
   hue_slider_=createSlider(0,360,vol_hue_);
   hue_group->layout()->addWidget(hue_slider_);
   connect(hue_slider_,SIGNAL(valueChanged(int)), this, SLOT(hueChange(int)));
   layout->addWidget(hue_group);

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

void QTV3PrefWindow::sliceOpacityChange2(QString opacity)
{
   slice_opacity2_ = opacity.toFloat();
   vrw_->setOuterOpacity(slice_opacity2_);
   slice_opacity_slider2_->setValue(slice_opacity2_ * 100 * 16);
}

void QTV3PrefWindow::sliceOpacityChange2(int opacity)
{
   slice_opacity2_ = opacity / 100.0f / 16.0f;
   vrw_->setOuterOpacity(slice_opacity2_);
   lineEdit_slice_opacity2_->setText(QString::number(slice_opacity2_));
}

void QTV3PrefWindow::hueChange(QString hue)
{
   vol_hue_ = hue.toFloat();
   vrw_->setColorHue(vol_hue_);
   hue_slider_->setValue(vol_hue_ * 16);
}

void QTV3PrefWindow::hueChange(int hue)
{
   vol_hue_ = hue / 16.0f;
   vrw_->setColorHue(vol_hue_);
   lineEdit_hue_->setText(QString::number(vol_hue_));
}
