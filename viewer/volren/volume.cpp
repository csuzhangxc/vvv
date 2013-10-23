// (c) by Stefan Roettger, licensed under GPL 2+

#define SOBEL
#define MULTILEVEL

#define FBO16
#undef FBOMM

#define TILEINC 1000
#define QUEUEINC 1000

#include "volume.h"

volume::volume(tfunc2D *tf,char *base)
   {
   TILEMAX=TILEINC;
   TILE=new tileptr[TILEMAX];
   TILECNT=0;

   TFUNC=tf;

   if (base==NULL) strncpy(BASE,"volren",MAXSTR);
   else snprintf(BASE,MAXSTR,"%s/volren",base);
   }

volume::~volume()
   {
   int i;

   for (i=0; i<TILECNT; i++) delete TILE[i];
   delete TILE;
   }

// check brick size
BOOLINT volume::check(int bricksize,float overmax)
   {
   int border=(int)fceil(overmax);
   return(bricksize>2*border);
   }

// set the volume data
void volume::set_data(unsigned char *data,
                      unsigned char *extra,
                      long long width,long long height,long long depth,
                      float mx,float my,float mz,
                      float sx,float sy,float sz,
                      int bricksize,float overmax,
                      void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int i;

   tileptr *tiles;

   long long px,py,pz;

   int border=(int)fceil(overmax);

   float mx2,my2,mz2,
         sx2,sy2,sz2;

   float newsize;

   if (bricksize<=2*border) ERRORMSG();

   for (TILEZ=0,pz=-2*border; pz<depth-1+border; pz+=bricksize-1-2*border,TILEZ++)
      {
      if (feedback!=NULL)
         {
         float tilesz=(float)(depth+3*border)/(bricksize-2*border);
         feedback("uploading data",fmin((TILEZ+1)/tilesz,1.0f),obj);
         }

      for (TILEY=0,py=-2*border; py<height-1+border; py+=bricksize-1-2*border,TILEY++)
         for (TILEX=0,px=-2*border; px<width-1+border; px+=bricksize-1-2*border,TILEX++)
            {
            sx2=(bricksize-1-2*border)*sx/(width-1);
            sy2=(bricksize-1-2*border)*sy/(height-1);
            sz2=(bricksize-1-2*border)*sz/(depth-1);

            mx2=mx-sx/2.0f+(px+border)*sx/(width-1)+sx2/2.0f;
            my2=my-sy/2.0f+(py+border)*sy/(height-1)+sy2/2.0f;
            mz2=mz-sz/2.0f+(pz+border)*sz/(depth-1)+sz2/2.0f;

            if (TILECNT>=TILEMAX)
               {
               TILEMAX+=TILEINC;
               tiles=new tileptr[TILEMAX];
               for (i=0; i<TILECNT; i++) tiles[i]=TILE[i];
               delete TILE;
               TILE=tiles;
               }

            TILE[TILECNT]=new tile(TFUNC,BASE);

            TILE[TILECNT]->set_data(data,
                                    width,height,depth,
                                    mx2,my2,mz2,
                                    sx2,sy2,sz2,
                                    px,py,pz,
                                    bricksize,border);

            if (extra!=NULL)
               TILE[TILECNT]->set_extra(extra,
                                        width,height,depth,
                                        px,py,pz,
                                        bricksize);

            if (px+bricksize>width+2*border)
               {
               newsize=sx2*(float)(width-1-px)/(bricksize-1-2*border);
               mx2-=(sx2-newsize)/2.0f;
               sx2=newsize;
               }

            if (py+bricksize>height+2*border)
               {
               newsize=sy2*(float)(height-1-py)/(bricksize-1-2*border);
               my2-=(sy2-newsize)/2.0f;
               sy2=newsize;
               }

            if (pz+bricksize>depth+2*border)
               {
               newsize=sz2*(float)(depth-1-pz)/(bricksize-1-2*border);
               mz2-=(sz2-newsize)/2.0f;
               sz2=newsize;
               }

            TILE[TILECNT++]->set_size(mx2,my2,mz2,
                                      sx2,sy2,sz2);
            }
      }

   MX=mx;
   MY=my;
   MZ=mz;

   SX=sx*(width-1+2*border)/(width-1);
   SY=sy*(height-1+2*border)/(height-1);
   SZ=sz*(depth-1+2*border)/(depth-1);

   BX=sx*(width+1)/(width-1);
   BY=sy*(height+1)/(height-1);
   BZ=sz*(depth+1)/(depth-1);

   SLAB=fmin(sx/(width-1),fmin(sy/(height-1),sz/(depth-1)));
   }

// set ambient/diffuse/specular lighting coefficients
void volume::set_light(float noise,float ambnt,float difus,float specl,float specx)
   {
   int i;

   for (i=0; i<TILECNT; i++) TILE[i]->set_light(noise,ambnt,difus,specl,specx);
   }

