// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "mainwindow.h"
#include "mainconst.h"

QTV3MainWindow::QTV3MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
   createMenus();
   createWidgets();

   prefs_=NULL;

   // accept drag and drop
   setAcceptDrops(true);

   setWindowTitle(APP_NAME" "APP_VERSION);

   reset();
}

QTV3MainWindow::~QTV3MainWindow()
{
   delete vrw_;
   delete prefs_;
}

void QTV3MainWindow::loadVolume(const char *filename)
{
   reset();

   if (label_)
   {
      mainLayout_->removeItem(mainLayout_->itemAt(0));
      delete label_;
      label_=NULL;
   }

   prefs_->setLabelFileName(filename);

   vrw_->loadVolume(filename);

   hasTeaserVolume_=false;
}

void QTV3MainWindow::loadSeries(const std::vector<std::string> list)
{
   reset();

   if (label_)
   {
      mainLayout_->removeItem(mainLayout_->itemAt(0));
      delete label_;
      label_=NULL;
   }

   prefs_->setLabelFileName("dicom series");

   vrw_->loadSeries(list);

   hasTeaserVolume_=false;
}

void QTV3MainWindow::loadSurface(const char *filename)
{
   if (hasTeaserVolume_)
   {
      vrw_->clearVolume();
      hasTeaserVolume_=false;
   }

   vrw_->loadSurface(filename);

   if (label_)
   {
      mainLayout_->removeItem(mainLayout_->itemAt(0));
      delete label_;
      label_=NULL;
   }
}

void QTV3MainWindow::setRotation(double omega)
{
   vrw_->setRotation(omega);

   rotateCheck_->setChecked(omega!=0.0);
}

void QTV3MainWindow::createMenus()
{
   QAction *quitAction = new QAction(tr("Q&uit"), this);
   quitAction->setShortcuts(QKeySequence::Quit);
   quitAction->setStatusTip(tr("Quit the application"));
   connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

   QAction *openAction = new QAction(tr("O&pen"), this);
   openAction->setShortcuts(QKeySequence::Open);
   openAction->setStatusTip(tr("Open Volume File"));
   connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

   QAction *prefAction = new QAction(tr("P&references"), this);
   prefAction->setShortcuts(QKeySequence::Preferences);
   prefAction->setStatusTip(tr("Set Volume Rendering Preferences"));
   connect(prefAction, SIGNAL(triggered()), this, SLOT(prefs()));

   fileMenu_ = menuBar()->addMenu(tr("&File"));
   fileMenu_->addAction(openAction);
   fileMenu_->addAction(prefAction);
   fileMenu_->addAction(quitAction);

   QAction *aboutAction = new QAction(tr("&About"), this);
   aboutAction->setShortcut(tr("Ctrl+A"));
   aboutAction->setStatusTip(tr("About this program"));
   connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

   helpMenu_ = menuBar()->addMenu(tr("&Help"));
   helpMenu_->addAction(aboutAction);
}

