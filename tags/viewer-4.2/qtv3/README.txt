! The QTV3 - Volume Rendering with Qt
Copyright (c) 2013-2014 by Stefan Roettger.

The QTV3 is a Qt application that displays volume data like CT or MRI
scans. It is free software licensed under the GPL.

Tested platforms are:
* MacOS X 10.5 and 10.6
* Ubuntu 12.10
* OpenSuSe 11.4
* Windows XP and Windows 7
Other platforms may work, but are untested.

!! Prerequisites

The compilation of the QTV3 requires the installation of:
 tcsh, autotools and cmake (unix)
 gnu/c++ (unix) or MSVC compiler (windows)
 svn and git (unix) or Tortoise SVN (windows)
 OpenGL (and GLUT)
 qt/qmake

The installation of OpenGL and GLUT is vendor specific: On MacOS X it
is already installed with the XCode development package, on Linux it
comes with the "mesa-dev", "X11-dev" and "free-glut3-dev" development
packages whereas on Windows it is usually installed with the MSVC IDE.

On MacOS X and Windows, it is recommended to build and install Qt from source!
On Linux, it is mostly sufficient to install a recent Qt binary package.

If you install Qt from source, grab the source tar ball from:
 Qt4.7: ftp://ftp.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.4.tar.gz
 Qt5.2: http://download.qt-project.org/official_releases/qt/5.2/5.2.1/single/qt-everywhere-opensource-src-5.2.1.tar.gz

In the following we restrict ourselves to describe the installation
process on Unix from source!

!! Qt Installation

Type on the unix console in your Qt source directory:
 ./configure -opengl -release -nomake examples -nomake demos -nomake tests -opensource -confirm-license && make && sudo make install

After the build process has finished (go get yourself a cup of coffee),
you will be asked to enter your root password for installation of Qt.

!! QTV3 Checkout and Compilation

Type on the unix console in your project directory:
 svn co http://vvv.googlecode.com/svn/vvv/viewer vvv

If you installed Qt5, be sure to enable the BUILD_WITH_QT5 option:
 (cmake .; cd qtv3; cmake -DBUILD_WITH_QT5 . && make)

Now we compile and install the QTV3:
 ./install.sh

!! Usage

After installation, the viewer is available as desktop application. On
Ubuntu, for example, you can simply search for it in the start menu.

!! Example Data

To get started, you can drag&drop the Bucky.pvm volume into the viewer
window. Everything else is pretty much self-explanatory.

That's it!

!! Postcard

If you found the software useful, please send a vacation postcard to:

 Prof. Dr. Stefan Roettger
 Wassertorstr. 10
 90489 Nuernberg
 Germany

Thanks!
