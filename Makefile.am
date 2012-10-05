SUBDIRS=src data
EXTRA_DIST = data
#dist_data_DATA = tinytrapd.spec
dist-hook:
	rm -rf `find $(distdir)/data -name Makefile`
	rm -rf `find $(distdir)/data -name Makefile.in`

