noinst_LIBRARIES = libget.a

libget_a_SOURCES = error_add_dummy.c float_convert.c getsect.c int_convert.c
libget_a_SOURCES += ferror_add_dummy.f io.f
libget_a_SOURCES += interface.cpp interface_data.cpp

noinst_HEADERS = float_convert.h geniedefs.h int_convert.h libget.h interface.h

AM_CPPFLAGS = -I$(top_srcdir)/src -I../ $(JVCFLAGS)