void QTV3MainWindow::createWidgets()
{
   QGroupBox *mainGroup = new QGroupBox(this);
   mainLayout_ = new QVBoxLayout(mainGroup);

   // create main group
   mainSplitter_ = new QSplitter;
   QGroupBox *viewerGroup = new QGroupBox;
   viewerLayout_ = new QVBoxLayout;
   viewerSplitter_ = new QSplitter;
   QGroupBox *sliderGroup = new QGroupBox;
   sliderLayout_ = new QHBoxLayout;

   // create main splitter group
   mainSplitter_->setOrientation(Qt::Vertical);
   mainSplitter_->addWidget(viewerGroup);
   mainSplitter_->addWidget(sliderGroup);
   viewerGroup->setLayout(viewerLayout_);
   sliderGroup->setLayout(sliderLayout_);

   mainLayout_->addWidget(mainSplitter_);
   mainGroup->setLayout(mainLayout_);

   viewerLayout_->addWidget(viewerSplitter_);
   viewerSplitter_->setOrientation(Qt::Horizontal);

   // create qtv3 volren widget
   vrw_stereo_ = false;
   vrw_ = new QTV3VolRenWidget(viewerSplitter_,vrw_stereo_);

   // create usage hint label
   label_ = new QLabel("Drag and drop a volume file (.pvm .rek .raw)\n"
                       "or drop a dicom series (*.ima *.dcm) into the window\n"
                       "to display it with the volume renderer!");

   label_->setAlignment(Qt::AlignHCenter);
   mainLayout_->insertWidget(0,label_);

   // create update info label
   update_ = new QLabel("");
   update_->setAlignment(Qt::AlignHCenter);
   connect(vrw_, SIGNAL(updating_signal()), this, SLOT(updating_slot()));
   connect(vrw_, SIGNAL(update_signal(QString)), this, SLOT(update_slot(QString)));
   connect(vrw_, SIGNAL(updated_signal()), this, SLOT(updated_slot()));
   mainLayout_->addWidget(update_);

   // create sliders
   QTV3Slider *s1=createSlider(0,100,0,true);
   QTV3Slider *s2=createSlider(0,100,0,true);
   QTV3Slider *s3=createSlider(-180,180,0,false);
   QTV3Slider *s4=createSlider(-90,90,0,true);
   QTV3Slider *s5=createSlider(0,100,25,true);
   QTV3Slider *s6=createSlider(0,100,25,true);

   clipSlider_=s1;
   zoomSlider_=s2;
   rotSlider_=s3;
   tiltSlider_=s4;
   emiSlider_=s5;
   attSlider_=s6;

   connect(s1, SIGNAL(valueChanged(int)), this, SLOT(clip(int)));
   connect(s2, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
   connect(s3, SIGNAL(valueChanged(int)), this, SLOT(rotate(int)));
   connect(s4, SIGNAL(valueChanged(int)), this, SLOT(tilt(int)));
   connect(s5, SIGNAL(valueChanged(int)), this, SLOT(emission(int)));
   connect(s6, SIGNAL(valueChanged(int)), this, SLOT(absorption(int)));

   // create clipping section
   QVBoxLayout *l1 = new QVBoxLayout;
   l1->addWidget(s1);
   QLabel *ll1=new QLabel("Clipping");
   ll1->setAlignment(Qt::AlignLeft);
   QPushButton *tackButton = new QPushButton(tr("Tack"));
   connect(tackButton, SIGNAL(pressed()), this, SLOT(tack()));
   QPushButton *clearButton = new QPushButton(tr("Clear"));
   connect(clearButton, SIGNAL(pressed()), this, SLOT(clear()));
   l1->addWidget(ll1);
   l1->addWidget(tackButton);
   l1->addWidget(clearButton);
   planeCheck_ = new QCheckBox(tr("Clip Plane"));
   planeCheck_->setChecked(false);
   connect(planeCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkPlane(int)));
   l1->addWidget(planeCheck_);

   QGroupBox *g1 = new QGroupBox;
   g1->setLayout(l1);
   viewerSplitter_->addWidget(g1);

   // create zoom section
   QVBoxLayout *l2 = new QVBoxLayout;
   l2->addWidget(s2);
   QLabel *ll2=new QLabel("Zoom");
   ll2->setAlignment(Qt::AlignHCenter);
   l2->addWidget(ll2);
   sliderLayout_->addLayout(l2);

   // create rotation and options section
   QVBoxLayout *l3 = new QVBoxLayout;
   QLabel *ll3=new QLabel("Rotation");
   ll3->setAlignment(Qt::AlignHCenter);
   l3->addWidget(ll3);
   l3->addStretch(1000);
   l3->addWidget(s3);
   l3->addStretch(1000);
   QHBoxLayout *h1 = new QHBoxLayout;
   gradMagCheck_ = new QCheckBox(tr("Gradient Magnitude"));
   gradMagCheck_->setChecked(false);
   connect(gradMagCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkGradMag(int)));
   h1->addWidget(gradMagCheck_);
   invModeCheck_ = new QCheckBox(tr("Inverse Mode"));
   invModeCheck_->setChecked(false);
   connect(invModeCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkInvMode(int)));
   h1->addWidget(invModeCheck_);
   rotateCheck_ = new QCheckBox(tr("Rotate"));
   rotateCheck_->setChecked(true);
   connect(rotateCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkRotate(int)));
   h1->addWidget(rotateCheck_);
   reverseCheck_ = new QCheckBox(tr("Reverse"));
   reverseCheck_->setChecked(false);
   connect(reverseCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkReverse(int)));
   h1->addWidget(reverseCheck_);
   QHBoxLayout *h2 = new QHBoxLayout;
   QButtonGroup *gb1 = new QButtonGroup(this);
   sfxOffCheck_ = new QRadioButton(tr("Normal Rendering"));
   connect(sfxOffCheck_, SIGNAL(toggled(bool)), this, SLOT(checkSFXoff(bool)));
   h2->addWidget(sfxOffCheck_);
   gb1->addButton(sfxOffCheck_);
   anaModeCheck_ = new QRadioButton(tr("Anaglyph Stereo Mode"));
   connect(anaModeCheck_, SIGNAL(toggled(bool)), this, SLOT(checkAnaMode(bool)));
   h2->addWidget(anaModeCheck_);
   gb1->addButton(anaModeCheck_);
   sfxOnCheck_ = new QRadioButton(tr("Dual Buffer Stereo Mode"));
   connect(sfxOnCheck_, SIGNAL(toggled(bool)), this, SLOT(checkSFXon(bool)));
   h2->addWidget(sfxOnCheck_);
   gb1->addButton(sfxOnCheck_);
   sfxOffCheck_->setChecked(true);
   sfxOffCheck_->hide();
   anaModeCheck_->hide();
   sfxOnCheck_->hide();
   QHBoxLayout *h3 = new QHBoxLayout;
   flipCheckXY1_ = new QCheckBox(tr("Flip +XY"));
   flipCheckXY1_->setChecked(false);
   connect(flipCheckXY1_, SIGNAL(stateChanged(int)), this, SLOT(checkFlipXY1(int)));
   h3->addWidget(flipCheckXY1_);
   flipCheckXY2_ = new QCheckBox(tr("Flip -XY"));
   flipCheckXY2_->setChecked(false);
   connect(flipCheckXY2_, SIGNAL(stateChanged(int)), this, SLOT(checkFlipXY2(int)));
   h3->addWidget(flipCheckXY2_);
   flipCheckYZ1_ = new QCheckBox(tr("Flip +YZ"));
   flipCheckYZ1_->setChecked(false);
   connect(flipCheckYZ1_, SIGNAL(stateChanged(int)), this, SLOT(checkFlipYZ1(int)));
   h3->addWidget(flipCheckYZ1_);
   flipCheckYZ2_ = new QCheckBox(tr("Flip -YZ"));
   flipCheckYZ2_->setChecked(false);
   connect(flipCheckYZ2_, SIGNAL(stateChanged(int)), this, SLOT(checkFlipYZ2(int)));
   h3->addWidget(flipCheckYZ2_);
   QHBoxLayout *h4 = new QHBoxLayout;
   QButtonGroup *gb2 = new QButtonGroup(this);
   sampleButton1_ = new QRadioButton(tr("Undersampling"));
   sampleButton2_ = new QRadioButton(tr("Regular Sampling"));
   sampleButton3_ = new QRadioButton(tr("Oversampling"));
   connect(sampleButton1_, SIGNAL(toggled(bool)), this, SLOT(samplingChanged1(bool)));
   connect(sampleButton2_, SIGNAL(toggled(bool)), this, SLOT(samplingChanged2(bool)));
   connect(sampleButton3_, SIGNAL(toggled(bool)), this, SLOT(samplingChanged3(bool)));
   h4->addWidget(sampleButton1_);
   gb2->addButton(sampleButton1_);
   h4->addWidget(sampleButton2_);
   gb2->addButton(sampleButton2_);
   h4->addWidget(sampleButton3_);
   gb2->addButton(sampleButton3_);
   sampleButton2_->setChecked(true);
   l3->addLayout(h1);
   l3->addLayout(h2);
   l3->addLayout(h3);
   l3->addLayout(h4);
   sliderLayout_->addLayout(l3);

   // create tilt section
   QVBoxLayout *l4 = new QVBoxLayout;
   l4->addWidget(s4);
   QLabel *ll4=new QLabel("Tilt");
   ll4->setAlignment(Qt::AlignHCenter);
   l4->addWidget(ll4);
   sliderLayout_->addLayout(l4);

   QFrame* line1 = new QFrame();
   line1->setFrameShape(QFrame::VLine);
   line1->setFrameShadow(QFrame::Raised);
   sliderLayout_->addWidget(line1);

   // create emission section
   QVBoxLayout *l5 = new QVBoxLayout;
   l5->addWidget(s5);
   QLabel *ll5=new QLabel("Emission");
   ll5->setAlignment(Qt::AlignHCenter);
   l5->addWidget(ll5);
   sliderLayout_->addLayout(l5);

   // create absorption section
   QVBoxLayout *l6 = new QVBoxLayout;
   l6->addWidget(s6);
   QLabel *ll6=new QLabel("Absorption");
   ll6->setAlignment(Qt::AlignHCenter);
   l6->addWidget(ll6);
   sliderLayout_->addLayout(l6);

   QFrame* line2 = new QFrame();
   line2->setFrameShape(QFrame::VLine);
   line2->setFrameShadow(QFrame::Raised);
   sliderLayout_->addWidget(line2);

   // create iso surface section
   QVBoxLayout *l7 = new QVBoxLayout;
   QPushButton *isoButton = new QPushButton(tr("Extract"));
   connect(isoButton, SIGNAL(pressed()), this, SLOT(extractSurface()));
   l7->addWidget(isoButton);
   QPushButton *isoClearButton = new QPushButton(tr("Clear"));
   connect(isoClearButton, SIGNAL(pressed()), this, SLOT(clearSurface()));
   l7->addWidget(isoClearButton);
   showIsoCheck_ = new QCheckBox(tr("Show"));
   showIsoCheck_->setChecked(true);
   connect(showIsoCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkShowIso(int)));
   l7->addWidget(showIsoCheck_);
   clipIsoCheck_ = new QCheckBox(tr("Clip Plane"));
   clipIsoCheck_->setChecked(true);
   connect(clipIsoCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkClipIso(int)));
   l7->addWidget(clipIsoCheck_);
   QLabel *ll7=new QLabel("Iso Surface");
   ll7->setAlignment(Qt::AlignHCenter);
   l7->addStretch(100);
   l7->addWidget(ll7);
   sliderLayout_->addLayout(l7);

   // add volren widget
   viewerSplitter_->addWidget(vrw_);

   // add interaction tool box
   QVBoxLayout *l8 = new QVBoxLayout;
   modeButton1_ = new QRadioButton(tr("Window"));
   modeButton2_ = new QRadioButton(tr("Move"));
   modeButton3_ = new QRadioButton(tr("Rotate"));
   modeButton4_ = new QRadioButton(tr("Zoom"));
   modeButton5_ = new QRadioButton(tr("Clip"));
   modeButton6_ = new QRadioButton(tr("Opacity"));
   connect(modeButton1_, SIGNAL(toggled(bool)), this, SLOT(modeChanged1(bool)));
   connect(modeButton2_, SIGNAL(toggled(bool)), this, SLOT(modeChanged2(bool)));
   connect(modeButton3_, SIGNAL(toggled(bool)), this, SLOT(modeChanged3(bool)));
   connect(modeButton4_, SIGNAL(toggled(bool)), this, SLOT(modeChanged4(bool)));
   connect(modeButton5_, SIGNAL(toggled(bool)), this, SLOT(modeChanged5(bool)));
   connect(modeButton6_, SIGNAL(toggled(bool)), this, SLOT(modeChanged6(bool)));
   l8->addWidget(modeButton1_);
   l8->addStretch(1);
   l8->addWidget(modeButton2_);
   l8->addStretch(1);
   l8->addWidget(modeButton3_);
   l8->addStretch(1);
   l8->addWidget(modeButton4_);
   l8->addStretch(1);
   l8->addWidget(modeButton5_);
   l8->addStretch(1);
   l8->addWidget(modeButton6_);
   l8->addStretch(1);
   QLabel *ll8=new QLabel("Interactions");
   ll8->setAlignment(Qt::AlignLeft);
   l8->addWidget(ll8);
   modeButton1_->setChecked(true);
   resetButton_ = new QPushButton(tr("Reset"));
   connect(resetButton_, SIGNAL(pressed()), this, SLOT(resetInteractions()));
   l8->addWidget(resetButton_);
   QGroupBox *g2 = new QGroupBox;
   g2->setLayout(l8);
   viewerSplitter_->addWidget(g2);

   // collapse slider widgets
   QList<int> mainSizes = mainSplitter_->sizes();
   mainSizes[0]=1;
   mainSizes[1]=0;
   mainSplitter_->setSizes(mainSizes);
   mainSplitter_->refresh();

   setCentralWidget(mainGroup);
}

