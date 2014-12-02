# v^3 header and module list

SET(VOLREN_HDRS
   volren/codebase.h
   volren/ddsbase.h volren/dicombase.h
   volren/dirbase.h volren/oglbase.h volren/shaderbase.h
   volren/tfbase.h volren/tilebase.h volren/progs.h
   volren/volume.h volren/volren.h
   volren/geobase.h
   volren/v3d.h
   )

SET(VOLREN_SRCS
   volren/ddsbase.cpp volren/dicombase.cpp
   volren/dirbase.cpp volren/oglbase.cpp volren/shaderbase.cpp
   volren/tfbase.cpp volren/tilebase.cpp
   volren/volume.cpp
   volren/geobase.cpp
   )

SET(VIEWER_HDRS
   glutbase.h guibase.h
   )

SET(VIEWER_SRCS
   glutbase.cpp guibase.cpp
   )
