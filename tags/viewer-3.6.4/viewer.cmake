# v^3 header and module list

SET(VOLREN_HDRS
   volren/ddsbase.h volren/dicombase.h volren/rekbase.h volren/rawbase.h
   volren/dirbase.h volren/oglbase.h
   volren/tfbase.h volren/tilebase.h
   volren/volume.h volren/volren.h
   )

SET(VOLREN_SRCS
   volren/ddsbase.cpp volren/dicombase.cpp volren/rekbase.cpp volren/rawbase.cpp
   volren/dirbase.cpp volren/oglbase.cpp
   volren/tfbase.cpp volren/tilebase.cpp
   volren/volume.cpp
   )

SET(VIEWER_HDRS
   glutbase.h guibase.h
   )

SET(VIEWER_SRCS
   glutbase.cpp guibase.cpp
   )
