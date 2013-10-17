# cmake build file

IF (WIN32)
   SET(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" CACHE STRING "" FORCE)
ENDIF (WIN32)

PROJECT(QTV3VolumeRenderer)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

OPTION(FIND_DCMTK_MANUALLY "Do not rely on CMake to find DCMTK." OFF)
OPTION(BUILD_WITH_FBO "Build with FBO." ON)

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

# find libmini library
FIND_PACKAGE(MINI)

# determine libmini status
IF (MINI_FOUND)
   INCLUDE_DIRECTORIES(${MINI_INCLUDE_DIR})
   ADD_DEFINITIONS(-DHAVE_MINI)
ENDIF (MINI_FOUND)

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

# determine DCMTK status
IF (DCMTK_FOUND)
   INCLUDE_DIRECTORIES(${DCMTK_INCLUDE_DIR})
   IF (NOT WIN32)
      ADD_DEFINITIONS(-DHAVE_CONFIG_H)
   ENDIF (NOT WIN32)

   # find threads library
   FIND_PACKAGE(Threads)

   # find ZLIB dependency
   FIND_PACKAGE(ZLIB)
   INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

   ADD_DEFINITIONS(-DHAVE_DCMTK)
ENDIF (DCMTK_FOUND)

# volren dependency
FIND_PATH(VOLREN_PATH volren.h PATHS . .. PATH_SUFFIXES volren NO_DEFAULT_PATH)
FIND_PATH(VOLREN_PATH volren.h PATHS . .. PATH_SUFFIXES volren)
IF (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
   FIND_LIBRARY(VOLREN_LIBRARY VolRen libVolren PATHS . .. PATH_SUFFIXES lib RelWithDebInfo NO_DEFAULT_PATH)
   FIND_LIBRARY(VOLREN_LIBRARY VolRen libVolren PATHS . .. PATH_SUFFIXES lib RelWithDebInfo)
ELSE (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
   FIND_LIBRARY(VOLREN_LIBRARY VolRend libVolrend PATHS . .. PATH_SUFFIXES lib RelWithDebInfo NO_DEFAULT_PATH)
   FIND_LIBRARY(VOLREN_LIBRARY VolRend libVolrend PATHS . .. PATH_SUFFIXES lib RelWithDebInfo)
ENDIF (NOT QTV3_BUILD_TYPE MATCHES DEBUG)
INCLUDE_DIRECTORIES(${VOLREN_PATH})

# header list
SET(LIB_HDRS
   volren_qgl.h
   mainwindow.h
   prefwindow.h
   )

# module list
SET(LIB_SRCS
   mainwindow.cpp
   prefwindow.cpp
   )

# check for fbo
IF (NOT BUILD_WITH_FBO)
   ADD_DEFINITIONS(-DHAVE_NO_FBO)
ENDIF (NOT BUILD_WITH_FBO)

# moc
QT4_WRAP_CPP(MOC_OUTFILES ${LIB_HDRS})

# library
SET(LIB_NAME ${PROJECT_NAME})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
ADD_LIBRARY(${LIB_NAME} ${LIB_SRCS} ${LIB_HDRS} ${MOC_OUTFILES})

# bundle icon
SET(MACOSX_BUNDLE_ICON_FILE "qtv3.icns")
SET_SOURCE_FILES_PROPERTIES(
   ${MACOSX_BUNDLE_ICON_FILE}
   PROPERTIES
   MACOSX_PACKAGE_LOCATION Resources
   )

# executable
SET(APPNAME qtv3)
SET(MAINNAME qtv3.cpp)
ADD_EXECUTABLE(${APPNAME} MACOSX_BUNDLE ${MAINNAME} ${MACOSX_BUNDLE_ICON_FILE})
TARGET_LINK_LIBRARIES(${APPNAME}
   ${LIB_NAME}
   ${QT_LIBRARIES}
   ${VOLREN_LIBRARY}
   ${OPENGL_LIBRARIES}
   )
IF (MINI_FOUND)
   TARGET_LINK_LIBRARIES(${APPNAME}
      ${MINI_LIBRARIES}
      )
ENDIF (MINI_FOUND)
IF (DCMTK_FOUND)
   IF (FIND_DCMTK_MANUALLY)
      TARGET_LINK_LIBRARIES(${APPNAME}
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
      TARGET_LINK_LIBRARIES(${APPNAME}
         ${DCMTK_LIBRARIES}
         ${ZLIB_LIBRARY}
         ${CMAKE_THREAD_LIBS_INIT}
         )
   ENDIF (FIND_DCMTK_MANUALLY)
ENDIF (DCMTK_FOUND)

# install target
INSTALL(
   TARGETS ${APPNAME}
   RUNTIME DESTINATION bin
   BUNDLE DESTINATION /Applications
   )

# install .desktop file and icon for KDE4 (and Gnome)
IF (UNIX AND NOT APPLE)
   INSTALL(FILES QTV3.desktop DESTINATION /usr/share/applications)
   INSTALL(FILES qtv3.png DESTINATION /usr/share/qtv3)
ENDIF (UNIX AND NOT APPLE)

# install data
IF (UNIX)
   INSTALL(FILES Drop.pvm DESTINATION /usr/share/qtv3)
ENDIF (UNIX)
