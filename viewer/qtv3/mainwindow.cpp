// (c) by Stefan Roettger, licensed under GPL 2+

#include <iostream>

#include "mainwindow.h"
#include "mainconst.h"

QTV3MainWindow::QTV3MainWindow(QWidget *parent,
                               bool stereo,
                               bool demo)
   : QMainWindow(parent)
{
   vrw_stereo_ = stereo;
   demo_ = demo;

   createMenus();
   createWidgets();

   prefs_ = NULL;

   max_idle_time_ = 3600;

   // accept drag and drop
   setAcceptDrops(true);

   // install event filter
   installEventFilter(this);

   // install periodic timer
   connect(&idle_check_, SIGNAL(timeout()), this, SLOT(idle_check()));
   idle_check_.start(1000); // every second

   // set window title to app name and version
   setWindowTitle(APP_NAME" "APP_VERSION);

   // set defaults
   default_omega_=30.0;
   default_angle_=0.0;
   default_tilt_=0.0;
   default_tiltXY_=0.0;
   default_tiltYZ_=0.0;
   default_clip_=0.0;
   default_zoom_=0.0;
   default_tfcenter_=0.5;
   default_tfsize_=1.0;
   default_tfinverse_=false;
   default_tfemi_=1.0;
   default_tfatt_=1.0;
   default_hue_=120.0;
   default_gradmag_=false;
   default_anaglyph_=false;

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

   prefs_->setLabelFileName(filename);

   vrw_->loadVolume(filename);

   hasTeaserVolume_=false;
}

void QTV3MainWindow::loadSeries(const std::vector<std::string> list)
{
   std::string prefix;

   reset();

   prefix=getPrefix(list);

   std::string series = "dicom series (";
   series += prefix;
   series += ")";

   prefs_->setLabelFileName(series.c_str());

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
}

void QTV3MainWindow::setAngle(double angle)
{
   vrw_->setAngle(angle);

   rotateCheck_->setChecked(false);

   default_angle_=angle;
   default_omega_=0.0;
}

void QTV3MainWindow::setRotation(double omega)
{
   vrw_->setRotation(omega);

   rotateCheck_->setChecked(omega!=0.0);

   default_omega_=omega;
}

void QTV3MainWindow::setTilt(double tilt)
{
   vrw_->setTilt(tilt);

   default_tilt_=tilt;
}

void QTV3MainWindow::setTiltXY(double tiltXY)
{
   vrw_->setTiltXY(tiltXY);

   default_tiltXY_=tiltXY;
}

void QTV3MainWindow::setTiltYZ(double tiltYZ)
{
   vrw_->setTiltYZ(tiltYZ);

   default_tiltYZ_=tiltYZ;
}

void QTV3MainWindow::setClip(double clip)
{
   vrw_->setClipDist(1.0-2*clip);

   default_clip_=clip;
}

void QTV3MainWindow::setZoom(double zoom)
{
   vrw_->setZoom(zoom);

   default_zoom_=zoom;
}

void QTV3MainWindow::clearVolume()
{
   prefs_->setLabelFileName("");

   vrw_->clearVolume();

   hasTeaserVolume_=false;
}

void QTV3MainWindow::clearSurface()
{
   vrw_->clearSurface();
}

void QTV3MainWindow::setTF(float center,float size,
                           bool inverse)
{
   vrw_->set_tfunc(center,size,inverse);

   default_tfcenter_=center;
   default_tfsize_=size;
   default_tfinverse_=inverse;
}

void QTV3MainWindow::setEmission(float emi)
{
   vrw_->setEmission(emi);

   default_tfemi_=emi;
}

float QTV3MainWindow::getEmission()
{
   return(vrw_->getEmission());
}

void QTV3MainWindow::setAbsorption(float att)
{
   vrw_->setAbsorption(att);

   default_tfatt_=att;
}

float QTV3MainWindow::getAbsorption()
{
   return(vrw_->getAbsorption());
}