// sort tiles
BOOLINT volume::sort(int x,int y,int z,
                     int sx,int sy,int sz,
                     float ex,float ey,float ez,
                     float dx,float dy,float dz,
                     float ux,float uy,float uz,
                     float nearp,float slab,float rslab,
                     BOOLINT lighting,
                     BOOLINT (*abort)(void *abortdata),
                     void *abortdata)
   {
   BOOLINT aborted=FALSE;

   tileptr t1,t2;

   if (sx>1)
      {
      t1=TILE[(x+sx/2)+(y+z*TILEY)*TILEX];
      t2=TILE[(x+sx/2-1)+(y+z*TILEY)*TILEX];

      if ((t1->get_mx()+t2->get_mx())/2.0f>ex)
         {
         aborted=sort(x+sx/2,y,z,sx-sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x,y,z,sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      else
         {
         aborted=sort(x,y,z,sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x+sx/2,y,z,sx-sx/2,sy,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      }
   else if (sy>1)
      {
      t1=TILE[x+((y+sy/2)+z*TILEY)*TILEX];
      t2=TILE[x+((y+sy/2-1)+z*TILEY)*TILEX];

      if ((t1->get_my()+t2->get_my())/2.0f>ey)
         {
         aborted=sort(x,y+sy/2,z,sx,sy-sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x,y,z,sx,sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      else
         {
         aborted=sort(x,y,z,sx,sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x,y+sy/2,z,sx,sy-sy/2,sz,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      }
   else if (sz>1)
      {
      t1=TILE[x+(y+(z+sz/2)*TILEY)*TILEX];
      t2=TILE[x+(y+(z+sz/2-1)*TILEY)*TILEX];

      if ((t1->get_mz()+t2->get_mz())/2.0f>ez)
         {
         aborted=sort(x,y,z+sz/2,sx,sy,sz-sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x,y,z,sx,sy,sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      else
         {
         aborted=sort(x,y,z,sx,sy,sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         if (!aborted) aborted=sort(x,y,z+sz/2,sx,sy,sz-sz/2,ex,ey,ez,dx,dy,dz,ux,uy,uz,nearp,slab,rslab,lighting,abort,abortdata);
         }
      }
   else
      {
      TILE[x+(y+z*TILEY)*TILEX]->render(ex,ey,ez,
                                        dx,dy,dz,
                                        ux,uy,uz,
                                        nearp,slab,rslab,
                                        lighting);

      if (abort!=NULL) aborted=abort(abortdata);
      }

   return(aborted);
   }

// render the volume
BOOLINT volume::render(float ex,float ey,float ez,
                       float dx,float dy,float dz,
                       float ux,float uy,float uz,
                       float nearp,float slab,float rslab,
                       BOOLINT lighting,
                       BOOLINT (*abort)(void *abortdata),
                       void *abortdata)
   {
   BOOLINT aborted;

   // enable alpha test for pre-multiplied tfs
   if (get_tfunc()->get_premult())
      {
      glAlphaFunc(GL_GREATER,0.0);
      glEnable(GL_ALPHA_TEST);
      }

   // render tiles in back-to-front sorted order
   aborted=sort(0,0,0,TILEX,TILEY,TILEZ,
                ex,ey,ez,dx,dy,dz,ux,uy,uz,
                nearp,slab,rslab,
                lighting,
                abort,abortdata);

   // disable alpha test for pre-multiplied tfs
   if (get_tfunc()->get_premult())
      glDisable(GL_ALPHA_TEST);

   return(aborted);
   }

// draw the surrounding wire frame box
void volume::drawwireframe(float mx,float my,float mz,
                           float sx,float sy,float sz)
   {
   float sx2,sy2,sz2;

   sx2=0.5f*sx;
   sy2=0.5f*sy;
   sz2=0.5f*sz;

   glPushMatrix();
   glTranslatef(mx,my,mz);

   glColor3f(0.5f,0.5f,0.5f);
   glBegin(GL_LINES);
   glVertex3f(-sx2,sy2,-sz2);
   glVertex3f(sx2,sy2,-sz2);
   glVertex3f(-sx2,sy2,-sz2);
   glVertex3f(-sx2,sy2,sz2);
   glVertex3f(sx2,sy2,sz2);
   glVertex3f(-sx2,sy2,sz2);
   glVertex3f(sx2,sy2,sz2);
   glVertex3f(sx2,sy2,-sz2);
   glVertex3f(-sx2,-sy2,-sz2);
   glVertex3f(-sx2,sy2,-sz2);
   glVertex3f(sx2,-sy2,-sz2);
   glVertex3f(sx2,sy2,-sz2);
   glVertex3f(sx2,-sy2,sz2);
   glVertex3f(sx2,sy2,sz2);
   glVertex3f(-sx2,-sy2,sz2);
   glVertex3f(-sx2,sy2,sz2);
   glEnd();

   glDisable(GL_CULL_FACE);
   glColor3f(0.25f,0.25f,0.25f);
   glBegin(GL_TRIANGLE_FAN);
   glVertex3f(-sx2,-sy2,-sz2);
   glVertex3f(sx2,-sy2,-sz2);
   glVertex3f(sx2,-sy2,sz2);
   glVertex3f(-sx2,-sy2,sz2);
   glVertex3f(-sx2,-sy2,-sz2);
   glEnd();
   glEnable(GL_CULL_FACE);

   glPopMatrix();
   }

// the volume hierarchy:

mipmap::mipmap(char *base,int res)
   {
   if (res==0) res=256;

   VOLCNT=0;

   TFUNC=new tfunc2D(res);
   HISTO=new histo;

   VOLUME=GRAD=NULL;

   DSX=DSY=DSZ=1.0f;
   GDSX=GDSY=GDSZ=1.0f;

   GRADMAX=1.0f;

   if (base==NULL) strncpy(BASE,"",MAXSTR);
   else strncpy(BASE,base,MAXSTR);

   strncpy(filestr,"",MAXSTR);
   strncpy(gradstr,"",MAXSTR);
   strncpy(commstr,"",MAXSTR);
   strncpy(zerostr,"",MAXSTR);

   xsflag=FALSE; ysflag=FALSE; zsflag=FALSE;
   xrflag=FALSE; zrflag=FALSE;

   hmvalue=0.0f; hfvalue=0.0f; hsvalue=0.0f;
   knvalue=0;

   ex_=ey_=ez_=0.0;
   dx_=dy_=dz_=0.0;
   ux_=uy_=uz_=0.0;

   px_=py_=pz_=0.0;
   nx_=ny_=nz_=0.0;

   disable_clip();

   set_vol_maxsize(512);
   set_iso_maxsize(256);

   CACHE=NULL;

   CSIZEX=0;
   CSIZEY=0;
   CSLICE=0;
   CSLICES=0;

   QUEUEMAX=QUEUEINC;

   QUEUEX=new int[QUEUEMAX];
   QUEUEY=new int[QUEUEMAX];
   QUEUEZ=new int[QUEUEMAX];

   HASFBO=FALSE;
   fboWidth=fboHeight=0;
   textureId=rboId=fboId=0;
   }

mipmap::~mipmap()
   {
   int i;

   for (i=0; i<VOLCNT; i++) delete VOL[i];
   if (VOLCNT>0) delete VOL;

   delete TFUNC;
   delete HISTO;

   if (VOLUME!=NULL) free(VOLUME);
   if (GRAD!=NULL) free(GRAD);

   if (CACHE!=NULL) delete CACHE;

   delete QUEUEX;
   delete QUEUEY;
   delete QUEUEZ;

   destroy();
   }

// create fbo
void mipmap::setup(int width,int height)
   {
   char *GL_EXTs;

   if ((GL_EXTs=(char *)glGetString(GL_EXTENSIONS))==NULL) ERRORMSG();

   if (strstr(GL_EXTs,"EXT_framebuffer_object")!=NULL)
      if (width>0 && height>0)
         if (!HASFBO || width!=fboWidth || height!=fboHeight)
            {
#ifdef GL_EXT_framebuffer_object

            destroy();

            HASFBO=TRUE;

            // save actual size
            fboWidth=width;
            fboHeight=height;

            // create a texture object
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef FBOMM
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#else
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef FBOMM
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
#else
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap off
#endif
#ifdef FBO16
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_HALF_FLOAT_ARB, 0);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
            glBindTexture(GL_TEXTURE_2D, 0);

            // create a renderbuffer object to store depth info
            glGenRenderbuffersEXT(1, &rboId);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            // create a framebuffer object
            glGenFramebuffersEXT(1, &fboId);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

            // attach the texture to fbo color attachment point
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);

            // attach the renderbuffer to depth attachment point
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);

            // get fbo status
            GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

            // switch back to window-system-provided framebuffer
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

            // check fbo status
            if (status != GL_FRAMEBUFFER_COMPLETE_EXT) destroy();

#endif
            }
   }

// destroy fbo
void mipmap::destroy()
   {
#ifdef GL_EXT_framebuffer_object

   if (textureId!=0) glDeleteTextures(1, &textureId);
   if (rboId!=0) glDeleteRenderbuffersEXT(1, &rboId);
   if (fboId!=0) glDeleteFramebuffersEXT(1, &fboId);

   textureId=0;
   rboId=0;
   fboId=0;

   HASFBO=FALSE;
   fboWidth=fboHeight=0;

#endif
   }

// update 16-bit fbo
void mipmap::updatefbo()
   {
   GLint viewport[4];
   glGetIntegerv(GL_VIEWPORT,viewport);
   int width=viewport[2];
   int height=viewport[3];

   setup(width,height);
   }

// reduce a volume to half its size
unsigned char *mipmap::reduce(unsigned char *data,
                              long long width,long long height,long long depth,
                              void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   long long i,j,k;

   unsigned char *data2,*ptr;

   long long idx,slx;

   if (data==NULL) return(NULL);

   if ((data2=(unsigned char *)malloc((width/2)*(height/2)*(depth/2)))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<depth-1; k+=2)
      {
      if (feedback!=NULL) feedback("reducing volume",(float)(k+1)/(depth-1),obj);

      for (j=0; j<height-1; j+=2)
         for (i=0; i<width-1; i+=2)
            {
            idx=i+(j+k*height)*width;
            slx=width*height;

            *ptr++=((int)data[idx]+
                    (int)data[idx+1]+
                    (int)data[idx+width]+
                    (int)data[idx+1+width]+
                    (int)data[idx+slx]+
                    (int)data[idx+1+slx]+
                    (int)data[idx+width+slx]+
                    (int)data[idx+1+width+slx]+4)/8;
            }
      }

   return(data2);
   }

// set the volume data
void mipmap::set_data(unsigned char *data,
                      unsigned char *extra,
                      long long width,long long height,long long depth,
                      float mx,float my,float mz,
                      float sx,float sy,float sz,
                      int bricksize,float overmax,
                      void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   int i;

   float o;

   unsigned char *data2,*extra2;

   if (VOLCNT!=0)
      {
      for (i=0; i<VOLCNT; i++) delete VOL[i];
      if (VOLCNT>0) delete VOL;
      }

   for (VOLCNT=0,i=bricksize,o=overmax; volume::check(i,o); i/=2,o/=2.0f,VOLCNT++)
      if ((width>>VOLCNT)<2 || (height>>VOLCNT)<2 || (depth>>VOLCNT)<2) break;

   if (VOLCNT==0) ERRORMSG();

   VOL=new volumeptr[VOLCNT];

   VOL[0]=new volume(TFUNC,BASE);

   VOL[0]->set_data(data,
                    extra,
                    width,height,depth,
                    mx,my,mz,
                    sx,sy,sz,
                    bricksize,overmax,
                    feedback,obj);

   for (i=1; i<VOLCNT; i++)
      {
      if (feedback!=NULL) feedback("calculating mipmap",(float)(i+1)/VOLCNT,obj);

      VOL[i]=new volume(TFUNC,BASE);

      data2=reduce(data,width,height,depth);
      extra2=reduce(extra,width,height,depth);

      width/=2;
      height/=2;
      depth/=2;

      bricksize/=2;
      overmax/=2.0f;

      VOL[i]->set_data(data2,
                       extra2,
                       width,height,depth,
                       mx,my,mz,
                       sx,sy,sz,
                       bricksize,overmax);

      if (i>1)
         {
         free(data);
         if (extra!=NULL) free(extra);
         }

      data=data2;
      extra=extra2;
      }

   if (VOLCNT>1)
      {
      free(data);
      if (extra!=NULL) free(extra);
      }
   }

// swap the volume along the axis
unsigned char *mipmap::swap(unsigned char *data,
                            long long *width,long long *height,long long *depth,
                            float *dsx,float *dsy,float *dsz,
                            BOOLINT xswap,BOOLINT yswap,BOOLINT zswap,
                            BOOLINT xrotate,BOOLINT zrotate)
   {
   long long i,j,k;

   unsigned char *data2,*ptr;

   long long size;
   float dim;

   if (xrotate)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*width; j++)
            for (i=0; i<*height; i++)
               *ptr++=data[j+(i+k*(*height))*(*width)];

      size=*width;
      *width=*height;
      *height=size;

      if (dsx!=NULL && dsy!=NULL && dsz!=NULL)
         {
         dim=*dsx;
         *dsx=*dsy;
         *dsy=dim;
         }

      free(data);
      data=data2;
      }

   if (zrotate)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*height; k++)
         for (j=0; j<*depth; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[i+(k+j*(*height))*(*width)];

      size=*height;
      *height=*depth;
      *depth=size;

      if (dsx!=NULL && dsy!=NULL && dsz!=NULL)
         {
         dim=*dsy;
         *dsy=*dsz;
         *dsz=dim;
         }

      free(data);
      data=data2;
      }

   if (xswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*height; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[*width-1-i+(j+k*(*height))*(*width)];

      free(data);
      data=data2;
      }

   if (yswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<*depth; k++)
         for (j=0; j<*height; j++)
            for (i=0; i<*width; i++)
               *ptr++=data[i+(*height-1-j+k*(*height))*(*width)];

      free(data);
      data=data2;
      }

   if (zswap)
      {
      if ((data2=(unsigned char *)malloc((*width)*(*height)*(*depth)))==NULL) ERRORMSG();

      for (ptr=data2,k=0; k<(*depth); k++)
         for (j=0; j<(*height); j++)
            for (i=0; i<(*width); i++)
               *ptr++=data[i+(j+(*depth-1-k)*(*height))*(*width)];

      free(data);
      data=data2;
      }

   return(data);
   }

// cache a row of slices
void mipmap::cache(const unsigned char *data,
                   long long width,long long height,long long depth,
                   long long slice,long long slices)
   {
   long long i;

   if (slices!=CSLICES)
      {
      if (CACHE!=NULL) delete CACHE;
      CACHE=NULL;

      if (slices>0)
         {
         CACHE=new unsigned char[width*height*slices];

         for (i=0; i<slices; i++)
            if (slice+i>=0 && slice+i<(long long)depth)
               memcpy(&CACHE[width*height*i],&data[width*height*(slice+i)],width*height);
         }

      CSIZEX=width;
      CSIZEY=height;
      CSLICE=slice;
      CSLICES=slices;
      }

   if (CSLICES>0)
      {
      if (slice>CSLICE)
         for (i=0; i<CSLICES; i++)
            if (slice+i>=0 && slice+i<(long long)depth)
               if (slice+i>=CSLICE && slice+i<CSLICE+CSLICES)
                  memcpy(&CACHE[CSIZEX*CSIZEY*i],&CACHE[CSIZEX*CSIZEY*(i+slice-CSLICE)],CSIZEX*CSIZEY);

      if (slice<CSLICE)
         for (i=CSLICES-1; i>=0; i--)
            if (slice+i>=0 && slice+i<(long long)depth)
               if (slice+i>=CSLICE && slice+i<CSLICE+CSLICES)
                  memcpy(&CACHE[CSIZEX*CSIZEY*i],&CACHE[CSIZEX*CSIZEY*(i+CSLICE-slice)],CSIZEX*CSIZEY);

      for (i=0; i<CSLICES; i++)
         if (slice+i>=0 && slice+i<(long long)depth)
            if (slice+i<CSLICE || slice+i>=CSLICE+CSLICES)
               memcpy(&CACHE[CSIZEX*CSIZEY*i],&data[CSIZEX*CSIZEY*(slice+i)],CSIZEX*CSIZEY);

      CSLICE=slice;
      }
   }

inline unsigned char mipmap::get(const unsigned char *data,
                                 const long long width,const long long height,const long long depth,
                                 const long long x,const long long y,const long long z)
   {
   if (CACHE!=NULL)
      if ((int)z>=CSLICE && (long long)z<CSLICE+CSLICES)
         return(CACHE[x+(y+(z-CSLICE)*height)*width]);

   return(data[x+(y+z*height)*width]);
   }

inline void mipmap::set(unsigned char *data,
                        const long long width,const long long height,const long long depth,
                        const long long x,const long long y,const long long z,unsigned char v)
   {data[x+(y+z*height)*width]=v;}

// calculate the gradient magnitude
unsigned char *mipmap::calc_gradmag(unsigned char *data,
                                    long long width,long long height,long long depth,
                                    float dsx,float dsy,float dsz,
                                    float *gradmax,
                                    void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
#ifdef MULTILEVEL
   return(gradmagML(data,
                    width,height,depth,
                    dsx,dsy,dsz,
                    gradmax,
                    feedback,obj));
#else
   return(gradmag(data,
                  width,height,depth,
                  dsx,dsy,dsz,
                  gradmax,
                  feedback,obj));
#endif
   }

// calculate the gradient magnitude
unsigned char *mipmap::gradmag(unsigned char *data,
                               long long width,long long height,long long depth,
                               float dsx,float dsy,float dsz,
                               float *gradmax,
                               void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   static const float mingrad=0.1f;

   long long i,j,k;

   unsigned char *data2,*ptr;

   float gx,gy,gz;
   float gm,gmax;

   float minds;

   minds=fmin(dsx,fmin(dsy,dsz));

   dsx/=minds;
   dsy/=minds;
   dsz/=minds;

   if (dsx>4.0f) dsx=4.0f;
   else if (dsx>2.0f) dsx=2.0f;
   else if (dsx>1.0f) dsx=1.0f;

   if (dsy>4.0f) dsy=4.0f;
   else if (dsy>2.0f) dsy=2.0f;
   else if (dsy>1.0f) dsy=1.0f;

   if (dsz>4.0f) dsz=4.0f;
   else if (dsz>2.0f) dsz=2.0f;
   else if (dsz>1.0f) dsz=1.0f;

   dsx=1.0f/dsx;
   dsy=1.0f/dsy;
   dsz=1.0f/dsz;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (gmax=1.0f,k=0; k<depth; k+=2)
      {
      if (feedback!=NULL) feedback("calculating gradients",0.5f*(k+1)/depth,obj);

      for (j=0; j<height; j+=2)
         for (i=0; i<width; i+=2)
            {
            if (i>0)
               if (i<width-1) gx=(get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i-1,j,k))/2.0f;
               else gx=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i-1,j,k);
            else gx=get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i,j,k);

            if (j>0)
               if (j<height-1) gy=(get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j-1,k))/2.0f;
               else gy=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j-1,k);
            else gy=get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j,k);

            if (k>0)
               if (k<depth-1) gz=(get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k-1))/2.0f;
               else gz=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j,k-1);
            else gz=get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k);

            gm=fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz);
            if (gm>gmax) gmax=gm;
            }
      }

   for (ptr=data2,k=0; k<depth; k++)
      {
      if (feedback!=NULL) feedback("calculating gradients",0.5f*(k+1)/depth+0.5f,obj);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            if (i>0)
               if (i<width-1) gx=(get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i-1,j,k))/2.0f;
               else gx=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i-1,j,k);
            else gx=get(data,width,height,depth,i+1,j,k)-get(data,width,height,depth,i,j,k);

            if (j>0)
               if (j<height-1) gy=(get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j-1,k))/2.0f;
               else gy=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j-1,k);
            else gy=get(data,width,height,depth,i,j+1,k)-get(data,width,height,depth,i,j,k);

            if (k>0)
               if (k<depth-1) gz=(get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k-1))/2.0f;
               else gz=get(data,width,height,depth,i,j,k)-get(data,width,height,depth,i,j,k-1);
            else gz=get(data,width,height,depth,i,j,k+1)-get(data,width,height,depth,i,j,k);

            *ptr++=ftrc(255.0f*threshold(fsqrt(fmin((fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz))/gmax,1.0f)),mingrad)+0.5f);
            }
      }

   if (gradmax!=NULL) *gradmax=fsqrt(gmax)/255.0f;

   return(data2);
   }

