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

   sfx_base_ = 1.0f;
   sfx_focus_ = 1.0f;

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

   shotname_="shot";
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
   shotname_=fname;
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

void QTV3PrefWindow::createWidgets()
{
   group_ = new QScrollArea;
   container_ = new QWidget;
   layout_ = new QVBoxLayout;

   label_filename_ = new QLabel;
   layout_->addWidget(label_filename_);
   label_dim_ = new QLabel;
   layout_->addWidget(label_dim_);
   label_voxel_ = new QLabel;
   layout_->addWidget(label_voxel_);

   QPushButton *shotButton = new QPushButton(tr("Grab"));
   connect(shotButton, SIGNAL(pressed()), this, SLOT(grab()));
   layout_->addWidget(shotButton);

   QFrame* line1 = new QFrame();
   line1->setFrameShape(QFrame::HLine);
   line1->setFrameShadow(QFrame::Raised);
   layout_->addWidget(line1);

   lineEdit_gfx_maxsize_ = new QLineEdit;
   QGroupBox *gfx_maxsize_group = createEdit("Maximum Graphics Memory Size (MB)", QString::number((int)(2.0*vol_maxsize_*vol_maxsize_*vol_maxsize_/1024/1024+0.5)), &lineEdit_gfx_maxsize_);
   connect(lineEdit_gfx_maxsize_,SIGNAL(textChanged(QString)),this,SLOT(volGfxSizeChange(QString)));
   layout_->addWidget(gfx_maxsize_group);

   lineEdit_vol_maxsize_ = new QLineEdit;
   QGroupBox *vol_maxsize_group = createEdit("Maximum Dimension for RAW/REK Processing (Voxels)", QString::number(vol_maxsize_), &lineEdit_vol_maxsize_);
   connect(lineEdit_vol_maxsize_,SIGNAL(textChanged(QString)),this,SLOT(volMaxSizeChange(QString)));
   layout_->addWidget(vol_maxsize_group);

   lineEdit_iso_maxsize_ = new QLineEdit;
   QGroupBox *iso_maxsize_group = createEdit("Maximum Dimension for RAW/REK Iso Surface Extraction (Voxels)", QString::number(iso_maxsize_), &lineEdit_iso_maxsize_);
   connect(lineEdit_iso_maxsize_,SIGNAL(textChanged(QString)),this,SLOT(isoMaxSizeChange(QString)));
   layout_->addWidget(iso_maxsize_group);

   QFrame* line2 = new QFrame();
   line2->setFrameShape(QFrame::HLine);
   line2->setFrameShadow(QFrame::Raised);
   layout_->addWidget(line2);

   QVBoxLayout *vl = new QVBoxLayout;
   QButtonGroup *gb = new QButtonGroup(this);
   sfxOffCheck_ = new QRadioButton(tr("Normal Rendering"));
   connect(sfxOffCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkSFXoff(bool)));
   vl->addWidget(sfxOffCheck_);
   gb->addButton(sfxOffCheck_);
   anaModeCheck_ = new QRadioButton(tr("Anaglyph Stereo"));
   connect(anaModeCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkAnaMode(bool)));
   vl->addWidget(anaModeCheck_);
   gb->addButton(anaModeCheck_);
   sfxModeCheck_ = new QRadioButton(tr("Interlaced Stereo"));
   connect(sfxModeCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkSFXMode(bool)));
   vl->addWidget(sfxModeCheck_);
   gb->addButton(sfxModeCheck_);
   sfxOnCheck_ = new QRadioButton(tr("Quad Buffered Stereo"));
   connect(sfxOnCheck_, SIGNAL(toggled(bool)), parent(), SLOT(checkSFXon(bool)));
   vl->addWidget(sfxOnCheck_);
   gb->addButton(sfxOnCheck_);
   if (vrw_stereo_) sfxOnCheck_->setChecked(true);
   else sfxOffCheck_->setChecked(true);
   layout_->addLayout(vl);

   QGroupBox *sfx_group = new QGroupBox();
   QVBoxLayout *sfx_layout = new QVBoxLayout();
   sfx_group->setLayout(sfx_layout);
   sfx_layout->addWidget(new QLabel("Stereo Base"));
   sfxBase_slider_ = createSlider(0,500,100);
   sfx_layout->addWidget(sfxBase_slider_);
   connect(sfxBase_slider_,SIGNAL(valueChanged(int)), this, SLOT(sfxBaseChange(int)));
   sfx_layout->addWidget(new QLabel("Stereo Focus"));
   sfxFocus_slider_ = createSlider(50,200,100);
   sfx_layout->addWidget(sfxFocus_slider_);
   connect(sfxFocus_slider_,SIGNAL(valueChanged(int)), this, SLOT(sfxFocusChange(int)));
   layout_->addWidget(sfx_group);

   QFrame* line3 = new QFrame();
   line3->setFrameShape(QFrame::HLine);
   line3->setFrameShadow(QFrame::Raised);
   layout_->addWidget(line3);

   lineEdit_slice_opacity_ = new QLineEdit;
   QGroupBox *slice_opacity_group = createEdit("Opacity of Clip Plane", QString::number(slice_opacity_), &lineEdit_slice_opacity_);
   connect(lineEdit_slice_opacity_,SIGNAL(textChanged(QString)),this,SLOT(sliceOpacityChange(QString)));
   slice_opacity_slider_=createSlider(0,100,100*slice_opacity_);
   slice_opacity_group->layout()->addWidget(slice_opacity_slider_);
   connect(slice_opacity_slider_,SIGNAL(valueChanged(int)), this, SLOT(sliceOpacityChange(int)));
   layout_->addWidget(slice_opacity_group);

   lineEdit_slice_opacity2_ = new QLineEdit;
   QGroupBox *slice_opacity_group2 = createEdit("Opacity of Outer Clip Plane", QString::number(slice_opacity2_), &lineEdit_slice_opacity2_);
   connect(lineEdit_slice_opacity2_,SIGNAL(textChanged(QString)),this,SLOT(sliceOpacityChange2(QString)));
   slice_opacity_slider2_=createSlider(0,100,100*slice_opacity2_);
   slice_opacity_group2->layout()->addWidget(slice_opacity_slider2_);
   connect(slice_opacity_slider2_,SIGNAL(valueChanged(int)), this, SLOT(sliceOpacityChange2(int)));
   layout_->addWidget(slice_opacity_group2);

   QFrame* line4 = new QFrame();
   line4->setFrameShape(QFrame::HLine);
   line4->setFrameShadow(QFrame::Raised);
   layout_->addWidget(line4);

   lineEdit_hue_ = new QLineEdit;
   QGroupBox *hue_group = createEdit("Volume Hue", QString::number(vol_hue_), &lineEdit_hue_);
   connect(lineEdit_hue_,SIGNAL(textChanged(QString)),this,SLOT(hueChange(QString)));
   hue_slider_=createSlider(0,360,vol_hue_);
   hue_group->layout()->addWidget(hue_slider_);
   connect(hue_slider_,SIGNAL(valueChanged(int)), this, SLOT(hueChange(int)));
   layout_->addWidget(hue_group);

   layout_->addStretch(1000);

   container_->setLayout(layout_);
   group_->setWidget(container_);
   group_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   group_->setMinimumSize(container_->size().width()+group_->verticalScrollBar()->sizeHint().width()+5,100);

   setWidget(group_);
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

