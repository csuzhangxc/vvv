// (c) by Stefan Roettger, licensed under GPL 2+

char slicer_frgprg[]=
"\
!!ARBfp1.0\n\
\n\
PARAM range=program.env[0];\n\
TEMP col, tmp;\n\
\n\
# get data from 3D texture and attenuate with primary color\n\
MOV col, fragment.color.primary;\n\
TEX tmp.x, fragment.texcoord[0], texture[0], 3D;\n\
MUL col.xyz, col, tmp.x;\n\
\n\
# calculate windowing opacities\n\
SUB tmp.w, tmp.x, range.x;\n\
CMP col.w, tmp.w, range.w, col.w;\n\
SUB tmp.w, range.y, tmp.x;\n\
CMP col.w, tmp.w, range.w, col.w;\n\
\n\
# write to output register\n\
MOV result.color, col;\n\
\n\
END\n\
";

char slicer_frgprg_sfx[]=
"\
!!ARBfp1.0\n\
\n\
PARAM range=program.env[0];\n\
TEMP col, tmp;\n\
\n\
# stereo interlacing\n\
MAD tmp.xy, fragment.position, program.env[2], program.env[2].zwxy;\n\
FRC tmp.xy, tmp;\n\
SUB tmp.xy, tmp, 0.5;\n\
KIL tmp.xyxy;\n\
\n\
# get data from 3D texture and attenuate with primary color\n\
MOV col, fragment.color.primary;\n\
TEX tmp.x, fragment.texcoord[0], texture[0], 3D;\n\
MUL col.xyz, col, tmp.x;\n\
\n\
# calculate windowing opacities\n\
SUB tmp.w, tmp.x, range.x;\n\
CMP col.w, tmp.w, range.w, col.w;\n\
SUB tmp.w, range.y, tmp.x;\n\
CMP col.w, tmp.w, range.w, col.w;\n\
\n\
# write to output register\n\
MOV result.color, col;\n\
\n\
END\n\
";
