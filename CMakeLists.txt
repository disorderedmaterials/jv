# Root CMakeLists for JournalViewer / JournalViewerLite

# Before we do anything else, set the target name
if(WIN32 OR APPLE)
	if(LITE)
		set(nice_target_name "JournalViewerLite")
	else(LITE)
		set(nice_target_name "JournalViewer")
	endif(LITE)
	set (target_name ${nice_target_name})
else(WIN32 OR APPLE)
	if(LITE)
		set (target_name jvlite)
	else(LITE)
		set (target_name jv)
	endif(LITE)
endif(WIN32 OR APPLE)

project(${target_name})

set(DESCRIPTION ${target_name})
set(AUTHOR "Tristan Youngs")
set(VERSION_MAJOR "1")
set(VERSION_MINOR "8")
set(VERSION_PATCH "4")

# Build release version
set(CMAKE_BUILD_TYPE "Release")

# -- LibGET source
OPTION(EXTERNAL_LIBGET "Switch to use external (precompiled) libget" OFF)
if(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "-L${LIBGET_DIR} -lget" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${LIBGET_DIR}" CACHE STRING "Include dir for libget (external)")
else(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "get" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${CMAKE_BINARY_DIR}/src/get" CACHE STRING "Include dir for libget (internal)")
endif(EXTERNAL_LIBGET)

include(ExternalProject)
enable_language(Fortran)

# ZLIB
# Locate installed library, or build as external version (if BUILD_ZLIB)
option(BUILD_ZLIB "Build ZLib as an external project" ON)
add_library(zliblib STATIC IMPORTED GLOBAL)
if(BUILD_ZLIB)
ExternalProject_Add(
  zlib
  PREFIX 3rdparty
  URL https://github.com/madler/zlib/archive/refs/tags/v1.2.11.zip
  BUILD_ALWAYS ON
  INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/3rdparty/lib/libz.a"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)

  set_property(
    TARGET zliblib PROPERTY IMPORTED_LOCATION
                             "${CMAKE_BINARY_DIR}/3rdparty/lib/libz.a")
else(BUILD_ZLIB)
  find_package(ZLIB REQUIRED)
  set_property(
    TARGET zliblib PROPERTY IMPORTED_LOCATION
                             "${ZLIB_LIBRARIES}")
endif(BUILD_ZLIB)

# Build SZIP as an external project
ExternalProject_Add(
  szip
  PREFIX 3rdparty
  URL https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz
  BUILD_ALWAYS ON
  INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/3rdparty/lib/libszip-static.a"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
add_library(sziplib STATIC IMPORTED GLOBAL)
set_property(
  TARGET sziplib PROPERTY IMPORTED_LOCATION
                           "${CMAKE_BINARY_DIR}/3rdparty/lib/libszip-static.a")

# Build HDF5 as an external project
ExternalProject_Add(
  HDF5
  PREFIX 3rdparty
  URL https://github.com/HDFGroup/hdf5/archive/refs/tags/hdf5-1_10_7.zip
  BUILD_ALWAYS ON
  INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5.a" "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_fortran.a" "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_f90cstub.a"
  CMAKE_ARGS -DHDF5_BUILD_FORTRAN:bool=True
             -DHDF5_INSTALL_MOD_FORTRAN:string=STATIC
             -DHDF5_ENABLE_SZIP_SUPPORT:bool=True
             -DHDF5_ENABLE_Z_LIB_SUPPORT:bool=True
             -DCMAKE_Fortran_COMPILER=${CMAKE_Fortran_COMPILER}
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
add_library(hdf5main STATIC IMPORTED GLOBAL)
set_property(
  TARGET hdf5main
  PROPERTY IMPORTED_LOCATION
           "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5.a")
add_dependencies(hdf5main HDF5 szip)
add_library(hdf5fortran STATIC IMPORTED GLOBAL)
set_property(
  TARGET hdf5fortran
  PROPERTY IMPORTED_LOCATION
           "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_fortran.a")
add_dependencies(hdf5fortran HDF5 szip)
add_library(hdf5f90cstub STATIC IMPORTED GLOBAL)
set_property(
  TARGET hdf5f90cstub
  PROPERTY IMPORTED_LOCATION
           "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_f90cstub.a")
add_dependencies(hdf5f90cstub HDF5 szip)

# Find required packages
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Svg REQUIRED)
if(USE_HDF)
	if(WIN32)
	FIND_PACKAGE(HDF5 COMPONENTS C HL NO_MODULE REQUIRED shared)
	else(WIN32)
		find_package(HDF5 REQUIRED)
	endif(WIN32)
	find_package(ZLIB REQUIRED)
endif(USE_HDF)

# Set output paths
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(SRCS src/)

# Set search locations for header files (for both project and external dependencies)
include_directories(
	${CMAKE_BINARY_DIR}/src
	${CMAKE_BINARY_DIR}/3rdparty/include
	${LIBGET_INCLUDES}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
)

if(UNIX)
	# Qt5 default is to build with -reduce-relocations, meaning we must build position-independent code...
	add_definitions(-fPIC)
endif(UNIX)

# Process CMakeLists.txt in src subdirectory
add_subdirectory(${SRCS})

# Set bundle information for Mac
If(APPLE)
	set(MACOSX_BUNDLE_ICON_FILE  "${target_name}.icns")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER  ${target_name})
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	set(MACOSX_BUNDLE_BUNDLE_NAME  ${target_name} )
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION  "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	set(MACOSX_BUNDLE_COPYRIGHT "${AUTHOR}")
endif(APPLE)



# Tag executable for generation
add_executable(${target_name}  MACOSX_BUNDLE
	${SRCS}/main.cpp 
)

# Request C++11 Standard Support (Required for Qt 5.7)
set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 11)

# Link line
link_directories (${LIBRARY_OUTPUT_PATH})
IF(WIN32 OR APPLE)
	target_link_libraries(${target_name} PUBLIC main ${LIBGET_LIBRARY} INTERFACE Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport PRIVATE hdf5fortran hdf5f90cstub hdf5main sziplib zliblib dl)
ELSE(WIN32 OR APPLE)
	target_link_libraries(${target_name} PUBLIC main ${LIBGET_LIBRARY} INTERFACE Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport PRIVATE hdf5fortran hdf5f90cstub hdf5main sziplib zliblib dl)
ENDIF(WIN32 OR APPLE)

