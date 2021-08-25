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
set(CMAKE_CXX_STANDARD 11)


# -- LibGET source
OPTION(EXTERNAL_LIBGET "Switch to use external (precompiled) libget" OFF)
if(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "-L${LIBGET_DIR} -lget" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${LIBGET_DIR}" CACHE STRING "Include dir for libget (external)")
else(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "get" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${CMAKE_BINARY_DIR}/src/get" CACHE STRING "Include dir for libget (internal)")
endif(EXTERNAL_LIBGET)

# Inclide application support
include(ExternalProject)
enable_language(Fortran)
option(
  LOCAL_STATIC_HDF5
  "Use local HDF5 installation (specified with HDF5_DIR) built with static ZLIB and SZIP support (so don't search for them)"
  OFF)

# ZLIB Locate installed library, or build as external version (if BUILD_ZLIB)
option(BUILD_ZLIB "Build ZLib as an external project" OFF)
if(BUILD_ZLIB)
  ExternalProject_Add(
    zlib
    PREFIX 3rdparty
    URL https://github.com/madler/zlib/archive/refs/tags/v1.2.11.zip
    BUILD_ALWAYS ON
    INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/3rdparty/lib/libzlibstatic.a"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)

  add_library(zliblib STATIC IMPORTED GLOBAL)
  set_property(
    TARGET zliblib PROPERTY IMPORTED_LOCATION
                            "${CMAKE_BINARY_DIR}/3rdparty/lib/libzlibstatic.a")
  list(APPEND LINK_LIBS "zliblib")
else(BUILD_ZLIB)
  if(NOT LOCAL_STATIC_HDF5)
    find_package(ZLIB REQUIRED)
    list(APPEND LINK_LIBS "${ZLIB_LIBRARIES}")
  endif(NOT LOCAL_STATIC_HDF5)
endif(BUILD_ZLIB)

# Build SZIP as an external project
option(BUILD_SZIP "Build SZIP as an external project" OFF)
if(BUILD_SZIP)
  ExternalProject_Add(
    szip
    PREFIX 3rdparty
    URL https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz
    BUILD_ALWAYS ON
    INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/3rdparty/lib/libszip-static.a"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    INSTALL_COMMAND ${CMAKE_COMMAND} --install .
    COMMAND ${CMAKE_COMMAND} -E remove -f
            ${CMAKE_BINARY_DIR}/3rdparty/src/szip-build/szip-config.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f
            ${CMAKE_BINARY_DIR}/3rdparty/src/szip-build/szip-targets.cmake)
  add_library(sziplib STATIC IMPORTED GLOBAL)
  set_property(
    TARGET sziplib PROPERTY IMPORTED_LOCATION
                            "${CMAKE_BINARY_DIR}/3rdparty/lib/libszip-static.a")
  list(APPEND LINK_LIBS "sziplib")
else(BUILD_SZIP)
  if(NOT LOCAL_STATIC_HDF5)
    find_package(SZIP REQUIRED)
    list(APPEND LINK_LIBS "${SZIP_LIBRARIES}")
  endif(NOT LOCAL_STATIC_HDF5)
endif(BUILD_SZIP)

# Build HDF5 as an external project
option(BUILD_HDF "Build HDF as an external project" OFF)
if(BUILD_HDF)
  if(WIN32)
    set(HDF_FORTRAN_LINK_FLAGS "-Wl,--allow-multiple-definition")
  endif(WIN32)
  ExternalProject_Add(
    HDF5
    PREFIX 3rdparty
    URL https://github.com/HDFGroup/hdf5/archive/refs/tags/hdf5-1_10_7.zip
    BUILD_ALWAYS ON
    INSTALL_DIR "${CMAKE_BINARY_DIR}/3rdparty/"
    BUILD_BYPRODUCTS
      "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5.a"
      "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_fortran.a"
      "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_f90cstub.a"
    CMAKE_ARGS -DHDF5_BUILD_FORTRAN:bool=True
               -DHDF5_INSTALL_MOD_FORTRAN:string=STATIC
               -DHDF5_ENABLE_HDF_SUPPORT:bool=True
               -DHDF5_ENABLE_Z_LIB_SUPPORT:bool=True
               -DBUILD_TESTING:bool=False
               -DCMAKE_Fortran_COMPILER=${CMAKE_Fortran_COMPILER}
               -DCMAKE_Fortran_FLAGS:string=${HDF_FORTRAN_LINK_FLAGS}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
  add_library(hdf5main STATIC IMPORTED GLOBAL)
  set_property(
    TARGET hdf5main PROPERTY IMPORTED_LOCATION
                             "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5.a")
  add_library(hdf5fortran STATIC IMPORTED GLOBAL)
  set_property(
    TARGET hdf5fortran
    PROPERTY IMPORTED_LOCATION
             "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_fortran.a")
  add_library(hdf5f90cstub STATIC IMPORTED GLOBAL)
  set_property(
    TARGET hdf5f90cstub
    PROPERTY IMPORTED_LOCATION
             "${CMAKE_BINARY_DIR}/3rdparty/lib/libhdf5_f90cstub.a")
  list(APPEND LINK_LIBS "hdf5f90cstub" "hdf5fortran" "hdf5main")
  if(BUILD_SZIP)
    add_dependencies(hdf5main HDF5 szip)
    add_dependencies(hdf5fortran HDF5 szip)
    add_dependencies(hdf5f90cstub HDF5 szip)
  endif(BUILD_SZIP)
else(BUILD_HDF)
  if(LOCAL_STATIC_HDF5)
    list(APPEND LINK_LIBS "${HDF5_DIR}/lib/libhdf5_fortran.a"
         "${HDF5_DIR}/lib/libhdf5_f90cstub.a" "${HDF5_DIR}/lib/libhdf5.a"
         "${HDF5_DIR}/lib/libszip-static.a")
    if(WIN32)
      list(APPEND LINK_LIBS "${HDF5_DIR}/lib/libzlibstatic.a")
    else(WIN32)
      list(APPEND LINK_LIBS "${HDF5_DIR}/lib/libz.a")
    endif(WIN32)
    include_directories(${HDF5_DIR}/include ${HDF5_DIR}/include/static)
  else(LOCAL_STATIC_HDF5)
    find_package(HDF5 REQUIRED)
    list(APPEND LINK_LIBS "${HDF5_CXX_LIBRARIES}" "${HDF5_C_LIBRARIES}" "${HDF5_Fortran_LIBRARIES}")
    include_directories(${HDF5_INCLUDE_DIRS})
  endif(LOCAL_STATIC_HDF5)
endif(BUILD_HDF)

# Set necessary include directories
if(BUILD_HDF)
  include_directories(${CMAKE_BINARY_DIR}/3rdparty/include)
endif(BUILD_HDF)

# Find required packages
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Svg REQUIRED)

# Set output paths
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(SRCS src/)

# Set search locations for header files (for both project and external dependencies)
include_directories(
	${CMAKE_BINARY_DIR}/src
	${LIBGET_INCLUDES}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
)

# Handle specific platforms
if(UNIX)
	# Qt5 default is to build with -reduce-relocations, meaning we must build position-independent code...
	add_definitions(-fPIC)
endif(UNIX)

if(WIN32)
	# Additional defines for building libget
	add_definitions(-D_WIN32 -DGCC_MINGW)
endif(WIN32)
	
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

# Link line
link_directories (${LIBRARY_OUTPUT_PATH})
IF(WIN32)
	target_link_libraries(${target_name} PUBLIC main ${LIBGET_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport PRIVATE ${LINK_LIBS})
ELSE(WIN32)
	target_link_libraries(${target_name} PUBLIC main ${LIBGET_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport PRIVATE ${LINK_LIBS} dl)
ENDIF(WIN32)
