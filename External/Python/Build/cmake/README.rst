CPython CMake Buildsystem
=========================

Overview
--------

A replacement buildsystem for CPython.

This `CMake <http://cmake.org>`_ buildsystem has the following advantages:

* No compiled program for the target architecture is used in the build
  itself.  This makes **cross-compiling** easier, less error prone, and
  reduces manual steps.
* Same build information for all platforms - there's no need to maintain the
  autotools configuration separately from four different MSVC project files.
* Support for other build systems and IDE's like `Ninja
  <https://martine.github.io/ninja/>`_, `Sublime Text
  <https://www.sublimetext.com/>`_, and many others.
* Easily build C-extensions against other C/C++ libraries built with CMake.
* It's much faster to compile: 7 seconds instead of 58 seconds in my
  unscientific test.

Usage
-----

How to use this buildsystem:

1. Checkout the buildsystem

.. code:: bash

  cd ~/scratch
  git clone git://github.com/python-cmake-buildsystem/python-cmake-buildsystem

2. Build

.. code:: bash

  cd ~/scratch
  mkdir -p python-build && mkdir -p python-install
  cd python-build
  cmake -DCMAKE_INSTALL_PREFIX:PATH=${HOME}/scratch/python-install ../python-cmake-buildsystem
  make -j10
  make install

.. note::

  By default, the build system will download the python 2.7.8 source from
  http://www.python.org/ftp/python/


CMake Options
-------------

You can pass options to CMake to control the way Python is built.  You only
need to give each option once - they get saved in `CMakeCache.txt`.  Pass
options on the commandline with `-DOPTION=VALUE`, or use the "ccmake" gui.

