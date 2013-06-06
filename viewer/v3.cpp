// (c) by Stefan Roettger, licensed under GPL 2+

#define VERSION "3.6.1 as of 6.June.2012"

#include "codebase.h" // universal code base
#include "oglbase.h" // OpenGL base rendering
#include "glutbase.h" // GLUT window handling
#include "guibase.h" // minimalistic OpenGL GUI
#include "tfbase.h" // transfer function handling
#include "volume.h" // volume mipmap pyramid
#include "volren.h" // volume renderer

// global definitions:

#define STR_MAX (256)

#define WIN_WIDTH (900)
#define WIN_HEIGHT (900)

#define WIN_FPS (30.0f)

#define EYE_FOVY (60.0f)
#define EYE_NEAR (0.01f)
#define EYE_FAR (10.0f)

#define EYE_MAXSPEED (0.5f)

#define VOL_EMISSION (1000.0f)
#define VOL_DENSITY (1000.0f)

#define VOL_OVERMIN (0.25f)
#define VOL_OVERMAX (8.0f)

#define VOL_BRICKSIZE (128)

#define VOL_DELAY1 (2.0f)
#define VOL_DELAY2 (600.0f)

// global variables:

char PROGNAME[STR_MAX],
     FILENAME[STR_MAX],
     GRADNAME[STR_MAX],
     OUTNAME[STR_MAX],
     CONFIG[STR_MAX],
     RECORD[STR_MAX];

volren *VOLREN;

float EYE_X,EYE_Y,EYE_Z,
      EYE_DX,EYE_DY,EYE_DZ,
      EYE_UX,EYE_UY,EYE_UZ;

float EYE_SPEED;

// GUI setup:

GUI OGL_GUI;

int GUI_hook1,
    GUI_hook2,
    GUI_hook3,
    GUI_hook4,
    GUI_hook5,
    GUI_hook6,
    GUI_hook7,
    GUI_hook8,
    GUI_hook9,
    GUI_hook10,
    GUI_hook11,
    GUI_hook12,
    GUI_hook13,
    GUI_hook14,
    GUI_hook15,
    GUI_hook16,
    GUI_hook17,
    GUI_hook18,
    GUI_hook19,
    GUI_hook20,
    GUI_hook21,
    GUI_hook22,
    GUI_hook23,
    GUI_hook24,
    GUI_hook25,
    GUI_hook26,
    GUI_hook27,
    GUI_hook28;

BOOLINT GUI_wire=FALSE,
        GUI_white=TRUE,
        GUI_premult=TRUE,
        GUI_preint=TRUE,
        GUI_reduced=FALSE,
        GUI_coupled=TRUE;

float GUI_rot=0.5f,
      GUI_rotx=0.0f,
      GUI_roty=0.0f,
      GUI_height=0.5f;

float GUI_slab1=0.5f;
float GUI_slab2=1.0f;

float GUI_re_scale=0.25f,
      GUI_ge_scale=0.25f,
      GUI_be_scale=0.25f,
      GUI_ra_scale=0.25f,
      GUI_ga_scale=0.25f,
      GUI_ba_scale=0.25f;

BOOLINT GUI_re_mod=TRUE,
        GUI_ge_mod=TRUE,
        GUI_be_mod=TRUE,
        GUI_ra_mod=TRUE,
        GUI_ga_mod=TRUE,
        GUI_ba_mod=TRUE;

BOOLINT GUI_xswap=FALSE,
        GUI_yswap=FALSE,
        GUI_zswap=FALSE,
        GUI_xrot=FALSE,
        GUI_zrot=FALSE;

BOOLINT GUI_grad=FALSE,
        GUI_light=FALSE;

BOOLINT GUI_gmc=FALSE,
        GUI_mod=FALSE,
        GUI_mat=FALSE;

BOOLINT GUI_extra=FALSE,
        GUI_STF=FALSE;

BOOLINT GUI_inv=FALSE,
        GUI_clip=FALSE;

BOOLINT GUI_fbo=TRUE;

float GUI_clip_dist=0.0f;

int GUI_mode=0;
char GUI_commands[STR_MAX]="";

BOOLINT GUI_blurvol=FALSE;
int GUI_histmin=5;
float GUI_histfreq=10.0f;
float GUI_histslide=0.5f;
float GUI_histtweak=0.5f;
int GUI_kneigh=1;
float GUI_histstep=1.0f;

BOOLINT GUI_hide=FALSE,
        GUI_capt=FALSE,
        GUI_points=FALSE;

float GUI_cycle=0.0f,
      GUI_range=0.0f;

BOOLINT GUI_record=FALSE,
        GUI_demo=FALSE,
        GUI_loop=FALSE;

FILE *GUI_recfile;

double GUI_time=0.0,
       GUI_start=0.0;

int GUI_texid=0;

void loadvolume()
   {
   if (!VOLREN->loadvolume(FILENAME,GRADNAME,
                           0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,
                           VOL_BRICKSIZE,VOL_OVERMAX,
                           GUI_xswap,GUI_yswap,GUI_zswap,
                           GUI_xrot,GUI_zrot,
                           GUI_extra,GUI_commands,
                           ftrc(2.0f*GUI_histtweak*GUI_histmin)+1,2.0f*GUI_histslide*GUI_histfreq,
                           GUI_kneigh,GUI_histstep)) exit(1);
   }

void setupGUI();

void reloadhook(float x=0.0f,float y=0.0f,void *data=NULL)
   {
   if (data!=NULL) GUI::pushbuttonhook(x,y,data);

   if (!GUI_grad && GUI_gmc) GUI_gmc=FALSE;
   if (!GUI_grad && GUI_mod) GUI_mod=FALSE;
   if (!GUI_grad && GUI_mat) GUI_mat=FALSE;

   if (GUI_grad && GUI_STF) GUI_gmc=FALSE;

   if (GUI_mat && GUI_gmc) GUI_gmc=FALSE;
   if (GUI_mat && GUI_light) GUI_light=FALSE;

   if (!GUI_grad && !GUI_light)
      {
      if (GUI_blurvol) strncpy(GUI_commands,"bb",STR_MAX);
      else strncpy(GUI_commands,"",STR_MAX);

      GUI_extra=FALSE;
      loadvolume();

      VOLREN->get_tfunc()->set_num(1);
      VOLREN->get_tfunc()->set_mode(GUI_mode=0);
      }

   if (GUI_grad || GUI_light)
      {
      if (GUI_mat)
         if (strlen(GRADNAME)!=0) strncpy(GUI_commands,"",STR_MAX);
         else if (GUI_mod) strncpy(GUI_commands,"orczF",STR_MAX);
         else strncpy(GUI_commands,"uc",STR_MAX);
      else
         if (GUI_blurvol) strncpy(GUI_commands,"bb",STR_MAX);
         else strncpy(GUI_commands,"",STR_MAX);

      GUI_extra=TRUE;
      loadvolume();

      GUI::deletetexmap(GUI_texid);

      if (GUI_STF) GUI_texid=GUI::buildtexmap2DRGBA(VOLREN->get_volume()->get_hist2DQRGBA(),256,256);
      else GUI_texid=GUI::buildtexmap2DL(VOLREN->get_volume()->get_hist2D(),256,256);

      if (GUI_grad)
         {
         if (GUI_STF || GUI_mat) VOLREN->get_tfunc()->set_num(64);
         else VOLREN->get_tfunc()->set_num(32);

         if (GUI_mat)
            if (GUI_mod) GUI_mode=9;
            else GUI_mode=8;
         else
            if (GUI_STF)
               if (GUI_mod) GUI_mode=14;
               else GUI_mode=12;
            else
               if (GUI_gmc)
                  if (GUI_mod) GUI_mode=7;
                  else GUI_mode=5;
               else
                  if (GUI_mod) GUI_mode=3;
                  else GUI_mode=1;

         VOLREN->get_tfunc()->set_mode(GUI_mode);
         }
      else
         {
         VOLREN->get_tfunc()->set_num(2);
         VOLREN->get_tfunc()->set_mode(GUI_mode=0);
         }
      }

   if (GUI_STF)
      if (!GUI_grad) VOLREN->get_tfunc()->copy_tfRGB(VOLREN->get_volume()->get_histRGBA(),256,1);
      else if (!GUI_mod) VOLREN->get_tfunc()->copy_2DTFRGB(VOLREN->get_volume()->get_hist2DTFRGBA(),256,256,1);
      else VOLREN->get_tfunc()->copy_2DTFRGBA(VOLREN->get_volume()->get_hist2DTFRGBA(),256,256,0);

   setupGUI();
   }

