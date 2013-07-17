// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "mainwindow.h"
#include "mainconst.h"

QTV3MainWindow::QTV3MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
   createMenus();
   createWidgets();

   // accept drag and drop
   setAcceptDrops(true);

   setWindowTitle(APP_NAME" "APP_VERSION);

   vrw_->loadVolume("Drop.pvm");
   vrw_->setRotation(30.0);

   flip1_=flip2_=0;
}

QTV3MainWindow::~QTV3MainWindow()
{
   delete vrw_;
}

void QTV3MainWindow::loadVolume(const char *filename)
{
   vrw_->loadVolume(filename);

   if (label_)
   {
      layout_->removeItem(layout_->itemAt(1));
      delete label_;
      label_=NULL;
   }
}

void QTV3MainWindow::loadSeries(const std::vector<std::string> list)
{
   vrw_->loadSeries(list);

   if (label_)
   {
      layout_->removeItem(layout_->itemAt(1));
      delete label_;
      label_=NULL;
   }
}

void QTV3MainWindow::setRotation(double omega)
{
   vrw_->setRotation(omega);
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

   vrw_ = new QTV3VolRenWidget(mainGroup);
   layout_->addWidget(vrw_);

   label_ = new QLabel("Drag and drop a volume file (.pvm .ima .dcm .rek .raw) here\n"
                       "to display it with the volume renderer!");

   label_->setAlignment(Qt::AlignHCenter);
   layout_->addWidget(label_);

   update_ = new QLabel("");
   update_->setAlignment(Qt::AlignHCenter);
   connect(vrw_, SIGNAL(update_signal(QString)), this, SLOT(update_slot(QString)));
   layout_->addWidget(update_);

   QTV3Slider *s1=createSlider(0,100,0,true);
   QTV3Slider *s2=createSlider(0,100,0,true);
   QTV3Slider *s3=createSlider(-180,180,0,false);
   QTV3Slider *s4=createSlider(-90,90,0,true);
   QTV3Slider *s5=createSlider(0,100,25,true);
   QTV3Slider *s6=createSlider(0,100,25,true);

   connect(s1, SIGNAL(valueChanged(int)), this, SLOT(clip(int)));
   connect(s2, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
   connect(s3, SIGNAL(valueChanged(int)), this, SLOT(rotate(int)));
   connect(s4, SIGNAL(valueChanged(int)), this, SLOT(tilt(int)));
   connect(s5, SIGNAL(valueChanged(int)), this, SLOT(emission(int)));
   connect(s6, SIGNAL(valueChanged(int)), this, SLOT(absorption(int)));

   QGroupBox *sliderGroup = new QGroupBox(mainGroup);
   QHBoxLayout *sliderLayout = new QHBoxLayout(sliderGroup);

   QVBoxLayout *l1 = new QVBoxLayout;
   l1->addWidget(s1);
   QLabel *ll1=new QLabel("Clipping");
   ll1->setAlignment(Qt::AlignHCenter);
   l1->addWidget(ll1);
   sliderLayout->addLayout(l1);

   QFrame* line1 = new QFrame();
   line1->setFrameShape(QFrame::VLine);
   line1->setFrameShadow(QFrame::Raised);
   sliderLayout->addWidget(line1);

   QVBoxLayout *l2 = new QVBoxLayout;
   l2->addWidget(s2);
   QLabel *ll2=new QLabel("Zoom");
   ll2->setAlignment(Qt::AlignHCenter);
   l2->addWidget(ll2);
   sliderLayout->addLayout(l2);

   QVBoxLayout *l3 = new QVBoxLayout;
   QLabel *ll3=new QLabel("Rotation");
   ll3->setAlignment(Qt::AlignHCenter);
   l3->addWidget(ll3);
   l3->addStretch(1000);
   l3->addWidget(s3);
   l3->addStretch(1000);
   QHBoxLayout *h = new QHBoxLayout;
   QCheckBox *invModeCheck = new QCheckBox(tr("Inverse Mode"));
   invModeCheck->setChecked(false);
   connect(invModeCheck, SIGNAL(stateChanged(int)), this, SLOT(checkInvMode(int)));
   h->addWidget(invModeCheck);
   QCheckBox *gradMagCheck = new QCheckBox(tr("Gradient Magnitude"));
   gradMagCheck->setChecked(false);
   connect(gradMagCheck, SIGNAL(stateChanged(int)), this, SLOT(checkGradMag(int)));
   h->addWidget(gradMagCheck);
   QCheckBox *flipCheck1 = new QCheckBox(tr("Flip XY"));
   flipCheck1->setChecked(false);
   connect(flipCheck1, SIGNAL(stateChanged(int)), this, SLOT(checkFlip1(int)));
   h->addWidget(flipCheck1);
   QCheckBox *flipCheck2 = new QCheckBox(tr("Flip YZ"));
   flipCheck2->setChecked(false);
   connect(flipCheck2, SIGNAL(stateChanged(int)), this, SLOT(checkFlip2(int)));
   h->addWidget(flipCheck2);
   l3->addLayout(h);
   sliderLayout->addLayout(l3);

   QVBoxLayout *l4 = new QVBoxLayout;
   l4->addWidget(s4);
   QLabel *ll4=new QLabel("Tilt");
   ll4->setAlignment(Qt::AlignHCenter);
   l4->addWidget(ll4);
   sliderLayout->addLayout(l4);

   QFrame* line2 = new QFrame();
   line2->setFrameShape(QFrame::VLine);
   line2->setFrameShadow(QFrame::Raised);
   sliderLayout->addWidget(line2);

   QVBoxLayout *l5 = new QVBoxLayout;
   l5->addWidget(s5);
   QLabel *ll5=new QLabel("Emission");
   ll5->setAlignment(Qt::AlignHCenter);
   l5->addWidget(ll5);
   sliderLayout->addLayout(l5);

   QVBoxLayout *l6 = new QVBoxLayout;
   l6->addWidget(s6);
   QLabel *ll6=new QLabel("Absorption");
   ll6->setAlignment(Qt::AlignHCenter);
   l6->addWidget(ll6);
   sliderLayout->addLayout(l6);

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
   fd->setFilter("All Files (*);;GIF Files (*.gif);;JPEG Files (*.jpg);;TIFF Files(*.tif *.tiff)");

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
            url.remove("file://");
            url.replace('\\', '/');
            if (url.contains(QRegExp("^/[A-Z]:"))) url.remove(0,1);

            loadVolume(url.toStdString().c_str());
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

         loadSeries(list);
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
      loadVolume(files.at(0).toStdString().c_str());
   }
   else
   {
      std::vector<std::string> list;

      for (int i=0; i<files.size(); i++)
      {
         list.push_back(files.at(i).toStdString());
      }

      loadSeries(list);
   }
}

