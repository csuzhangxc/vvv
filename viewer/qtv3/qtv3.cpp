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

   if (args.size()>1)
      main.loadvolume(args[1].toStdString().c_str());
   else
      main.loadvolume("Drop.pvm");

   main.setrotation(30.0);
   main.show();

   return(app.exec());
}
