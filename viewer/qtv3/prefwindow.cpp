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

   ratio_ = 0.25f;

   QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

   if (env.contains("QTV3_VOL_LIMIT"))
   {
      vol_maxsize_ = env.value("QTV3_VOL_LIMIT").toUInt();
   }

   if (env.contains("QTV3_ISO_LIMIT"))
   {
      iso_maxsize_ = env.value("QTV3_ISO_LIMIT").toUInt();
   }

   vrw_->set_vol_maxsize(vol_maxsize_, ratio_);
   vrw_->set_iso_maxsize(iso_maxsize_, ratio_);

   createWidgets();
}

QTV3PrefWindow::~QTV3PrefWindow()
{}

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
   QGroupBox *vol_maxsize_group = createEdit("Maximum Displayable Volume Size", QString::number(vol_maxsize_), &lineEdit_vol_maxsize);
   connect(lineEdit_vol_maxsize,SIGNAL(textChanged(QString)),this,SLOT(volMaxSizeChange(QString)));
   layout->addWidget(vol_maxsize_group);

   QLineEdit *lineEdit_iso_maxsize = new QLineEdit;
   QGroupBox *iso_maxsize_group = createEdit("Maximum Volume Size For Iso Surface Extraction", QString::number(iso_maxsize_), &lineEdit_iso_maxsize);
   connect(lineEdit_iso_maxsize,SIGNAL(textChanged(QString)),this,SLOT(isoMaxSizeChange(QString)));
   layout->addWidget(iso_maxsize_group);

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

void QTV3PrefWindow::volMaxSizeChange(QString maxsize)
{
   vol_maxsize_ = maxsize.toUInt();
   vrw_->set_vol_maxsize(vol_maxsize_, ratio_);
}

void QTV3PrefWindow::isoMaxSizeChange(QString maxsize)
{
   iso_maxsize_ = maxsize.toUInt();
   vrw_->set_iso_maxsize(iso_maxsize_, ratio_);
}
