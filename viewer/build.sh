#!/bin/csh -f

# available rules:
# no rule -> same as all
# all     -> build all programs
# tools   -> build pvm tools
# dcmtk   -> build with dcmtk
# deps    -> make dependencies
# clean   -> remove object files
# tidy    -> clean up all files

# make command
set mmfile=V3Makefile
set make="make -f $mmfile"

set rule=$1
if ($rule == "") set rule="all"

set dcmtk=""
if ($rule == "dcmtk") then
   set dcmtk=" -DVIEWER_HAVE_DCMTK"
   set rule="all"
endif

if ($rule == "deps") then
   if ($HOSTTYPE == "iris4d") $make MAKEDEPEND="CC -M" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=IRIX depend
   if ($HOSTTYPE == "i386") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i386-linux") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i486") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i486-linux") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i586") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i586-linux") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i686") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "i686-linux") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "powerpc") $make MAKEDEPEND="c++ -M -I/usr/X11R6/include" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=LINUX depend
   if ($HOSTTYPE == "powermac") $make MAKEDEPEND="c++ -M" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=MACOSX depend
   if ($HOSTTYPE == "intel-pc") $make MAKEDEPEND="c++ -M" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=MACOSX depend
   if ($HOSTTYPE == "intel-mac") $make MAKEDEPEND="c++ -M" OPTS="-DHAVE_CONFIG_H$dcmtk" TARGET=MACOSX depend
else
   if ($HOSTTYPE == "iris4d") $make COMPILER="CC" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -mips3 -OPT:Olimit=0" LINK="-Wl,-woff84 -lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lX11 -lXm -lXt -lXmu" TARGET=IRIX $rule
   if ($HOSTTYPE == "i386") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i386-linux") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i486") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i486-linux") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i586") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i586-linux") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i686") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "i686-linux") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "powerpc") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -I/usr/X11R6/include" LINK="-lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lglut -lGLU -L/usr/X11R6/lib -lX11 -lXm -lXt -lXmu" TARGET=LINUX $rule
   if ($HOSTTYPE == "powermac") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3" LINK="-Wl,-w -lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework GLUT -lobjc" TARGET=MACOSX $rule
   if ($HOSTTYPE == "intel-pc") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -Wall -Wno-parentheses" LINK="-Wl,-w -lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework GLUT -lobjc" TARGET=MACOSX $rule
   if ($HOSTTYPE == "intel-mac") $make COMPILER="c++" OPTS="-DHAVE_CONFIG_H$dcmtk -O3 -Wall -Wno-parentheses" LINK="-Wl,-w -lz -lofstd -ldcmdata -ldcmtls -ldcmjpeg -lijg8 -lijg12 -lijg16 -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework GLUT -lobjc" TARGET=MACOSX $rule
endif
