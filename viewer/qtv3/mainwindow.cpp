// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "mainwindow.h"

#define APP_NAME "QTV3"
#define APP_VERSION "0.3"

QTV3MainWindow::QTV3MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
   createMenus();
   createWidgets();

   // accept drag and drop
   setAcceptDrops(true);

   setWindowTitle(APP_NAME" "APP_VERSION);

   vrw_->loadvolume("Drop.pvm");
   vrw_->setrotation(30.0);
}

QTV3MainWindow::~QTV3MainWindow()
{
   delete vrw_;
}

void QTV3MainWindow::loadvolume(const char *filename)
{
   vrw_->loadvolume(filename);

   if (label_)
   {
      layout_->removeItem(layout_->itemAt(1));
      delete label_;
      label_=NULL;
   }
}

void QTV3MainWindow::loadseries(const std::vector<std::string> list)
{
   vrw_->loadseries(list);

   if (label_)
   {
      layout_->removeItem(layout_->itemAt(1));
      delete label_;
      label_=NULL;
   }
}

void QTV3MainWindow::setrotation(double omega)
{
   vrw_->setrotation(omega);
}

void QTV3MainWindow::createMenus()
{
   QAction *quitAction = new QAction(tr("Q&uit"), this);
   quitAction->setShortcuts(QKeySequence::Quit);
   quitAction->setStatusTip(tr("Quit the application"));
   connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

   QAction *openAction = new QAction(tr("O&pen"), this);
   openAction->setShortcuts(QKeySequence::Open);
   openAction->setStatusTip(tr("Open volume"));
   connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

   QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
   fileMenu->addAction(openAction);
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
   QGroupBox *mainGroup = new QGroupBox(this);
   layout_ = new QVBoxLayout(mainGroup);

   vrw_ = new QGLVolRenWidget(mainGroup);
   layout_->addWidget(vrw_);

   label_ = new QLabel("Drag and drop a volume file (.pvm) here\nto display it with the volume renderer!");
   label_->setAlignment(Qt::AlignHCenter);
   layout_->addWidget(label_);

   QTV3Slider *s1=createSlider(0,100,0,true);
   QTV3Slider *s2=createSlider(-180,180,0,false);
   QTV3Slider *s3=createSlider(0,100,50,true);
   QTV3Slider *s4=createSlider(0,100,50,true);

   connect(s1, SIGNAL(valueChanged(int)), this, SLOT(clip(int)));
   connect(s2, SIGNAL(valueChanged(int)), this, SLOT(rotate(int)));

   QGroupBox *sliderGroup = new QGroupBox(mainGroup);
   QHBoxLayout *sliderLayout = new QHBoxLayout(sliderGroup);

   QVBoxLayout *l1 = new QVBoxLayout;
   l1->addWidget(s1);
   QLabel *ll1=new QLabel("Clipping");
   ll1->setAlignment(Qt::AlignHCenter);
   l1->addWidget(ll1);
   sliderLayout->addLayout(l1);

   QVBoxLayout *l2 = new QVBoxLayout;
   l2->addWidget(s2);
   l2->addStretch(1000);
   QLabel *ll2=new QLabel("Rotation");
   ll2->setAlignment(Qt::AlignHCenter);
   l2->addWidget(ll2);
   sliderLayout->addLayout(l2);

   QVBoxLayout *l3 = new QVBoxLayout;
   l3->addWidget(s3);
   QLabel *ll3=new QLabel("Emission");
   ll3->setAlignment(Qt::AlignHCenter);
   l3->addWidget(ll3);
   sliderLayout->addLayout(l3);

   QVBoxLayout *l4 = new QVBoxLayout;
   l4->addWidget(s4);
   QLabel *ll4=new QLabel("Absorption");
   ll4->setAlignment(Qt::AlignHCenter);
   l4->addWidget(ll4);
   sliderLayout->addLayout(l4);

   sliderGroup->setLayout(sliderLayout);
   layout_->addWidget(sliderGroup);

   mainGroup->setLayout(layout_);
   setCentralWidget(mainGroup);
}

QStringList QTV3MainWindow::browse(QString path,
                                   bool newfile)
{
   QFileDialog* fd = new QFileDialog(this, "File Selector Dialog");

   if (!newfile) fd->setFileMode(QFileDialog::ExistingFiles);
   else fd->setFileMode(QFileDialog::AnyFile);
   fd->setViewMode(QFileDialog::List);
   if (newfile) fd->setAcceptMode(QFileDialog::AcceptSave);
   fd->setFilter("All Files (*.*);;GIF Files (*.gif);;JPEG Files (*.jpg);;TIFF Files(*.tif *.tiff)");

   if (path!="") fd->setDirectory(path);

   QStringList files;

   if (fd->exec() == QDialog::Accepted)
      for (int i=0; i<fd->selectedFiles().size(); i++)
      {
         QString fileName = fd->selectedFiles().at(i);

         if (!fileName.isNull())
            files += fileName;
      }

   delete fd;

   return(files);
}

QTV3Slider *QTV3MainWindow::createSlider(int minimum, int maximum, int value,
                                         bool vertical)
{
   QTV3Slider *slider = new QTV3Slider(vertical?Qt::Vertical:Qt::Horizontal);
   slider->setRange(minimum * 16, maximum * 16);
   slider->setSingleStep(16);
   slider->setPageStep((maximum - minimum) / 10 * 16);
   slider->setTickInterval((maximum - minimum) / 10 * 16);
   slider->setTickPosition(QSlider::TicksBelow);
   slider->setValue(value * 16);
   return(slider);
}

QSize QTV3MainWindow::minimumSizeHint() const
{
   return(QSize(100, 100));
}

QSize QTV3MainWindow::sizeHint() const
{
   return(QSize(512, 512));
}

void QTV3MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
   event->acceptProposedAction();
}

void QTV3MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
   event->acceptProposedAction();
}

void QTV3MainWindow::dropEvent(QDropEvent *event)
{
   const QMimeData *mimeData = event->mimeData();

   if (mimeData->hasUrls())
   {
      event->acceptProposedAction();

      QList<QUrl> urlList = mimeData->urls();

      if (urlList.size()==1)
      {
         QUrl qurl = urlList.at(0);
         QString url = qurl.toString();

         if (url.startsWith("file://"))
         {
            url = url.remove("file://");
            loadvolume(url.toStdString().c_str());
         }
      }
      else
      {
         std::vector<std::string> list;

         for (int i=0; i<urlList.size(); i++)
         {
            QUrl qurl = urlList.at(i);
            QString url = qurl.toString();

            if (url.startsWith("file://"))
            {
               url = url.remove("file://");
               list.push_back(url.toStdString());
            }
         }

         loadseries(list);
      }
   }
}

void QTV3MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
   event->accept();
}

void QTV3MainWindow::open()
{
   QStringList files = browse();

   if (files.size()==1)
   {
      loadvolume(files.at(0).toStdString().c_str());
   }
   else
   {
      std::vector<std::string> list;

      for (int i=0; i<files.size(); i++)
      {
         list.push_back(files.at(i).toStdString());
      }

      loadseries(list);
   }
}

void QTV3MainWindow::clip(int v)
{
   double dist = v / 16.0 / 100.0;
   vrw_->setclipdist(1.0-2*dist);
}

void QTV3MainWindow::rotate(int v)
{
   double angle = v / 16.0;
   vrw_->setangle(angle);
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
