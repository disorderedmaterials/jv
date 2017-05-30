SUBDIRS = get

if LITE
bin_PROGRAMS = jvlite
else
bin_PROGRAMS = jv
endif

SUFFIXES: .ui .hui .qrc

.ui.o:
	$(QTUIC) -o ui_$*.h $<
	$(QTMOC) -o moc_$*.cpp $(top_srcdir)/src/$*.h
	$(CXX) -I$(top_srcdir)/src -I$(top_srcdir)/src/ $(CXXFLAGS) $(JVCFLAGS) -c -o $@ moc_$*.cpp
	rm moc_$*.*

.hui.o:
	$(QTMOC) -o $*.cpp -f$< $<
	$(CXX) -I$(top_srcdir)/src -I../ -I./ $(CXXFLAGS) $(JVCFLAGS) -c -o $@ $*.cpp
	rm $*.cpp

.qrc.o:
	$(QTRCC) -o $*.cpp -name $* $<
	$(CXX) -I$(top_srcdir)/src -I../ -I./ $(CXXFLAGS) $(JVCFLAGS) -c -o $@ $*.cpp
	rm $*.cpp

clean-local:
	-rm -f *.o ui_* plotwidget.cpp messenger.cpp preview.cpp jv jvlite moc_*

# Targets common to both builds
GUI_UI = icons.qrc findwindow.ui licensewindow.ui logwindow.ui jv.ui datainterface.ui messenger.hui plotwidget.hui preview.hui quickreport.ui report.ui rundatawindow.ui samplereport.ui treplytimeout.hui settings.ui
GUI_FUNCS = datainterface_funcs.cpp jv_cli.cpp jv_funcs.cpp jv_instrument.cpp jv_local.cpp jv_rundata.cpp jv_printing.cpp jv_settings.cpp findwindow_funcs.cpp licensewindow_funcs.cpp logwindow_funcs.cpp preview_funcs.cpp quickreport_funcs.cpp report_funcs.cpp samplereport_funcs.cpp settings_funcs.cpp treplytimeout_funcs.cpp
GUI_OTHER = messenger_funcs.cpp ttablewidgetitem_funcs.cpp ttreewidgetitem_funcs.cpp
GUI_PLOT = rundatawindow_funcs.cpp plotwidget_data.cpp plotwidget_funcs.cpp plotwidget_paintevent.cpp
JV_CLASSES = data2d.cpp document.cpp documentcommands.cpp enumeration.cpp instrument.cpp isis.cpp isis_data.cpp journal.cpp rbdata.cpp rundata.cpp

if LITE
jvlite_SOURCES = ${GUI_UI} ${GUI_FUNCS} $(GUI_PLOT) $(GUI_OTHER) $(JV_CLASSES) main.cpp
jvlite_LDADD = get/libget.a @MAC_LIBS@ $(JVLDLIBS)
jvlite_CPPFLAGS = -I$(top_srcdir)/src $(JVCFLAGS)
else
jv_SOURCES = ${GUI_UI} ${GUI_FUNCS} $(GUI_PLOT) $(GUI_OTHER) $(JV_CLASSES) main.cpp
jv_LDADD = get/libget.a @MAC_LIBS@ $(JVLDLIBS)
jv_CPPFLAGS = -I$(top_srcdir)/src $(JVCFLAGS)
endif

noinst_HEADERS = array.h data2d.h datainterface.h document.h documentcommands.h enumeration.h instrument.h isis.h journal.h jv.h findwindow.h licensewindow.h logwindow.h ttablewidgetitem.h ttreewidgetitem.h list.h quickreport.h rbdata.h reflist.h report.h rundata.h rundatawindow.h samplereport.h settings.h version.h

noinst_DIST = icon.svg

