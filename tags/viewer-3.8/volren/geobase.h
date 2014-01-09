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
   int has_geo_shown();

   void clear();

   void show(int yes);
   void setmatrix(double mtx[16]);
   void render();

   double getscale();

   protected:

   ministrip *strip_;
   int is_shown_;
   };

#endif