void save(FILE *file)
   {
   fprintf(file,"GUI2:\n");

   fprintf(file,"guiex=%f\n",EYE_X);
   fprintf(file,"guiey=%f\n",EYE_Y);
   fprintf(file,"guiez=%f\n",EYE_Z);

   fprintf(file,"guirot=%f\n",GUI_rot);
   fprintf(file,"guirotx=%f\n",GUI_rotx);
   fprintf(file,"guiroty=%f\n",GUI_roty);
   fprintf(file,"guiheight=%f\n",GUI_height);

   fprintf(file,"guiwire=%d\n",GUI_wire);
   fprintf(file,"guiwhite=%d\n",GUI_white);

   fprintf(file,"guiinv=%d\n",GUI_inv);
   fprintf(file,"guiclip=%d\n",GUI_clip);
   fprintf(file,"guiclipdist=%f\n",GUI_clip_dist);

   fprintf(file,"guislab1=%f\n",GUI_slab1);
   fprintf(file,"guislab2=%f\n",GUI_slab2);

   fprintf(file,"guipremult=%d\n",GUI_premult);
   fprintf(file,"guipreint=%d\n",GUI_preint);

   fprintf(file,"guixswap=%d\n",GUI_xswap);
   fprintf(file,"guiyswap=%d\n",GUI_yswap);
   fprintf(file,"guizswap=%d\n",GUI_zswap);

   if (strlen(GRADNAME)==0) fprintf(file,"guigradname=_none_\n");
   else fprintf(file,"guigradname=%s\n",GRADNAME);

   fprintf(file,"guigrad=%d\n",GUI_grad);
   fprintf(file,"guilight=%d\n",GUI_light);

   fprintf(file,"guigmc=%d\n",GUI_gmc);
   fprintf(file,"guimod=%d\n",GUI_mod);
   fprintf(file,"guimat=%d\n",GUI_mat);

   fprintf(file,"guicycle=%g\n",GUI_cycle);
   fprintf(file,"guirange=%g\n",GUI_range);

   fprintf(file,"guixrot=%d\n",GUI_xrot);
   fprintf(file,"guizrot=%d\n",GUI_zrot);

   fprintf(file,"guistf=%d\n",GUI_STF);
   fprintf(file,"guiall=%d\n",GUI_coupled);

   fprintf(file,"guihistmin=%d\n",GUI_histmin);
   fprintf(file,"guihistfreq=%g\n",GUI_histfreq);
   fprintf(file,"guihistslide=%g\n",GUI_histslide);
   fprintf(file,"guihisttweak=%g\n",GUI_histtweak);
   fprintf(file,"guikneigh=%d\n",GUI_kneigh);
   fprintf(file,"guihiststep=%g\n",GUI_histstep);

   fprintf(file,"guiblurvol=%d\n",GUI_blurvol);

   fprintf(file,"guihide=%d\n",GUI_hide);
   fprintf(file,"guicapt=%d\n",GUI_capt);

   fprintf(file,"guipoints=%d\n",GUI_points);

   fprintf(file,"guiloop=%d\n",GUI_loop);
   }

void load(FILE *file)
   {
   BOOLINT version=0;

   char ch;
   int v;

   if (fscanf(file,"GUI")!=0) return;
   version++;

   if ((ch=fgetc(file))!='2') ungetc(ch,file);
   else version++;

   if (fscanf(file,":\n")!=0) return;

   fscanf(file,"guiex=%g\n",&EYE_X);
   fscanf(file,"guiey=%g\n",&EYE_Y);
   fscanf(file,"guiez=%g\n",&EYE_Z);

   fscanf(file,"guirot=%g\n",&GUI_rot);
   fscanf(file,"guirotx=%g\n",&GUI_rotx);
   fscanf(file,"guiroty=%g\n",&GUI_roty);
   fscanf(file,"guiheight=%g\n",&GUI_height);

   if (fscanf(file,"guiwire=%d\n",&v)==1) GUI_wire=v;
   if (fscanf(file,"guiwhite=%d\n",&v)==1) GUI_white=v;

   if (version>=2)
      {
      if (fscanf(file,"guiinv=%d\n",&v)==1) GUI_inv=v;
      if (fscanf(file,"guiclip=%d\n",&v)==1) GUI_clip=v;
      fscanf(file,"guiclipdist=%g\n",&GUI_clip_dist);
      }

   fscanf(file,"guislab1=%g\n",&GUI_slab1);
   fscanf(file,"guislab2=%g\n",&GUI_slab2);

   if (fscanf(file,"guipremult=%d\n",&v)==1) GUI_premult=v;
   if (fscanf(file,"guipreint=%d\n",&v)==1) GUI_preint=v;

   if (fscanf(file,"guixswap=%d\n",&v)==1) GUI_xswap=v;
   if (fscanf(file,"guiyswap=%d\n",&v)==1) GUI_yswap=v;
   if (fscanf(file,"guizswap=%d\n",&v)==1) GUI_zswap=v;

   fscanf(file,"guigradname=%s\n",GRADNAME);
   if (strncmp(GRADNAME,"_none_",STR_MAX)==0) strncpy(GRADNAME,"",STR_MAX);

   if (fscanf(file,"guigrad=%d\n",&v)==1) GUI_grad=v;
   if (fscanf(file,"guilight=%d\n",&v)==1) GUI_light=v;

   if (fscanf(file,"guigmc=%d\n",&v)==1) GUI_gmc=v;
   if (fscanf(file,"guimod=%d\n",&v)==1) GUI_mod=v;
   if (fscanf(file,"guimat=%d\n",&v)==1) GUI_mat=v;

   fscanf(file,"guicycle=%g\n",&GUI_cycle);
   fscanf(file,"guirange=%g\n",&GUI_range);

   if (fscanf(file,"guixrot=%d\n",&v)==1) GUI_xrot=v;
   if (fscanf(file,"guizrot=%d\n",&v)==1) GUI_zrot=v;

   if (fscanf(file,"guistf=%d\n",&v)==1) GUI_STF=v;
   if (fscanf(file,"guiall=%d\n",&v)==1) GUI_coupled=v;

   fscanf(file,"guihistmin=%d\n",&GUI_histmin);
   fscanf(file,"guihistfreq=%g\n",&GUI_histfreq);
   fscanf(file,"guihistslide=%g\n",&GUI_histslide);
   fscanf(file,"guihisttweak=%g\n",&GUI_histtweak);
   fscanf(file,"guikneigh=%d\n",&GUI_kneigh);
   fscanf(file,"guihiststep=%g\n",&GUI_histstep);

   if (fscanf(file,"guiblurvol=%d\n",&v)==1) GUI_blurvol=v;

   if (fscanf(file,"guihide=%d\n",&v)==1) GUI_hide=v;
   if (fscanf(file,"guicapt=%d\n",&v)==1) GUI_capt=v;

   if (fscanf(file,"guipoints=%d\n",&v)==1) GUI_points=v;

   if (fscanf(file,"guiloop=%d\n",&v)==1) GUI_loop=v;
   }

