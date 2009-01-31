char inline_prog1[]=
"\
!!ARBfp1.0\n\
\n\
TEMP tmp, col;\n\
\n\
# get data from 3D textures\n\
TEX tmp.x, fragment.texcoord[0], texture[0], 3D;   # get the scalar value at the back of the slab\n\
TEX tmp.y, fragment.texcoord[1], texture[0], 3D;   # get the scalar value at the front of the slab\n\
TEX tmp.z, fragment.texcoord[2], texture[1], 3D;   # get the gradient magnitude at the mid of the slab\n\
\n\
# dependent 3D texture lookup\n\
TEX col, tmp, texture[3], 3D;   # perform 3D dependent texture lookup in pre-integration table\n\
\n\
# write to output register\n\
MUL result.color, col, fragment.color.primary;   # attenuate with primary color\n\
\n\
END\n\
";

char inline_prog2[]=
"\
!!ARBfp1.0\
\n\
TEMP tmp, col;\n\
\n\
# get data from 3D textures\n\
TEX tmp.x, fragment.texcoord[0], texture[0], 3D;   # get the scalar value at the back of the slab\n\
TEX tmp.y, fragment.texcoord[1], texture[0], 3D;   # get the scalar value at the front of the slab\n\
TEX tmp.z, fragment.texcoord[2], texture[1], 3D;   # get the gradient magnitude at the mid of the slab\n\
\n\
# dependent 3D texture lookup\n\
TEX col, tmp, texture[3], 3D;   # perform 3D dependent texture lookup in pre-integration table\n\
\n\
# compute the diffuse and specular illumination factors of a head light\n\
SUB tmp.x, tmp.x, tmp.y;              # calculate frontal gradient (not yet normalized)\n\
ABS tmp.x, tmp.x;                     # use front and back lighting\n\
MUL tmp.x, tmp.x, program.env[0].x;   # scale gradient with the reciprocal of the slab thickness\n\
MAX tmp.z, tmp.z, program.env[0].y;   # neglect small noisy gradients below a threshold\n\
RCP tmp.z, tmp.z;                     # compute the reciprocal of the gradient magnitude\n\
MUL_SAT tmp.x, tmp.x, tmp.z;          # normalize frontal gradient\n\
POW tmp.y, tmp.x, program.env[1].w;   # compute specular highlight\n\
\n\
# attenuate the emission with the calculated illumination factors\n\
MAD tmp.x, tmp.x, program.env[1].y, program.env[1].x;   # ambient and diffuse illumination\n\
MAD tmp.x, tmp.y, program.env[1].z, tmp.x;              # specular illumination\n\
MUL col.rgb, col, tmp.x;                                # apply to emission\n\
\n\
# write to output register\n\
MUL result.color, col, fragment.color.primary;   # attenuate with primary color\n\
\n\
END\n\
";

char *inline_prog[2]={inline_prog1,inline_prog2};
