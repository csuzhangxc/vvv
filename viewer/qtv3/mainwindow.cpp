// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "mainwindow.h"

#define APP_NAME "QTV3"
#define APP_VERSION "0.1"

QTV3MainWindow::QTV3MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
   createMenus();
   createWidgets();

   setWindowTitle(APP_NAME" "APP_VERSION);
}

QTV3MainWindow::~QTV3MainWindow()
{
   delete vr_;
}

void QTV3MainWindow::loadvolume(const char *filename)
{
   vr_->loadvolume(filename);
}

void QTV3MainWindow::createMenus()
{
   QAction *quitAction = new QAction(tr("Q&uit"), this);
   quitAction->setShortcuts(QKeySequence::Quit);
   quitAction->setStatusTip(tr("Quit the application"));
   connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

   QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
   fileMenu->addAction(quitAction);

   QAction *aboutAction = new QAction(tr("&About"), this);
   aboutAction->setShortcut(tr("Ctrl+A"));
   aboutAction->setStatusTip(tr("About this program"));
   connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

   QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
   helpMenu->addAction(aboutAction);
}

void QTV3MainWindow::createWidgets()
{
   QGroupBox *mainGroup = new QGroupBox;
   QVBoxLayout *layout = new QVBoxLayout;

   vr_ = new QGLVolRenWidget;
   layout->addWidget(vr_);

   mainGroup->setLayout(layout);
   setCentralWidget(mainGroup);
}

QSize QTV3MainWindow::minimumSizeHint() const
{
   return(QSize(100, 100));
}

QSize QTV3MainWindow::sizeHint() const
{
   return(QSize(512, 512));
}

void QTV3MainWindow::about()
{
   QMessageBox::about(this, tr("About this program"),
                      APP_NAME" "APP_VERSION
                      "\n"
                      "\n(c) by Stefan Roettger"
                      "\nmailto:snroettg@googlemail.com"
                      "\nlicensed under GPL 2.0+");
}
