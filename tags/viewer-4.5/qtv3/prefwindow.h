// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef PREFWINDOW_H
#define PREFWINDOW_H

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

class QGLVolRenWidget;

class QTV3PrefWindow: public QDockWidget
{
   Q_OBJECT; // Qt Metacall object for signal/slot connections

public:

   //! default ctor
   QTV3PrefWindow(QWidget *parent, QGLVolRenWidget *vrw, bool vrw_stereo);

   //! dtor
   ~QTV3PrefWindow();

   //! set label about actual filename
   void setLabelFileName(QString fname);

   //! set label about actual volume dimension
   void setLabelDim(long long sx,long long sy,long long sz);

   //! set label about actual voxel size
   void setLabelVoxel(float dx,float dy,float dz);

   void volMaxSizeChange(unsigned int vol_maxsize);
   void isoMaxSizeChange(unsigned int iso_maxsize);
   void colorHueChange(float hue);

   long long vol_maxsize_;
   long long iso_maxsize_;

   float border_ratio_;

   float slice_opacity_;
   float slice_opacity2_;

   float vol_hue_;

   QString shotname_;

protected:

   QGLVolRenWidget *vrw_;
   bool vrw_stereo_;

   QScrollArea *group_;
   QWidget *container_;
   QVBoxLayout *layout_;

   QLabel *label_filename_;
   QLabel *label_dim_;
   QLabel *label_voxel_;

   QLineEdit *lineEdit_vol_maxsize_;
   QLineEdit *lineEdit_gfx_maxsize_;
   QLineEdit *lineEdit_iso_maxsize_;

   QRadioButton *sfxOffCheck_;
   QRadioButton *anaModeCheck_;
   QRadioButton *sfxOnCheck_;

   QLineEdit *lineEdit_slice_opacity_;
   QSlider *slice_opacity_slider_;

   QLineEdit *lineEdit_slice_opacity2_;
   QSlider *slice_opacity_slider2_;

   QLineEdit *lineEdit_hue_;
   QSlider *hue_slider_;

   void createWidgets();

   QGroupBox *createEdit(QString name, QString value,
                         QLineEdit **lineEdit);

   QSlider *createSlider(int minimum, int maximum, int value);

   bool checkFormat(QString format);

protected slots:

   void volMaxSizeChange(QString);
   void volGfxSizeChange(QString);
   void isoMaxSizeChange(QString);

   void sliceOpacityChange(QString);
   void sliceOpacityChange(int);

   void sliceOpacityChange2(QString);
   void sliceOpacityChange2(int);

   void hueChange(QString);
   void hueChange(int);

public slots:

   bool grab(QString format="png");
};

#endif