void QTV3MainWindow::setColorHue(float hue)
{
   if (prefs_)
      prefs_->colorHueChange(hue);

   default_hue_=hue;
}

void QTV3MainWindow::setOversampling()
{
   sampleButton3_->setChecked(true);
}

void QTV3MainWindow::setUndersampling()
{
   sampleButton1_->setChecked(true);
}

void QTV3MainWindow::setGradMag()
{
   checkGradMag(true);

   default_gradmag_=true;
}

void QTV3MainWindow::setAnaglyph()
{
   if (!vrw_stereo_)
      checkAnaMode(true);

   default_anaglyph_=true;
}

void QTV3MainWindow::setMaxIdle(double t)
{
   max_idle_time_ = t;
}

void QTV3MainWindow::volMaxSizeChange(unsigned int vol_maxsize)
{
   if (prefs_)
      prefs_->volMaxSizeChange(vol_maxsize);
}

void QTV3MainWindow::isoMaxSizeChange(unsigned int iso_maxsize)
{
   if (prefs_)
      prefs_->isoMaxSizeChange(iso_maxsize);
}

void QTV3MainWindow::grab()
{
   prefs_->grab();
}

void QTV3MainWindow::createMenus()
{
   QAction *quitAction = new QAction(tr("Q&uit"), this);
   quitAction->setShortcut(tr("Ctrl+Q"));
   quitAction->setStatusTip(tr("Quit the application"));
   connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

   QAction *openAction = new QAction(tr("O&pen"), this);
   openAction->setShortcut(tr("Ctrl+O"));
   openAction->setStatusTip(tr("Open Volume File"));
   connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

   QAction *prefAction = new QAction(tr("P&references"), this);
   prefAction->setShortcut(tr("Ctrl+P"));
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
   // create main group
   QGroupBox *mainGroup = new QGroupBox;
   mainGroup->setObjectName("mainGroupBox");
   mainLayout_ = new QVBoxLayout(mainGroup);

   // create main parts
   mainSplitter_ = new QSplitter;
   QGroupBox *viewerGroup = new QGroupBox;
   viewerGroup->setObjectName("viewerGroupBox");
   viewerLayout_ = new QVBoxLayout;
   viewerSplitter_ = new QSplitter;
   QGroupBox *sliderGroup = new QGroupBox;
   sliderLayout_ = new QHBoxLayout;

   // set main inherited style sheet
   QString css("QGroupBox { background-color: #eeeeee; border: 2px solid #999999; border-radius: 5px; }"
               "QGroupBox#mainGroupBox { border: 0; border-radius: 0; }"
               "QGroupBox#viewerGroupBox { background-color: #d9d9d9; }");
   if (demo_) css+="QGroupBox#viewerGroupBox { border: 0; border-radius: 0; }";
   mainSplitter_->setStyleSheet(css);

   // assemble main splitter group
   mainSplitter_->setOrientation(Qt::Vertical);
   mainSplitter_->addWidget(viewerGroup);
   if (!demo_) mainSplitter_->addWidget(sliderGroup);
   viewerGroup->setLayout(viewerLayout_);
   sliderGroup->setLayout(sliderLayout_);

   // assemble main group
   mainLayout_->addWidget(mainSplitter_);
   mainGroup->setLayout(mainLayout_);

   // assemble viewer group
   viewerLayout_->addWidget(viewerSplitter_);
   viewerSplitter_->setOrientation(Qt::Horizontal);

   // create qtv3 volren widget
   vrw_ = new QTV3VolRenWidget(viewerSplitter_,vrw_stereo_);

   // do not show cross in demo mode
   vrw_->showCross(!demo_);

   // create update info label
   update_ = new QLabel("");
   update_->setAlignment(Qt::AlignHCenter);
   connect(vrw_, SIGNAL(updating_signal()), this, SLOT(updating_slot()));
   connect(vrw_, SIGNAL(update_signal(QString)), this, SLOT(update_slot(QString)));
   connect(vrw_, SIGNAL(updated_signal()), this, SLOT(updated_slot()));
   connect(vrw_, SIGNAL(interaction_signal()), this, SLOT(interaction_slot()));
   connect(vrw_, SIGNAL(measuring_signal(double,double,double,double,double)), this, SLOT(measuring_slot(double,double,double,double,double)));
   if (!demo_) mainLayout_->addWidget(update_);

   // create sliders
   if (!demo_)
      {
      clipSlider_=createSlider(0,100,0,true); // clip
      zoomSlider_=createSlider(0,100,0,true); // zoom
      }
   else
      {
      clipDemoSlider_=createSwipeSlider(25,60,0,true,"Touch to Clip"); // clip
      zoomDemoSlider_=createSwipeSlider(20,90,0,true,"Touch to Zoom"); // zoom
      }
   rotSlider_=createSlider(-180,180,0,false); // rotate
   tiltSlider_=createSlider(-90,90,0,true); // tilt
   emiSlider_=createSlider(0,100,25,true); // emission
   attSlider_=createSlider(0,100,25,true); // absorption

   // connect sliders
   if (!demo_)
      {
      connect(clipSlider_, SIGNAL(sliderMoved(int)), this, SLOT(clip(int)));
      connect(zoomSlider_, SIGNAL(sliderMoved(int)), this, SLOT(zoom(int)));
      }
   else
      {
      connect(clipDemoSlider_, SIGNAL(valueChanged(double)), this, SLOT(clipDemo(double)));
      connect(zoomDemoSlider_, SIGNAL(valueChanged(double)), this, SLOT(zoomDemo(double)));
      }
   connect(rotSlider_, SIGNAL(sliderMoved(int)), this, SLOT(rotate(int)));
   connect(tiltSlider_, SIGNAL(sliderMoved(int)), this, SLOT(tilt(int)));
   connect(emiSlider_, SIGNAL(sliderMoved(int)), this, SLOT(emission(int)));
   connect(attSlider_, SIGNAL(sliderMoved(int)), this, SLOT(absorption(int)));

   // create clipping section
   QVBoxLayout *l1 = new QVBoxLayout;
   if (!demo_)
      {
      QLabel *ll1=new QLabel("Clipping");
      ll1->setAlignment(Qt::AlignLeft);
      l1->addWidget(ll1);
      }
   if (!demo_)
      l1->addWidget(clipSlider_);
   else
      l1->addWidget(clipDemoSlider_);
   QPushButton *tackButton = new QPushButton(tr("Tack"));
   connect(tackButton, SIGNAL(pressed()), this, SLOT(tack()));
   QPushButton *clearButton = new QPushButton(tr("Clear"));
   connect(clearButton, SIGNAL(pressed()), this, SLOT(clear()));
   if (!demo_) l1->addWidget(tackButton);
   if (!demo_) l1->addWidget(clearButton);
   planeCheck_ = new QCheckBox(tr("Plane View"));
   planeCheck_->setChecked(false);
   connect(planeCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkPlane(int)));
   if (!demo_) l1->addWidget(planeCheck_);
   gradMagCheck_ = new QCheckBox(tr("GradMag"));
   gradMagCheck_->setChecked(false);
   connect(gradMagCheck_, SIGNAL(stateChanged(int)), this, SLOT(checkGradMag(int)));
   if (!demo_) l1->addWidget(gradMagCheck_);

   // assemble clipping section
   QGroupBox *g1 = new QGroupBox;
   g1->setLayout(l1);
   viewerSplitter_->addWidget(g1);

   // create zoom section
   QVBoxLayout *l2 = new QVBoxLayout;
   if (!demo_)
      l2->addWidget(zoomSlider_);
   else
      l2->addWidget(zoomDemoSlider_);
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
   l3->addWidget(rotSlider_);
   l3->addStretch(1000);
   QHBoxLayout *h1 = new QHBoxLayout;
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
   if (vrw_stereo_) sfxOnCheck_->setChecked(true);
   else sfxOffCheck_->setChecked(true);
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
   QButtonGroup *gb2 = new QButtonGroup;
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
   l4->addWidget(tiltSlider_);
   QLabel *ll4=new QLabel("Tilt");
   ll4->setAlignment(Qt::AlignHCenter);
   l4->addWidget(ll4);
   sliderLayout_->addLayout(l4);

   // create separator line
   QFrame* line1 = new QFrame();
   line1->setFrameShape(QFrame::VLine);
   line1->setFrameShadow(QFrame::Raised);
   sliderLayout_->addWidget(line1);

   // create emission section
   QVBoxLayout *l5 = new QVBoxLayout;
   l5->addWidget(emiSlider_);
   QLabel *ll5=new QLabel("Emission");
   ll5->setAlignment(Qt::AlignHCenter);
   l5->addWidget(ll5);
   sliderLayout_->addLayout(l5);

   // create absorption section
   QVBoxLayout *l6 = new QVBoxLayout;
   l6->addWidget(attSlider_);
   QLabel *ll6=new QLabel("Absorption");
   ll6->setAlignment(Qt::AlignHCenter);
   l6->addWidget(ll6);
   sliderLayout_->addLayout(l6);

   // create separator line
   QFrame* line2 = new QFrame();
   line2->setFrameShape(QFrame::VLine);
   line2->setFrameShadow(QFrame::Raised);
   sliderLayout_->addWidget(line2);

   // create iso surface section
   QVBoxLayout *l7 = new QVBoxLayout;
   QPushButton *isoButton = new QPushButton(tr("Extract"));
   connect(isoButton, SIGNAL(pressed()), this, SLOT(extractIsoSurface()));
   l7->addWidget(isoButton);
   QPushButton *isoClearButton = new QPushButton(tr("Clear"));
   connect(isoClearButton, SIGNAL(pressed()), this, SLOT(clearIsoSurface()));
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
   QLabel *ll8=new QLabel("Interaction");
   ll8->setAlignment(Qt::AlignLeft);
   l8->addWidget(ll8);
   l8->addStretch(2);
   modeButton1_ = new QPushButton(tr("Window"));
   modeButton2_ = new QPushButton(tr("Move"));
   modeButton3_ = new QPushButton(tr("Roll"));
   modeButton4_ = new QPushButton(tr("Zoom"));
   modeButton5_ = new QPushButton(tr("Clip"));
   modeButton6_ = new QPushButton(tr("Clip+Roll"));
   modeButton7_ = new QPushButton(tr("Opacity"));
   modeButton8_ = new QPushButton(tr("Measure"));
   modeButton1_->setCheckable(true);
   modeButton2_->setCheckable(true);
   modeButton3_->setCheckable(true);
   modeButton4_->setCheckable(true);
   modeButton5_->setCheckable(true);
   modeButton6_->setCheckable(true);
   modeButton7_->setCheckable(true);
   modeButton8_->setCheckable(true);
   modeButton1_->setAutoExclusive(true);
   modeButton2_->setAutoExclusive(true);
   modeButton3_->setAutoExclusive(true);
   modeButton4_->setAutoExclusive(true);
   modeButton5_->setAutoExclusive(true);
   modeButton6_->setAutoExclusive(true);
   modeButton7_->setAutoExclusive(true);
   modeButton8_->setAutoExclusive(true);
   connect(modeButton1_, SIGNAL(clicked(bool)), this, SLOT(modeChanged1(bool)));
   connect(modeButton2_, SIGNAL(clicked(bool)), this, SLOT(modeChanged2(bool)));
   connect(modeButton3_, SIGNAL(clicked(bool)), this, SLOT(modeChanged3(bool)));
   connect(modeButton4_, SIGNAL(clicked(bool)), this, SLOT(modeChanged4(bool)));
   connect(modeButton5_, SIGNAL(clicked(bool)), this, SLOT(modeChanged5(bool)));
   connect(modeButton6_, SIGNAL(clicked(bool)), this, SLOT(modeChanged6(bool)));
   connect(modeButton7_, SIGNAL(clicked(bool)), this, SLOT(modeChanged7(bool)));
   connect(modeButton8_, SIGNAL(clicked(bool)), this, SLOT(modeChanged8(bool)));
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
   l8->addWidget(modeButton7_);
   l8->addStretch(1);
   l8->addWidget(modeButton8_);
   l8->addStretch(5);
   modeButton1_->setChecked(true);
   grabButton_ = new QPushButton(tr("Grab"));
   connect(grabButton_, SIGNAL(pressed()), this, SLOT(grabWindow()));
   l8->addWidget(grabButton_);
   resetButton_ = new QPushButton(tr("Reset"));
   connect(resetButton_, SIGNAL(pressed()), this, SLOT(resetInteractions()));
   l8->addWidget(resetButton_);
   QGroupBox *g2 = new QGroupBox;
   g2->setLayout(l8);
   if (!demo_) viewerSplitter_->addWidget(g2);

   // collapse slider widgets
   if (!demo_)
   {
      QList<int> mainSizes = mainSplitter_->sizes();
      mainSizes[0]=1;
      mainSizes[1]=0;
      mainSplitter_->setSizes(mainSizes);
      mainSplitter_->refresh();
   }

   // create demo widgets
   if (demo_)
   {
      // create zoom section
      QGroupBox *g = new QGroupBox;
      QVBoxLayout *l = new QVBoxLayout;
      l->addWidget(zoomDemoSlider_);
      g->setLayout(l);

      // assemble zoom section
      viewerSplitter_->addWidget(g);
   }

   // assemble central widget
   setMinimumSize(QSize(MAIN_WIDTH,MAIN_HEIGHT));
   if (!demo_) setCentralWidget(mainGroup);
   else setCentralWidget(mainSplitter_);
}