void quithook(float x=0.0f,float y=0.0f)
   {
   delete VOLREN;
   GUI::deletetexmap(GUI_texid);
   closewindow();

   if (GUI_record || GUI_demo) fclose(GUI_recfile);

   exit(0);
   }

void savehook(float x=0.0f,float y=0.0f)
   {
   FILE *file;

   if ((file=fopen(CONFIG,"wb"))==NULL) ERRORMSG();

   VOLREN->get_tfunc()->save(file);

   save(file);

   VOLREN->get_histo()->save(file);

   fclose(file);
   }

void loadhook(float x=0.0f,float y=0.0f)
   {
   FILE *file;

   if ((file=fopen(CONFIG,"rb"))==NULL) reloadhook();
   else
      {
      VOLREN->get_tfunc()->load(file);

      load(file);
      reloadhook();

      VOLREN->get_histo()->load(file);
      reloadhook();

      VOLREN->get_tfunc()->get_escale(&GUI_re_scale,&GUI_ge_scale,&GUI_be_scale);
      VOLREN->get_tfunc()->get_ascale(&GUI_ra_scale,&GUI_ga_scale,&GUI_ba_scale);

      GUI_re_scale=fsqrt(GUI_re_scale);
      GUI_ge_scale=fsqrt(GUI_ge_scale);
      GUI_be_scale=fsqrt(GUI_be_scale);

      GUI_ra_scale=fsqrt(GUI_ra_scale);
      GUI_ga_scale=fsqrt(GUI_ga_scale);
      GUI_ba_scale=fsqrt(GUI_ba_scale);

      fclose(file);
      }
   }

void xrothook(float x=0.0f,float y=0.0f)
   {
   GUI_xrot=!GUI_xrot;
   reloadhook();
   }

void zrothook(float x=0.0f,float y=0.0f)
   {
   GUI_zrot=!GUI_zrot;
   reloadhook();
   }

void xswaphook(float x=0.0f,float y=0.0f)
   {
   GUI_xswap=!GUI_xswap;
   reloadhook();
   }

void yswaphook(float x=0.0f,float y=0.0f)
   {
   GUI_yswap=!GUI_yswap;
   reloadhook();
   }

void zswaphook(float x=0.0f,float y=0.0f)
   {
   GUI_zswap=!GUI_zswap;
   reloadhook();
   }

void randhook(float x=0.0f,float y=0.0f)
   {VOLREN->get_tfunc()->randomize();}

BOOLINT checktf(float *tf)
   {
   int i;

   for (i=0; i<VOLREN->get_tfunc()->get_res(); i++)
      if (tf[i]>0.0f) return(FALSE);

   return(TRUE);
   }

void clearhook(float x=0.0f,float y=0.0f)
   {
   if (GUI_grad && GUI_STF) return;

   if (GUI_re_mod)
      if (checktf(VOLREN->get_tfunc()->get_re()))
         VOLREN->get_tfunc()->set_line(0.0f,1.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_re());
      else
         VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOLREN->get_tfunc()->get_re());

   if (GUI_ge_mod)
      if (checktf(VOLREN->get_tfunc()->get_ge()))
         VOLREN->get_tfunc()->set_line(0.0f,1.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_ge());
      else
         VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOLREN->get_tfunc()->get_ge());

   if (GUI_be_mod)
      if (checktf(VOLREN->get_tfunc()->get_be()))
         VOLREN->get_tfunc()->set_line(0.0f,1.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_be());
      else
         VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,0.0f,VOLREN->get_tfunc()->get_be());
   }

void cyclehook(float x=0.0f,float y=0.0f)
   {
   GUI_cycle+=1.0f/VOLREN->get_tfunc()->get_num();
   if (GUI_cycle>1.0f) GUI_cycle-=1.0f;
   VOLREN->get_tfunc()->set_imp(GUI_cycle,GUI_range);
   }

void recyclehook(float x=0.0f,float y=0.0f)
   {
   GUI_cycle-=1.0f/VOLREN->get_tfunc()->get_num();
   if (GUI_cycle<0.0f) GUI_cycle+=1.0f;
   VOLREN->get_tfunc()->set_imp(GUI_cycle,GUI_range);
   }

void incrangehook(float x=0.0f,float y=0.0f)
   {
   GUI_range+=1.0f/VOLREN->get_tfunc()->get_num();
   if (GUI_range>1.0f) GUI_range=1.0f;
   VOLREN->get_tfunc()->set_imp(GUI_cycle,GUI_range);
   }

void decrangehook(float x=0.0f,float y=0.0f)
   {
   GUI_range-=1.0f/VOLREN->get_tfunc()->get_num();
   if (GUI_range<0.0f) GUI_range=0.0f;
   VOLREN->get_tfunc()->set_imp(GUI_cycle,GUI_range);
   }

void acchook(float x=0.0f,float y=0.0f)
   {EYE_SPEED+=EYE_MAXSPEED/10.0f;}

void stophook(float x=0.0f,float y=0.0f)
   {EYE_SPEED=0.0f;}

void dechook(float x=0.0f,float y=0.0f)
   {EYE_SPEED-=EYE_MAXSPEED/10.0f;}

void tfEhook(float x,float y,void *data)
   {
   static BOOLINT clicked=FALSE;

   static float lx,ly;

   if (GUI_STF)
      {
      if (x<0.0f && y<0.0f) return;

      if (GUI_coupled)
         {
         GUI_histslide=x;
         GUI_histtweak=y;
         }
      else
         VOLREN->get_histo()->click(x,y,0.01f);

      reloadhook();
      }
   else
      {
      if (x<0.0f && y<0.0f) clicked=FALSE;
      else if (!clicked)
         {
         if (!GUI_re_mod && !GUI_ge_mod && !GUI_be_mod)
            {
            VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_re());
            VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ge());
            VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_be());
            }
         else
            {
            if (GUI_re_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_re());
            if (GUI_ge_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_ge());
            if (GUI_be_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_be());
            }

         clicked=TRUE;
         }
      else
         if (!GUI_re_mod && !GUI_ge_mod && !GUI_be_mod)
            {
            VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_re());
            VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ge());
            VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_be());
            }
         else
            {
            if (GUI_re_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_re());
            if (GUI_ge_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_ge());
            if (GUI_be_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_be());
            }

      lx=x;
      ly=y;
      }
   }

