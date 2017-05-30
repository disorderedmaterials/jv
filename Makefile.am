SUBDIRS = src

SUFFIXES: .dt

.dt.desktop:
	sed -e "s:PREFIX:@INSTALLDIR@:" $< > $@

EXTRA_DIST = CMakeLists.txt src/CMakeLists.txt extra/* 
EXTRA_DIST += src/icons/jv.svg src/icons/clear.svg src/icons/refresh.svg src/icons/up.svg src/icons/down.svg src/icons/left.svg src/icons/right.svg src/icons/lock.svg src/icons/unlock.svg

icondir = @BUILDDIR@/usr/share/pixmaps
icon_DATA = extra/JournalViewer.png

desktopdir = @BUILDDIR@/usr/share/applications
desktop_DATA = extra/JournalViewer.desktop