void QTV3MainWindow::createDocks()
{
   if (prefs_)
   {
      removeDockWidget(prefs_);
      delete prefs_;
   }

   prefs_ = new QTV3PrefWindow(this, vrw_, vrw_stereo_);
   prefs_->setAllowedAreas(Qt::RightDockWidgetArea);

   addDockWidget(Qt::RightDockWidgetArea, prefs_);
   prefs_->hide();
}

void QTV3MainWindow::reset(const char *teaser, const char *path)
{
   resetInteractions();

   if (teaser!=NULL)
   {
      vrw_->loadVolume(teaser,path);
      hasTeaserVolume_=true;
   }
   else
   {
      vrw_->clearVolume();
      hasTeaserVolume_=false;
   }

   vrw_->clearSurface();

   resetDefaults();

   flipXY1_=flipXY2_=0;
   flipYZ1_=flipYZ2_=0;

   clear();

   if (!demo_)
      {
      clipSlider_->setValue(16*0);
      zoomSlider_->setValue(16*0);
      }
   else
      {
      clipDemoSlider_->setValue(0);
      zoomDemoSlider_->setValue(0);
      }
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

   if (hasTeaserVolume_)
      prefs_->setLabelFileName("teaser");
}

std::string QTV3MainWindow::getPrefix(const std::vector<std::string> list)
{
   unsigned int i,j;

   bool finished=false;

   if (list.size()==0) return("");
   if (list.size()==1) return(list[0]);

   for (i=0; !finished; i++)
      for (j=1; j<list.size(); j++)
         if (list[j].size()<=i || list[j-1].size()<=i)
         {
            finished=true;
            break;
         }
         else
            if (list[j][i]!=list[j-1][i])
            {
               finished=true;
               break;
            }

   if (i==0) return("");

   return(list[0].substr(0,i-1));
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

   setFocus();

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

SwipeSlider *QTV3MainWindow::createSwipeSlider(int minimum, int maximum, int value,
                                               bool vertical, QString text)
{
   SwipeSlider *slider = new SwipeSlider(vertical?Qt::Vertical:Qt::Horizontal, text);
   slider->setRange(minimum, maximum);
   slider->setValue(value);
   return(slider);
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

void QTV3MainWindow::zoomDemo(double v)
{
   double zoom = v / 100.0;
   vrw_->setZoom(zoom);
}

void QTV3MainWindow::rotate(int v)
{
   double angle = v / 16.0;
   setAngle(angle);
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

void QTV3MainWindow::clipDemo(double v)
{
   double dist = v / 100.0;
   vrw_->setClipDist(1.0-2*dist);
}

void QTV3MainWindow::tack()
{
   double px,py,pz;
   double nx,ny,nz;

   vrw_->getNearPlane(px,py,pz, nx,ny,nz);
   vrw_->setClipPlane(clipNum_, px,py,pz, nx,ny,nz);
   vrw_->enableClipPlane(clipNum_,1);

   vrw_->setClipDist(1.0);

   clipNum_++;
   if (clipNum_>=6) clear();
}

void QTV3MainWindow::clear()
{
   vrw_->disableClipPlanes();

   clipNum_=0;

   vrw_->setClipDist(1.0);

   vrw_->clearLine();
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
   if (vrw_)
   {
      vrw_->setGradMag(on);
      gradMagCheck_->setChecked(on);
      emiSlider_->setValue(16*100*vrw_->getEmission());
      attSlider_->setValue(16*100*vrw_->getAbsorption());
   }
}

void QTV3MainWindow::checkInvMode(int on)
{
   vrw_->setInvMode(on);
}

void QTV3MainWindow::checkRotate(int on)
{
   if (on)
      if (!reverseCheck_->isChecked())
         vrw_->setRotation(default_omega_);
      else
         vrw_->setRotation(-default_omega_);
   else
      vrw_->setRotation(0);
}

void QTV3MainWindow::checkReverse(int on)
{
   if (rotateCheck_->isChecked())
      if (!on)
         vrw_->setRotation(default_omega_);
      else
         vrw_->setRotation(-default_omega_);
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
   if (vrw_)
   {
      checkSFX(false);

      if (on)
         vrw_->setSFX(false);
   }
}

void QTV3MainWindow::checkAnaMode(bool on)
{
   if (vrw_)
   {
      checkSFX(false);

      if (on)
      {
         vrw_->setSFX(true);
         vrw_->setAnaglyph(on);
      }
   }
}

void QTV3MainWindow::checkSFXon(bool on)
{
   if (vrw_)
   {
      checkSFX(true);

      if (on)
      {
         vrw_->setSFX(true);
         vrw_->setAnaglyph(false);
      }
   }
}

void QTV3MainWindow::checkTilt()
{
   double tiltXY=0.0;
   double tiltYZ=0.0;

   if (default_tiltXY_==0.0 && default_tiltYZ_==0.0)
   {
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
}

void QTV3MainWindow::checkFlipXY1(int on)
{
   flipXY1_=on;
   checkTilt();
}

void QTV3MainWindow::checkFlipXY2(int on)
{
   flipXY2_=on;
   checkTilt();
}

void QTV3MainWindow::checkFlipYZ1(int on)
{
   flipYZ1_=on;
   checkTilt();
}

void QTV3MainWindow::checkFlipYZ2(int on)
{
   flipYZ2_=on;
   checkTilt();
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
      disableRotation();
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Move);
   }
}

void QTV3MainWindow::modeChanged3(bool on)
{
   if (on)
   {
      disableRotation();
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_RotateCenter);
   }
}

void QTV3MainWindow::modeChanged4(bool on)
{
   if (on)
   {
      disableRotation();
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Zoom);
   }
}