void QTV3MainWindow::createDocks()
{
   if (prefs_) delete prefs_;

   prefs_ = new QTV3PrefWindow(this, vrw_, vrw_stereo_);
   prefs_->setAllowedAreas(Qt::RightDockWidgetArea);

   addDockWidget(Qt::RightDockWidgetArea, prefs_);
   prefs_->hide();
}

void QTV3MainWindow::reset()
{
   vrw_->resetInteractions();
   vrw_->loadVolume("Drop.pvm","/usr/share/qtv3/");
   vrw_->clearSurface();

   vrw_->setRotation(30.0);

   hasTeaserVolume_=true;

   flipXY1_=flipXY2_=0;
   flipYZ1_=flipYZ2_=0;

   clipNum_=0;

   clipSlider_->setValue(16*0);
   zoomSlider_->setValue(16*0);
   rotSlider_->setValue(16*0);
   tiltSlider_->setValue(16*0);
   emiSlider_->setValue(16*25);
   attSlider_->setValue(16*25);

   gradMagCheck_->setChecked(false);
   invModeCheck_->setChecked(false);
   rotateCheck_->setChecked(true);
   reverseCheck_->setChecked(false);
   planeCheck_->setChecked(false);
   showIsoCheck_->setChecked(true);
   clipIsoCheck_->setChecked(true);
   flipCheckXY1_->setChecked(false);
   flipCheckXY2_->setChecked(false);
   flipCheckYZ1_->setChecked(false);
   flipCheckYZ2_->setChecked(false);
   sampleButton2_->setChecked(true);

   createDocks();

   prefs_->setLabelFileName("teaser text");
}