void QTV3MainWindow::zoom(int v)
{
   double zoom = v / 16.0 / 100.0;
   vrw_->setZoom(zoom);
}

void QTV3MainWindow::rotate(int v)
{
   double angle = v / 16.0;
   vrw_->setAngle(angle);
}

void QTV3MainWindow::tilt(int v)
{
   double tilt = v / 16.0;
   vrw_->setTilt(tilt);
}

void QTV3MainWindow::clip(int v)
{
   double dist = v / 16.0 / 100.0;
   vrw_->setClipDist(1.0-2*dist);
}

void QTV3MainWindow::emission(int v)
{
   double emi = v / 16.0 / 100.0;
   vrw_->setEmission(emi);
}

void QTV3MainWindow::absorption(int v)
{
   double att = v / 16.0 / 100.0;
   vrw_->setAbsorption(att);
}

void QTV3MainWindow::checkInvMode(int on)
{
   vrw_->setInvMode(on);
}

void QTV3MainWindow::checkGradMag(int on)
{
   vrw_->setGradMag(on);
}

void QTV3MainWindow::checkFlip1(int on)
{
   flip1_=on;

   if (flip1_ && flip2_)
   {
      vrw_->setTilt1(180);
      vrw_->setTilt2(0);
   }
   else
   {
      vrw_->setTilt1(flip1_?90.0:0.0);
      vrw_->setTilt2(flip2_?90.0:0.0);
   }
}

void QTV3MainWindow::checkFlip2(int on)
{
   flip2_=on;

   if (flip1_ && flip2_)
   {
      vrw_->setTilt1(180);
      vrw_->setTilt2(0);
   }
   else
   {
      vrw_->setTilt1(flip1_?90.0:0.0);
      vrw_->setTilt2(flip2_?90.0:0.0);
   }
}

void QTV3MainWindow::about()
{
   QMessageBox::about(this, tr("About this program"),
                      APP_NAME" "APP_VERSION
                      "\n"
                      "\n"APP_LICENSE
                      "\n"APP_COPYRIGHT
                      "\n"
                      "\n"APP_DISCLAIMER);
}

void QTV3MainWindow::update_slot(QString text)
{
   update_->setText(text);
}
