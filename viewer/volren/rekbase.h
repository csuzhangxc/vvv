// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef REKBASE_H
#define REKBASE_H

// read a REK volume (Fraunhofer EZRT volume format)
// note: the REK format has a 2048 byte header
// note: the header contains short int values in LSB format
// note:  short int #1 of header: x size
// note:  short int #2 of header: y size
// note:  short int #3 of header: bits (usually 8 or 16)
// note:  short int #4 of header: z size
// note: after the header the raw volume data is attached
// note:  the values of a 16-bit volume are provided in LSB format
unsigned char *readREKvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components=NULL,
                             float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

#endif
