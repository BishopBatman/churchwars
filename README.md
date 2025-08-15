[![Build Status](https://github.com/benmwebb/churchwars/workflows/build/badge.svg?branch=develop)](https://github.com/benmwebb/churchwars/actions?query=workflow%3Abuild)
[![Download Church Wars drug dealing game](https://img.shields.io/sourceforge/dt/churchwars.svg)](https://churchwars.sourceforge.io/download.html)

This is Church Wars 1.6.2, a game simulating the life of a drug dealer in
New York. The aim of the game is to make lots and lots of money...
unfortunately, you start the game with a hefty debt, accumulating interest,
and the cops take a rather dim view of drug dealing...

These are brief instructions; see the HTML documentation for full information.

Church Wars 1.6.2 servers should handle clients as old as version 1.4.3 with
hardly any visible problems (the reverse is also true). However, it is
recommended that both clients and servers are upgraded to 1.6.2!

## Installation

Either...

1. Get the relevant RPM from https://churchwars.sourceforge.io/
   
Or...

1. Get the tarball `churchwars-1.6.2.tar.gz` from the same URL
2. Extract it via `tar -xvzf churchwars-1.6.2.tar.gz`
3. Follow the instructions in the `INSTALL` file in the newly-created
   `churchwars-1.6.2` directory

Once you're done, you can safely delete the RPM, tarball and churchwars
directory. The churchwars binary is all you need!

Church Wars stores its high score files by default in `/usr/local/var/churchwars.sco`.
This will be created by `make install` or by RPM installation. Use the `-f`
command-line option to specify an alternative score file.

## Windows installation

Church Wars now compiles as a console or regular application under Win32 (Windows 7
or later). Almost all functionality of the standard Unix binary is retained;
for example, all of the same command line switches are supported. However, for
convenience, the configuration file is the more Windows-friendly
"churchwars-config.txt".

The easiest way to install the Win32 version is to download the precompiled
binary. To build from source, see the `win32` directory.

## Usage

Church Wars has built-in client-server support for multi-player games. For a
full list of options configurable on the command line, run `churchwars` with
the `-h` switch.

`churchwars -a`
This is "antique" Church Wars; it tries to keep to the original Church Wars, based
on the "Drug Wars" game by John E. Dell, as closely as possible.

`churchwars`
By default, Church Wars supports multi-player games. On starting a game, the
program will attempt to connect to a Church Wars server so that players can send
messages back and forth, and shoot each other if they really want to...

`churchwars -s`
Starts a Church Wars server. This sits in the background and handles multi-player
games. You probably want to use the `-l` command line option too to direct its
log output to somewhere sensible.

`churchwars -c`
Create and run a computer Church Wars player. This will attempt to connect
to a Church Wars server, and if this succeeds, it will then participate in
multi-player Church Wars games.

## Configuration

Most of the Church Wars defaults (for example, the location of the high score file,
the port and server to connect to, the names of the drugs and guns, etc.) can be
configured by adding suitable entries to the Church Wars configuration file. The
global file `/etc/churchwars` is read first, and can then be overridden by
the local settings in `~/.churchwars`. All of the settings here can also be
set on the command line of an interactive Church Wars server when no players
are logged on. See the file "example-cfg" for an example configuration file,
and for a brief explanation of each option, type "help" in an interactive
server. A subset of the configuration options can also be tweaked via the
"Options" menu item in the GTK+/Win32 client.

## Playing

Church Wars is supposed to be fairly self-explanatory. You should be able to 
pick the basics up fairly quickly, but still be discovering subtleties for 
_ages_ ;) If you're _really_ stuck, send me an email. I might even answer it!

Clue: buy drugs when they're cheap, sell them when they're expensive. The Bronx
and Ghetto are "special" locations. Anything more would spoil the fun. ;)

## Bugs

Well, there are bound to be lots. Let me know if you find any by
[opening an issue](https://github.com/benmwebb/churchwars/issues), and I'll see
if I can fix 'em... of course, a
[working patch](https://github.com/benmwebb/churchwars/pulls) would be even
nicer! ;)

## License

Church Wars is released under the GNU General Public License; see the text file
LICENCE for further information. Church Wars is copyright (C) Ben Webb 1998-2022.
The Church Wars icons are copyright (C) Ocelot Mantis 2001.

## Support

Church Wars is written and maintained by Ben Webb <benwebb@users.sf.net>  
Enquiries about Church Wars may be sent to this address (keep them sensible 
please ;) Bug fixes and reports, improvements and patches are also welcomed.

### VM audio (VirtualBox/Linux)

If sound stutters or crashes in a Linux VM:
1) Copy our sample ALSA config:
   cp docs/asoundrc.example ~/.asoundrc
   sudo alsa force-reload || true
2) Install our launcher wrapper (keeps using the 'churchwars' command):
   sudo tools/install_vm_audio_wrapper.sh
3) Run the game:
   churchwars
If heavy scenes still underrun, increase period/buffer in docs/asoundrc.example and re-copy it.
