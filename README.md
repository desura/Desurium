Content
==================

 * Desura
 * Build Desura on Linux
 * Build Desura on Windows
 * Special notes for advanced users
 * Closing Remarks

Desura
===================

Desura is a gaming client that allows users to one click download and install games and game modification.
For more information, visit http://www.moddb.com/groups/desura/forum/thread/open-sourcing-desura


Build Desura on Linux
===================

Dependencies
------------

You can install dependencies by package manager or install_deps.sh

Required dependencies:

 * GCC (4.6 or later, multilib support for 32 bit compatibility on 64 bit systems)
 * cmake (2.8.5 or later)
 * PkgConfig (for building DEB and RPM packages)
 * Boost (date_time, filesystem, thread system, test_exec_monitor, unit_test_framework)
 * Freetype
 * GTK2
 * libX11 (with libXt)
 * libEvent
 * libNotify (0.7 or later)
 * GLib2
 * OpenSSL (only for building Curl)
 * BZip2
 * Sqlite3
 * tinyxml2
 * V8
 * Python
 * C-ares

Optional dependencies (will be built by cmake if not existing):
- Curl (if there is no ares support)
- wxWidgets (2.9.3 or later)
- Breakpad
- CEF (Chromium Embedded)

Installation
------------

Open Terminal in Desurium source directory and run:

    ./build_desura.sh

or you can speed up things by running:

    ./build_desura.sh rebuild_all -j `getconf _NPROCESSORS_ONLN`

or you can build DEB package:

    ./build_desura.sh pack_deb

or you can build RPM package:

    ./build_desura.sh pack_rpm

Finally run Desurium using:

    ./install/desura

or install created DEB or RPM package

For informations on advanced arguments of the build script, run:

    ./build_desura.sh help

Build Desura on Windows
=====================

Download and install:

 * Svn http://www.sliksvn.com/en/download or http://tortoisesvn.net/downloads.html
 * Msvc 2008 Professional or higher
 * Python 2.7 http://www.python.org/getit/
 * CMake 2.8 http://www.cmake.org/
 * Patch either from msysGit or http://gnuwin32.sourceforge.net/packages/patch.htm (please use Complete package, except sources)
 * Windows DirectX 9 SDK for the CEF build https://www.microsoft.com/en-us/download/details.aspx?id=6812

Note: Make sure you add svn and python to your environment PATH

Open a 32bit cmd.exe with env vars set up by vc or other scripts (so we have a full build environment) and run:

    ./build_desura.bat

wait

Special notes for advanced users
=====================

Desura is using the cmake build system for configuration on prject files for several build systems (make, VS, nmake, ...).
At the top of CMakeFile.txt is a documented list of options

a "normal" way to configure cmake would be something like this:

  * UNIX-based systems:
    1. mkdir build
    2. cd build
    3. cmake ..
    4. make
    5. sudo make install (optionally)

  * win32 based systems:
    1. md build
    2. cd build
    3. cmake ..
    4. open project files and compile them

Closing Remarks
=====================

If you need help, want to give some suggestions or just want to talk, feel free to join our IRC channel #desura on irc.freenode.net.