inline float mipmap::getgrad(unsigned char *data,
                             long long width,long long height,long long depth,
                             long long i,long long j,long long k,
                             float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v;

   float gx,gy,gz;

   ptr=&data[i+(j+k*height)*width];
   v=*ptr;

   if (i>0) gx=v-ptr[-1];
   else gx=ptr[1]-v;

   if (j>0) gy=v-ptr[-width];
   else gy=ptr[width]-v;

   if (k>0) gz=v-ptr[-width*height];
   else gz=ptr[width*height]-v;

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::getgrad2(unsigned char *data,
                              long long width,long long height,long long depth,
                              long long i,long long j,long long k,
                              float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v;

   float gx,gy,gz;

   ptr=&data[i+(j+k*height)*width];
   v=*ptr;

   if (i>0)
      if (i<width-1) gx=0.5f*(ptr[1]-ptr[-1]);
      else gx=v-ptr[-1];
   else gx=ptr[1]-v;

   if (j>0)
      if (j<height-1) gy=0.5f*(ptr[width]-ptr[-width]);
      else gy=v-ptr[-width];
   else gy=ptr[width]-v;

   if (k>0)
      if (k<depth-1) gz=0.5f*(ptr[width*height]-ptr[-width*height]);
      else gz=v-ptr[-width*height];
   else gz=ptr[width*height]-v;

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::getsobel(unsigned char *data,
                              long long width,long long height,long long depth,
                              long long i,long long j,long long k,
                              float dsx,float dsy,float dsz)
   {
   unsigned char *ptr;
   int v0,v[27];

   float gx,gy,gz;

   if (i>0 && i<width-1 && j>0 && j<height-1 && k>0 && k<depth-1)
      {
      ptr=&data[i-1+(j-1+(k-1)*height)*width];

      v[0]=*ptr++;
      v[1]=3*(*ptr++);
      v[2]=*ptr;
      ptr+=width-2;
      v[3]=3*(*ptr++);
      v[4]=6*(*ptr++);
      v[5]=3*(*ptr);
      ptr+=width-2;
      v[6]=*ptr++;
      v[7]=3*(*ptr++);
      v[8]=*ptr;
      ptr+=width*height-2*width-2;
      v[9]=3*(*ptr++);
      v[10]=6*(*ptr++);
      v[11]=3*(*ptr);
      ptr+=width-2;
      v[12]=6*(*ptr++);
      v[13]=*ptr++;
      v[14]=6*(*ptr);
      ptr+=width-2;
      v[15]=3*(*ptr++);
      v[16]=6*(*ptr++);
      v[17]=3*(*ptr);
      ptr+=width*height-2*width-2;
      v[18]=*ptr++;
      v[19]=3*(*ptr++);
      v[20]=*ptr;
      ptr+=width-2;
      v[21]=3*(*ptr++);
      v[22]=6*(*ptr++);
      v[23]=3*(*ptr);
      ptr+=width-2;
      v[24]=*ptr++;
      v[25]=3*(*ptr++);
      v[26]=*ptr;

      gx=(-v[0]-v[3]-v[6]-v[9]-v[12]-v[15]-v[18]-v[21]-v[24]+v[2]+v[5]+v[8]+v[11]+v[14]+v[17]+v[20]+v[23]+v[26])/44.0f;
      gy=(-v[0]-v[1]-v[2]-v[9]-v[10]-v[11]-v[18]-v[19]-v[20]+v[6]+v[7]+v[8]+v[15]+v[16]+v[17]+v[24]+v[25]+v[26])/44.0f;
      gz=(-v[0]-v[1]-v[2]-v[3]-v[4]-v[5]-v[6]-v[7]-v[8]+v[18]+v[19]+v[20]+v[21]+v[22]+v[23]+v[24]+v[25]+v[26])/44.0f;
      }
   else
      {
      ptr=&data[i+(j+k*height)*width];
      v0=*ptr;

      if (i>0)
         if (i<width-1) gx=0.5f*(ptr[1]-ptr[-1]);
         else gx=v0-ptr[-1];
      else gx=ptr[1]-v0;

      if (j>0)
         if (j<height-1) gy=0.5f*(ptr[width]-ptr[-width]);
         else gy=v0-ptr[-width];
      else gy=ptr[width]-v0;

      if (k>0)
         if (k<depth-1) gz=0.5f*(ptr[width*height]-ptr[-width*height]);
         else gz=v0-ptr[-width*height];
      else gz=ptr[width*height]-v0;
      }

   return(fsqrt(fsqr(gx*dsx)+fsqr(gy*dsy)+fsqr(gz*dsz)));
   }

inline float mipmap::threshold(float x,float thres)
   {
   if (x>=thres) return(x);
   return(x*fexp(-3.0f*fsqr((thres-x)/thres)));
   }

// calculate the gradient magnitude with multi-level averaging
unsigned char *mipmap::gradmagML(unsigned char *data,
                                 long long width,long long height,long long depth,
                                 float dsx,float dsy,float dsz,
                                 float *gradmax,
                                 void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   static const int maxlevel=2;
   static const float mingrad=0.1f;

   long long i,j,k;

   unsigned short int *data2,*ptr2;
   unsigned short int *data3,*ptr3;
   unsigned char *data4,*ptr4;
   unsigned char *data5,*ptr5;

   long long width2,height2,depth2;

   float gscale,greci;
   float gm,gmax,gmax2;

   int level;
   float weight;

   float minds;

   minds=fmin(dsx,fmin(dsy,dsz));

   dsx/=minds;
   dsy/=minds;
   dsz/=minds;

   if (dsx>4.0f) dsx=4.0f;
   else if (dsx>2.0f) dsx=2.0f;
   else if (dsx>1.0f) dsx=1.0f;

   if (dsy>4.0f) dsy=4.0f;
   else if (dsy>2.0f) dsy=2.0f;
   else if (dsy>1.0f) dsy=1.0f;

   if (dsz>4.0f) dsz=4.0f;
   else if (dsz>2.0f) dsz=2.0f;
   else if (dsz>1.0f) dsz=1.0f;

   dsx=1.0f/dsx;
   dsy=1.0f/dsy;
   dsz=1.0f/dsz;

   if ((data2=(unsigned short int *)malloc(width*height*depth*sizeof(unsigned short int)))==NULL) ERRORMSG();

   gscale=65535.0f/(255.0f*fsqrt(dsx*dsx+dsy*dsy+dsz*dsz));
   greci=1.0f/gscale;

   gmax=0.0f;

   for (ptr2=data2,k=0; k<depth; k++)
      {
      if (feedback!=NULL) feedback("calculating gradients",0.5f*(k+1)/depth,obj);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
#ifndef SOBEL
            gm=getgrad(data,width,height,depth,i,j,k,dsx,dsy,dsz);
#else
            gm=getsobel(data,width,height,depth,i,j,k,dsx,dsy,dsz);
#endif
            if (gm>gmax) gmax=gm;

            *ptr2++=ftrc(gm*gscale+0.5f);
            }
      }

   if (gmax==0.0f) gmax=1.0f;

   for (ptr2=data2,k=0; k<depth; k++)
      {
      if (feedback!=NULL) feedback("calculating gradients",0.5f*(k+1)/depth+0.5f,obj);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            gm=*ptr2*greci/gmax;
            *ptr2++=ftrc(0.5f*65535.0f*threshold(gm,mingrad)+0.5f);
            }
      }

   width2=width;
   height2=height;
   depth2=depth;

   data4=ptr4=data;

   level=0;
   weight=1.0f;

   gmax2=0.0f;

   while (width2>5 && height2>5 && depth2>5 && level<maxlevel)
      {
      data5=reduce(data4,width2,height2,depth2,feedback,obj);
      if (data4!=data) free(data4);
      data4=data5;

      width2=width2/2;
      height2=height2/2;
      depth2=depth2/2;

      level++;
      weight*=0.5f;

      if ((data3=(unsigned short int *)malloc(width2*height2*depth2*sizeof(unsigned short int)))==NULL) ERRORMSG();

      gmax=0.0f;

      for (ptr3=data3,k=0; k<depth2; k++)
         {
         if (feedback!=NULL) feedback("calculating reduced gradients",(float)(k+1)/depth2,obj);

         for (j=0; j<height2; j++)
            for (i=0; i<width2; i++)
            {
#ifndef SOBEL
            gm=getgrad(data4,width2,height2,depth2,i,j,k,dsx,dsy,dsz);
#else
            gm=getsobel(data4,width2,height2,depth2,i,j,k,dsx,dsy,dsz);
#endif
            if (gm>gmax) gmax=gm;

            *ptr3++=ftrc(gm*gscale+0.5f);
            }
         }

      if (gmax==0.0f) gmax=1.0f;

      for (ptr2=data2,k=0; k<depth; k++)
         {
         if (feedback!=NULL) feedback("interpolating reduced gradients",(float)(k+1)/depth,obj);

         for (j=0; j<height; j++)
            for (i=0; i<width; i++)
               {
               gm=*ptr2/(0.5f*65535.0f);

               gm+=weight*threshold(getscalar(data3,width2,height2,depth2,
                                              (float)i/(width-1),(float)j/(height-1),(float)k/(depth-1))*greci/gmax,mingrad);

               if (gm>gmax2) gmax2=gm;

               *ptr2++=ftrc(0.5f*65535.0f*gm+0.5f);
               }
         }

      free(data3);
      }

   if (gmax2==0.0f) gmax2=1.0f;

   if (data4!=data) free(data4);

   if ((data5=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr2=data2,ptr5=data5,k=0; k<depth; k++)
      {
      if (feedback!=NULL) feedback("normalizing gradients",(float)(k+1)/depth,obj);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            gm=*ptr2++/(0.5f*65535.0f);
            *ptr5++=ftrc(255.0f*gm/gmax2+0.5f);
            }
      }

   free(data2);

   if (gradmax!=NULL) *gradmax=gmax2/255.0f;

   return(data5);
   }