void tfE2hook(float x,float y,void *data)
   {
   if (x<0.0f && y<0.0f) return;
   if (y<0.5f) tfEhook(x,0.0f,data);
   else tfEhook(x,1.0f,data);
   }

void tfAhook(float x,float y,void *data)
   {
   static BOOLINT clicked=FALSE;

   static float lx,ly;

   if (x<0.0f && y<0.0f) clicked=FALSE;
   else if (!clicked)
      {
      if (!GUI_ra_mod && !GUI_ga_mod && !GUI_ba_mod)
         {
         VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ra());
         VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ga());
         VOLREN->get_tfunc()->set_line(x,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ba());
         }
      else
         {
         if (GUI_ra_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_ra());
         if (GUI_ga_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_ga());
         if (GUI_ba_mod) VOLREN->get_tfunc()->set_line(x,y,x,y,VOLREN->get_tfunc()->get_ba());
         }

      clicked=TRUE;
      }
   else
      if (!GUI_ra_mod && !GUI_ga_mod && !GUI_ba_mod)
         {
         VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ra());
         VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ga());
         VOLREN->get_tfunc()->set_line(lx,0.0f,x,0.0f,VOLREN->get_tfunc()->get_ba());
         }
      else
         {
         if (GUI_ra_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_ra());
         if (GUI_ga_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_ga());
         if (GUI_ba_mod) VOLREN->get_tfunc()->set_line(lx,ly,x,y,VOLREN->get_tfunc()->get_ba());
         }

   lx=x;
   ly=y;
   }

void tfA2hook(float x,float y,void *data)
   {
   if (x<0.0f && y<0.0f) return;
   if (y<0.5f) tfAhook(x,0.0f,data);
   else tfAhook(x,1.0f,data);
   }

void drawtfunc(float *tf,int res,float hue)
   {
   int i;

   glBlendFunc(GL_ONE,GL_ONE);

   for (i=0; i<res; i++)
      GUI::drawquad((float)i/res,0.0f,1.0f/res,tf[i],
                    hue,1.0f,1.0f,1.0f);

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   }

void drawhist(BOOLINT colored=TRUE)
   {
   int i;

   for (i=0; i<256; i++)
      if (!colored)
         GUI::drawquad((float)i/256,0.0f,1.0f/256,VOLREN->get_volume()->get_hist()[i],
                       0.0f,0.0f,0.5f,0.5f);
      else
         GUI::drawquadRGBA((float)i/256,0.0f,1.0f/256,VOLREN->get_volume()->get_hist()[i],
                           VOLREN->get_volume()->get_histRGBA()[4*i],VOLREN->get_volume()->get_histRGBA()[4*i+1],VOLREN->get_volume()->get_histRGBA()[4*i+2],
                           0.5f*VOLREN->get_volume()->get_histRGBA()[4*i+3]);
   }

void drawhist2D()
   {
   if (!GUI_grad) drawhist();
   else
      {
      GUI::drawtexture(0.0f,0.0f,1.0f,1.0f,
                       0.0f,0.0f,0.9f,0.75f,
                       GUI_texid,256,256);

      GUI::drawline(0.0f,fmax(GUI_cycle-GUI_range,0.0f),1.0f,fmax(GUI_cycle-GUI_range,0.0f),
                    0.0f,1.0f,1.0f,0.75f);

      GUI::drawline(0.0f,fmin(GUI_cycle+GUI_range,1.0f),1.0f,fmin(GUI_cycle+GUI_range,1.0f),
                    0.0f,1.0f,1.0f,0.75f);
      }
   }

void tfEdraw(void *data1,void *data2)
   {
   if (!GUI_grad || !GUI_STF)
      {
      drawtfunc(VOLREN->get_tfunc()->get_re(),VOLREN->get_tfunc()->get_res(),0.0f);
      drawtfunc(VOLREN->get_tfunc()->get_ge(),VOLREN->get_tfunc()->get_res(),120.0f);
      drawtfunc(VOLREN->get_tfunc()->get_be(),VOLREN->get_tfunc()->get_res(),240.0f);
      drawhist();
      }
   else
      GUI::drawtexture(0.0f,0.0f,1.0f,1.0f,
                       0.0f,0.0f,1.0f,0.75f,
                       GUI_texid,256,256);
   }

void tfAdraw(void *data1,void *data2)
   {
   drawtfunc(VOLREN->get_tfunc()->get_ra(),VOLREN->get_tfunc()->get_res(),0.0f);
   drawtfunc(VOLREN->get_tfunc()->get_ga(),VOLREN->get_tfunc()->get_res(),120.0f);
   drawtfunc(VOLREN->get_tfunc()->get_ba(),VOLREN->get_tfunc()->get_res(),240.0f);
   drawhist2D();
   }

void tfEsliderhook(float x,float y,void *data)
   {
   GUI::sliderhookX(x,y,data);

   float *scale=(float *)data;

   if (GUI_coupled || (GUI_grad && GUI_STF)) GUI_re_scale=GUI_ge_scale=GUI_be_scale=*scale;
   }

void tfAsliderhook(float x,float y,void *data)
   {
   GUI::sliderhookX(x,y,data);

   float *scale=(float *)data;

   if (GUI_coupled || (GUI_grad && GUI_STF)) GUI_ra_scale=GUI_ga_scale=GUI_ba_scale=*scale;
   }

