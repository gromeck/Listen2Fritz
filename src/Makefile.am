#
#	Makefile.am
#
#	Copyright (c) 2007 by Christian Lorenz
#
#	====================================================================
#
#	This file is part of listen2fritz.
#	
#	listen2fritz is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	listen2fritz is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with listen2fritz.  If not, see <http://www.gnu.org/licenses/>.
#

SUBDIRS=src scripts

EXTRA_DIST=etc

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	if [ ! -f $(DESTDIR)$(sysconfdir)/listen2fritz.conf ]; then \
		$(INSTALL_SCRIPT) --mode=644 $(srcdir)/etc/listen2fritz.conf $(DESTDIR)$(sysconfdir)/listen2fritz.conf ; \
	fi

	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/init.d
	$(INSTALL_SCRIPT) --mode=755 $(srcdir)/etc/init.d/listen2fritz $(DESTDIR)$(sysconfdir)/init.d/listen2fritz

	$(mkinstalldirs) $(DESTDIR)$(sbindir)
	ln -sf $(sysconfdir)/init.d/listen2fritz $(DESTDIR)$(sbindir)/rclisten2fritz