// calculate the variance
unsigned char *mipmap::variance(unsigned char *data,
                                long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *data2,*ptr;

   int val,cnt;

   int dev,dmax;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (dmax=1,k=0; k<depth; k+=2)
      for (j=0; j<height; j+=2)
         for (i=0; i<width; i+=2)
            {
            // central voxel
            val=get(data,width,height,depth,i,j,k);
            dev=cnt=0;

            // x-axis
            if (i>0) {dev+=abs(get(data,width,height,depth,i-1,j,k)-val); cnt++;}
            if (i<width-1) {dev+=abs(get(data,width,height,depth,i+1,j,k)-val); cnt++;}

            // y-axis
            if (j>0) {dev+=abs(get(data,width,height,depth,i,j-1,k)-val); cnt++;}
            if (j<height-1) {dev+=abs(get(data,width,height,depth,i,j+1,k)-val); cnt++;}

            // z-axis
            if (k>0) {dev+=abs(get(data,width,height,depth,i,j,k-1)-val); cnt++;}
            if (k<depth-1) {dev+=abs(get(data,width,height,depth,i,j,k+1)-val); cnt++;}

            // xy-plane
            if (i>0 && j>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k)-val); cnt++;}
            if (i<width-1 && j>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k)-val); cnt++;}
            if (i>0 && j<height-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k)-val); cnt++;}
            if (i<width-1 && j<height-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k)-val); cnt++;}

            // xz-plane
            if (i>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j,k-1)-val); cnt++;}
            if (i<width-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j,k-1)-val); cnt++;}
            if (i>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j,k+1)-val); cnt++;}
            if (i<width-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j,k+1)-val); cnt++;}

            // yz-plane
            if (j>0 && k>0) {dev+=abs(get(data,width,height,depth,i,j-1,k-1)-val); cnt++;}
            if (j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i,j+1,k-1)-val); cnt++;}
            if (j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i,j-1,k+1)-val); cnt++;}
            if (j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i,j+1,k+1)-val); cnt++;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k-1)-val); cnt++;}
            if (i<width-1 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k-1)-val); cnt++;}
            if (i>0 && j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j+1,k-1)-val); cnt++;}
            if (i<width-1 && j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j+1,k-1)-val); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j-1,k+1)-val); cnt++;}
            if (i<width-1 && j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j-1,k+1)-val); cnt++;}
            if (i>0 && j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k+1)-val); cnt++;}
            if (i<width-1 && j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k+1)-val); cnt++;}

            dev=(dev+cnt/2)/cnt;
            if (dev>dmax) dmax=dev;
            }

   for (ptr=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            // central voxel
            val=get(data,width,height,depth,i,j,k);
            dev=cnt=0;

            // x-axis
            if (i>0) {dev+=abs(get(data,width,height,depth,i-1,j,k)-val); cnt++;}
            if (i<width-1) {dev+=abs(get(data,width,height,depth,i+1,j,k)-val); cnt++;}

            // y-axis
            if (j>0) {dev+=abs(get(data,width,height,depth,i,j-1,k)-val); cnt++;}
            if (j<height-1) {dev+=abs(get(data,width,height,depth,i,j+1,k)-val); cnt++;}

            // z-axis
            if (k>0) {dev+=abs(get(data,width,height,depth,i,j,k-1)-val); cnt++;}
            if (k<depth-1) {dev+=abs(get(data,width,height,depth,i,j,k+1)-val); cnt++;}

            // xy-plane
            if (i>0 && j>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k)-val); cnt++;}
            if (i<width-1 && j>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k)-val); cnt++;}
            if (i>0 && j<height-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k)-val); cnt++;}
            if (i<width-1 && j<height-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k)-val); cnt++;}

            // xz-plane
            if (i>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j,k-1)-val); cnt++;}
            if (i<width-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j,k-1)-val); cnt++;}
            if (i>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j,k+1)-val); cnt++;}
            if (i<width-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j,k+1)-val); cnt++;}

            // yz-plane
            if (j>0 && k>0) {dev+=abs(get(data,width,height,depth,i,j-1,k-1)-val); cnt++;}
            if (j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i,j+1,k-1)-val); cnt++;}
            if (j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i,j-1,k+1)-val); cnt++;}
            if (j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i,j+1,k+1)-val); cnt++;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j-1,k-1)-val); cnt++;}
            if (i<width-1 && j>0 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j-1,k-1)-val); cnt++;}
            if (i>0 && j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i-1,j+1,k-1)-val); cnt++;}
            if (i<width-1 && j<height-1 && k>0) {dev+=abs(get(data,width,height,depth,i+1,j+1,k-1)-val); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j-1,k+1)-val); cnt++;}
            if (i<width-1 && j>0 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j-1,k+1)-val); cnt++;}
            if (i>0 && j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i-1,j+1,k+1)-val); cnt++;}
            if (i<width-1 && j<height-1 && k<depth-1) {dev+=abs(get(data,width,height,depth,i+1,j+1,k+1)-val); cnt++;}

            dev=(dev+cnt/2)/cnt;
            *ptr++=ftrc(255.0f*fmin((float)dev/dmax,1.0f)+0.5f);
            }

   return(data2);
   }

// blur the volume
void mipmap::blur(unsigned char *data,
                  long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *ptr;

   int val,cnt;

   for (ptr=data,k=0; k<depth; k++)
      {
      cache(data,width,height,depth,k-1,2);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            // central voxel
            val=54*get(data,width,height,depth,i,j,k);
            cnt=54;

            // x-axis
            if (i>0) {val+=6*get(data,width,height,depth,i-1,j,k); cnt+=6;}
            if (i<width-1) {val+=6*get(data,width,height,depth,i+1,j,k); cnt+=6;}

            // y-axis
            if (j>0) {val+=6*get(data,width,height,depth,i,j-1,k); cnt+=6;}
            if (j<height-1) {val+=6*get(data,width,height,depth,i,j+1,k); cnt+=6;}

            // z-axis
            if (k>0) {val+=6*get(data,width,height,depth,i,j,k-1); cnt+=6;}
            if (k<depth-1) {val+=6*get(data,width,height,depth,i,j,k+1); cnt+=6;}

            // xy-plane
            if (i>0 && j>0) {val+=2*get(data,width,height,depth,i-1,j-1,k); cnt+=2;}
            if (i<width-1 && j>0) {val+=2*get(data,width,height,depth,i+1,j-1,k); cnt+=2;}
            if (i>0 && j<height-1) {val+=2*get(data,width,height,depth,i-1,j+1,k); cnt+=2;}
            if (i<width-1 && j<height-1) {val+=2*get(data,width,height,depth,i+1,j+1,k); cnt+=2;}

            // xz-plane
            if (i>0 && k>0) {val+=2*get(data,width,height,depth,i-1,j,k-1); cnt+=2;}
            if (i<width-1 && k>0) {val+=2*get(data,width,height,depth,i+1,j,k-1); cnt+=2;}
            if (i>0 && k<depth-1) {val+=2*get(data,width,height,depth,i-1,j,k+1); cnt+=2;}
            if (i<width-1 && k<depth-1) {val+=2*get(data,width,height,depth,i+1,j,k+1); cnt+=2;}

            // yz-plane
            if (j>0 && k>0) {val+=2*get(data,width,height,depth,i,j-1,k-1); cnt+=2;}
            if (j<height-1 && k>0) {val+=2*get(data,width,height,depth,i,j+1,k-1); cnt+=2;}
            if (j>0 && k<depth-1) {val+=2*get(data,width,height,depth,i,j-1,k+1); cnt+=2;}
            if (j<height-1 && k<depth-1) {val+=2*get(data,width,height,depth,i,j+1,k+1); cnt+=2;}

            // bottom xy-plane
            if (i>0 && j>0 && k>0) {val+=get(data,width,height,depth,i-1,j-1,k-1); cnt++;}
            if (i<width-1 && j>0 && k>0) {val+=get(data,width,height,depth,i+1,j-1,k-1); cnt++;}
            if (i>0 && j<height-1 && k>0) {val+=get(data,width,height,depth,i-1,j+1,k-1); cnt++;}
            if (i<width-1 && j<height-1 && k>0) {val+=get(data,width,height,depth,i+1,j+1,k-1); cnt++;}

            // top xy-plane
            if (i>0 && j>0 && k<depth-1) {val+=get(data,width,height,depth,i-1,j-1,k+1); cnt++;}
            if (i<width-1 && j>0 && k<depth-1) {val+=get(data,width,height,depth,i+1,j-1,k+1); cnt++;}
            if (i>0 && j<height-1 && k<depth-1) {val+=get(data,width,height,depth,i-1,j+1,k+1); cnt++;}
            if (i<width-1 && j<height-1 && k<depth-1) {val+=get(data,width,height,depth,i+1,j+1,k+1); cnt++;}

            *ptr++=(val+cnt/2)/cnt;
            }
      }

   cache();
   }

// set gradient to maximum where transfer function is transparent
void mipmap::usetf(unsigned char *data,unsigned char *grad,
                   long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *ptr1,*ptr2;

   int mindata,maxdata;

   int val,nbg[26];

   int oldmode;

   oldmode=TFUNC->get_mode();

   if (!TFUNC->get_imode()) TFUNC->set_mode(0);

   TFUNC->preint(TRUE);

   for (ptr1=data,ptr2=grad,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++,ptr1++,ptr2++)
            {
            // central voxel
            mindata=maxdata=*ptr1;

            // x-axis:

            if (i>0) nbg[0]=get(data,width,height,depth,i-1,j,k); else nbg[0]=*ptr1;
            if (i<width-1) nbg[1]=get(data,width,height,depth,i+1,j,k); else nbg[1]=*ptr1;

            val=(*ptr1)+nbg[0]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[1]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // y-axis:

            if (j>0) nbg[2]=get(data,width,height,depth,i,j-1,k); else nbg[2]=*ptr1;
            if (j<height-1) nbg[3]=get(data,width,height,depth,i,j+1,k); else nbg[3]=*ptr1;

            val=(*ptr1)+nbg[2]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[3]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // z-axis:

            if (k>0) nbg[4]=get(data,width,height,depth,i,j,k-1); else nbg[4]=*ptr1;
            if (k<depth-1) nbg[5]=get(data,width,height,depth,i,j,k+1); else nbg[5]=*ptr1;

            val=(*ptr1)+nbg[4]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[5]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // xy-plane:

            if (i>0 && j>0) nbg[6]=get(data,width,height,depth,i-1,j-1,k); else nbg[6]=*ptr1;
            if (i<width-1 && j>0) nbg[7]=get(data,width,height,depth,i+1,j-1,k); else nbg[7]=*ptr1;
            if (i>0 && j<height-1) nbg[8]=get(data,width,height,depth,i-1,j+1,k); else nbg[8]=*ptr1;
            if (i<width-1 && j<height-1) nbg[9]=get(data,width,height,depth,i+1,j+1,k); else nbg[9]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // xz-plane:

            if (i>0 && k>0) nbg[10]=get(data,width,height,depth,i-1,j,k-1); else nbg[10]=*ptr1;
            if (i<width-1 && k>0) nbg[11]=get(data,width,height,depth,i+1,j,k-1); else nbg[11]=*ptr1;
            if (i>0 && k<depth-1) nbg[12]=get(data,width,height,depth,i-1,j,k+1); else nbg[12]=*ptr1;
            if (i<width-1 && k<depth-1) nbg[13]=get(data,width,height,depth,i+1,j,k+1); else nbg[13]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[4]+nbg[10]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[4]+nbg[11]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[5]+nbg[12]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[5]+nbg[13]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // yz-plane:

            if (j>0 && k>0) nbg[14]=get(data,width,height,depth,i,j-1,k-1); else nbg[14]=*ptr1;
            if (j<height-1 && k>0) nbg[15]=get(data,width,height,depth,i,j+1,k-1); else nbg[15]=*ptr1;
            if (j>0 && k<depth-1) nbg[16]=get(data,width,height,depth,i,j-1,k+1); else nbg[16]=*ptr1;
            if (j<height-1 && k<depth-1) nbg[17]=get(data,width,height,depth,i,j+1,k+1); else nbg[17]=*ptr1;

            val=(*ptr1)+nbg[2]+nbg[4]+nbg[14]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[4]+nbg[15]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[2]+nbg[5]+nbg[16]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[5]+nbg[17]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // bottom xy-plane:

            if (i>0 && j>0 && k>0) nbg[18]=get(data,width,height,depth,i-1,j-1,k-1); else nbg[18]=*ptr1;
            if (i<width-1 && j>0 && k>0) nbg[19]=get(data,width,height,depth,i+1,j-1,k-1); else nbg[19]=*ptr1;
            if (i>0 && j<height-1 && k>0) nbg[20]=get(data,width,height,depth,i-1,j+1,k-1); else nbg[20]=*ptr1;
            if (i<width-1 && j<height-1 && k>0) nbg[21]=get(data,width,height,depth,i+1,j+1,k-1); else nbg[21]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[4]+nbg[10]+nbg[14]+nbg[18]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[4]+nbg[11]+nbg[14]+nbg[19]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[4]+nbg[10]+nbg[15]+nbg[20]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[4]+nbg[11]+nbg[15]+nbg[21]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // top xy-plane:

            if (i>0 && j>0 && k<depth-1) nbg[22]=get(data,width,height,depth,i-1,j-1,k+1); else nbg[22]=*ptr1;
            if (i<width-1 && j>0 && k<depth-1) nbg[23]=get(data,width,height,depth,i+1,j-1,k+1); else nbg[23]=*ptr1;
            if (i>0 && j<height-1 && k<depth-1) nbg[24]=get(data,width,height,depth,i-1,j+1,k+1); else nbg[24]=*ptr1;
            if (i<width-1 && j<height-1 && k<depth-1) nbg[25]=get(data,width,height,depth,i+1,j+1,k+1); else nbg[25]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[5]+nbg[12]+nbg[16]+nbg[22]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[5]+nbg[13]+nbg[16]+nbg[23]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[5]+nbg[12]+nbg[17]+nbg[24]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[5]+nbg[13]+nbg[17]+nbg[25]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // check for transparency
            if (TFUNC->zot(mindata/255.0f,maxdata/255.0f,(*ptr2)/255.0f,(*ptr2)/255.0f)) *ptr2=255;
            else *ptr2/=2;
            }

   TFUNC->set_mode(oldmode);
   }

