AM_CXXFLAGS = $(GEANY_CFLAGS) $(GTKMM_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
AM_LDFLAGS = $(GEANY_LIBS) $(GTKMM_LIBS)

lib_LTLIBRARIES = libgeany++.la

libgeany___la_SOURCES = \
	document.cpp \
	editor.cpp \
	filetype.cpp \
	geany.cpp \
	geany_p.hpp \
	iplugin.cpp \
	pluginconfig.cpp \
	scintilla.cpp \
	ui.cpp

plugindir = $(libdir)/geany
plugin_LTLIBRARIES = geany++.la demo++.la
plugin_DATA = $(srcdir)/demo++.plugin

geany___la_SOURCES = proxy.cpp
geany___la_LDFLAGS = -module -avoid-version
geany___la_LIBADD = libgeany++.la
demo___la_SOURCES = demo.cpp
demo___la_LDFLAGS = -module -avoid-version
demo___la_LIBADD = libgeany++.la

geanycppincludedir = $(includedir)/geany++
geanycppinclude_HEADERS = \
	common.hpp \
	document.hpp \
	editor.hpp \
	filetype.hpp \
	geany.hpp \
	iplugin.hpp \
	pluginconfig.hpp \
	project.hpp \
	scintilla.hpp \
	ui.hpp \
	utils.hpp

SCIGEN = $(top_srcdir)/scripts/scigen.py
SCIFACE = $(top_srcdir)/scripts/Scintilla.iface

SCIDEPS = \
	$(SCIGEN) $(SCIFACE) \
	$(top_srcdir)/scripts/Face.py \
	$(top_srcdir)/scripts/sciface.py

scintilla.cpp: $(srcdir)/scintilla.cpp.in $(SCIDEPS)
	$(AM_V_GEN)$(PYTHON) $(SCIGEN) -i $(SCIFACE) -o $@ $(srcdir)/scintilla.cpp.in

scintilla.hpp: $(srcdir)/scintilla.hpp.in $(SCIDEPS)
	$(AM_V_GEN)$(PYTHON) $(SCIGEN) -i $(SCIFACE) -o $@ $(srcdir)/scintilla.hpp.in

EXTRA_DIST = \
	$(SCIDEPS) \
	$(srcdir)/scintilla.cpp.in \
	$(srcdir)/scintilla.hpp.in

BUILT_SOURCES = scintilla.cpp scintilla.hpp
CLEANFILES = scintilla.cpp scintilla.hpp
