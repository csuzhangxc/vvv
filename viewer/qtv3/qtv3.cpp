#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include "volren_qgl.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   if (!QGLFormat::hasOpenGL()) return(1);

   QTV3MainWindow main;
   main.show();

   main.loadvolume("Bucky.pvm");

   return(app.exec());
}