// set gradient to maximum where opacity is zero
void mipmap::useop(unsigned char *data,unsigned char *grad,
                   long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *ptr1,*ptr2;

   int mindata,maxdata;

   int val,nbg[26];

   int oldmode;

   oldmode=TFUNC->get_mode();

   if (!TFUNC->get_imode()) TFUNC->set_mode(0);

   TFUNC->premin();

   for (ptr1=data,ptr2=grad,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++,ptr1++,ptr2++)
            {
            // central voxel
            mindata=maxdata=*ptr1;

            // x-axis:

            if (i>0) nbg[0]=get(data,width,height,depth,i-1,j,k); else nbg[0]=*ptr1;
            if (i<width-1) nbg[1]=get(data,width,height,depth,i+1,j,k); else nbg[1]=*ptr1;

            val=(*ptr1)+nbg[0]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[1]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // y-axis:

            if (j>0) nbg[2]=get(data,width,height,depth,i,j-1,k); else nbg[2]=*ptr1;
            if (j<height-1) nbg[3]=get(data,width,height,depth,i,j+1,k); else nbg[3]=*ptr1;

            val=(*ptr1)+nbg[2]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[3]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // z-axis:

            if (k>0) nbg[4]=get(data,width,height,depth,i,j,k-1); else nbg[4]=*ptr1;
            if (k<depth-1) nbg[5]=get(data,width,height,depth,i,j,k+1); else nbg[5]=*ptr1;

            val=(*ptr1)+nbg[4]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;
            val=(*ptr1)+nbg[5]; if (val/2<mindata) mindata=val/2; else if ((val+1)/2>maxdata) maxdata=(val+1)/2;

            // xy-plane:

            if (i>0 && j>0) nbg[6]=get(data,width,height,depth,i-1,j-1,k); else nbg[6]=*ptr1;
            if (i<width-1 && j>0) nbg[7]=get(data,width,height,depth,i+1,j-1,k); else nbg[7]=*ptr1;
            if (i>0 && j<height-1) nbg[8]=get(data,width,height,depth,i-1,j+1,k); else nbg[8]=*ptr1;
            if (i<width-1 && j<height-1) nbg[9]=get(data,width,height,depth,i+1,j+1,k); else nbg[9]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // xz-plane:

            if (i>0 && k>0) nbg[10]=get(data,width,height,depth,i-1,j,k-1); else nbg[10]=*ptr1;
            if (i<width-1 && k>0) nbg[11]=get(data,width,height,depth,i+1,j,k-1); else nbg[11]=*ptr1;
            if (i>0 && k<depth-1) nbg[12]=get(data,width,height,depth,i-1,j,k+1); else nbg[12]=*ptr1;
            if (i<width-1 && k<depth-1) nbg[13]=get(data,width,height,depth,i+1,j,k+1); else nbg[13]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[4]+nbg[10]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[4]+nbg[11]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[0]+nbg[5]+nbg[12]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[1]+nbg[5]+nbg[13]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // yz-plane:

            if (j>0 && k>0) nbg[14]=get(data,width,height,depth,i,j-1,k-1); else nbg[14]=*ptr1;
            if (j<height-1 && k>0) nbg[15]=get(data,width,height,depth,i,j+1,k-1); else nbg[15]=*ptr1;
            if (j>0 && k<depth-1) nbg[16]=get(data,width,height,depth,i,j-1,k+1); else nbg[16]=*ptr1;
            if (j<height-1 && k<depth-1) nbg[17]=get(data,width,height,depth,i,j+1,k+1); else nbg[17]=*ptr1;

            val=(*ptr1)+nbg[2]+nbg[4]+nbg[14]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[4]+nbg[15]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[2]+nbg[5]+nbg[16]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;
            val=(*ptr1)+nbg[3]+nbg[5]+nbg[17]; if (val/4<mindata) mindata=val/4; else if ((val+3)/4>maxdata) maxdata=(val+3)/4;

            // bottom xy-plane:

            if (i>0 && j>0 && k>0) nbg[18]=get(data,width,height,depth,i-1,j-1,k-1); else nbg[18]=*ptr1;
            if (i<width-1 && j>0 && k>0) nbg[19]=get(data,width,height,depth,i+1,j-1,k-1); else nbg[19]=*ptr1;
            if (i>0 && j<height-1 && k>0) nbg[20]=get(data,width,height,depth,i-1,j+1,k-1); else nbg[20]=*ptr1;
            if (i<width-1 && j<height-1 && k>0) nbg[21]=get(data,width,height,depth,i+1,j+1,k-1); else nbg[21]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[4]+nbg[10]+nbg[14]+nbg[18]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[4]+nbg[11]+nbg[14]+nbg[19]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[4]+nbg[10]+nbg[15]+nbg[20]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[4]+nbg[11]+nbg[15]+nbg[21]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // top xy-plane:

            if (i>0 && j>0 && k<depth-1) nbg[22]=get(data,width,height,depth,i-1,j-1,k+1); else nbg[22]=*ptr1;
            if (i<width-1 && j>0 && k<depth-1) nbg[23]=get(data,width,height,depth,i+1,j-1,k+1); else nbg[23]=*ptr1;
            if (i>0 && j<height-1 && k<depth-1) nbg[24]=get(data,width,height,depth,i-1,j+1,k+1); else nbg[24]=*ptr1;
            if (i<width-1 && j<height-1 && k<depth-1) nbg[25]=get(data,width,height,depth,i+1,j+1,k+1); else nbg[25]=*ptr1;

            val=(*ptr1)+nbg[0]+nbg[2]+nbg[6]+nbg[5]+nbg[12]+nbg[16]+nbg[22]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[2]+nbg[7]+nbg[5]+nbg[13]+nbg[16]+nbg[23]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[0]+nbg[3]+nbg[8]+nbg[5]+nbg[12]+nbg[17]+nbg[24]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;
            val=(*ptr1)+nbg[1]+nbg[3]+nbg[9]+nbg[5]+nbg[13]+nbg[17]+nbg[25]; if (val/8<mindata) mindata=val/8; else if ((val+7)/8>maxdata) maxdata=(val+7)/8;

            // check for transparency
            if (TFUNC->mot(mindata/255.0f,maxdata/255.0f,(*ptr2)/255.0f,(*ptr2)/255.0f)) *ptr2=255;
            else *ptr2/=2;
            }

   TFUNC->set_mode(oldmode);
   }

// remove bubbles
void mipmap::remove(unsigned char *grad,
                    long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *ptr;

   int nbg,cnt;

   for (ptr=grad,k=0; k<depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++,ptr++)
            if (*ptr==255)
               {
               nbg=cnt=0;

               if (i>0) {if (get(grad,width,height,depth,i-1,j,k)==255) nbg++; cnt++;}
               if (i<width-1) {if (get(grad,width,height,depth,i+1,j,k)==255) nbg++; cnt++;}

               if (j>0) {if (get(grad,width,height,depth,i,j-1,k)==255) nbg++; cnt++;}
               if (j<height-1) {if (get(grad,width,height,depth,i,j+1,k)==255) nbg++; cnt++;}

               if (k>0) {if (get(grad,width,height,depth,i,j,k-1)==255) nbg++; cnt++;}
               if (k<depth-1) {if (get(grad,width,height,depth,i,j,k+1)==255) nbg++; cnt++;}

               if (nbg==cnt) *ptr=0;
               }
      }

   cache();
   }

// tangle material
void mipmap::tangle(unsigned char *grad,
                    long long width,long long height,long long depth)
   {
   long long i,j,k;

   unsigned char *ptr;

   int cnt;

   for (ptr=grad,k=0; k<depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++,ptr++)
            if (*ptr==255)
               {
               cnt=0;

               // x-axis
               if (i>0) if (get(grad,width,height,depth,i-1,j,k)<255) cnt++;
               if (i<width-1) if (get(grad,width,height,depth,i+1,j,k)<255) cnt++;

               // y-axis
               if (j>0) if (get(grad,width,height,depth,i,j-1,k)<255) cnt++;
               if (j<height-1) if (get(grad,width,height,depth,i,j+1,k)<255) cnt++;

               // z-axis
               if (k>0) if (get(grad,width,height,depth,i,j,k-1)<255) cnt++;
               if (k<depth-1) if (get(grad,width,height,depth,i,j,k+1)<255) cnt++;

               // xy-plane
               if (i>0 && j>0) if (get(grad,width,height,depth,i-1,j-1,k)<255) cnt++;
               if (i<width-1 && j>0) if (get(grad,width,height,depth,i+1,j-1,k)<255) cnt++;
               if (i>0 && j<height-1) if (get(grad,width,height,depth,i-1,j+1,k)<255) cnt++;
               if (i<width-1 && j<height-1) if (get(grad,width,height,depth,i+1,j+1,k)<255) cnt++;

               // xz-plane
               if (i>0 && k>0) if (get(grad,width,height,depth,i-1,j,k-1)<255) cnt++;
               if (i<width-1 && k>0) if (get(grad,width,height,depth,i+1,j,k-1)<255) cnt++;
               if (i>0 && k<depth-1) if (get(grad,width,height,depth,i-1,j,k+1)<255) cnt++;
               if (i<width-1 && k<depth-1) if (get(grad,width,height,depth,i+1,j,k+1)<255) cnt++;

               // yz-plane
               if (j>0 && k>0) if (get(grad,width,height,depth,i,j-1,k-1)<255) cnt++;
               if (j<height-1 && k>0) if (get(grad,width,height,depth,i,j+1,k-1)<255) cnt++;
               if (j>0 && k<depth-1) if (get(grad,width,height,depth,i,j-1,k+1)<255) cnt++;
               if (j<height-1 && k<depth-1) if (get(grad,width,height,depth,i,j+1,k+1)<255) cnt++;

               // bottom xy-plane
               if (i>0 && j>0 && k>0) if (get(grad,width,height,depth,i-1,j-1,k-1)<255) cnt++;
               if (i<width-1 && j>0 && k>0) if (get(grad,width,height,depth,i+1,j-1,k-1)<255) cnt++;
               if (i>0 && j<height-1 && k>0) if (get(grad,width,height,depth,i-1,j+1,k-1)<255) cnt++;
               if (i<width-1 && j<height-1 && k>0) if (get(grad,width,height,depth,i+1,j+1,k-1)<255) cnt++;

               // top xy-plane
               if (i>0 && j>0 && k<depth-1) if (get(grad,width,height,depth,i-1,j-1,k+1)<255) cnt++;
               if (i<width-1 && j>0 && k<depth-1) if (get(grad,width,height,depth,i+1,j-1,k+1)<255) cnt++;
               if (i>0 && j<height-1 && k<depth-1) if (get(grad,width,height,depth,i-1,j+1,k+1)<255) cnt++;
               if (i<width-1 && j<height-1 && k<depth-1) if (get(grad,width,height,depth,i+1,j+1,k+1)<255) cnt++;

               if (cnt>0) *ptr=0;
               }
      }

   cache();
   }

