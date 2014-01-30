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

   main.show();

   return(app.exec());
}
