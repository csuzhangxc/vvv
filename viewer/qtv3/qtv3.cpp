#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include "volren_qgl.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   if (!QGLFormat::hasOpenGL()) return(1);

   QGLVolRenWidget vr;
   vr.show();

   return(app.exec());
}
