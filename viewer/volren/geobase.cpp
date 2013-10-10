// (c) by Stefan Roettger, licensed under GPL 2+

#include "geobase.h"

#ifdef HAVE_MINI
#include <mini/minibase.h>
#include <mini/ministrip.h>
#endif

Surface::Surface()
   {
#ifdef HAVE_MINI
   strip_=new ministrip();
#endif
   }

Surface::~Surface()
   {
#ifdef HAVE_MINI
   delete strip_;
#endif
   }

int Surface::readGEOfile(const char *filename)
   {
#ifdef HAVE_MINI
   return(strip_->readGEOfile(filename));
#endif
   }

void Surface::setmatrix(double mtx[16])
   {
#ifdef HAVE_MINI
   strip_->setmatrix(mtx);
#endif
   }

void Surface::render()
   {
#ifdef HAVE_MINI
   ministrip::setglobal_shade(TRUE);
   strip_->render();
#endif
   }
