SUBDIRS = templates

AM_CXXFLAGS = $(GEANY_CFLAGS) $(GTKMM_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
AM_LDFLAGS = $(GEANY_LIBS) $(GTKMM_LIBS)

plugindir = $(libdir)/geany
plugin_LTLIBRARIES = plugingen.la
plugin_DATA = $(srcdir)/plugingen.plugin

gladeuidir = $(pkgdatadir)/geany++/plugingen
gladeui_DATA = plugingen.glade

plugingen_la_SOURCES = \
	plugingen.cpp \
	gendialog.cpp \
	gendialog.hpp \
	genprocessor.cpp \
	genprocessor.hpp

plugingen_la_LDFLAGS = -module -avoid-version
plugingen_la_LIBADD = $(top_builddir)/geany++/libgeany++.la
plugingen_la_CPPFLAGS = \
	-DPLUGINGEN_UI_FILE=\""$(gladeuidir)/plugingen.glade"\" \
	-DPLUGINGEN_TEMPLATE_DIR=\""$(pkgdatadir)/templates/autotools"\"