QStringList QTV3MainWindow::browse(QString path,
                                   bool newfile)
{
   QFileDialog* fd = new QFileDialog(this, "File Selector Dialog");

   if (!newfile) fd->setFileMode(QFileDialog::ExistingFiles);
   else fd->setFileMode(QFileDialog::AnyFile);
   fd->setViewMode(QFileDialog::List);
   if (newfile) fd->setAcceptMode(QFileDialog::AcceptSave);
#ifdef HAVE_QT5
   fd->setFilter(QDir::Dirs|QDir::Files);
   fd->setNameFilter("All Files (*);;PVM Files(*.pvm);;DICOM Files(*.dcm *.ima);;REK Files(*.rek);;RAW Files(*.raw);;GEO Files(*.geo)");
#else
   fd->setFilter("All Files (*);;PVM Files(*.pvm);;DICOM Files(*.dcm *.ima);;REK Files(*.rek);;RAW Files(*.raw);;GEO Files(*.geo)");
#endif

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
   return(QSize(MAIN_WIDTH, MAIN_HEIGHT));
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
            url = normalizeFile(url);

            if (url.endsWith(".geo"))
            {
               loadSurface(url.toStdString().c_str());
            }
            else
            {
               loadVolume(url.toStdString().c_str());
            }
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
               list.push_back(normalizeFile(url).toStdString());
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

QString QTV3MainWindow::normalizeFile(QString file)
{
   file.remove("file://");
   file.replace('\\', '/');
   if (file.contains(QRegExp("^/[A-Z]:"))) file.remove(0,1);

   return(file);
}

void QTV3MainWindow::open()
{
   QStringList files = browse();

   if (files.size()>0)
      if (files.size()==1)
      {
         QString file = normalizeFile(files.at(0));

         if (file.endsWith(".geo"))
         {
            loadSurface(file.toStdString().c_str());
         }
         else
         {
            loadVolume(file.toStdString().c_str());
         }
      }
      else
      {
         std::vector<std::string> list;

         for (int i=0; i<files.size(); i++)
         {
            list.push_back(normalizeFile(files.at(i)).toStdString());
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

   rotateCheck_->setChecked(false);
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

void QTV3MainWindow::tack()
{
   double px,py,pz;
   double nx,ny,nz;

   vrw_->getNearPlane(px,py,pz, nx,ny,nz);
   vrw_->setClipPlane(clipNum_, px,py,pz, nx,ny,nz);
   vrw_->enableClipPlane(clipNum_,1);

   clipSlider_->setValue(0);

   clipNum_++;
   if (clipNum_>=6) clear();
}

void QTV3MainWindow::clear()
{
   vrw_->disableClipPlanes();

   clipNum_=0;

   clipSlider_->setValue(0);
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

void QTV3MainWindow::checkGradMag(int on)
{
   vrw_->setGradMag(on);
   emiSlider_->setValue(16*100*vrw_->getEmission());
   attSlider_->setValue(16*100*vrw_->getAbsorption());
}

void QTV3MainWindow::checkInvMode(int on)
{
   vrw_->setInvMode(on);
}

void QTV3MainWindow::checkRotate(int on)
{
   rotSlider_->setValue(vrw_->getAngle() * 16);

   if (on)
      if (!reverseCheck_->isChecked())
         vrw_->setRotation(30);
      else
         vrw_->setRotation(-30);
   else
      vrw_->setRotation(0);
}

void QTV3MainWindow::checkReverse(int on)
{
   if (rotateCheck_->isChecked())
      if (!on)
         vrw_->setRotation(30);
      else
         vrw_->setRotation(-30);
}

void QTV3MainWindow::checkPlane(int on)
{
   vrw_->setClipOpacity(on);
}

void QTV3MainWindow::checkShowIso(int on)
{
   vrw_->showSurface(on);
}

void QTV3MainWindow::checkClipIso(int on)
{
   vrw_->enableGeoClip(on);
}

void QTV3MainWindow::checkSFX(bool stereo)
{
   if (stereo!=vrw_stereo_)
   {
      delete vrw_;
      vrw_ = new QTV3VolRenWidget(viewerSplitter_, stereo);
      connect(vrw_, SIGNAL(update_signal(QString)), this, SLOT(update_slot(QString)));
      viewerSplitter_->addWidget(vrw_);

      vrw_stereo_ = stereo;

      reset();
   }
}

void QTV3MainWindow::checkSFXoff(bool on)
{
   checkSFX(false);

   if (on)
      vrw_->setSFX(false);
}

void QTV3MainWindow::checkAnaMode(bool on)
{
   checkSFX(false);

   if (on)
   {
      vrw_->setSFX(true);
      vrw_->setAnaglyph(on);
   }
}

void QTV3MainWindow::checkSFXon(bool on)
{
   checkSFX(true);

   if (on)
   {
      vrw_->setSFX(true);
      vrw_->setAnaglyph(false);
   }
}

void QTV3MainWindow::setTilt()
{
   double tiltXY=0.0;
   double tiltYZ=0.0;

   if (flipXY1_) tiltXY+=90.0;
   if (flipXY2_) tiltXY-=90.0;

   if (flipYZ1_) tiltYZ+=90.0;
   if (flipYZ2_) tiltYZ-=90.0;

   if (fabs(tiltXY)>0.0 && fabs(tiltYZ)>0.0)
   {
      vrw_->setTiltXY(180.0);
      vrw_->setTiltYZ(0.0);
   }
   else
   {
      vrw_->setTiltXY(tiltXY);
      vrw_->setTiltYZ(tiltYZ);
   }
}

void QTV3MainWindow::checkFlipXY1(int on)
{
   flipXY1_=on;
   setTilt();
}

void QTV3MainWindow::checkFlipXY2(int on)
{
   flipXY2_=on;
   setTilt();
}

void QTV3MainWindow::checkFlipYZ1(int on)
{
   flipYZ1_=on;
   setTilt();
}

void QTV3MainWindow::checkFlipYZ2(int on)
{
   flipYZ2_=on;
   setTilt();
}

void QTV3MainWindow::samplingChanged1(bool on)
{
   if (on)
      vrw_->setOversampling(0.5);
}

void QTV3MainWindow::samplingChanged2(bool on)
{
   if (on)
      vrw_->setOversampling(1.0);
}

void QTV3MainWindow::samplingChanged3(bool on)
{
   if (on)
      vrw_->setOversampling(2.0);
}

void QTV3MainWindow::modeChanged1(bool on)
{
   if (on)
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Window);
}

void QTV3MainWindow::modeChanged2(bool on)
{
   if (on)
   {
      setRotation(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Move);
   }
}

void QTV3MainWindow::modeChanged3(bool on)
{
   if (on)
   {
      setRotation(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Rotate);
   }
}

void QTV3MainWindow::modeChanged4(bool on)
{
   if (on)
   {
      setRotation(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Zoom);
   }
}

void QTV3MainWindow::modeChanged5(bool on)
{
   if (on)
   {
      setRotation(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Clip);
      planeCheck_->setChecked(true);
   }
}

void QTV3MainWindow::modeChanged6(bool on)
{
   if (on)
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Opacity);
}

void QTV3MainWindow::resetInteractions()
{
   vrw_->resetInteractions();
}

void QTV3MainWindow::extractSurface()
{
   if (!hasTeaserVolume_)
      vrw_->extractSurface();
}

void QTV3MainWindow::clearSurface()
{
   vrw_->clearSurface();
}

void QTV3MainWindow::prefs()
{
   prefs_->show();
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

void QTV3MainWindow::updating_slot()
{
   prefs_->setLabelDim(0,0,0);
   prefs_->setLabelVoxel(0,0,0);
}

void QTV3MainWindow::update_slot(QString text)
{
   update_->setText(text);
}

void QTV3MainWindow::updated_slot()
{
   if (prefs_)
   {
      prefs_->setLabelDim(vrw_->getVR()->getdimx(),vrw_->getVR()->getdimy(),vrw_->getVR()->getdimz());
      prefs_->setLabelVoxel(vrw_->getVR()->getvoxelx(),vrw_->getVR()->getvoxely(),vrw_->getVR()->getvoxelz());
   }

   double dist = 0.5*(1-vrw_->getClipDist());
   clipSlider_->setValue(dist*100*16);

   emiSlider_->setValue(16*100*vrw_->getEmission());
   attSlider_->setValue(16*100*vrw_->getAbsorption());
}