::

  CMAKE_BUILD_TYPE=Debug|Release
    Build with debugging symbols or with optimisations.

  CMAKE_INSTALL_PREFIX=<path>   (defaults to /usr/local)
    Path in which to install Python.

  DOWNLOAD_SOURCES=ON|OFF      (defaults to ON)
    Download, check MD5 sum and extract python sources in the parent directory.
    Source archive is downloaded from http://www.python.org/ftp/python/2.7.8/Python-2.7.8.tgz

  BUILD_LIBPYTHON_SHARED=ON|OFF (defaults to OFF)
    Build libpython as a shared library (.so or .dll) or a static library
    (.a).

    Note that Python extensions are always built as shared libraries.  On
    Windows it is not possible to build shared .dll extensions against a
    static libpython, so you must build any extensions you want into libpython
    itself (see the BUILTIN flags below).

  BUILD_EXTENSIONS_AS_BUILTIN=ON|OFF (defaults to OFF)
    If enabled, all extensions are statically compiled into the built python
    libraries (static and/or shared).

    Note that all previously set BUILTIN_<extension> options are ignored and
    reset to their original value.

  WITH_STATIC_DEPENDENCIES=ON|OFF    (defaults to OFF, available only on UNIX)
    If this is set to ON then cmake will compile statically libpython and all
    extensions. External dependencies (ncurses, sqlite, ...) will be builtin
    only if they are available as static libraries.

  BUILD_WININST=ON|OFF (only for windows, defaults to ON if not crosscompiling)
    If enabled, build the 'Windows Installer' program for distutils if not
    already provided in the source tree.

  BUILD_WININST_ALWAYS=ON|OFF (only for windows, defaults to OFF)
    If enabled, always build 'Windows Installer' program for distutils even
    if it is already provided in the source tree.

  INSTALL_DEVELOPMENT=ON|OFF (defaults to ON)
    If enabled, install files required to develop C extensions.

  INSTALL_MANUAL=ON|OFF (defaults to ON)
    If enabled, install manuals.

  INSTALL_TEST=ON|OFF (defaults to ON)
    If enabled, install test files.

  ENABLE_<extension>=ON|OFF     (defaults to ON)
  BUILTIN_<extension>=ON|OFF    (defaults to OFF except for POSIX, PWD and
                                 NT extensions which are builtin by default)
    These two options control how individual python extensions are built.
    <extension> is the name of the extension in upper case, and without any
    leading underscore (_).  Known extensions for 2.7.8 include:

      ARRAY AUDIOOP BINASCII BISECT BSDDB BZ2 CMATH CODECS_CN CODECS_HK
      CODECS_ISO2022 CODECS_JP CODECS_KR CODECS_TW COLLECTIONS CPICKLE CRYPT
      CSTRINGIO CSV CTYPES CTYPES_TEST CURSES CURSES_PANEL DATETIME DBM
      ELEMENTTREE FCNTL FUNCTOOLS FUTURE_BUILTINS GDBM GRP HASHLIB HEAPQ
      HOTSHOT IO ITERTOOLS JSON LINUXAUDIODEV LOCALE LSPROF MATH MMAP
      MULTIBYTECODEC MULTIPROCESSING NIS NT OPERATOR OSSAUDIODEV PARSER POSIX
      PWD PYEXPAT RANDOM READLINE RESOURCE SELECT SOCKET SPWD SQLITE3 SSL
      STROP STRUCT SYSLOG TERMIOS TESTCAPI TIME TKINTER UNICODEDATA ZLIB

    All extensions are enabled by default, but some might depend on system
    libraries and will get disabled if they're not available (a list of
    extensions that didn't have all their prerequisites available will be
    printed when you run cmake).

    By default extensions are compiled as separate shared libraries (.so or
    .dll files) and installed in lib/python2.7/lib-dynload.  If you set
    BUILTIN_<extension> to ON then the extension is compiled into libpython
    instead.

  USE_LIB64=ON|OFF              (defaults to OFF)
    If this is set to ON then cmake will look for dependencies in lib64 as
    well as lib directories.  Compiled python extensions will also be
    installed into lib64/python2.7/lib-dynload instead of
    lib/python2.7/lib-dynload.

  Py_USING_UNICODE             (only for python2, defaults to ON)
    Enable unicode support. By default, ucs2 is used. It can be
    forced to ucs4 setting Py_UNICODE_SIZE to 4.

  EXTRA_PYTHONPATH=dir1:dir2    (defaults to "")
    Colon (:) separated list of extra directories to add to the compiled-in
    PYTHONPATH.

  USE_SYSTEM_LIBRARIES=ON|OFF   (defaults to ON)
    If set to OFF, no attempt to detect system libraries will be done.
    Options documented below allow to enable/disable detection of particular
    libraries.

  USE_SYSTEM_Curses=ON|OFF      (defaults to ON)
    If set to OFF, no attempt to detect Curses libraries will be done.
    Associated python extensions are: CURSES, CURSES_PANEL, READLINE
    Following CMake variables can manually be set: CURSES_LIBRARIES, PANEL_LIBRARIES

  USE_SYSTEM_EXPAT=ON|OFF       (defaults to ON)
    If set to OFF, no attempt to detect Expat libraries will be done.
    Associated python extensions are: ELEMENTTREE, PYEXPAT
    Following CMake variables can manually be set: EXPAT_LIBRARIES, EXPAT_INCLUDE_DIRS

  USE_SYSTEM_OpenSSL=ON|OFF     (defaults to ON)
    If set to OFF, no attempt to detect OpenSSL libraries will be done.
    Associated python extensions are: HASHLIB, SSL, MD5, SHA, SHA256, SHA512
    Following CMake variables can manually be set: OPENSSL_INCLUDE_DIR, OPENSSL_LIBRARIES
    If [OPENSSL_INCLUDE_DIR, OPENSSL_LIBRARIES] are found, extensions [HASHLIB, SSL] will be built
    If [OPENSSL_INCLUDE_DIR, OPENSSL_LIBRARIES] are NOT found, extensions [SHA, SHA256, SHA512] will be built

  USE_SYSTEM_TCL=ON|OFF         (defaults to ON)
    If set to OFF, no attempt to detect Tcl libraries will be done.
    Associated python extensions are: TKINTER
    Following CMake variables can manually be set: TCL_LIBRARY, TK_LIBRARY, TCL_INCLUDE_PATH, TK_INCLUDE_PATH

  USE_SYSTEM_ZLIB=ON|OFF        (defaults to ON)
    If set to OFF, no attempt to detect ZLIB libraries will be done.
    Associated python extensions are: BINASCII, ZLIB
    Following CMake variables can manually be set: ZLIB_LIBRARY, ZLIB_INCLUDE_DIR, ZLIB_ROOT
    ZLIB_ROOT should be set only if USE_SYSTEM_ZLIB is ON
    If [ZLIB_LIBRARY, ZLIB_INCLUDE_DIR] are found, extensions [BINASCII] will be built with ZLIB_CRC32

  USE_SYSTEM_DB=ON|OFF          (defaults to ON)
    If set to OFF, no attempt to detect DB libraries will be done.
    Associated python extensions are: BSDDB
    Following CMake variables can manually be set: DB_INCLUDE_PATH, DB_LIBRARIES

  USE_SYSTEM_GDBM=ON|OFF        (defaults to ON)
    If set to OFF, no attempt to detect GDBM libraries will be done.
    Associated python extensions are: DBM, GDBM
    Following CMake variables can manually be set: GDBM_INCLUDE_PATH, GDBM_LIBRARY, GDBM_COMPAT_LIBRARY

  USE_SYSTEM_READLINE=ON|OFF    (defaults to ON)
    If set to OFF, no attempt to detect Readline libraries will be done.
    Associated python extensions are: READLINE
    Following CMake variables can manually be set: READLINE_INCLUDE_PATH, READLINE_LIBRARY

  USE_SYSTEM_SQLITE3=ON|OFF     (defaults to ON)
    If set to OFF, no attempt to detect SQLITE3 libraries will be done.
    Associated python extensions are: SQLITE3
    Following CMake variables can manually be set: SQLITE3_INCLUDE_PATH, SQLITE3_LIBRARY

  CMAKE_OSX_SDK                (MacOSX, default is autodetected, e.g 'macosx10.06')
    By default, the variable is automatically set running `xcrun` and/or `xcodebuild`. Note that its
    value can also be expicitly set when doing a clean configuration either by adding a cache entry in
    `cmake-gui` or by passing the argument `-DCMAKE_OSX_SDK:STRING=macosx10.6` when running `cmake`.
    Then, this variable is used to initialize `CMAKE_OSX_SYSROOT`, `CMAKE_OSX_DEPLOYMENT_TARGET`
    and `MACOSX_DEPLOYMENT_TARGET` variables.


Cross-compiling
---------------

Cross-compiling for Windows from Linux
......................................

There are some patches in the cmake/patches-win32 directory that make it
possible to compile Python using the mingw32 compiler.  You have to apply
these before running make::

  patch -p0 < cmake/patches-win32/01-dynload_win.patch
  patch -p0 < cmake/patches-win32/02-signalmodule.patch
  patch -p0 < cmake/patches-win32/03-mingw32.patch

Remarks
-------

Note: Currently, multiple versions of Python 2.7 are supported. This
repository is maintained separately from Python itself it needs to be manually
updated whenever there is a new release of Python.

Licenses
--------

Materials in this repository are distributed under the following licenses:

  All software is licensed under the Apache 2.0 License.
  See `LICENSE_Apache_20 <LICENSE_Apache_20>`_ file for details.


FAQ
---

Why Apache 2.0 License?
.......................

From the python.org wiki, the answer to the question `What if I want to
contribute my code to the PSF
<https://wiki.python.org/moin/PythonSoftwareFoundationLicenseFaq#What_if_I_want_to_contribute_my_code_to_the_PSF.3F>`_
mentions that if code is going to end up in Python or the standard library,
the PSF will require you to license code under "Academic Free License" or
"Apache License 2.0".