// grow material
long long mipmap::grow(unsigned char *grad,
                       long long width,long long height,long long depth)
   {
   long long i,j,k;
   int v,c;

   unsigned char *ptr;

   long long cnt,maxcnt;

   int val,nbg[26];

   unsigned int found;

   found=0;

   for (ptr=grad,k=0; k<depth; k++)
      {
      cache(grad,width,height,depth,k-1,2);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++,ptr++)
            if (*ptr==0)
               {
               cnt=0;

               // x-axis
               if (i>0) {val=get(grad,width,height,depth,i-1,j,k); if (val>0) cnt++; nbg[0]=val;} else nbg[0]=0;
               if (i<width-1) {val=get(grad,width,height,depth,i+1,j,k); if (val>0) cnt++; nbg[1]=val;} else nbg[1]=0;

               // y-axis
               if (j>0) {val=get(grad,width,height,depth,i,j-1,k); if (val>0) cnt++; nbg[2]=val;} else nbg[2]=0;
               if (j<height-1) {val=get(grad,width,height,depth,i,j+1,k); if (val>0) cnt++; nbg[3]=val;} else nbg[3]=0;

               // z-axis
               if (k>0) {val=get(grad,width,height,depth,i,j,k-1); if (val>0) cnt++; nbg[4]=val;} else nbg[4]=0;
               if (k<depth-1) {val=get(grad,width,height,depth,i,j,k+1); if (val>0) cnt++; nbg[5]=val;} else nbg[5]=0;

               // xy-plane
               if (i>0 && j>0) {val=get(grad,width,height,depth,i-1,j-1,k); if (val>0) cnt++; nbg[6]=val;} else nbg[6]=0;
               if (i<width-1 && j>0) {val=get(grad,width,height,depth,i+1,j-1,k); if (val>0) cnt++; nbg[7]=val;} else nbg[7]=0;
               if (i>0 && j<height-1) {val=get(grad,width,height,depth,i-1,j+1,k); if (val>0) cnt++; nbg[8]=val;} else nbg[8]=0;
               if (i<width-1 && j<height-1) {val=get(grad,width,height,depth,i+1,j+1,k); if (val>0) cnt++; nbg[9]=val;} else nbg[9]=0;

               // xz-plane
               if (i>0 && k>0) {val=get(grad,width,height,depth,i-1,j,k-1); if (val>0) cnt++; nbg[10]=val;} else nbg[10]=0;
               if (i<width-1 && k>0) {val=get(grad,width,height,depth,i+1,j,k-1); if (val>0) cnt++; nbg[11]=val;} else nbg[11]=0;
               if (i>0 && k<depth-1) {val=get(grad,width,height,depth,i-1,j,k+1); if (val>0) cnt++; nbg[12]=val;} else nbg[12]=0;
               if (i<width-1 && k<depth-1) {val=get(grad,width,height,depth,i+1,j,k+1); if (val>0) cnt++; nbg[13]=val;} else nbg[13]=0;

               // yz-plane
               if (j>0 && k>0) {val=get(grad,width,height,depth,i,j-1,k-1); if (val>0) cnt++; nbg[14]=val;} else nbg[14]=0;
               if (j<height-1 && k>0) {val=get(grad,width,height,depth,i,j+1,k-1); if (val>0) cnt++; nbg[15]=val;} else nbg[15]=0;
               if (j>0 && k<depth-1) {val=get(grad,width,height,depth,i,j-1,k+1); if (val>0) cnt++; nbg[16]=val;} else nbg[16]=0;
               if (j<height-1 && k<depth-1) {val=get(grad,width,height,depth,i,j+1,k+1); if (val>0) cnt++; nbg[17]=val;} else nbg[17]=0;

               // bottom xy-plane
               if (i>0 && j>0 && k>0) {val=get(grad,width,height,depth,i-1,j-1,k-1); if (val>0) cnt++; nbg[18]=val;} else nbg[18]=0;
               if (i<width-1 && j>0 && k>0) {val=get(grad,width,height,depth,i+1,j-1,k-1); if (val>0) cnt++; nbg[19]=val;} else nbg[19]=0;
               if (i>0 && j<height-1 && k>0) {val=get(grad,width,height,depth,i-1,j+1,k-1); if (val>0) cnt++; nbg[20]=val;} else nbg[20]=0;
               if (i<width-1 && j<height-1 && k>0) {val=get(grad,width,height,depth,i+1,j+1,k-1); if (val>0) cnt++; nbg[21]=val;} else nbg[21]=0;

               // top xy-plane
               if (i>0 && j>0 && k<depth-1) {val=get(grad,width,height,depth,i-1,j-1,k+1); if (val>0) cnt++; nbg[22]=val;} else nbg[22]=0;
               if (i<width-1 && j>0 && k<depth-1) {val=get(grad,width,height,depth,i+1,j-1,k+1); if (val>0) cnt++; nbg[23]=val;} else nbg[23]=0;
               if (i>0 && j<height-1 && k<depth-1) {val=get(grad,width,height,depth,i-1,j+1,k+1); if (val>0) cnt++; nbg[24]=val;} else nbg[24]=0;
               if (i<width-1 && j<height-1 && k<depth-1) {val=get(grad,width,height,depth,i+1,j+1,k+1); if (val>0) cnt++; nbg[25]=val;} else nbg[25]=0;

               if (cnt>0)
                  {
                  val=0;
                  maxcnt=0;

                  for (v=0; v<26; v++)
                     if (nbg[v]>0)
                        {
                        cnt=1;

                        for (c=v+1; c<26; c++)
                           if (nbg[c]==nbg[v])
                              {
                              cnt++;
                              nbg[c]=0;
                              }

                        if (cnt>maxcnt)
                           {
                           maxcnt=cnt;
                           val=nbg[v];
                           }
                        }

                  *ptr=val;
                  found++;
                  }
               }
      }

   cache();

   return(found);
   }

// floodfill a segment of the volume based on scalar value
long long mipmap::floodfill(const unsigned char *data,unsigned char *mark,
                            const long long width,const long long height,const long long depth,
                            const long long x,const long long y,const long long z,
                            const int value,const int maxdev,
                            const int token)
   {
   unsigned int i;
   long long cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)==token) continue;
      if (abs(get(data,width,height,depth,xs,ys,zs)-value)>maxdev) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt++;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// floodfill a segment of the volume counting space
double mipmap::countfill(const unsigned char *data,unsigned char *mark,
                         const long long width,const long long height,const long long depth,
                         const long long x,const long long y,const long long z,
                         const int value,const int maxdev,
                         const int token)
   {
   unsigned int i;
   double cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0.0;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)==token) continue;
      if (abs(get(data,width,height,depth,xs,ys,zs)-value)>maxdev) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt+=1.0-get(data,width,height,depth,xs,ys,zs)/255.0;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// floodfill a segment of the volume based on gradient magnitude