void setupGUI()
   {
   static char text1[]="Quit";
   static char text2[]="Move\nForth";
   static char text3[]="Stop";
   static char text4[]="Move\nBack";
   static char text5[]="Wire";
   static char text6[]="Pre\nMult";
   static char text7[]="Pre\nInt";
   static char text8[]="Rotate";
   static char text9[]="H\ne\ni\ng\nh\nt";
   static char text10[]="Red";
   static char text11[]="Green";
   static char text12[]="Blue";
   static char text13[]="N\nu\nm";
   static char text14[]="All";
   static char text15[]="Save";
   static char text16[]="Load";
   static char text17[]="X";
   static char text18[]="Y";
   static char text19[]="Z";
   static char text20[]="Rand";
   static char text21[]="Clear";
   static char text22[]="White";
   static char text23[]="2DTF";
   static char text24[]="Light";
   static char text25[]="Hue";
   static char text26[]="Val";
   static char text27[]="Seg";
   static char text28[]="Cyc";
   static char text29[]="XY";
   static char text30[]="YZ";
   static char text31[]="Stf";
   static char text32[]="Inv";
   static char text33[]="Clp";

   OGL_GUI.delhooks();

   GUI_hook1=GUI_hook2=GUI_hook3=GUI_hook4=GUI_hook5=GUI_hook6=GUI_hook7=GUI_hook8=GUI_hook9=GUI_hook10=-1;
   GUI_hook11=GUI_hook12=GUI_hook13=GUI_hook14=GUI_hook15=GUI_hook16=GUI_hook17=GUI_hook18=GUI_hook19=GUI_hook20=-1;
   GUI_hook21=GUI_hook22=GUI_hook23=GUI_hook24=GUI_hook25=GUI_hook26=GUI_hook27=GUI_hook28=-1;

   if (GUI_hide) return;

   if (!GUI_capt)
      {
      // quit button
      OGL_GUI.addhook(0.025f,1.0f-0.025f-0.05f,0.075f,0.05f,
                      0.0f,1.0f,1.0f,0.75f,
                      GUI::buttonhook,(void *)quithook,GUI::buttondraw,text1);

      // move button
      GUI_hook1=OGL_GUI.addhook(0.025f,0.6f,0.075f,0.05f,
                                120.0f,0.9f,0.9f,0.75f,
                                GUI::buttonhook,(void *)acchook,GUI::buttondraw,text2);

      // stop button
      GUI_hook2=OGL_GUI.addhook(0.025f,0.55f,0.075f,0.05f,
                                240.0f,0.75f,0.9f,0.75f,
                                GUI::buttonhook,(void *)stophook,GUI::buttondraw,text3);

      // back button
      GUI_hook3=OGL_GUI.addhook(0.025f,0.5f,0.075f,0.05f,
                                120.0f,0.9f,0.9f,0.75f,
                                GUI::buttonhook,(void *)dechook,GUI::buttondraw,text4);

      // wire frame check button
      GUI_hook4=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-0.025f-0.025f,0.075f,0.025f,
                                0.0f,0.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_wire,GUI::pushbuttondraw,text5);

      // premult check button
      GUI_hook5=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-2*(0.025f+0.05f),0.075f,0.05f,
                                240.0f,0.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_premult,GUI::pushbuttondraw,text6);

      // preint check buton
      GUI_hook6=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-3*(0.025f+0.05f),0.075f,0.05f,
                                240.0f,0.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_preint,GUI::pushbuttondraw,text7);

      // rotate wheel
      OGL_GUI.addhook(0.5f-0.2f,1.0f-0.0625f,0.4f,0.0375f,
                      0.0f,0.0f,0.9f,0.75f,
                      GUI::wheelhookX,&GUI_rot,GUI::wheeldrawX,text8);

      // height slider
      OGL_GUI.addhook(1.0f-0.0625f,0.5f-0.15f,0.0375f,0.325f,
                      0.0f,0.0f,0.9f,0.75f,
                      GUI::sliderhookY,&GUI_height,GUI::sliderdrawY,text9);
      }

   if (!GUI_grad || !GUI_STF)
      if (!GUI_capt)
         {
         // small 1D transfer function for emission:

         OGL_GUI.addhook(0.025f,0.025f,0.7f,0.1f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfEhook,NULL,tfEdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f-0.008f,0.7f,0.1f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfE2hook,NULL,NULL,NULL);

         // small 1D transfer function for opacity:

         OGL_GUI.addhook(0.025f,0.025f+0.1f+0.025f,0.7f,0.1f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfAhook,NULL,tfAdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f+0.1f+0.025f-0.008f,0.7f,0.1f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfA2hook,NULL,NULL,NULL);
         }
      else
         {
         // full width 1D transfer function for emission:

         OGL_GUI.addhook(0.025f,0.025f,0.95f,0.1f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfEhook,NULL,tfEdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f-0.008f,0.95f,0.1f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfE2hook,NULL,NULL,NULL);

         // full width 1D transfer function for opacity:

         OGL_GUI.addhook(0.025f,0.025f+0.1f+0.025f,0.95f,0.1f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfAhook,NULL,tfAdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f+0.1f+0.025f-0.008f,0.95f,0.1f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfA2hook,NULL,NULL,NULL);
         }
   else
      if (!GUI_capt)
         {
         // large 2D transfer function:

         OGL_GUI.addhook(0.025f,0.025f,0.7f,0.2f+0.025f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfEhook,NULL,tfEdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f-0.008f,0.7f,0.2f+0.025f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfE2hook,NULL,NULL,NULL);
         }
      else
         {
         // full width 2D transfer function:

         OGL_GUI.addhook(0.025f,0.025f,0.95f,0.2f+0.025f,
                         0.0f,0.0f,0.2f,0.9f,
                         tfEhook,NULL,tfEdraw,NULL);

         OGL_GUI.addhook(0.025f,0.025f-0.008f,0.95f,0.2f+0.025f+2*0.008f,
                         0.0f,0.0f,0.0f,0.0f,
                         tfE2hook,NULL,NULL,NULL);
         }

   if (!GUI_capt)
      {
      // blue emission scaling slider
      OGL_GUI.addhook(0.8f,0.025f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfEsliderhook,&GUI_be_scale,GUI::sliderdrawX,text12);

      // blue emission modification check button
      GUI_hook7=OGL_GUI.addhook(0.7375f,0.025f,0.05f,0.03f,
                                240.0f,1.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_be_mod,GUI::pushbuttondraw,text12);

      // green emission scaling slider
      OGL_GUI.addhook(0.8f,0.025f+0.05f-0.015f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfEsliderhook,&GUI_ge_scale,GUI::sliderdrawX,text11);

      // green emission modification check button
      GUI_hook8=OGL_GUI.addhook(0.7375f,0.025f+0.05f-0.015f,0.05f,0.03f,
                                120.0f,1.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_ge_mod,GUI::pushbuttondraw,text11);

      // red emission scaling slider
      OGL_GUI.addhook(0.8f,0.025f+0.1f-0.03f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfEsliderhook,&GUI_re_scale,GUI::sliderdrawX,text10);

      // red emission modification check button
      GUI_hook9=OGL_GUI.addhook(0.7375f,0.025f+0.1f-0.03f,0.05f,0.03f,
                                0.0f,1.0f,0.9f,0.75f,
                                GUI::pushbuttonhook,&GUI_re_mod,GUI::pushbuttondraw,text10);

      // blue opacity scaling slider
      OGL_GUI.addhook(0.8f,0.025f+0.1f+0.025f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfAsliderhook,&GUI_ba_scale,GUI::sliderdrawX,text12);

      // blue opacity modification check button
      GUI_hook10=OGL_GUI.addhook(0.7375f,0.025f+0.1f+0.025f,0.05f,0.03f,
                                 240.0f,1.0f,0.9f,0.75f,
                                 GUI::pushbuttonhook,&GUI_ba_mod,GUI::pushbuttondraw,text12);

      // green opacity scaling slider
      OGL_GUI.addhook(0.8f,0.025f+0.1f+0.025f+0.05f-0.015f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfAsliderhook,&GUI_ga_scale,GUI::sliderdrawX,text11);

      // green opacity modification check button
      GUI_hook11=OGL_GUI.addhook(0.7375f,0.025f+0.1f+0.025f+0.05f-0.015f,0.05f,0.03f,
                                 120.0f,1.0f,0.9f,0.75f,
                                 GUI::pushbuttonhook,&GUI_ga_mod,GUI::pushbuttondraw,text11);

      // red opacity scaling slider
      OGL_GUI.addhook(0.8f,0.025f+0.1f+0.025f+0.1f-0.03f,0.175f,0.03f,
                      0.0f,0.0f,0.9f,0.75f,
                      tfAsliderhook,&GUI_ra_scale,GUI::sliderdrawX,text10);

      // red opacity modification check button
      GUI_hook12=OGL_GUI.addhook(0.7375f,0.025f+0.1f+0.025f+0.1f-0.03f,0.05f,0.03f,
                                 0.0f,1.0f,0.9f,0.75f,
                                 GUI::pushbuttonhook,&GUI_ra_mod,GUI::pushbuttondraw,text10);
      }

   // dummy hook for transfer function area
   OGL_GUI.addhook(0.0f,0.0f,1.0f,3*0.025f+0.2f,
                   0.0f,0.0f,0.0f,0.0f,
                   GUI::buttonhook,NULL,NULL,NULL);

   if (!GUI_capt)
      {
      // slab thickness slider
      GUI_hook13=OGL_GUI.addhook(0.025f,0.35f,0.035f,0.125f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::sliderhookY,&GUI_slab1,GUI::sliderdrawY,text13);

      // reduced slab thickness slider
      OGL_GUI.addhook(0.065f,0.35f,0.035f,0.125f,
                      0.0f,0.0f,0.9f,0.75f,
                      GUI::sliderhookY,&GUI_slab2,GUI::sliderdrawY,NULL);

      // coupled modification check button
      GUI_hook14=OGL_GUI.addhook(1.0f-0.025f-0.075f,0.25f+0.025f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::pushbuttonhook,&GUI_coupled,GUI::pushbuttondraw,text14);

      // save button
      GUI_hook15=OGL_GUI.addhook(0.025f,1.0f-2*(0.025f+0.05f),0.075f,0.05f,
                                 240.0f,0.5f,1.0f,0.75f,
                                 GUI::buttonhook,(void *)savehook,GUI::buttondraw,text15);

      // load button
      GUI_hook16=OGL_GUI.addhook(0.025f,1.0f-3*(0.025f+0.05f),0.075f,0.05f,
                                 240.0f,0.5f,1.0f,0.75f,
                                 GUI::buttonhook,(void *)loadhook,GUI::buttondraw,text16);

      // x-swap button
      OGL_GUI.addhook(0.025f,1.0f-4*(0.025f+0.05f),0.025f,0.025f,
                      300.0f,0.5f,1.0f,0.75f,
                      GUI::buttonhook,(void *)xswaphook,GUI::buttondraw,text17);

      // y-swap button
      OGL_GUI.addhook(0.05f,1.0f-4*(0.025f+0.05f),0.025f,0.025f,
                      300.0f,0.5f,1.0f,0.75f,
                      GUI::buttonhook,(void *)yswaphook,GUI::buttondraw,text18);

      // z-swap button
      OGL_GUI.addhook(0.075f,1.0f-4*(0.025f+0.05f),0.025f,0.025f,
                      300.0f,0.5f,1.0f,0.75f,
                      GUI::buttonhook,(void *)zswaphook,GUI::buttondraw,text19);

      // randomize button
      GUI_hook17=OGL_GUI.addhook(1.0f-0.2f,0.275f+0.025f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::buttonhook,(void *)randhook,GUI::buttondraw,text20);

      // clear button
      GUI_hook18=OGL_GUI.addhook(1.0f-0.2f,0.275f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::buttonhook,(void *)clearhook,GUI::buttondraw,text21);

      // white background check button
      GUI_hook19=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-0.025f-0.05f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::pushbuttonhook,&GUI_white,GUI::pushbuttondraw,text22);

      // 2DTF check button
      GUI_hook20=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-4*(0.025f+0.05f)+0.025f,0.075f,0.025f,
                                 200.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_grad,GUI::pushbuttondraw,text23);

      // lighting check button
      GUI_hook21=OGL_GUI.addhook(1.0f-0.025f-0.075f,1.0f-4*(0.025f+0.05f),0.075f,0.025f,
                                 200.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_light,GUI::pushbuttondraw,text24);

      // hue shift check button
      GUI_hook22=OGL_GUI.addhook(0.025f,0.275f+0.025f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_gmc,GUI::pushbuttondraw,text25);

      // display mode check button
      GUI_hook23=OGL_GUI.addhook(0.025f,0.275f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_mod,GUI::pushbuttondraw,text26);

      // segmentation check button
      GUI_hook24=OGL_GUI.addhook(0.025f+0.1f,0.275f+0.025f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_mat,GUI::pushbuttondraw,text27);

      // material cycling button
      GUI_hook25=OGL_GUI.addhook(0.025f+0.1f,0.275f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 GUI::buttonhook,(void *)cyclehook,GUI::buttondraw,text28);

      // xy-rotate button
      OGL_GUI.addhook(0.025f,1.0f-4*(0.025f+0.05f)+0.025f,0.0375f,0.025f,
                      300.0f,0.5f,1.0f,0.75f,
                      GUI::buttonhook,(void *)xrothook,GUI::buttondraw,text29);

      // yz-rotate button
      OGL_GUI.addhook(0.025f+0.0375f,1.0f-4*(0.025f+0.05f)+0.025f,0.0375f,0.025f,
                      300.0f,0.5f,1.0f,0.75f,
                      GUI::buttonhook,(void *)zrothook,GUI::buttondraw,text30);

      // STF check button
      GUI_hook26=OGL_GUI.addhook(1.0f-0.025f-0.075f,0.25f+0.05f,0.075f,0.025f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_STF,GUI::pushbuttondraw,text31);

      // inverse check button
      GUI_hook27=OGL_GUI.addhook(0.025f+0.1f,1.0f-0.025f-0.05f,0.075f,0.05f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_inv,GUI::pushbuttondraw,text32);

      // clip check button
      GUI_hook28=OGL_GUI.addhook(1.0f-0.025f-0.075f-0.1f,1.0f-0.025f-0.05f,0.075f,0.05f,
                                 0.0f,0.0f,0.9f,0.75f,
                                 reloadhook,&GUI_clip,GUI::pushbuttondraw,text33);
      }

   GUI_time=gettime();
   }

