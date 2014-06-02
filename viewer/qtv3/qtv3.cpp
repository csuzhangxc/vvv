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
   bool fullscreen=false;
   bool gradmag=false;
   bool anaglyph=false;
   bool stereo=false;
   double maxidle=0;

   // scan option list
   for (int i=0; i<opt.size(); i++)
      if (opt[i]=="demo") demo=true;
      else if (opt[i]=="fullscreen") fullscreen=true;
      else if (opt[i]=="gradmag") gradmag=true;
      else if (opt[i]=="anaglyph") anaglyph=true;
      else if (opt[i]=="stereo") stereo=true;
      else if (opt[i].startsWith("maxidle="))
         {
         QString o=opt[i].mid(8);
         maxidle=o.toDouble();
         }

   QTV3MainWindow main(NULL, stereo, demo);

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

   if (anaglyph) main.setAnaglyph();
   if (gradmag) main.setGradMag();
   if (maxidle>0) main.setMaxIdle(maxidle);

   return(app.exec());
}