long long mipmap::gradfill(const unsigned char *grad,unsigned char *mark,
                           const long long width,const long long height,const long long depth,
                           const long long x,const long long y,const long long z,
                           const int token,const int maxgrad)
   {
   unsigned int i;
   long long cnt;

   int xs,ys,zs;
   int *qx,*qy,*qz;

   cnt=0;

   QUEUECNT=1;
   QUEUESTART=0;
   QUEUEEND=1;

   QUEUEX[QUEUESTART]=x;
   QUEUEY[QUEUESTART]=y;
   QUEUEZ[QUEUESTART]=z;

   while (QUEUECNT>0)
      {
      xs=QUEUEX[QUEUESTART];
      ys=QUEUEY[QUEUESTART];
      zs=QUEUEZ[QUEUESTART];

      QUEUESTART++;
      QUEUESTART%=QUEUEMAX;
      QUEUECNT--;

      if (get(mark,width,height,depth,xs,ys,zs)!=0) continue;
      if (get(grad,width,height,depth,xs,ys,zs)>=maxgrad) continue;

      set(mark,width,height,depth,xs,ys,zs,token);
      cnt++;

      if (QUEUECNT+6>QUEUEMAX)
         {
         qx=new int[QUEUEMAX+QUEUEINC];
         qy=new int[QUEUEMAX+QUEUEINC];
         qz=new int[QUEUEMAX+QUEUEINC];

         for (i=0; i<QUEUECNT; i++)
            {
            qx[i]=QUEUEX[QUEUESTART];
            qy[i]=QUEUEY[QUEUESTART];
            qz[i]=QUEUEZ[QUEUESTART];

            QUEUESTART++;
            QUEUESTART%=QUEUEMAX;
            }

         delete QUEUEX;
         delete QUEUEY;
         delete QUEUEZ;

         QUEUEX=qx;
         QUEUEY=qy;
         QUEUEZ=qz;

         QUEUEMAX+=QUEUEINC;

         QUEUESTART=0;
         QUEUEEND=QUEUECNT;
         }

      if (xs>0)
         {
         QUEUEX[QUEUEEND]=xs-1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (xs<width-1)
         {
         QUEUEX[QUEUEEND]=xs+1;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys-1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (ys<height-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys+1;
         QUEUEZ[QUEUEEND]=zs;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs>0)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs-1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }

      if (zs<depth-1)
         {
         QUEUEX[QUEUEEND]=xs;
         QUEUEY[QUEUEEND]=ys;
         QUEUEZ[QUEUEEND]=zs+1;

         QUEUEEND++;
         QUEUEEND%=QUEUEMAX;
         QUEUECNT++;
         }
      }

   return(cnt);
   }

// classify the volume by the segment size
unsigned char *mipmap::sizify(unsigned char *data,
                              long long width,long long height,long long depth,
                              float maxdev)
   {
   long long i,j,k;

   unsigned char *data2,*ptr2;

   long long size,maxsize;
   int maxd;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr2=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++) *ptr2++=0;

   maxsize=1;
   maxd=ftrc(255.0f*maxdev+0.5f);

   for (k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            if (get(data2,width,height,depth,i,j,k)==0)
               {
               size=floodfill(data,data2,
                              width,height,depth,
                              i,j,k,get(data,width,height,depth,i,j,k),
                              maxd,1);

               if (size>maxsize) maxsize=size;
               }

   for (ptr2=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++) *ptr2++=0;

   for (k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            if (get(data2,width,height,depth,i,j,k)==0)
               {
               size=floodfill(data,data2,
                              width,height,depth,
                              i,j,k,get(data,width,height,depth,i,j,k),
                              maxd,1);

               size=ftrc(255.0f*(1.0f-fpow((float)size/maxsize,1.0f/3))+0.5f);
               if (size==0) size=1;

               floodfill(data,data2,
                         width,height,depth,
                         i,j,k,get(data,width,height,depth,i,j,k),
                         maxd,size);
               }

   return(data2);
   }

// classify the volume by gradient border
unsigned char *mipmap::classify(unsigned char *grad,
                                long long width,long long height,long long depth,
                                float maxgrad,
                                unsigned int *classes)
   {
   const int stepping=71;

   long long i,j,k;

   unsigned char *data2,*ptr;

   int token;
   int maxg;

   unsigned int cnt;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++) *ptr++=0;

   token=128;
   maxg=ftrc(255.0f*maxgrad+0.5f);
   cnt=0;

   for (ptr=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            if (*ptr++==0)
               if (get(grad,width,height,depth,i,j,k)<maxg)
                  {
                  if (token==0) token=(token+stepping)%256;

                  gradfill(grad,data2,
                           width,height,depth,
                           i,j,k,token,maxg);

                  token=(token+stepping)%256;
                  cnt++;
                  }

   if (classes!=NULL) *classes=cnt;

   return(data2);
   }

// zero space
void mipmap::zero(unsigned char *data,unsigned char *grad,
                  long long width,long long height,long long depth,
                  float maxdev)
   {
   long long i,j,k;

   unsigned char *data2,*ptr;

   int maxd;

   double cnt,maxcnt;
   unsigned int mi,mj,mk;

   if ((data2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (ptr=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++) *ptr++=0;

   maxd=ftrc(255.0f*maxdev+0.5f);

   maxcnt=0.0;
   mi=mj=mk=0;

   for (ptr=data2,k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            if (*ptr++==0)
               {
               cnt=countfill(grad,data2,
                             width,height,depth,
                             i,j,k,get(grad,width,height,depth,i,j,k),maxd,1);

               if (cnt>maxcnt)
                  {
                  maxcnt=cnt;

                  mi=i;
                  mj=j;
                  mk=k;
                  }
               }

   free(data2);

   floodfill(grad,grad,
             width,height,depth,
             mi,mj,mk,get(grad,width,height,depth,mi,mj,mk),maxd,0);
   }

// parse command string
void mipmap::parsecommands(unsigned char *volume,
                           long long width,long long height,long long depth,
                           char *commands)
   {
   char command;

   if (commands==NULL) return;

   while ((command=*commands++)!='\0')
      switch (command)
         {
         case ' ': // nop
            break;
         case 'b': // blur volume
            blur(volume,width,height,depth);
            break;
         }
   }

// parse gradient command string
void mipmap::parsegradcommands(unsigned char *volume,unsigned char *grad,
                               long long width,long long height,long long depth,
                               char *commands)
   {
   const float maxdev=0.1f;
   const float maxgrad=0.5f;

   char command;

   unsigned char *volume2;

   if (commands==NULL) return;

   while ((command=*commands++)!='\0')
      switch (command)
         {
         case ' ': // nop
         case 'b':
            break;
         case 'd': // derive curvature
            volume2=gradmag(grad,width,height,depth,DSX,DSY,DSZ);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'v': // calculate variance
            volume2=variance(volume,width,height,depth);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'u': // use transfer function
            usetf(volume,grad,width,height,depth);
            break;
         case 'o': // use opacity
            useop(volume,grad,width,height,depth);
            break;
         case 't': // tangle material
            tangle(grad,width,height,depth);
            break;
         case 'r': // remove bubbles
            remove(grad,width,height,depth);
            break;
         case 's': // sizify volume
            volume2=sizify(volume,width,height,depth,maxdev);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'c': // classify volume
            volume2=classify(grad,width,height,depth,maxgrad);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'z': // zero space
            zero(volume,grad,width,height,depth,0.0f);
            break;
         case 'S': // sizify gradient
            volume2=sizify(grad,width,height,depth,0.0f);
            memcpy(grad,volume2,width*height*depth);
            free(volume2);
            break;
         case 'T': // grow material
            grow(grad,width,height,depth);
            break;
         case 'F': // fill border
            grow(grad,width,height,depth);
            break;
         case 'R': // fill space
            while (grow(grad,width,height,depth)>0);
            break;
         default: ERRORMSG();
         }
   }

// get interpolated scalar value from volume
unsigned char mipmap::getscalar(unsigned char *volume,
                                long long width,long long height,long long depth,
                                float x,float y,float z)
   {
   long long i,j,k;

   unsigned char *ptr1,*ptr2;

   x*=width-1;
   y*=height-1;
   z*=depth-1;

   i=ftrc(x);
   j=ftrc(y);
   k=ftrc(z);

   x-=i;
   y-=j;
   z-=k;

   if (i<0)
      {
      i=0;
      x=0.0f;
      }

   if (j<0)
      {
      j=0;
      y=0.0f;
      }

   if (k<0)
      {
      k=0;
      z=0.0f;
      }

   if (i>=width-1)
      {
      i=width-2;
      x=1.0f;
      }

   if (j>=height-1)
      {
      j=height-2;
      y=1.0f;
      }

   if (k>=depth-1)
      {
      k=depth-2;
      z=1.0f;
      }

   ptr1=&volume[i+(j+k*height)*width];
   ptr2=ptr1+width*height;

   return(ftrc((1.0f-z)*((1.0f-y)*((1.0f-x)*ptr1[0]+x*ptr1[1])+
                         y*((1.0f-x)*ptr1[width]+x*ptr1[width+1]))+
               z*((1.0f-y)*((1.0f-x)*ptr2[0]+x*ptr2[1])+
                  y*((1.0f-x)*ptr2[width]+x*ptr2[width+1]))+0.5f));
   }

// get interpolated scalar value from volume
float mipmap::getscalar(unsigned short int *volume,
                        long long width,long long height,long long depth,
                        float x,float y,float z)
   {
   long long i,j,k;

   unsigned short int *ptr1,*ptr2;

   x*=width-1;
   y*=height-1;
   z*=depth-1;

   i=ftrc(x);
   j=ftrc(y);
   k=ftrc(z);

   x-=i;
   y-=j;
   z-=k;

   if (i<0)
      {
      i=0;
      x=0.0f;
      }

   if (j<0)
      {
      j=0;
      y=0.0f;
      }

   if (k<0)
      {
      k=0;
      z=0.0f;
      }

   if (i>=width-1)
      {
      i=width-2;
      x=1.0f;
      }

   if (j>=height-1)
      {
      j=height-2;
      y=1.0f;
      }

   if (k>=depth-1)
      {
      k=depth-2;
      z=1.0f;
      }

   ptr1=&volume[i+(j+k*height)*width];
   ptr2=ptr1+width*height;

   return((1.0f-z)*((1.0f-y)*((1.0f-x)*ptr1[0]+x*ptr1[1])+
                    y*((1.0f-x)*ptr1[width]+x*ptr1[width+1]))+
          z*((1.0f-y)*((1.0f-x)*ptr2[0]+x*ptr2[1])+
             y*((1.0f-x)*ptr2[width]+x*ptr2[width+1])));
   }

// get interpolated scalar value from volume
float mipmap::getscalar(float *volume,
                        long long width,long long height,long long depth,
                        float x,float y,float z)
   {
   long long i,j,k;

   float *ptr1,*ptr2;

   x*=width-1;
   y*=height-1;
   z*=depth-1;

   i=ftrc(x);
   j=ftrc(y);
   k=ftrc(z);

   x-=i;
   y-=j;
   z-=k;

   if (i<0)
      {
      i=0;
      x=0.0f;
      }

   if (j<0)
      {
      j=0;
      y=0.0f;
      }

   if (k<0)
      {
      k=0;
      z=0.0f;
      }

   if (i>=width-1)
      {
      i=width-2;
      x=1.0f;
      }

   if (j>=height-1)
      {
      j=height-2;
      y=1.0f;
      }

   if (k>=depth-1)
      {
      k=depth-2;
      z=1.0f;
      }

   ptr1=&volume[i+(j+k*height)*width];
   ptr2=ptr1+width*height;

   return((1.0f-z)*((1.0f-y)*((1.0f-x)*ptr1[0]+x*ptr1[1])+
                    y*((1.0f-x)*ptr1[width]+x*ptr1[width+1]))+
          z*((1.0f-y)*((1.0f-x)*ptr2[0]+x*ptr2[1])+
             y*((1.0f-x)*ptr2[width]+x*ptr2[width+1])));
   }

// scale volume
unsigned char *mipmap::scale(unsigned char *volume,
                             long long width,long long height,long long depth,
                             long long nwidth,long long nheight,long long ndepth)
   {
   long long i,j,k;

   unsigned char *volume2;

   if (nwidth==width && nheight==height && ndepth==depth) return(volume);

   if ((volume2=(unsigned char *)malloc(nwidth*nheight*ndepth))==NULL) ERRORMSG();

   for (i=0; i<nwidth; i++)
      for (j=0; j<nheight; j++)
         for (k=0; k<ndepth; k++)
            volume2[i+(j+k*nheight)*nwidth]=getscalar(volume,width,height,depth,(float)i/(nwidth-1),(float)j/(nheight-1),(float)k/(ndepth-1));

   free(volume);
   return(volume2);
   }

// read a volume by trying any known format
unsigned char *mipmap::readANYvolume(const char *filename,
                                     long long *width,long long *height,long long *depth,unsigned int *components,
                                     float *scalex,float *scaley,float *scalez,
                                     BOOLINT *msb,
                                     void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   unsigned char *volume;
   BOOLINT order;

   unsigned int pvmwidth,pvmheight,pvmdepth;

   order=TRUE;

   if (strchr(filename,'*')!=NULL)
      {
      // read a DICOM series identified by the * in the filename pattern
      if ((volume=readDICOMvolume(filename,width,height,depth,components,
                                  scalex,scaley,scalez,
                                  feedback,obj))!=NULL)
         order=FALSE;
      }
   else
      {
      volume=NULL;

#ifdef HAVE_MINI

      long long steps;

      // read a RAW volume
      if (volume==NULL)
         volume=readRAWvolume(filename,width,height,depth,&steps,components,
                              NULL,NULL,&order,scalex,scaley,scalez);

      // read a REK volume out-of-core
      if (volume==NULL)
         volume=readREKvolume_ooc(filename,width,height,depth,components,
                                  scalex,scaley,scalez,
                                  vol_ratio_,vol_target_cells_,
                                  feedback,obj);

      // read a REK volume
      if (volume==NULL)
         if ((volume=readREKvolume(filename,width,height,depth,components,
                                   scalex,scaley,scalez))!=NULL)
            order=FALSE;

#endif

      // read a PVM volume
      if (volume==NULL)
         {
         volume=readPVMvolume(filename,&pvmwidth,&pvmheight,&pvmdepth,components,
                              scalex,scaley,scalez);

         *width=pvmwidth;
         *height=pvmheight;
         *depth=pvmdepth;
         }
      }

   if (msb!=NULL) *msb=order;

   return(volume);
   }

// load the volume and convert it to 8 bit
BOOLINT mipmap::loadvolume(const char *filename, // filename of PVM to load
                           const char *gradname, // optional filename of gradient volume
                           float mx,float my,float mz, // midpoint of volume (assumed to be fixed)
                           float sx,float sy,float sz, // size of volume (assumed to be fixed)
                           int bricksize,float overmax, // bricksize/overlap of volume (assumed to be fixed)
                           BOOLINT xswap,BOOLINT yswap,BOOLINT zswap, // swap volume flags
                           BOOLINT xrotate,BOOLINT zrotate, // rotate volume flags
                           BOOLINT usegrad, // use gradient volume
                           char *commands, // filter commands
                           int histmin,float histfreq,int kneigh,float histstep, // parameters for histogram computation
                           void (*feedback)(const char *info,float percent,void *obj),void *obj) // feedback callback
   {
   BOOLINT msb;
   BOOLINT upload;

   float maxsize;

   upload=FALSE;

   if (gradname==NULL) gradname=zerostr;
   if (commands==NULL) commands=zerostr;

   if (VOLUME==NULL ||
       strncmp(filename,filestr,MAXSTR)!=0 ||
       (usegrad && strlen(gradname)==0 && (GRAD==NULL || strlen(gradstr)>0)) ||
       strncmp(commands,commstr,MAXSTR)!=0 ||
       xswap!=xsflag || yswap!=ysflag || zswap!=zsflag ||
       xrotate!=xrflag || zrotate!=zrflag)
      {
      if (feedback!=NULL) feedback("loading data",0,obj);

      if (VOLUME!=NULL) free(VOLUME);
      if ((VOLUME=readANYvolume(filename,&WIDTH,&HEIGHT,&DEPTH,&COMPONENTS,&DSX,&DSY,&DSZ,&msb,feedback,obj))==NULL)
         {
         if (feedback!=NULL) feedback("unable to load volume",0,obj);
         return(FALSE);
         }

      if (feedback!=NULL) feedback("processing data",0,obj);

      if (COMPONENTS==2) VOLUME=quantize(VOLUME,WIDTH,HEIGHT,DEPTH,msb);
      else if (COMPONENTS!=1)
         {
         free(VOLUME);
         if (feedback!=NULL) feedback("",0,obj);
         return(FALSE);
         }

      VOLUME=swap(VOLUME,
                  &WIDTH,&HEIGHT,&DEPTH,
                  &DSX,&DSY,&DSZ,
                  xswap,yswap,zswap,
                  xrotate,zrotate);

      parsecommands(VOLUME,
                    WIDTH,HEIGHT,DEPTH,
                    commands);

      if (GRAD!=NULL)
         {
         free(GRAD);
         GRAD=NULL;
         }

      if (usegrad && strlen(gradname)==0)
         {
         if (feedback!=NULL) feedback("calculating gradients",0,obj);

         GRAD=calc_gradmag(VOLUME,
                           WIDTH,HEIGHT,DEPTH,
                           DSX,DSY,DSZ,
                           &GRADMAX,
                           feedback,obj);

         parsegradcommands(VOLUME,GRAD,
                           WIDTH,HEIGHT,DEPTH,
                           commands);
         }

      strncpy(filestr,filename,MAXSTR);
      strncpy(gradstr,"",MAXSTR);
      strncpy(commstr,commands,MAXSTR);

      xsflag=xswap;
      ysflag=yswap;
      zsflag=zswap;

      xrflag=xrotate;
      zrflag=zrotate;

      upload=TRUE;
      }

   if (usegrad && strlen(gradname)>0)
      if (GRAD==NULL ||
          strncmp(gradname,gradstr,MAXSTR)!=0)
         {
         if (feedback!=NULL) feedback("loading gradients",0,obj);

         if (GRAD!=NULL) free(GRAD);
         if ((GRAD=readANYvolume(gradname,&GWIDTH,&GHEIGHT,&GDEPTH,&GCOMPONENTS,&GDSX,&GDSY,&GDSZ,&msb))==NULL) exit(1);
         GRADMAX=1.0f;

         if (GCOMPONENTS==2) GRAD=quantize(GRAD,GWIDTH,GHEIGHT,GDEPTH,msb);
         else if (GCOMPONENTS!=1) exit(1);

         GRAD=swap(GRAD,
                   &GWIDTH,&GHEIGHT,&GDEPTH,
                   NULL,NULL,NULL,
                   xswap,yswap,zswap,
                   xrotate,zrotate);

         GRAD=scale(GRAD,
                    GWIDTH,GHEIGHT,GDEPTH,
                    WIDTH,HEIGHT,DEPTH);

         parsegradcommands(VOLUME,GRAD,
                           WIDTH,HEIGHT,DEPTH,
                           commands);

         strncpy(gradstr,gradname,MAXSTR);

         upload=TRUE;
         }

   if (upload)
      {
      maxsize=getscale();

      set_data(VOLUME,
               usegrad?GRAD:NULL,
               WIDTH,HEIGHT,DEPTH,
               mx,my,mz,
               sx*DSX*(WIDTH-1)/maxsize,sy*DSY*(HEIGHT-1)/maxsize,sz*DSZ*(DEPTH-1)/maxsize,
               bricksize,overmax,
               feedback,obj);
      }

   if (upload)
      HISTO->set_histograms(VOLUME,GRAD,WIDTH,HEIGHT,DEPTH,histmin,histfreq,kneigh,histstep,feedback,obj);

   if (!upload && (hmvalue!=histmin || hfvalue!=histfreq))
      HISTO->inithist(VOLUME,WIDTH,HEIGHT,DEPTH,histmin,histfreq,FALSE,feedback,obj);

   if (!upload && (hmvalue!=histmin || hfvalue!=histfreq || kneigh!=knvalue || histstep!=hsvalue))
      HISTO->inithist2DQ(VOLUME,GRAD,WIDTH,HEIGHT,DEPTH,histmin,histfreq,kneigh,histstep,FALSE,feedback,obj);

   hmvalue=histmin;
   hfvalue=histfreq;
   knvalue=kneigh;
   hsvalue=histstep;

   if (feedback!=NULL) feedback("",0,obj);

   return(TRUE);
   }

// load a DICOM series
BOOLINT mipmap::loadseries(const std::vector<std::string> list, // DICOM series to load
                           float mx,float my,float mz, // midpoint of volume (assumed to be fixed)
                           float sx,float sy,float sz, // size of volume (assumed to be fixed)
                           int bricksize,float overmax, // bricksize/overlap of volume (assumed to be fixed)
                           BOOLINT xswap,BOOLINT yswap,BOOLINT zswap, // swap volume flags
                           BOOLINT xrotate,BOOLINT zrotate, // rotate volume flags
                           BOOLINT usegrad, // use gradient volume
                           int histmin,float histfreq,int kneigh,float histstep, // parameters for histogram computation
                           void (*feedback)(const char *info,float percent,void *obj),void *obj) // feedback callback
   {
   float maxsize;

   int msb=FALSE;

   if (feedback!=NULL) feedback("loading data",0,obj);

   if (VOLUME!=NULL) free(VOLUME);
   if ((VOLUME=readDICOMvolume(list,&WIDTH,&HEIGHT,&DEPTH,&COMPONENTS,&DSX,&DSY,&DSZ,feedback,obj))==NULL)
      {
      if (feedback!=NULL) feedback("unable to load dicom series",0,obj);
      return(FALSE);
      }

   if (feedback!=NULL) feedback("processing data",0,obj);

   if (COMPONENTS==2) VOLUME=quantize(VOLUME,WIDTH,HEIGHT,DEPTH,msb);
   else if (COMPONENTS!=1)
      {
      free(VOLUME);
      return(FALSE);
      }

   VOLUME=swap(VOLUME,
               &WIDTH,&HEIGHT,&DEPTH,
               &DSX,&DSY,&DSZ,
               xswap,yswap,zswap,
               xrotate,zrotate);

   if (GRAD!=NULL)
      {
      free(GRAD);
      GRAD=NULL;
      }

   if (usegrad)
      {
      if (feedback!=NULL) feedback("calculating gradients",0,obj);

      GRAD=calc_gradmag(VOLUME,
                        WIDTH,HEIGHT,DEPTH,
                        DSX,DSY,DSZ,
                        &GRADMAX,
                        feedback,obj);
      }

   strncpy(filestr,"",MAXSTR);
   strncpy(gradstr,"",MAXSTR);
   strncpy(commstr,"",MAXSTR);

   xsflag=xswap;
   ysflag=yswap;
   zsflag=zswap;

   xrflag=xrotate;
   zrflag=zrotate;

   maxsize=getscale();

   set_data(VOLUME,GRAD,
            WIDTH,HEIGHT,DEPTH,
            mx,my,mz,
            sx*DSX*(WIDTH-1)/maxsize,sy*DSY*(HEIGHT-1)/maxsize,sz*DSZ*(DEPTH-1)/maxsize,
            bricksize,overmax,
            feedback,obj);

   HISTO->set_histograms(VOLUME,NULL,WIDTH,HEIGHT,DEPTH,histmin,histfreq,kneigh,histstep,feedback,obj);

   hmvalue=histmin;
   hfvalue=histfreq;
   knvalue=kneigh;
   hsvalue=histstep;

   if (feedback!=NULL) feedback("",0,obj);

   return(TRUE);
   }

// save the volume data as PVM
void mipmap::savePVMvolume(const char *filename)
   {
   if (VOLUME==NULL) return;

   writePVMvolume(filename,VOLUME,
                  WIDTH,HEIGHT,DEPTH,COMPONENTS,
                  DSX,DSY,DSZ);
   }

// return the histogram
float *mipmap::get_hist()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_hist());
   }

// return the colored histogram
float *mipmap::get_histRGBA()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_histRGBA());
   }

// return the scatter plot
float *mipmap::get_hist2D()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_hist2D());
   }

