#include <viewer/codebase.h>

#include "texture.h"

// nearest neighbour
unsigned char nearest(unsigned char *source, unsigned int qx, unsigned int qy, unsigned int qz,
                      unsigned int zi, unsigned int zj, unsigned int zk)
{
   return source[zi + qx * zj + qx * qy * zk];
}

// get a pixel from the texture
unsigned char pixelget(unsigned char *source, unsigned int qx, unsigned int qy, unsigned int qz,
                       unsigned int zi, unsigned int zj, unsigned int zk)
{
   if (zi == qx) zi--;
   if (zj == qy) zj--;
   if (zk == qz) zk--;

   return source[zi + qx * zj + qx * qy * zk];
}

// trilinear interpolation with normalized coordinates
unsigned char interpol(unsigned char *source, unsigned int qw, unsigned int qh, unsigned int qd,
                       double ii, double ji, double ki)
{
   unsigned int i = (int)floor(ii * (qw - 1)),
                j = (int)floor(ji * (qh-1)),
                k = (int)(floor(ki * (qd-1)));

    unsigned char p[2][2][2];
    double u, v, w;

    p[0][0][0] = pixelget(source, qw, qh, qd, i, j, k);
    p[1][0][0] = pixelget(source, qw, qh, qd, i+1, j, k);
    p[1][0][1] = pixelget(source, qw, qh, qd, i+1, j, k+1);
    p[0][0][1] = pixelget(source, qw, qh, qd, i, j, k+1);

    p[0][1][0] = pixelget(source, qw, qh, qd, i, j+1, k);
    p[1][1][0] = pixelget(source, qw, qh, qd, i+1, j+1, k);
    p[1][1][1] = pixelget(source, qw, qh, qd, i+1, j+1, k+1);
    p[0][1][1] = pixelget(source, qw, qh, qd, i, j+1, k+1);

    u = ii * (qw - 1) - floor(ii * (qw - 1));
    v = ji * (qh - 1) - floor(ji * (qh - 1));
    w = ki * (qd - 1) - floor(ki * (qd - 1));

    return( (1-w) * ( (1-v) * ( (1-u) * p[0][0][0] +
                                 u * p[1][0][0]) +
                       v    * ( (1-u) * p[0][1][0] +
                                 u * p[1][1][0])) + 
             w    * ( (1-v) * ( (1-u) * p[0][0][1] +
                                 u * p[1][0][1]) +
                       v    * ( (1-u) * p[0][1][1] +
                                 u * p[1][1][1] ) ) );
}

// trilinear interpolation with pixel coordinates
unsigned char interpol(unsigned char *source, unsigned int qw, unsigned int qh, unsigned int qd,
                       unsigned int zw, unsigned int zh, unsigned int zd,
                       unsigned int zi, unsigned int zj, unsigned int zk)
{
    double ii, ji, ki;

    ii = zi * 1.0 / (zw - 1);
    ji = zj * 1.0 / (zh - 1);
    ki = zk * 1.0 / (zd - 1);

    return(interpol(source, qw, qh, qd, ii, ji, ki));
}

// resample a volume
unsigned char *resampleVolume(unsigned char *volume,
                              unsigned int width, unsigned int height, unsigned int depth,
                              unsigned int &size)
{
   unsigned int i, j, k;

   unsigned char *volume2;
   unsigned int nw, nh, nd;

   nw = (int) pow(2,ceil(log10((double)width)/log10(2.0)));
   nh = (int) pow(2,ceil(log10((double)height)/log10(2.0)));
   nd = (int) pow(2,ceil(log10((double)depth)/log10(2.0)));

   // this version only supports nxnxn volumes
   size = max(nw, max(nh, nd));
   nw = nh = nd = size;
   volume2 = (unsigned char *)calloc(nw * nd * nh, sizeof(char));

   // resample to power of two
   for(k = 0; k < nd; k++)
      for(j = 0; j < nh; j++)
         for(i = 0; i < nw; i++)
            volume2[i + nw * j + nw * nh * k] =
               interpol(volume, width, height, depth, nw, nd, nh, i, j, k);

   return(volume2);
}

// generates a nxn texture from a nxnxn volume
unsigned char *getTextureFromVolume(unsigned char *volume, unsigned int size,
                                    char plane, int row)
{
   int i, j, k;

   unsigned char *texture = (unsigned char *) calloc(size * size, sizeof(char));

   // if x-plane, get the values from y and z, etc...
   if (plane == 'x')
   {
      i = row;

      for (j = 0; j < size; j++)
         for (k = 0; k < size; k++)
            texture[k + (size-1-j)*size]= volume[i + size * j + size * size * k];
   }
   else if (plane == 'y')
   {
      j = row;
        
      for (k = 0; k < size; k++)
            for (i = 0; i < size; i++)
                texture[i + (size-1-k)*size]= volume[i + size * j + size * size * k];
   }
   else
   {
      k = row;

      for (j = 0; j < size; j++)
         for (i = 0; i < size; i++)
            texture[i + (size-1-j)*size]= volume[i + size * j + size * size * k];
   }
    
   return(texture);
}
