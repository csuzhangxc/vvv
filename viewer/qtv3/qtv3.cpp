// (c) by Stefan Roettger, licensed under GPL 2+

#ifdef HAVE_QT5
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#endif

#include "volren_qgl.h"
#include "mainwindow.h"

double get_opt(QString o)
{
   o=o.mid(o.indexOf("=")+1);
   return(o.toDouble());
}

void usage(const char *prog)
{
   QString name(prog);
   std::cout << "usage:" << std::endl;
   std::cout << " " << name.mid(name.lastIndexOf("/")+1).toStdString() << " {options} [volume | series]" << std::endl;
   std::cout << "where options are:" << std::endl;
   std::cout << " --demo: demo gui" << std::endl;
   std::cout << " --touch: demo gui with touch sliders" << std::endl;
   std::cout << " --omega=x: auto-rotation speed (degrees/s)" << std::endl;
   std::cout << " --angle=x: rotation angle (degrees)" << std::endl;
   std::cout << " --tilt=x: tilt angle (degrees)" << std::endl;
   std::cout << " --tiltXY=x: XY rotation angle (degrees)" << std::endl;
   std::cout << " --tiltYZ=x: YZ rotation angle (degrees)" << std::endl;
   std::cout << " --tilt-delta=x: animated tilt angle (degrees)" << std::endl;
   std::cout << " --tilt-duration=x: animated tilt duration (s)" << std::endl;
   std::cout << " --clip=x: clip (percent)" << std::endl;
   std::cout << " --zoom=x: zoom (percent)" << std::endl;
   std::cout << " --zoom-factor=x: animated zoom factor (percent)" << std::endl;
   std::cout << " --zoom-duration=x: animated zoom duration (s)" << std::endl;
   std::cout << " --fullscreen: use full screen rendering mode" << std::endl;
   std::cout << " --tfcenter=x: center of the linear transfer function window (0-1)" << std::endl;
   std::cout << " --tfsize=x: size of the linear transfer function window (0-1)" << std::endl;
   std::cout << " --tfemi=x: global emission (percent)" << std::endl;
   std::cout << " --tfatt=x: global attenuation (percent)" << std::endl;
   std::cout << " --hue: color hue in degrees (0-360)" << std::endl;
   std::cout << " --gradmag: use gradient magnitude rendering mode" << std::endl;
   std::cout << " --anaglyph: use anaglyph stereo rendering mode" << std::endl;
   std::cout << " --stereo: use quad buffered stereo rendering mode" << std::endl;
   std::cout << " --interlace-horizontal: use left-right interlaced stereo rendering mode" << std::endl;
   std::cout << " --interlace-vertical: use bottom-top interlaced stereo rendering mode" << std::endl;
   std::cout << " --interlace-vertical-inverse: use top-bottom interlaced stereo rendering mode" << std::endl;
   std::cout << " --stereo-base=x: stereoscopic base (percent of default setting)" << std::endl;
   std::cout << " --stereo-focus=x: stereoscopic focus (percent of default setting)" << std::endl;
   std::cout << " --over: use over-sampling" << std::endl;
   std::cout << " --under: use under-sampling" << std::endl;
   std::cout << " --isovalue=x: extracted iso surface value (0-1)" << std::endl;
   std::cout << " --maxidle=x: maximum idle time before gui reset is x seconds" << std::endl;
   std::cout << " --maxvol=x: maximum edge size of volume to be loaded (default=512)" << std::endl;
   std::cout << " --maxiso=x: maximum edge size of volume to be extracted (default=256)" << std::endl;
   std::cout << " --help: this help text" << std::endl;
   std::cout << "where volume is:" << std::endl;
   std::cout << " a single .pvm or .rek volume file" << std::endl;
   std::cout << "where series is:" << std::endl;
   std::cout << " a series of DICOM .dcm or .imd image files" << std::endl;
   std::cout << "example:" << std::endl;
   std::cout << " ./qtv3 --demo --fullscreen --zoom=50 --maxidle=60 Bucky.pvm" << std::endl;
   exit(1);
}

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   if (!QGLFormat::hasOpenGL()) return(1);

   setlocale(LC_NUMERIC, "C");

   // get argument list
   QStringList args = QCoreApplication::arguments();

   // scan for arguments and options
   QStringList arg,opt;
   for (int i=1; i<args.size(); i++)
      if (args[i].startsWith("--")) opt.push_back(args[i].mid(2));
      else if (args[i].startsWith("-")) opt.push_back(args[i].mid(1));
      else arg.push_back(args[i]);

   bool demo=false;
   bool touch=false;
   double omega=30;
   double angle=0;
   double tilt=0;
   double tiltXY=0;
   double tiltYZ=0;
   double tiltDelta=0;
   double tiltDuration=0;
   double clip=0;
   double zoom=0;
   double zoomFactor=0;
   double zoomDuration=0;
   bool fullscreen=false;
   double tfcenter=0.5;
   double tfsize=1.0;
   bool tfinv=true;
   double tfemi=100;
   double tfatt=100;
   double hue=360.0;
   bool gradmag=false;
   bool anaglyph=false;
   bool stereo=false;
   int sfxmode=0;
   float sfxbase=1.0f;
   float sfxfocus=1.0f;
   bool over=false;
   bool under=false;
   double isovalue=0;
   double maxidle=0;
   unsigned int maxvol=0;
   unsigned int maxiso=0;

   // scan option list
   for (int i=0; i<opt.size(); i++)
      if (opt[i]=="demo") demo=true;
      else if (opt[i]=="touch") touch=true;
      else if (opt[i].startsWith("omega=")) omega=get_opt(opt[i]);
      else if (opt[i].startsWith("angle=")) angle=get_opt(opt[i]);
      else if (opt[i].startsWith("tilt=")) tilt=get_opt(opt[i]);
      else if (opt[i].startsWith("tiltXY=")) tiltXY=get_opt(opt[i]);
      else if (opt[i].startsWith("tiltYZ=")) tiltYZ=get_opt(opt[i]);
      else if (opt[i].startsWith("tilt-delta=")) tiltDelta=get_opt(opt[i]);
      else if (opt[i].startsWith("tilt-duration=")) tiltDuration=get_opt(opt[i]);
      else if (opt[i].startsWith("clip=")) clip=get_opt(opt[i]);
      else if (opt[i].startsWith("zoom=")) zoom=get_opt(opt[i]);
      else if (opt[i].startsWith("zoom-factor=")) zoomFactor=get_opt(opt[i])/100;
      else if (opt[i].startsWith("zoom-duration=")) zoomDuration=get_opt(opt[i]);
      else if (opt[i]=="fullscreen") fullscreen=true;
      else if (opt[i].startsWith("tfcenter=")) tfcenter=get_opt(opt[i]);
      else if (opt[i].startsWith("tfsize=")) tfsize=get_opt(opt[i]);
      else if (opt[i]=="tflin") tfinv=false;
      else if (opt[i]=="tfinv") tfinv=true;
      else if (opt[i].startsWith("tfemi=")) tfemi=get_opt(opt[i]);
      else if (opt[i].startsWith("tfatt=")) tfatt=get_opt(opt[i]);
      else if (opt[i].startsWith("hue=")) hue=get_opt(opt[i]);
      else if (opt[i]=="gradmag") gradmag=true;
      else if (opt[i]=="anaglyph") anaglyph=true;
      else if (opt[i]=="stereo") stereo=true;
      else if (opt[i]=="interlace-horizontal") sfxmode=1;
      else if (opt[i]=="interlace-vertical") sfxmode=3;
      else if (opt[i]=="interlace-vertical-inverse") sfxmode=4;
      else if (opt[i].startsWith("stereo-base=")) sfxbase=get_opt(opt[i])/100;
      else if (opt[i].startsWith("stereo-focus=")) sfxfocus=get_opt(opt[i])/100;
      else if (opt[i]=="over") {over=true; under=false;}
      else if (opt[i]=="under") {under=true; over=false;}
      else if (opt[i].startsWith("isovalue=")) isovalue=get_opt(opt[i]);
      else if (opt[i].startsWith("maxidle=")) maxidle=get_opt(opt[i]);
      else if (opt[i].startsWith("maxvol=")) maxvol=get_opt(opt[i]);
      else if (opt[i].startsWith("maxiso=")) maxiso=get_opt(opt[i]);
      else if (opt[i]=="help") usage(argv[0]);
      else usage(argv[0]);

   QTV3MainWindow main(NULL, stereo, demo, touch);

   if (arg.size()==1)
   {
      QString file=arg[0];

      if (file.endsWith(".geo"))
      {
         main.loadSurface(file.toStdString().c_str());
      }
      else
      {
         main.loadVolume(file.toStdString().c_str());
      }
   }
   else if (arg.size()>1)
   {
      std::vector<std::string> list;
      for (int i=0; i<arg.size(); i++) list.push_back(arg[i].toStdString());
      main.loadSeries(list);
   }

   if (fullscreen) main.showFullScreen();
   else main.show();

   main.setRotation(omega);
   if (omega==0.0) main.setAngle(angle);
   if (tilt!=0.0) main.setTilt(tilt);
   if (tiltXY!=0.0) main.setTiltXY(tiltXY);
   if (tiltYZ!=0.0) main.setTiltYZ(tiltYZ);
   if (tiltDelta!=0.0 && tiltDuration!=0.0) main.setAnimatedTilt(tiltDelta,360/tiltDuration);
   if (clip!=0.0) main.setClip(clip/100.0);
   if (zoom!=0.0) main.setZoom(zoom/100.0);
   if (zoomFactor!=0.0 && zoomDuration!=0.0) main.setAnimatedZoom(zoomFactor,1.0/zoomDuration);
   if (tfcenter!=0.5 || tfsize!=1.0) main.setTF(tfcenter,tfsize,tfinv);
   if (hue!=360.0) main.setColorHue(hue);
   if (gradmag) main.setGradMag();
   if (anaglyph) main.setAnaglyph();
   if (sfxmode!=0) main.setSFXmode(sfxmode);
   if (sfxbase!=1.0f || sfxfocus!=1.0f) main.setSFXparams(sfxbase,sfxfocus);
   if (tfemi!=100.0) main.setEmission(tfemi/100.0*main.getEmission());
   if (tfatt!=100.0) main.setAbsorption(tfatt/100.0*main.getAbsorption());
   if (over) main.setOversampling();
   if (under) main.setUndersampling();
   if (isovalue!=0.0) main.extractIsoSurface(isovalue);
   if (maxidle>0.0) main.setMaxIdle(maxidle);
   if (maxvol>0) main.volMaxSizeChange(maxvol);
   if (maxiso>0) main.isoMaxSizeChange(maxiso);

   return(app.exec());
}
