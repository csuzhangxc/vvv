// (c) by Stefan Roettger, licensed under GPL 2+

#ifndef SHADERBASE_H
#define SHADERBASE_H

int buildvtxprog(const char *prog);
void bindvtxprog(int progid);
void setvtxprogpar(int n,float p1,float p2,float p3,float p4);
void setvtxprogpars(int n,int count,const float *params);
void deletevtxprog(int progid);

int buildfrgprog(const char *prog);
void bindfrgprog(int progid);
void setfrgprogpar(int n,float p1,float p2,float p3,float p4);
void setfrgprogpars(int n,int count,const float *params);
void deletefrgprog(int progid);

static const char default_vtxprg[]=
   "!!ARBvp1.0\n"
   "OPTION ARB_position_invariant; \n"
   "MOV result.color,vertex.color; \n"
   "END\n";

static const char default_frgprg[]=
   "!!ARBfp1.0\n"
   "MOV result.color,fragment.color; \n"
   "END\n";

#endif
