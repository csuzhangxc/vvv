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
   is_shown_=TRUE;
   sfxmode_=0;
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
#else
   return(0);
#endif
   }

int Surface::has_geo()
   {
#ifdef HAVE_MINI
   return(!strip_->empty());
#else
   return(0);
#endif
   }

int Surface::has_geo_shown()
   {
#ifdef HAVE_MINI
   return(!strip_->empty() && is_shown_);
#else
   return(0);
#endif
   }

void Surface::clear()
   {
#ifdef HAVE_MINI
   strip_->clear();
#endif
   }

void Surface::show(int yes)
   {
#ifdef HAVE_MINI
   is_shown_=yes;
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
   ministrip::setglobal_invariant(TRUE);
   ministrip::setglobal_shade(TRUE);
   ministrip::setglobal_sfx(sfxmode_!=0);
   ministrip::setglobalsfxparams(sfxmode_);
   strip_->autodisableculling();
   if (is_shown_) strip_->render();
#endif
   }

void Surface::setSFXmode(int sfxmode)
   {sfxmode_=sfxmode;}

double Surface::getscale()
   {
#ifdef HAVE_MINI
   miniv3d bboxmin,bboxmax,bbox;

   strip_->getbbox(bboxmin,bboxmax);
   bbox=bboxmax-bboxmin;

   return(fmax(bbox.x,fmax(bbox.y,bbox.z)));
#else
   return(0.0);
#endif
   }
