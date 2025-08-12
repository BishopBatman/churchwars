SUBDIRS        = src doc po sounds
DESKTOPDIR     = ${DESTDIR}${datadir}/gnome/apps/Games
DESKTOP        = churchwars.desktop
SCOREDIR       = ${DESTDIR}${localstatedir}
SCORE          = ${SCOREDIR}/churchwars.sco
EXTRA_DIST     = ABOUT-NLS LICENCE churchwars.desktop rpm/dopewars.spec.in \
                 runindent.sh win32/README.md win32/install.nsi.in README.md \
		 win32/mingw/Dockerfile ChangeLog.md
CLEANFILES     = churchwars.sco churchwars-log.txt churchwars-config.txt
DISTCLEANFILES = rpm/dopewars.spec
ACLOCAL_AMFLAGS= -I m4

install-data-local:
	${mkinstalldirs} ${SCOREDIR}
	touch ${SCORE}
	@chgrp games ${SCORE} || chgrp wheel ${SCORE} || \
	  echo "Unable to change group ownership of the high score file"
	chmod 0660 ${SCORE}
	${mkinstalldirs} ${DESKTOPDIR}
	${INSTALL} -m 0644 ${srcdir}/${DESKTOP} ${DESKTOPDIR}

uninstall-local:
	/bin/rm -f ${SCORE} ${DESKTOPDIR}/${DESKTOP}

.PHONY: update-po
update-po: ; $(MAKE) -C po update-po
