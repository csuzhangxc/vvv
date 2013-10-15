// (c) by Stefan Roettger, licensed under GPL 2+

#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include "volren_qgl.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   if (!QGLFormat::hasOpenGL()) return(1);

   QTV3MainWindow main;

   QStringList args = QCoreApplication::arguments();

   if (args.size()==2)
   {
      QString file=args[1];

      if (file.endsWith(".geo"))
      {
         main.loadSurface(file.toStdString().c_str());
      }
      else
      {
         main.loadVolume(file.toStdString().c_str());
      }
   }
   else if (args.size()>2)
   {
      std::vector<std::string> list;
      for (unsigned int i=1; i<list.size(); i++) list.push_back(args[i].toStdString());
      main.loadSeries(list);
   }

   QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

   if (env.contains("QTV3_VOL_LIMIT"))
      VOL_TARGET_CELLS=env.value("QTV3_VOL_LIMIT").toUInt()*1000000;

   if (env.contains("QTV3_ISO_LIMIT"))
      ISO_TARGET_CELLS=env.value("QTV3_ISO_LIMIT").toUInt()*1000000;

   main.show();

   return(app.exec());
}
