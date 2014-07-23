
Debian
====================
This directory contains files used to package ekwicoind/ekwicoin-qt
for Debian-based Linux systems. If you compile ekwicoind/ekwicoin-qt yourself, there are some useful files here.

## ekwicoin: URI support ##


ekwicoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install ekwicoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your ekwicoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/ekwicoin128.png` to `/usr/share/pixmaps`

ekwicoin-qt.protocol (KDE)