// return the colored scatter plot
float *mipmap::get_hist2DRGBA()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_hist2DRGBA());
   }

// return the quantized scatter plot
float *mipmap::get_hist2DQRGBA()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_hist2DQRGBA());
   }

// return the quantized transfer function
float *mipmap::get_hist2DTFRGBA()
   {
   if (VOLCNT==0) return(NULL);
   return(HISTO->get_hist2DTFRGBA());
   }

// check whether or not the hierarchy has volume data
BOOLINT mipmap::has_data()
   {return(VOLCNT!=0);}

// check whether or not the hierarchy has gradient data
BOOLINT mipmap::has_grad()
   {return(VOLCNT!=0 && GRAD!=NULL);}

// return the slab thickness
float mipmap::get_slab()
   {
   if (VOLCNT==0) return(0.0f);
   else return(VOL[0]->get_slab());
   }

// set ambient/diffuse/specular lighting coefficients
void mipmap::set_light(float noise,float ambnt,float difus,float specl,float specx)
   {
   int i;

   if (VOLCNT==0) return;

   for (i=0; i<VOLCNT; i++) VOL[i]->set_light(noise,ambnt,difus,specl,specx);
   }

// extract iso surface
char *mipmap::extractsurface(double isovalue,
                             void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   char *surface;

   surface=NULL;

#ifdef HAVE_MINI

   if (strlen(filestr)>0)
      {
      char *output;

      output=processRAWvolume(filestr,"_iso",
                              iso_ratio_,iso_target_cells_,
                              feedback,obj);

      if (output==NULL)
         output=processREKvolume(filestr,"_iso",
                                 iso_ratio_,iso_target_cells_,
                                 feedback,obj);

      if (output==NULL)
         output=processPVMvolume(filestr);

      if (output!=NULL)
         {
         surface=extractRAWvolume(output,output,isovalue,feedback,obj);
         free(output);

         if (feedback!=NULL) feedback("loading geometry",0,obj);

         loadsurface(surface);

         if (feedback!=NULL) feedback("",0,obj);
         }
      else
         if (feedback!=NULL) feedback("unable to extract iso surface",0,obj);
      }
   else
      if (feedback!=NULL) feedback("volume required to extract iso surface",0,obj);

#else

   feedback("iso surface extraction is unavailable",0,obj);

#endif

   return(surface);
   }

// extract iso surface for the smallest non-zero tfunc entry
char *mipmap::extractTFsurface(void (*feedback)(const char *info,float percent,void *obj),void *obj)
   {
   double isomin,isomax;

   isomin=TFUNC->get_nonzero_min();
   isomax=TFUNC->get_nonzero_max();

   if (isomin==0.0 && isomax==1.0) isomin=0.5;

   return(extractsurface(isomin,feedback,obj));
   }

// load the surface data
BOOLINT mipmap::loadsurface(const char *filename)
   {
   BOOLINT result;

   double scale;

   result=SURFACE.readGEOfile(filename);

   if (has_data())
      scale=getscale();
   else
      scale=SURFACE.getscale();

   if (scale>0.0) scale=1.0/scale;

   double mtx[16]={scale,0,0,0,
                   0,scale,0,0,
                   0,0,scale,0,
                   0,0,0,1};

   SURFACE.setmatrix(mtx);

   return(result);
   }

// clear the surface data
void mipmap::clearsurface()
   {SURFACE.clear();}

// check whether or not a surface is present
BOOLINT mipmap::has_geo()
   {return(SURFACE.has_geo());}

// set limit for maximum displayable volume size
void mipmap::set_vol_maxsize(long long maxsize,
                             float ratio)
   {
   if (maxsize>1)
      {
      vol_target_cells_=maxsize*maxsize*maxsize;
      vol_ratio_=ratio;
      }
   }

// set limit for maximum volume size for iso surface extraction
void mipmap::set_iso_maxsize(long long maxsize,
                             float ratio)
   {
   if (maxsize>1)
      {
      iso_target_cells_=maxsize*maxsize*maxsize;
      iso_ratio_=ratio;
      }
   }

// render the volume
BOOLINT mipmap::render(float ex,float ey,float ez,
                       float dx,float dy,float dz,
                       float ux,float uy,float uz,
                       float nearp,float slab,
                       BOOLINT lighting,
                       BOOLINT usefbo,
                       BOOLINT (*abort)(void *abortdata),
                       void *abortdata)
   {
   int i;

   BOOLINT aborted=FALSE;

   int map=0;

   // save eye point
   ex_=ex;
   ey_=ey;
   ez_=ez;

   // save viewing direction
   dx_=dx;
   dy_=dy;
   dz_=dz;

   // save up vector
   ux_=ux;
   uy_=uy;
   uz_=uz;

   // save near plane point
   px_=ex+dx*nearp;
   py_=ey+dy*nearp;
   pz_=ez+dz*nearp;

   // save near plane normal
   nx_=dx;
   ny_=dy;
   nz_=dz;

   // update fbo
   if (usefbo && has_data()) updatefbo();

   // render to fbo
   if (HASFBO && usefbo)
      if (get_tfunc()->checkRGBA())
         {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

         // clear buffers
         glClearColor(0,0,0,0);
         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
         }

   // render opaque geometry
   rendergeometry();

   // enable clipping planes
   for (i=0; i<6; i++)
      if (clip_on[i])
         {
         GLdouble equ[4];

         equ[0]=clip_a[i];
         equ[1]=clip_b[i];
         equ[2]=clip_c[i];
         equ[3]=clip_d[i];

         glClipPlane(GL_CLIP_PLANE0+i,equ);

         glEnable(GL_CLIP_PLANE0+i);
         }

   // render surface
   SURFACE.render();

   // render transparent volume
   if (VOLCNT>0)
      {
      // choose volume
      if (TFUNC->get_imode())
         while (map<VOLCNT-1 && slab/VOL[map]->get_slab()>1.5f) map++;

      // render volume
      aborted=VOL[map]->render(ex,ey,ez,
                               dx,dy,dz,
                               ux,uy,uz,
                               nearp,slab,
                               1.0f/get_slab(),
                               lighting,
                               abort,abortdata);
      }

   // disable clipping planes
   for (i=0; i<6; i++)
      if (clip_on[i])
         glDisable(GL_CLIP_PLANE0+i);

   // render from fbo
   if (HASFBO && usefbo)
      if (get_tfunc()->checkRGBA())
         {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

         glBindTexture(GL_TEXTURE_2D, textureId);
         glEnable(GL_TEXTURE_2D);

         glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

         glAlphaFunc(GL_GREATER,0.0);
         glEnable(GL_ALPHA_TEST);

         glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
         glEnable(GL_BLEND);

         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
         glLoadIdentity();
         gluOrtho2D(-1.0f,1.0f,-1.0f,1.0f);
         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();
         glLoadIdentity();

         glBegin(GL_QUADS);
         glColor3f(1.0f,1.0f,1.0f);
         glTexCoord2f(0.0f,0.0f);
         glVertex2f(-1.0f,-1.0f);
         glTexCoord2f(1.0f,0.0f);
         glVertex2f(1.0f,-1.0f);
         glTexCoord2f(1.0f,1.0f);
         glVertex2f(1.0f,1.0f);
         glTexCoord2f(0.0f,1.0f);
         glVertex2f(-1.0f,1.0f);
         glEnd();

         glPopMatrix();
         glMatrixMode(GL_PROJECTION);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);

         glBindTexture(GL_TEXTURE_2D, 0);
         glDisable(GL_TEXTURE_2D);

         glDisable(GL_ALPHA_TEST);

         glDisable(GL_BLEND);
         }

   // invert frame buffer
   if (TFUNC->get_invmode()) invertbuffer();

   return(aborted);
   }

// draw the surrounding wire frame box
void mipmap::drawwireframe()
   {
   if (VOLCNT==0) volume::drawwireframe();
   else
      volume::drawwireframe(VOL[0]->getcenterx(),VOL[0]->getcentery(),VOL[0]->getcenterz(),
                            VOL[0]->getsizex(),VOL[0]->getsizey(),VOL[0]->getsizez());
   }