void parseargs(int argc,char *argv[])
   {
   unsigned int i;

   int arg;

   char *str1,*str2;

   int tmp;

   if (argc<2)
      {
      printf("version: %s\n",VERSION);
      printf("usage: %s <data.pvm> | <dicom*.ima> {[-]<option>=<value>}\n",argv[0]);
      printf("       basic options: bv | gf | of | im | hi\n");
      printf("        option bv = blur volume for noisy data sets\n");
      printf("        option gf = load gradient magnitude from file\n");
      printf("        option of = save input data to pvm output file\n");
      printf("        option im = use inverse mode for dark room\n");
      printf("        option hi = use high-accuracy fbo\n");
      printf("       advanced options: hm | hf | kn | hs | rd | ld\n");
      }

   if (argc<2)
      if (checkfile("Bucky.pvm")) strncpy(FILENAME,"Bucky.pvm",STR_MAX);
      else exit(1);
   else strncpy(FILENAME,argv[1],STR_MAX);

   strncpy(PROGNAME,argv[0],STR_MAX);
   strncpy(GRADNAME,"",STR_MAX);
   strncpy(OUTNAME,"",STR_MAX);

   snprintf(CONFIG,STR_MAX,"%s.sav",FILENAME);

   for (i=0; i<strlen(CONFIG); i++)
      if (CONFIG[i]=='*') CONFIG[i]='_';

   snprintf(RECORD,STR_MAX,"%s.rec",FILENAME);

   for (i=0; i<strlen(RECORD); i++)
      if (RECORD[i]=='*') RECORD[i]='_';

   for (arg=2; arg<argc; arg++)
      {
      str1=argv[arg];
      if (*str1=='-') str1++;

      str2=strchr(str1,'=');
      if (str2!=NULL) *str2++='\0';
      else continue;

      if (strcasecmp(str1,"bv")==0) {sscanf(str2,"%d",&tmp); GUI_blurvol=(tmp!=0);}
      else if (strcasecmp(str1,"gf")==0) strncpy(GRADNAME,str2,STR_MAX); // gradient file
      else if (strcasecmp(str1,"of")==0) strncpy(OUTNAME,str2,STR_MAX); // output file
      else if (strcasecmp(str1,"hm")==0) sscanf(str2,"%d",&GUI_histmin); // histogram mincount
      else if (strcasecmp(str1,"hf")==0) sscanf(str2,"%f",&GUI_histfreq); // histogram frequency
      else if (strcasecmp(str1,"kn")==0) sscanf(str2,"%d",&GUI_kneigh); // histogram neighbourhood
      else if (strcasecmp(str1,"hs")==0) sscanf(str2,"%g",&GUI_histstep); // histogram sampling
      else if (strcasecmp(str1,"rd")==0) {sscanf(str2,"%d",&tmp); GUI_record=(tmp!=0);} // record demo
      else if (strcasecmp(str1,"ld")==0) {sscanf(str2,"%d",&tmp); GUI_loop=(tmp!=0);} // loop demo
      else if (strcasecmp(str1,"im")==0) {sscanf(str2,"%d",&tmp); GUI_inv=(tmp!=0);} // inverse mode
      else if (strcasecmp(str1,"hi")==0) {sscanf(str2,"%d",&tmp); GUI_fbo=(tmp!=0);} // fbo mode
      }
   }

