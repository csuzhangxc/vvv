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
                             long long *width,long long *height,long long *depth,long long *steps,
                             unsigned int *components=NULL,unsigned int *bits=NULL,BOOLINT *sign=NULL,BOOLINT *msb=NULL,
                             float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// analyze RAW file format
BOOLINT readRAWinfo(char *filename,
                    long long *width,long long *height,long long *depth,long long *steps,
                    unsigned int *components=NULL,unsigned int *bits=NULL,BOOLINT *sign=NULL,BOOLINT *msb=NULL,
                    float *scalex=NULL,float *scaley=NULL,float *scalez=NULL);

// define RAW file format
char *makeRAWinfo(long long width,long long height,long long depth=1,long long steps=1,
                  unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                  float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// write a RAW volume
BOOLINT writeRAWvolume(const char *filename, // /wo suffix .raw
                       unsigned char *volume,
                       long long width,long long height,long long depth=1,long long steps=1,
                       unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                       float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume
char *copyRAWvolume(FILE *file, // source file desc
                    const char *output, // destination file name /wo suffix .raw
                    long long width,long long height,long long depth=1,long long steps=1,
                    unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                    float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume
char *copyRAWvolume(const char *filename, // source file
                    const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core linear quantization
char *copyRAWvolume_linear(FILE *file, // source file desc
                           const char *output, // destination file name /wo suffix .raw
                           long long width,long long height,long long depth=1,long long steps=1,
                           unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                           float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core linear quantization
char *copyRAWvolume_linear(const char *filename, // source file
                           const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core non-linear quantization
char *copyRAWvolume_nonlinear(FILE *file, // source file desc
                              const char *output, // destination file name /wo suffix .raw
                              long long width,long long height,long long depth=1,long long steps=1,
                              unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                              float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core non-linear quantization
char *copyRAWvolume_nonlinear(const char *filename, // source file
                              const char *output); // destination file name /wo suffix .raw

// copy a RAW volume with out-of-core cropping
char *cropRAWvolume(FILE *file, // source file desc
                    const char *output, // destination file name /wo suffix .raw
                    long long width,long long height,long long depth=1,long long steps=1,
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
                        long long width,long long height,long long depth=1,long long steps=1,
                        unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                        float scalex=1.0f,float scaley=1.0f,float scalez=1.0f);

// copy a RAW volume with out-of-core down-sizing
char *downsizeRAWvolume(const char *filename, // source file
                        const char *output); // destination file name /wo suffix .raw

// process a RAW volume with out-of-core cropping and non-linear quantization
char *processRAWvolume(FILE *file, // source file desc
                       const char *output, // destination file name
                       long long width,long long height,long long depth=1,long long steps=1,
                       unsigned int components=1,unsigned int bits=8,BOOLINT sign=FALSE,BOOLINT msb=TRUE,
                       float scalex=1.0f,float scaley=1.0f,float scalez=1.0f,
                       float ratio=0.5f, // crop volume ratio
                       long long maxcells=300000000); // down-size threshold

// process a RAW volume with out-of-core cropping and non-linear quantization
char *processRAWvolume(const char *filename, // source file
                       float ratio=0.5f, // crop volume ratio
                       long long maxcells=300000000); // down-size threshold

// swap the hi and lo byte of 16 bit data
void swapbytes(unsigned char *data,long long bytes);

// convert from signed short to unsigned short
void convbytes(unsigned char *data,long long bytes);

// convert from float to unsigned short
void convfloat(unsigned char *data,long long bytes);

// quantize 16 bit data to 8 bit using a non-linear mapping
unsigned char *quantize(unsigned char *volume,
                        long long width,long long height,long long depth,
                        BOOLINT msb=TRUE,
                        BOOLINT linear=FALSE,BOOLINT nofree=FALSE);

#endif
