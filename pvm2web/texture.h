#ifndef TEXTURE_H
#define TEXTURE_H

// nearest neighbor
unsigned char nearest(unsigned char *source, unsigned int qx, unsigned int qy, unsigned int qz,
                      unsigned int zi, unsigned int zj, unsigned int zk);

// trilinear interpolation
unsigned char interpol(unsigned char *quelle, unsigned int qw, unsigned int qh, unsigned int qd,
                       float ii, float ji, float ki);
unsigned char interpol(unsigned char *quelle, unsigned int qw, unsigned int qh, unsigned int qd,
                       unsigned int zw, unsigned int zh, unsigned int zd,
                       unsigned int zi, unsigned int zj, unsigned int zk);

// resample a volume
unsigned char *resampleVolume(unsigned char *volume,
                              unsigned int width, unsigned int height, unsigned int depth,
                              unsigned int &size);

// generates a nxn texture from a nxnxn volume
unsigned char *getTextureFromVolume(unsigned char *volume, unsigned int size,
                                    char plane, int row);

#endif