// main functions:

void initglobal(int argc,char *argv[])
   {
   char *ptr;

   parseargs(argc,argv);

   ptr=strrchr(PROGNAME,'/');
   if (ptr==NULL) PROGNAME[0]='\0';
   else *ptr='\0';

   VOLREN=new volren(PROGNAME);

   if (strlen(OUTNAME)>0)
      {
      loadvolume();
      VOLREN->savePVMvolume(OUTNAME);
      exit(0);
      }

   EYE_X=0.0f;
   EYE_Y=0.0f;
   EYE_Z=3.0f;

   EYE_SPEED=0.0f;

   VOLREN->get_tfunc()->set_line(0.0f,0.0f,0.33f,1.0f,VOLREN->get_tfunc()->get_be());
   VOLREN->get_tfunc()->set_line(0.33f,1.0f,0.67f,0.0f,VOLREN->get_tfunc()->get_be());
   VOLREN->get_tfunc()->set_line(0.33f,0.0f,0.67f,1.0f,VOLREN->get_tfunc()->get_ge());
   VOLREN->get_tfunc()->set_line(0.67f,1.0f,1.0f,0.0f,VOLREN->get_tfunc()->get_ge());
   VOLREN->get_tfunc()->set_line(0.67f,0.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_re());

   VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_ra());
   VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_ga());
   VOLREN->get_tfunc()->set_line(0.0f,0.0f,1.0f,1.0f,VOLREN->get_tfunc()->get_ba());

   loadhook();

   if ((GUI_recfile=fopen(RECORD,"rb"))!=NULL)
      {
      GUI_demo=TRUE;
      fclose(GUI_recfile);
      }

   if (GUI_record)
      {
      if ((GUI_recfile=fopen(RECORD,"wb"))==NULL) ERRORMSG();
      GUI_demo=FALSE;
      }

   if (GUI_demo)
      {
      if ((GUI_recfile=fopen(RECORD,"rb"))==NULL) ERRORMSG();
      GUI_start=gettime();
      }
   }