void QTV3MainWindow::modeChanged5(bool on)
{
   if (on)
   {
      disableRotation();
      if (vrw_->getClipDist()>=1.0) vrw_->setClipDist(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Clip);
   }
}

void QTV3MainWindow::modeChanged6(bool on)
{
   if (on)
   {
      disableRotation();
      if (vrw_->getClipDist()>=1.0) vrw_->setClipDist(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_RotateAnchor);

      planeCheck_->setChecked(true);
   }
}

void QTV3MainWindow::modeChanged7(bool on)
{
   if (on)
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Opacity);
}

void QTV3MainWindow::modeChanged8(bool on)
{
   if (on)
   {
      disableRotation();
      if (vrw_->getClipDist()>=1.0) vrw_->setClipDist(0.0);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Measure);
      vrw_->clearLine();

      planeCheck_->setChecked(true);
   }
}

void QTV3MainWindow::grabWindow()
{
   grab();
}

void QTV3MainWindow::disableRotation()
{
   vrw_->setRotation(0.0);

   rotateCheck_->setChecked(false);
}

void QTV3MainWindow::resetInteractions()
{
   disableRotation();
   vrw_->resetInteractions();

   modeButton1_->setChecked(true);
   if (demo_)
   {
      modeButton3_->setChecked(true);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_RotateCenter);
   }
   else
   {
      modeButton1_->setChecked(true);
      vrw_->setInteractionMode(QGLVolRenWidget::InteractionMode_Window);
   }

   planeCheck_->setChecked(false);
}

