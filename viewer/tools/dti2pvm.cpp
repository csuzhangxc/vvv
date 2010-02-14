#include "codebase.h"

#include "ddsbase.h"

#define VOLUME(num,pos) (256*volume[num][2*pos]+volume[num][2*pos+1])

// swap two doubles
inline void swap(double &a, double &b)
   {
   double tmp=a;
   a=b;
   b=tmp;
   }

// compute the eigenvals of a 3x3 symmetric matrix
void eigenvals(double M11,double M12,double M13,double M22,double M23,double M33,
               double &l1, double &l2, double &l3)
   {
   const double sqrt3 = sqrt(3.0);

   // compute the coefficients of the characteristic polynom cp(x) for det|M - xI| = 0
   // cp(x) = x^3 + b x^2 + c x + d = 0
   double b = -M11-M22-M33;
   double c = M11*M22 + M11*M33 + M22*M33 - M12*M12 - M13*M13 - M23*M23;
   double d = M11*M23*M23 + M12*M12*M33 + M13*M13*M22 - 2.0*M12*M13*M23 - M11*M22*M33;

   double f = c - 1.0/3*b*b;
   double g = 2.0/27*b*b*b - 1.0/3*b*c + d;
   double h = 1.0/4*g*g + 1.0/27*f*f*f;

   // trivial solution
   if (h>=0.0)
      {
      if (d>0.0) d=0.0; // d should be non-positive
      l1 = l2 = l3 = pow(-d, 1.0/3);
      return;
      }

   double i = sqrt(1.0/4*g*g - h);
   double j = pow(i, 1.0/3);
   double k = acos(-1.0/2*g/i);
   double m = cos(1.0/3*k);
   double n = sqrt3*sin(1.0/3*k);
   double p = -1.0/3*b;

   // non-trivial solutions
   l1 = 2.0*j*m + p;
   l2 = -j*(m + n) + p;
   l3 = -j*(m - n) + p;

   // sort eigenvals
   if (l1 > l2) swap(l1, l2);
   if (l2 > l3)
      {
      swap(l2, l3);
      if (l1 > l2) swap(l1, l2);
      }
   }

