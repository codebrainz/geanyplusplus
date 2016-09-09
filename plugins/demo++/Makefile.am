AM_CXXFLAGS = $(GEANY_CFLAGS) $(GTKMM_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
AM_LDFLAGS = $(GEANY_LIBS) $(GTKMM_LIBS)

plugindir = $(libdir)/geany
plugin_LTLIBRARIES = demo++.la
plugin_DATA = $(srcdir)/demo++.plugin

demo___la_SOURCES = demo++.cpp
demo___la_LDFLAGS = -module -avoid-version
demo___la_LIBADD = $(top_builddir)/geany++/libgeany++.la
