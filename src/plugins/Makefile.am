noinst_LTLIBRARIES = libsound_winmm.la
if ESD
noinst_LTLIBRARIES += libsound_esd.la
endif
if PULSEAUDIO
noinst_LTLIBRARIES += libsound_pulseaudio.la
endif
if SDL
noinst_LTLIBRARIES += libsound_sdl.la
endif
if COCOA
noinst_LTLIBRARIES += libsound_cocoa.la
endif
libsound_esd_la_SOURCES = sound_esd.c sound_esd.h
libsound_esd_la_LDFLAGS = @ESD_LIBS@
libsound_pulseaudio_la_SOURCES = sound_pulseaudio.c sound_pulseaudio.h
libsound_pulseaudio_la_LDFLAGS = @PULSEAUDIO_LIBS@
libsound_sdl_la_SOURCES = sound_sdl.c sound_sdl.h
libsound_sdl_la_LDFLAGS = @SDL_LIBS@
libsound_winmm_la_SOURCES = sound_winmm.c sound_winmm.h
libsound_cocoa_la_LDFLAGS = -module -avoid-version -no-undefined -Wl,-framework,Foundation,-framework,AppKit
libsound_cocoa_la_SOURCES = sound_cocoa.m
LIBS = @GLIB_LIBS@
AM_CPPFLAGS = @SOUND_CFLAGS@ @GLIB_CFLAGS@
LINKNOO = $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS)

if PLUGINS
PLUGINDIR = ${DESTDIR}${plugindir}
if ESD
ESD_SO = .libs/libsound_esd.so
endif
if PULSEAUDIO
PULSEAUDIO_SO = .libs/libsound_pulseaudio.so
endif
if SDL
SDL_SO = .libs/libsound_sdl.so
endif
if COCOA
COCOA_SO = .libs/libsound_cocoa.so
endif
PLUGINS   = $(ESD_SO) $(PULSEAUDIO_SO) $(SDL_SO) $(COCOA_SO)

all-local: ${PLUGINS}

.libs/libsound_esd.so: $(libsound_esd_la_OBJECTS)
	$(LINKNOO) -o libsound_esd.la -rpath $(libdir) $(libsound_esd_la_LDFLAGS) $(libsound_esd_la_OBJECTS) $(libsound_esd_la_LIBADD) $(LIBS)

.libs/libsound_pulseaudio.so: $(libsound_pulseaudio_la_OBJECTS)
	$(LINKNOO) -o libsound_pulseaudio.la -rpath $(libdir) $(libsound_pulseaudio_la_LDFLAGS) $(libsound_pulseaudio_la_OBJECTS) $(libsound_pulseaudio_la_LIBADD) $(LIBS)

.libs/libsound_sdl.so: $(libsound_sdl_la_OBJECTS)
	$(LINKNOO) -o libsound_sdl.la -rpath $(libdir) $(libsound_sdl_la_LDFLAGS) $(libsound_sdl_la_OBJECTS) $(libsound_sdl_la_LIBADD) $(LIBS)

.libs/libsound_winmm.so: $(libsound_winmm_la_OBJECTS)
	$(LINKNOO) -o libsound_winmm.la -rpath $(libdir) $(libsound_winmm_la_LDFLAGS) $(libsound_winmm_la_OBJECTS) $(libsound_winmm_la_LIBADD) $(LIBS)

.libs/libsound_cocoa.so: $(libsound_cocoa_la_OBJECTS)
	$(LINKNOO) -o libsound_cocoa.la -rpath $(libdir) $(libsound_cocoa_la_LDFLAGS) $(libsound_cocoa_la_OBJECTS) $(libsound_cocoa_la_LIBADD) $(LIBS)

install-exec-am:
	${mkinstalldirs} ${PLUGINDIR}
	for plug in ${PLUGINS}; do \
	${INSTALL} $${plug} ${PLUGINDIR}; \
	done

uninstall-local:
	for plug in ${PLUGINS}; do \
	/bin/rm -f ${PLUGINDIR}/`basename $${plug}`; \
	done
endif
