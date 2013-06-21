// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef REKBASE_H
#define REKBASE_H

// read a REK volume (Fraunhofer EZRT volume format)
//  the REK format has a 2048 byte header
//  the header contains short int values in LSB format
//   short int #1 of header: x size
//   short int #2 of header: y size
//   short int #3 of header: bits (usually 8 or 16)
//   short int #4 of header: z size
//  after the header the raw volume data is attached
//   the values of a 16-bit volume are saved in LSB order
unsigned char *readREKvolume(const char *filename,
                             long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                             float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// read REK file format header
BOOLINT readREKheader(const char *filename,
                      long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                      float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// copy a REK volume to a RAW volume
char *copyREKvolume(const char *filename,const char *output);

// read a REK volume out-of-core
unsigned char *readREKvolume_ooc(const char *filename,
                                 long long *width,long long *height,long long *depth,unsigned int *components=NULL,
                                 float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                                 void (*feedback)(const char *info,float percent)=NULL);

#endif
