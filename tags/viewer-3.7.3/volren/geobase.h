// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef GEOBASE_H
#define GEOBASE_H

class ministrip;

// a wrapper for ministrip
class Surface
   {
   public:

   Surface();
   ~Surface();

   int readGEOfile(const char *filename);
   int has_geo();

   void clear();

   void setmatrix(double mtx[16]);
   void render();

   double getscale();

   protected:

   ministrip *strip_;
   };

#endif