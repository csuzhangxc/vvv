# cmake build file

IF (WIN32)
   SET(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" CACHE STRING "" FORCE)
ENDIF (WIN32)

PROJECT(QTV3VolumeRenderer)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

OPTION(BUILD_WITH_DCMTK "Build with DICOM support." OFF)
OPTION(FIND_DCMTK_MANUALLY "Do not rely on CMake to find DCMTK." OFF)

# path to custom cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# determine build type
IF (CMAKE_BUILD_TYPE)
   STRING(TOUPPER ${CMAKE_BUILD_TYPE} QTV3_BUILD_TYPE)
ELSE (CMAKE_BUILD_TYPE)
   SET(QTV3_BUILD_TYPE DEFAULT)
ENDIF (CMAKE_BUILD_TYPE)

# non-standard path to zlib
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};
    ../zlib;../../zlib;../../../zlib;
   )

# non-standard path to dcmtk
# caution: when using a cmake built dcmtk library on WIN32,
#          make sure that the option DCMTK_OVERWRITE_WIN32_COMPILER_FLAGS is OFF.
#          otherwise dcmtk is built with /MT and not as usual with the cmake default /MD.
#          this will conflict with the /MD setting of this project.
FIND_PATH(DCMTK_DIR include/dcmtk/config/osconfig.h PATHS /usr/local /usr/local/dcmtk ../dcmtk ../../dcmtk ../../../dcmtk)

# non-standard path to Qt4
# note: when building a static version of qt to distribute an executable /wo having to distribute the qt dlls
#       set the cmake option BUILD_SHARED_LIBS to OFF, e.g. with "cmake -DBUILD_SHARED_LIBS=OFF .".
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};
    /usr/local/Trolltech/Qt-4.7.4;
   )

# Qt4 dependency
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui REQUIRED)
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

# OpenGL dependency
FIND_PACKAGE(OpenGL)

IF (BUILD_WITH_DCMTK)

   # find threads library
   FIND_PACKAGE(Threads)

   # find ZLIB dependency
   FIND_PACKAGE(ZLIB)
   INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

   # find DCMTK dependencies
   IF (FIND_DCMTK_MANUALLY)
      FIND_LIBRARY(DCMTK_ofstd_LIBRARY ofstd PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_oflog_LIBRARY oflog PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_dcmdata_LIBRARY dcmdata PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_dcmjpeg_LIBRARY dcmjpeg PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_ijg8_LIBRARY ijg8 PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_ijg12_LIBRARY ijg12 PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_ijg16_LIBRARY ijg16 PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_LIBRARY(DCMTK_dcmtls_LIBRARY dcmtls PATHS ${DCMTK_DIR} PATH_SUFFIXES lib release minsizerel relwithdebinfo)
      FIND_PATH(DCMTK_INCLUDE_DIR dcmtk/dcmdata/dctk.h PATHS ${DCMTK_DIR} PATH_SUFFIXES include)
   ELSE (FIND_DCMTK_MANUALLY)
      FIND_PACKAGE(DCMTK)
   ENDIF (FIND_DCMTK_MANUALLY)
   INCLUDE_DIRECTORIES(${DCMTK_INCLUDE_DIR})
   IF (NOT WIN32)
      ADD_DEFINITIONS(-DHAVE_CONFIG_H)
   ENDIF (NOT WIN32)

   ADD_DEFINITIONS(-DVIEWER_HAVE_DCMTK)

ENDIF (BUILD_WITH_DCMTK)

# volren dependency
FIND_PATH(VOLREN_PATH volren.h PATHS .. PATH_SUFFIXES volren)
IF (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
   FIND_LIBRARY(VOLREN_LIBRARY VolRen libVolren PATHS .. PATH_SUFFIXES lib RelWithDebInfo)
ELSE (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
   FIND_LIBRARY(VOLREN_LIBRARY VolRend libVolrend PATHS .. PATH_SUFFIXES lib RelWithDebInfo)
ENDIF (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
INCLUDE_DIRECTORIES(${VOLREN_PATH})

# header list
SET(LIB_HDRS
   volren_qgl.h
   mainwindow.h
   )

# module list
SET(LIB_SRCS
   mainwindow.cpp
   )

# moc
QT4_WRAP_CPP(MOC_OUTFILES ${LIB_HDRS})

# library
SET(LIB_NAME ${PROJECT_NAME})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
ADD_LIBRARY(${LIB_NAME} ${LIB_SRCS} ${LIB_HDRS} ${MOC_OUTFILES})

# executable
ADD_EXECUTABLE(qtv3 qtv3.cpp)
TARGET_LINK_LIBRARIES(qtv3
   ${LIB_NAME}
   ${QT_LIBRARIES}
   ${VOLREN_LIBRARY}
   ${OPENGL_LIBRARIES}
   )
IF (BUILD_WITH_DCMTK)
   IF (FIND_DCMTK_MANUALLY)
      TARGET_LINK_LIBRARIES(qtv3
         ${DCMTK_dcmdata_LIBRARY}
         ${DCMTK_dcmjpeg_LIBRARY}
         ${DCMTK_ijg8_LIBRARY}
         ${DCMTK_ijg12_LIBRARY}
         ${DCMTK_ijg16_LIBRARY}
         ${DCMTK_dcmtls_LIBRARY}
         ${DCMTK_oflog_LIBRARY}
         ${DCMTK_ofstd_LIBRARY}
         ${ZLIB_LIBRARY}
         ${CMAKE_THREAD_LIBS_INIT}
         )
   ELSE (FIND_DCMTK_MANUALLY)
      TARGET_LINK_LIBRARIES(qtv3
         ${DCMTK_LIBRARIES}
         ${ZLIB_LIBRARY}
         ${CMAKE_THREAD_LIBS_INIT}
         )
   ENDIF (FIND_DCMTK_MANUALLY)
ENDIF (BUILD_WITH_DCMTK)

# install target
INSTALL(
   TARGETS qtv3
   RUNTIME DESTINATION bin
)

# install .desktop file and icon for KDE4 (and Gnome)
IF (UNIX AND NOT APPLE)
   INSTALL(FILES QTV3.desktop DESTINATION /usr/share/applications)
   INSTALL(FILES qtv3.png DESTINATION /usr/share/qtv3)
ENDIF (UNIX AND NOT APPLE)