void handler(float time)
   {
   static float oldtime=0.0f;
   static float lx=0.0f,ly=0.0f;

   static float mx,my;
   static BOOLINT b1,b2,b3;
   static int key;

   int t1,t2,t3;
   float newtime;

   BOOLINT update;

   int hook;

   float over;

   if (!GUI_demo)
      {
      mx=getmousex();
      my=getmousey();

      b1=getbutton1();
      b2=getbutton2();
      b3=getbutton3();

      key=getkey();
      }
   else
      switch (key=getkey())
         {
         case '\033': // quit application
         case 'q':
         case 'Q':
         case 'x': quithook();
         }

   if (GUI_record)
      fprintf(GUI_recfile,"time=%g dt=%g mx=%g my=%g b1=%d b2=%d b3=%d key=%d\n",
              gettime()-GUI_start,time,mx,my,b1,b2,b3,(key!='D')?key:0);

   update=TRUE;

   if (GUI_demo)
      if (oldtime<gettime()-GUI_start)
         if (fscanf(GUI_recfile,"time=%g dt=%g mx=%g my=%g b1=%d b2=%d b3=%d key=%d\n",
                    &newtime,&time,&mx,&my,&t1,&t2,&t3,&key)!=8)
            if (GUI_loop==0)
               {
               oldtime=newtime;
               GUI_demo=FALSE;
               b1=b2=b3=FALSE;
               key=0;
               }
            else
               {
               fclose(GUI_recfile);
               if ((GUI_recfile=fopen(RECORD,"rb"))==NULL) ERRORMSG();

               oldtime=0.0f;
               GUI_start=gettime();
               b1=b2=b3=FALSE;
               key=0;
               }
         else
            {
            oldtime=newtime;
            b1=(t1!=0);
            b2=(t2!=0);
            b3=(t3!=0);
            }
      else update=FALSE;

   if (update)
      {
      switch (key)
         {
         case ' ': // start/stop of flight
            if (EYE_SPEED==0.0f) EYE_SPEED=EYE_MAXSPEED;
            else stophook();
            break;
         case 'a': // accelerate flight
            acchook();
            break;
         case 's': // decelerate flight
            dechook();
            break;
         case 'w': // toggle wireframe
            GUI_wire=!GUI_wire;
            break;
         case 'b': // toggle white background
            GUI_white=!GUI_white;
            break;
         case 'B': // toggle inverse mode
            GUI_inv=!GUI_inv;
            break;
         case 'm': // toggle premultiplied alpha
            GUI_premult=!GUI_premult;
            break;
         case 'i': // toggle preintegration
            GUI_preint=!GUI_preint;
            break;
         case 'g': // toggle 2DTF
            GUI_grad=!GUI_grad;
            reloadhook();
            break;
         case 'l': // toggle lighting
            GUI_light=!GUI_light;
            reloadhook();
            break;
         case 'h': // hide GUI
            if (GUI_capt) GUI_capt=FALSE;
            else GUI_hide=!GUI_hide;
            reloadhook();
            break;
         case 'H': // hide GUI except TF
            GUI_hide=FALSE;
            GUI_capt=!GUI_capt;
            reloadhook();
            break;
         case 'p': // toggle barycenters
            GUI_points=!GUI_points;
            break;
         case '+': // select higher gradients
         case 'A':
            cyclehook();
            break;
         case '-': // select lower gradients
         case 'S':
            recyclehook();
            break;
         case '>': // select more gradients
            incrangehook();
            break;
         case '<': // select fewer gradients
            decrangehook();
            break;
         case '0': // disable gradient selection
            VOLREN->get_tfunc()->unset_imp();
            break;
         case '1': // enable gradient selection
            VOLREN->get_tfunc()->set_imp(GUI_cycle,GUI_range);
            break;
         case 'C': // toggle coupling
            GUI_coupled=!GUI_coupled;
            break;
         case 'W': // write .sav file
            savehook();
            break;
         case 'R': // read .sav file
            loadhook();
            break;
         case 'D': // toggle event recording
            if (!GUI_demo)
               {
               GUI_record=!GUI_record;

               if (GUI_record)
                  {
                  if ((GUI_recfile=fopen(RECORD,"wb"))==NULL) ERRORMSG();
                  GUI_start=gettime();
                  }
               }
            break;
         case '\033': // quit application
         case 'q':
         case 'Q':
         case 'x': quithook();
         }

      if (gettime()-GUI_time>VOL_DELAY1) GUI_reduced=FALSE;

      if (!b1) OGL_GUI.release(mx,my);
      else
         {
         hook=OGL_GUI.click(mx,my);

         if (hook==0)
            if (!GUI_clip)
               {
               GUI_rotx+=mx-lx;
               GUI_roty-=my-ly;

               GUI_reduced=TRUE;
               }
            else
               {
               GUI_clip_dist-=my-ly;

               GUI_reduced=TRUE;
               }
         else
            if (hook!=GUI_hook1 &&
                hook!=GUI_hook2 &&
                hook!=GUI_hook3 &&
                hook!=GUI_hook4 &&
                hook!=GUI_hook5 &&
                hook!=GUI_hook6 &&
                hook!=GUI_hook7 &&
                hook!=GUI_hook8 &&
                hook!=GUI_hook9 &&
                hook!=GUI_hook10 &&
                hook!=GUI_hook11 &&
                hook!=GUI_hook12 &&
                hook!=GUI_hook14 &&
                hook!=GUI_hook15 &&
                hook!=GUI_hook16 &&
                hook!=GUI_hook17 &&
                hook!=GUI_hook18 &&
                hook!=GUI_hook19 &&
                hook!=GUI_hook20 &&
                hook!=GUI_hook21 &&
                hook!=GUI_hook22 &&
                hook!=GUI_hook23 &&
                hook!=GUI_hook24 &&
                hook!=GUI_hook25 &&
                hook!=GUI_hook26 &&
                hook!=GUI_hook27 &&
                hook!=GUI_hook28) GUI_reduced=TRUE;

         GUI_time=gettime();
         }

      lx=mx;
      ly=my;

      GUI_roty=fmin(fmax(GUI_roty,-0.5f),0.5f);

      EYE_DX=fsin(GUI_rotx*PI)*fcos(GUI_roty*PI);
      EYE_DY=fsin(GUI_roty*PI);
      EYE_DZ=-fcos(GUI_rotx*PI)*fcos(GUI_roty*PI);

      EYE_UX=-fsin(GUI_rotx*PI)*fsin(GUI_roty*PI);
      EYE_UY=fcos(GUI_roty*PI);
      EYE_UZ=fcos(GUI_rotx*PI)*fsin(GUI_roty*PI);

      if (b2)
         if (b3) stophook();
         else acchook();
      else if (b3) dechook();

      if (fabs(EYE_SPEED)>EYE_MAXSPEED/20.0f)
         {
         GUI_reduced=TRUE;
         GUI_time=gettime();
         }

      EYE_X+=EYE_DX*EYE_SPEED*time;
      EYE_Y+=EYE_DY*EYE_SPEED*time;
      EYE_Z+=EYE_DZ*EYE_SPEED*time;
      }

   if (!GUI_reduced)
      if (gettime()-GUI_time>VOL_DELAY2) over=VOL_OVERMIN;
      else
         if (GUI_slab1<0.5f)
            {
            float w=2.0f*GUI_slab1;
            over=((1.0f-w)*VOL_OVERMAX+w);
            }
         else
            {
            float w=2.0f*(1.0f-GUI_slab1);
            over=((1.0f-w)*VOL_OVERMIN+w);
            }
   else
      if (GUI_slab1<0.5f)
         {
         float w=2.0f*GUI_slab1;
         over=((1.0f-w)*VOL_OVERMAX+w)*fmin(1.0f/GUI_slab2,VOL_OVERMAX);
         }
      else
         {
         float w=2.0f*(1.0f-GUI_slab1);
         over=((1.0f-w)*VOL_OVERMIN+w)*fmin(1.0f/GUI_slab2,VOL_OVERMAX);
         }

   clearwindow();

   VOLREN->render(EYE_X,EYE_Y,EYE_Z,
                  EYE_DX,EYE_DY,EYE_DZ,
                  EYE_UX,EYE_UY,EYE_UZ,
                  EYE_FOVY,getaspect(),EYE_NEAR,EYE_FAR,
                  GUI_fbo,
                  360.0f*GUI_rot,
                  0.0f,4.0f*(GUI_height-0.5f),0.0f,
                  VOL_EMISSION,VOL_DENSITY,
                  GUI_re_scale,GUI_ge_scale,GUI_be_scale,
                  GUI_ra_scale,GUI_ga_scale,GUI_ba_scale,
                  GUI_premult,GUI_preint,
                  GUI_white,GUI_inv,
                  over,
                  GUI_light,
                  GUI_clip,GUI_clip_dist,
                  GUI_wire,
                  GUI_points);

   OGL_GUI.refresh();

   if (GUI_demo)
      {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0.0f,getwinwidth()-1,0.0f,getwinheight()-1);
      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      glScalef(getwinwidth()-1,getwinheight()-1,0.0f);

      glDisable(GL_DEPTH_TEST);

      glColor3f(1.0f,0.0f,0.0f);
      glBegin(GL_LINES);
      glVertex2f(0.5f+mx-0.01f/getaspect(),0.5f+my-0.01f);
      glVertex2f(0.5f+mx+0.01f/getaspect(),0.5f+my+0.01f);
      glVertex2f(0.5f+mx+0.01f/getaspect(),0.5f+my-0.01f);
      glVertex2f(0.5f+mx-0.01f/getaspect(),0.5f+my+0.01f);
      glEnd();

      glEnable(GL_DEPTH_TEST);

      glPopMatrix();
      }

   swapbuffers();
   }

int main(int argc,char *argv[])
   {
   char title[STR_MAX];

   snprintf(title,STR_MAX,"V^3 %s: Versatile Volume Viewer (c) by Stefan Roettger",VERSION);
   openwindow(WIN_WIDTH,WIN_HEIGHT,WIN_FPS,title);

   initglobal(argc,argv);
   addhandler(handler);

   return(0);
   }
