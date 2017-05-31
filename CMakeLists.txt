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

# Set minimum versions for CMake, and tweak some policies
cmake_minimum_required(VERSION 2.8.11)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif(COMMAND cmake_policy)
# -- Policy for automatic linkage of Qt libs to project
CMAKE_POLICY(SET CMP0020 OLD)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Make sure CMake won't try to generate rules for moc (we will do it ourselves)
set(CMAKE_AUTOMOC OFF)

# Enable Fortran bindings (for libget)
enable_language (Fortran)

# Set macro definition if building lite version
if(LITE)
	add_definitions(-DLITE)
endif(LITE)

# Options
# -- HDF5 Support
OPTION(USE_HDF "Include HDF5 support to read NEXUS files" ON)
if(NOT USE_HDF)
	ADD_DEFINITIONS("-DNOHDF")
endif(NOT USE_HDF)

# Platform-specific configuration

# -- Linux
if(UNIX)
	# Qt5 default is to build with -reduce-relocations, meaning we must build position-independent code...
	add_definitions(-fPIC)
endif(UNIX)

# -- Windows
if(WIN32)
	# Additional defines for building libget
	add_definitions(-D_WIN32 -DGCC_MINGW)

	# Request C++11 Standard Support (Required for Qt 5.7)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

	# Find ZLib
	set(ZLIB_ROOT "C:/Users/axr61803/src/zlib_build")
	set(ZLIB_INCLUDE_DIR "C:/Users/axr61803/src/zlib-1.2.5")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/Users/axr61803/src/zlib_build")

	# HDF5 support?
	if(USE_HDF)
		set(HDF5_DIR "C:/Users/axr61803/src/hdf5_build")
		set(HDF5_TOOLS_DIR "C:/Users/axr61803/src/hdf5_build/bin")
		set(HDF5_INCLUDE_DIRS "C:/Users/axr61803/src/hdf5-1.10.1/src;C:/Users/axr61803/src/hdf5_build")
		set(HDF5_LIBS "-LC:/Users/axr61803/src/hdf5_build/bin -lhdf5-static" CACHE STRING "Link-time commands to use HDF5 (Windows only)")
	endif(USE_HDF)

	# Add global NOMINMAX define for Windows systems, and inhibit console creation
	ADD_DEFINITIONS("-DNOMINMAX")
	SET(GUI_TYPE WIN32)
	if(MINGW)
		SET(CMAKE_EXE_LINKER_FLAGS "-mwindows")
	endif(MINGW)
endif(WIN32)

# -- Mac
if(APPLE)
	ADD_DEFINITIONS(-D_MAC)

	# Additional defines for building libget
	add_definitions(-D__APPLE__)

	# Set some specific C++11 related options here (set_property below does not seem to persist)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11 ")

	# Point directly to the HDF libraries, since find_package() doesn't seem to set the LIBRARIES variables
	set(HDF5_DIR "/usr/local/hdf-1.8.13")
	set(HDF5_LIBS "-L${HDF5_DIR}/lib -lhdf5" CACHE STRING "Link-time commands to use HDF5 (Mac only)")
endif(APPLE)

# -- LibGET source
OPTION(EXTERNAL_LIBGET "Switch to use external (precompiled) libget" OFF)
if(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "-L${LIBGET_DIR} -lget" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${LIBGET_DIR}" CACHE STRING "Include dir for libget (external)")
else(EXTERNAL_LIBGET)
	set(LIBGET_LIBRARY "get" CACHE STRING "Link-time commands for external libget")
	set(LIBGET_INCLUDES "${CMAKE_BINARY_DIR}/src/get" CACHE STRING "Include dir for libget (internal)")
endif(EXTERNAL_LIBGET)

# Find required packages
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
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
	${LIBGET_INCLUDES}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
)

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
	target_link_libraries(${target_name} main ${LIBGET_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport ${HDF5_LIBS} ${ZLIB_LIBRARIES})
ELSE(WIN32 OR APPLE)
	target_link_libraries(${target_name} main ${LIBGET_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Network Qt5::PrintSupport ${HDF5_LIBRARIES})
ENDIF(WIN32 OR APPLE)