void QTV3PrefWindow::volMaxSizeChange(unsigned int vol_maxsize)
{
   vol_maxsize_ = vol_maxsize;
   lineEdit_gfx_maxsize_->setText(QString::number((int)(2.0*vol_maxsize_*vol_maxsize_*vol_maxsize_/1024/1024+0.5)));
   vrw_->set_vol_maxsize(vol_maxsize_, border_ratio_);
}

void QTV3PrefWindow::volMaxSizeChange(QString maxsize)
{
   volMaxSizeChange(maxsize.toUInt());
}

void QTV3PrefWindow::volGfxSizeChange(QString maxsize)
{
   vol_maxsize_ = (int)(pow(0.5*maxsize.toUInt()*1024*1024,1.0/3)+0.5);
   lineEdit_vol_maxsize_->setText(QString::number(vol_maxsize_));
   vrw_->set_vol_maxsize(vol_maxsize_, border_ratio_);
}

void QTV3PrefWindow::isoMaxSizeChange(unsigned int iso_maxsize)
{
   iso_maxsize_ = iso_maxsize;
   vrw_->set_iso_maxsize(iso_maxsize_, border_ratio_);
}

void QTV3PrefWindow::isoMaxSizeChange(QString maxsize)
{
   isoMaxSizeChange(maxsize.toUInt());
}

void QTV3PrefWindow::sfxBaseChange(int value)
{
   sfx_base_ = value/100.0f/16;
   vrw_->setSFXparams(sfx_base_, sfx_focus_);
}

void QTV3PrefWindow::sfxFocusChange(int value)
{
   sfx_focus_ = value/100.0f/16;
   vrw_->setSFXparams(sfx_base_, sfx_focus_);
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

void QTV3PrefWindow::colorHueChange(float hue)
{
   vol_hue_ = hue;
   vrw_->setColorHue(vol_hue_);
   hue_slider_->setValue(vol_hue_ * 16);
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

bool QTV3PrefWindow::checkFormat(QString format)
{
   QList<QByteArray> formats = QImageWriter::supportedImageFormats();

   for (QList<QByteArray>::iterator i=formats.begin(); i!=formats.end(); i++)
      if (QString(*i).toLower() == format.toLower())
         return(true);

   return(false);
}

bool QTV3PrefWindow::grab(QString format)
{
   if (!checkFormat(format))
   {
      format = "png";
      if (!checkFormat(format))
      {
         format = "tif";
         if (!checkFormat(format))
         {
            format = "bmp";
            if (!checkFormat(format))
            {
               QMessageBox::information(this, tr("Error"),
                                        "Unsupported image format",
                                        QMessageBox::Ok);

               return(false);
            }
         }
      }
   }

   QImage image = vrw_->grabFrameBuffer();
   QPixmap window = QPixmap::fromImage(image);

   QString date = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
   QString name = shotname_ + "_" + date + "." + format;

   bool saved = window.save(name, format.toUpper().toStdString().c_str());

   if (!saved)
      QMessageBox::information(this, tr("Error"),
                               "Could not save image",
                               QMessageBox::Ok);

   return(saved);
}