int main(int argc,char *argv[])
   {
   unsigned int i,j,k,p;

   unsigned char *volume[6];

   unsigned int width[6],height[6],depth[6],
                components[6];

   float scalex[6],scaley[6],scalez[6];

   int maxval;

   float MD,
         Dxx,Dyy,Dzz,Dxy,Dxz,Dyz,
         FA;

   double l1,l2,l3;

   if (argc!=5 && argc!=9)
      {
      printf("usage: %s <1.pvm><2.pvm><3.pvm>[<4.pvm><5.pvm><6.pvm>] <output1.pvm>[<output2.pvm>]\n",argv[0]);
      exit(1);
      }

   if (argc==9)
      {
      for (i=0; i<6; i++)
         {
         if ((volume[i]=readPVMvolume(argv[i+1],&width[i],&height[i],&depth[i],&components[i],&scalex[i],&scaley[i],&scalez[i]))==NULL) exit(1);

         if (i>0)
            if (width[i]!=width[0] || height[i]!=height[0] || depth[i]!=depth[0] || components[i]!=components[0]) exit(1);
         }

      if (components[0]==1)
         {
         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  Dxx=(volume[2][p]+volume[3][p]-volume[0][p]-volume[1][p]-volume[4][p]-volume[5][p])/255.0f/2.0f;
                  Dyy=(volume[0][p]+volume[1][p]-volume[2][p]-volume[3][p]-volume[4][p]-volume[5][p])/255.0f/2.0f;
                  Dzz=(volume[4][p]+volume[5][p]-volume[0][p]-volume[1][p]-volume[2][p]-volume[3][p])/255.0f/2.0f;

                  Dxy=(volume[5][p]-volume[4][p])/255.0f/2.0f;
                  Dxz=(volume[1][p]-volume[0][p])/255.0f/2.0f;
                  Dyz=(volume[3][p]-volume[2][p])/255.0f/2.0f;

                  eigenvals(-Dxx,-Dxy,-Dxz,-Dyy,-Dyz,-Dzz,l1,l2,l3);

                  MD=(volume[0][p]+volume[1][p]+volume[2][p]+volume[3][p]+volume[4][p]+volume[5][p])/255.0f/6.0f;
                  volume[0][p]=ftrc(255.0f*fmin(fmax(MD,0.0f),1.0f)+0.5f);

                  FA=fsqrt((fsqr(l1-l2)+fsqr(l2-l3)+fsqr(l3-l1))/(2.0f*(l1*l1+l2*l2+l3*l3)));
                  volume[1][p]=ftrc(255.0f*fmin(fmax(fpow(MD,1.0f/3)*FA,0.0f),1.0f)+0.5f);
                  }

         writePVMvolume(argv[7],volume[0],width[0],height[0],depth[0],1,scalex[0],scaley[0],scalez[0]);
         writePVMvolume(argv[8],volume[1],width[1],height[1],depth[1],1,scalex[1],scaley[1],scalez[1]);
         }
      else if (components[0]==2)
         {
         maxval=1;

         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  if (VOLUME(0,p)>maxval) maxval=VOLUME(0,p);
                  if (VOLUME(1,p)>maxval) maxval=VOLUME(1,p);
                  if (VOLUME(2,p)>maxval) maxval=VOLUME(2,p);
                  if (VOLUME(3,p)>maxval) maxval=VOLUME(3,p);
                  if (VOLUME(4,p)>maxval) maxval=VOLUME(4,p);
                  if (VOLUME(5,p)>maxval) maxval=VOLUME(5,p);
                  }

         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  Dxx=(float)(VOLUME(2,p)+VOLUME(3,p)-VOLUME(0,p)-VOLUME(1,p)-VOLUME(4,p)-VOLUME(5,p))/maxval/2.0f;
                  Dyy=(float)(VOLUME(0,p)+VOLUME(1,p)-VOLUME(2,p)-VOLUME(3,p)-VOLUME(4,p)-VOLUME(5,p))/maxval/2.0f;
                  Dzz=(float)(VOLUME(4,p)+VOLUME(5,p)-VOLUME(0,p)-VOLUME(1,p)-VOLUME(2,p)-VOLUME(3,p))/maxval/2.0f;

                  Dxy=(float)(VOLUME(5,p)-VOLUME(4,p))/maxval/2.0f;
                  Dxz=(float)(VOLUME(1,p)-VOLUME(0,p))/maxval/2.0f;
                  Dyz=(float)(VOLUME(3,p)-VOLUME(2,p))/maxval/2.0f;

                  eigenvals(-Dxx,-Dxy,-Dxz,-Dyy,-Dyz,-Dzz,l1,l2,l3);

                  MD=(float)(VOLUME(0,p)+VOLUME(1,p)+VOLUME(2,p)+VOLUME(3,p)+VOLUME(4,p)+VOLUME(5,p))/maxval/6.0f;
                  volume[0][p]=ftrc(255.0f*fmin(fmax(MD,0.0f),1.0f)+0.5f);

                  FA=fsqrt((fsqr(l1-l2)+fsqr(l2-l3)+fsqr(l3-l1))/(2.0f*(l1*l1+l2*l2+l3*l3)));
                  volume[1][p]=ftrc(255.0f*fmin(fmax(fpow(MD,1.0f/3)*FA,0.0f),1.0f)+0.5f);
                  }

         writePVMvolume(argv[7],volume[0],width[0],height[0],depth[0],1,scalex[0],scaley[0],scalez[0]);
         writePVMvolume(argv[8],volume[1],width[1],height[1],depth[1],1,scalex[1],scaley[1],scalez[1]);
         }

      for (i=0; i<6; i++) free(volume[i]);
      }
   else
      {
      for (i=0; i<3; i++)
         {
         if ((volume[i]=readPVMvolume(argv[i+1],&width[i],&height[i],&depth[i],&components[i],&scalex[i],&scaley[i],&scalez[i]))==NULL) exit(1);

         if (i>0)
            if (width[i]!=width[0] || height[i]!=height[0] || depth[i]!=depth[0] || components[i]!=components[0]) exit(1);
         }

      if (components[0]==1)
         {
         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  MD=fsqrt((fsqr(volume[0][p])+fsqr(volume[1][p])+fsqr(volume[2][p]))/3.0f)/255.0f;
                  volume[0][p]=ftrc(255.0f*fmin(fmax(MD,0.0f),1.0f)+0.5f);
                  }

         writePVMvolume(argv[4],volume[0],width[0],height[0],depth[0],1,scalex[0],scaley[0],scalez[0]);
         }
      else if (components[0]==2)
         {
         maxval=1;

         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  if (VOLUME(0,p)>maxval) maxval=VOLUME(0,p);
                  if (VOLUME(1,p)>maxval) maxval=VOLUME(1,p);
                  if (VOLUME(2,p)>maxval) maxval=VOLUME(2,p);
                  }

         for (k=0; k<depth[0]; k++)
            for (j=0; j<height[0]; j++)
               for (i=0; i<width[0]; i++)
                  {
                  p=i+(j+k*height[0])*width[0];

                  MD=fsqrt((fsqr(VOLUME(0,p))+fsqr(VOLUME(1,p))+fsqr(VOLUME(2,p)))/3.0f)/maxval;
                  volume[0][p]=ftrc(255.0f*fmin(fmax(MD,0.0f),1.0f)+0.5f);
                  }

         writePVMvolume(argv[4],volume[0],width[0],height[0],depth[0],1,scalex[0],scaley[0],scalez[0]);
         }

      for (i=0; i<3; i++) free(volume[i]);
      }

   return(0);
   }
