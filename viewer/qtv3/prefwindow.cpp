// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "prefwindow.h"

QTV3PrefWindow::QTV3PrefWindow(QWidget *parent)
   : QDockWidget(parent)
{
   setWindowTitle("QTV3 Volume Rendering Preferences");

   QTextEdit *textEdit = new QTextEdit;
   setWidget(textEdit);
}

QTV3PrefWindow::~QTV3PrefWindow()
{
}

QSize QTV3PrefWindow::minimumSizeHint() const
{
   return(QSize(512, 100));
}

QSize QTV3PrefWindow::sizeHint() const
{
   return(QSize(512, 768));
}
