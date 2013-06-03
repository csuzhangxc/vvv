// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef RAWBASE_H
#define RAWBASE_H

// read a RAW volume
//  the RAW file format is encoded into the filename
//   as in name.size_cellformat_cellsize.raw
//    e.g. name.256x256_u2m.raw
//    e.g. name.256x256x256_2_100x100x50.raw
//    e.g. name.256x256x256x100.raw
//   cell format modifiers:
//    1 = 8 bit
//    2 = 16 bit
//    3 = rgb
//    4 = rgba
//    6 = rgb 16 bit
//    8 = rgba 16 bit
//    f = 32 bit float
//    u = unsigned
//    s = signed
//    m = msb
//    l = lsb
//   default modifiers = u1m
unsigned char *readRAWvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                             unsigned int *components=NULL,unsigned int *bits=NULL,BOOLINT *sign=NULL,BOOLINT *msb=NULL,
                             float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// analyze RAW file format
BOOLINT readRAWinfo(char *filename,
                    unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *steps,
                    unsigned int *components=NULL,unsigned int *bits=NULL,BOOLINT *sign=NULL,BOOLINT *msb=NULL,
                    float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// define RAW file format
char *makeRAWinfo(unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                  unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                  float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// write a RAW volume
BOOLINT writeRAWvolume(const char *filename, // /wo suffix .raw
                       unsigned char *volume,
                       unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                       unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                       float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume
char *copyRAWvolume(FILE *file, // source file desc
                    const char *output, // destination file name /wo suffix .raw
                    unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                    unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                    float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume
char *copyRAWvolume(const char *filename, // source file
                    const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core linear quantization
char *copyRAWvolume_linear(FILE *file, // source file desc
                           const char *output, // destination file name /wo suffix .raw
                           unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                           unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                           float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core linear quantization
char *copyRAWvolume_linear(const char *filename, // source file
                           const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core non-linear quantization
char *copyRAWvolume_nonlinear(FILE *file, // source file desc
                              const char *output, // destination file name /wo suffix .raw
                              unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                              unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                              float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core non-linear quantization
char *copyRAWvolume_nonlinear(const char *filename, // source file
                              const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core cropping
char *cropRAWvolume(FILE *file, // source file desc
                    const char *output, // destination file name /wo suffix .raw
                    unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                    unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                    float scalex=1.0f,float scaley=1.0f,float scalez=1.0f,
                    float ratio=0.5f);

// copy a RAW volume with out-of-core cropping
char *cropRAWvolume(const char *filename, // source file
                    const char *output, // destination file name /wo suffix .raw
                    float ratio=0.5f); // crop volume ratio

// copy a RAW volume with out-of-core down-sizing
char *downsizeRAWvolume(FILE *file, // source file desc
                        const char *output, // destination file name /wo .raw
                        unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                        unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                        float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core down-sizing
char *downsizeRAWvolume(const char *filename, // source file
                        const char *output); // destination file name /wo suffix .raw

// process a RAW volume with out-of-core cropping and non-linear quantization
char *processRAWvolume(FILE *file, // source file desc
                       const char *output, // destination file name
                       unsigned int width,unsigned int height,unsigned int depth=1,unsigned int steps=1,
                       unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                       float scalex=1.0f,float scaley=1.0f,float scalez=1.0f,
                       float ratio=0.5f, // crop volume ratio
                       unsigned long long maxcells=250000000); // down-size threshold

// process a RAW volume with out-of-core cropping and non-linear quantization
char *processRAWvolume(const char *filename, // source file
                       float ratio=0.5f, // crop volume ratio
                       unsigned long long maxcells=250000000); // down-size threshold

#endif