void QTV3MainWindow::resetDefaults()
{
   setRotation(default_omega_);
   if (default_omega_==0.0) setAngle(default_angle_);
   setTilt(default_tilt_);
   setTiltXY(default_tiltXY_);
   setTiltYZ(default_tiltYZ_);
   setClip(default_clip_);
   setZoom(default_zoom_);

   if (default_tfcenter_!=0.5 || default_tfsize_!=1.0)
      setTF(default_tfcenter_,default_tfsize_,default_tfinverse_);

   setColorHue(default_hue_);

   if (default_gradmag_) setGradMag();
   if (default_anaglyph_) setAnaglyph();

   if (default_tfemi_!=1.0) setEmission(default_tfemi_);
   if (default_tfatt_!=1.0) setAbsorption(default_tfatt_);
}

void QTV3MainWindow::extractIsoSurface(float isovalue)
{
   if (!hasTeaserVolume_)
   {
      vrw_->extractSurfaceAfter(isovalue);
   }
}

void QTV3MainWindow::extractIsoSurface()
{
   if (!hasTeaserVolume_)
   {
      char *surface = vrw_->extractSurface();

      if (surface!=NULL)
         free(surface);
   }
}

void QTV3MainWindow::clearIsoSurface()
{
   vrw_->clearSurface();
}

