// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef VOLREN_H
#define VOLREN_H

#include "volume.h"

// the volume renderer
class volren
   {
   public:

   // default constructor
   volren(char *base=NULL)
      {VOL=new mipmap(base);}

   // destructor
   ~volren()
      {delete VOL;}

   // default constructor
   mipmap *get_volume() {return(VOL);} // return the volume
   tfunc2D *get_tfunc() {return(VOL->get_tfunc());} // return the transfer function
   histo *get_histo() {return(VOL->get_histo());} // return the histogram

   protected:

   mipmap *VOL;
   };

#endif
