if HAVE_DOXYGEN

doxygen.stamp: Doxyfile.in
	$(AM_V_GEN)$(DOXYGEN) Doxyfile
	-touch $@

CLEANFILES = doxygen.stamp doxygen.log
EXTRA_DIST = geany++.png

all-local: doxygen.stamp

clean-local:
	$(RM) -rf $(top_builddir)/doc/html

endif
