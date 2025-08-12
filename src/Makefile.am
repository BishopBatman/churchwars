# Nasty hack; there seems to be no other way of disabling libtool for the
# link of the main executable...
if PLUGINS
MYLINK = $(LIBTOOL) --mode=link --tag=CC $(CCLD)
else
MYLINK = $(CCLD)
endif

if GUI_CLIENT
GUISUBDIR = gui_client
endif
if CURSES_CLIENT
CURSESSUBDIR = curses_client
endif
if GTKPORT
GTKPORTSUBDIR = gtkport
endif
if CURSESPORT
CURSESPORTSUBDIR = cursesport
endif

SUBDIRS = $(GUISUBDIR) $(CURSESSUBDIR) $(GTKPORTSUBDIR) $(CURSESPORTSUBDIR) plugins
churchwars_LDADD = @GUILIB@ @CURSESLIB@ @GTKPORTLIB@ @CURSESPORTLIB@ @GTK_LIBS@ @LTLIBINTL@ @WNDRES@ @PLUGOBJS@ @PLUGLIBS@ @GLIB_LIBS@ @LIBCURL@
churchwars_DEPENDENCIES = @GUILIB@ @CURSESLIB@ @GTKPORTLIB@ @CURSESPORTLIB@ @WNDRES@ @PLUGOBJS@

bin_PROGRAMS = churchwars
churchwars_SOURCES = admin.c admin.h AIPlayer.c AIPlayer.h util.c util.h \
                   configfile.c configfile.h convert.c convert.h \
                   dopewars.c dopewars.h error.c error.h log.c log.h \
                   message.c message.h network.c network.h nls.h \
                   serverside.c serverside.h sound.c sound.h \
                   tstring.c tstring.h winmain.c winmain.h mac_helpers.h
AM_CPPFLAGS= -I${srcdir} @GLIB_CFLAGS@ @GTK_CFLAGS@ @LIBCURL_CPPFLAGS@
if APPLE
churchwars_SOURCES += mac_helpers.m
MACLDFLAGS = -framework AppKit
else
MACLDFLAGS =
endif
# Since we included (even optionally) an Objective C file, automake will
# use OBJC rather than CC to link
LINK = $(MYLINK) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) $(MACLDFLAGS) -o $@
OBJCLINK = $(MYLINK) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) $(MACLDFLAGS) -o $@
DEFS       = @DEFS@
PIXDIR     = ${DESTDIR}${datadir}/pixmaps
CHURCHDIR    = ${DESTDIR}${bindir}
CHURCHBIN    = ${CHURCHDIR}/churchwars
PIXMAPS    = churchwars-pill.png churchwars-shot.png churchwars-weed.png
EXTRA_DIST = ${PIXMAPS} pill.ico magic churchwars.rc churchwars.manifest
CLEANFILES = churchwars.res churchwars.exe
WINDRES    = @WINDRES@

install-exec-hook:
	@chgrp games ${CHURCHBIN} || chgrp wheel ${CHURCHBIN} || \
	  ( echo "WARNING: Cannot change group of churchwars binary - the high"; \
	    echo "score file may be unreadable or unwriteable by some users" )
	chmod 2755 ${CHURCHBIN}

install-data-local:
	${mkinstalldirs} ${PIXDIR}
	for pix in ${PIXMAPS}; do \
	  ${INSTALL} -m 0644 ${srcdir}/$${pix} ${PIXDIR}; \
	done

uninstall-local:
	for pix in ${PIXMAPS}; do \
	  /bin/rm -f ${PIXDIR}/$${pix}; \
	done

%.res: %.rc
	${WINDRES} -O coff -o $@ $<
