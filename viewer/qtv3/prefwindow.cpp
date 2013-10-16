// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "prefwindow.h"

QTV3PrefWindow::QTV3PrefWindow(QWidget *parent)
   : QDockWidget(parent)
{
   setWindowTitle("QTV3 Volume Rendering Preferences");

   vol_maxsize_=512;
   iso_maxsize_=256;

   QGroupBox *group = new QGroupBox;
   QVBoxLayout *layout = new QVBoxLayout;

   QLineEdit *lineEdit_vol_maxsize = new QLineEdit;
   QGroupBox *vol_maxsize_group = createEdit("Maximum Displayable Volume Size", QString::number(vol_maxsize_), &lineEdit_vol_maxsize);
   connect(lineEdit_vol_maxsize,SIGNAL(textChanged(QString)),this,SLOT(volMaxSizeChange(QString)));
   layout->addWidget(vol_maxsize_group);

   QLineEdit *lineEdit_iso_maxsize = new QLineEdit;
   QGroupBox *iso_maxsize_group = createEdit("Maximum Volume Size for Iso Surface Extraction", QString::number(iso_maxsize_), &lineEdit_iso_maxsize);
   connect(lineEdit_iso_maxsize,SIGNAL(textChanged(QString)),this,SLOT(isoMaxSizeChange(QString)));
   layout->addWidget(iso_maxsize_group);

   layout->addStretch(1000);

   group->setLayout(layout);
   setWidget(group);
}

QTV3PrefWindow::~QTV3PrefWindow()
{
}

QSize QTV3PrefWindow::minimumSizeHint() const
{
   return(QSize(256, 100));
}

QSize QTV3PrefWindow::sizeHint() const
{
   return(QSize(256, 768));
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
}

void QTV3PrefWindow::isoMaxSizeChange(QString maxsize)
{
   iso_maxsize_ = maxsize.toUInt();
}