void QTV3MainWindow::prefs()
{
   if (prefs_->isVisible())
      prefs_->hide();
   else
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
}

void QTV3MainWindow::interaction_slot()
{
   if (vrw_)
   {
      rotSlider_->setValue(vrw_->getAngle() * 16);
      tiltSlider_->setValue(vrw_->getTilt() * 16);

      if (!demo_)
         zoomSlider_->setValue(vrw_->getZoom() * 100 * 16);
      else
         zoomDemoSlider_->setValue(vrw_->getZoom() * 100);

      double dist = 0.5*(1-vrw_->getClipDist());
      if (!demo_)
         clipSlider_->setValue(dist * 100*16);
      else
         clipDemoSlider_->setValue(dist * 100);

      emiSlider_->setValue(vrw_->getEmission() * 100 * 16);
      attSlider_->setValue(vrw_->getAbsorption() * 100 * 16);
   }
}

void QTV3MainWindow::measuring_slot(double px,double py,double pz,double length,double endlength)
{
   update_->setText(QString("x/y/z in mm: ")+
                    QString::number(px*1E3)+" / "+
                    QString::number(py*1E3)+" / "+
                    QString::number(pz*1E3)+", "+
                    QString("line length: ")+
                    QString::number(length*1E3)+"mm, "+
                    QString("end to end length: ")+
                    QString::number(endlength*1E3)+"mm");
}

void QTV3MainWindow::idle_check()
{
   if (idle()>max_idle_time_)
   {
      resetInteractions();
      resetDefaults();

      last_event_.start();
   }
}
