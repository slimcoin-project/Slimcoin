
## Ubuntu-hosted cross-compilation of Windows 32bit binary 

### Installing the [MXE](http://pkg.mxe.cc/) cross-compilation tool.

Make the distribution ppa known to the APT package system:

    $ echo "deb http://pkg.mxe.cc/repos/apt/debian wheezy main" > \
       /etc/apt/sources.list.d/mxeapt.list

Add the GPG key to the APT package system:

    $ apt-key adv --keyserver keyserver.ubuntu.com --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB

Update the APT cache:

    $ apt-get update

Install some MXE package:

    $ apt-get install mxe-i686-w64-mingw32.static-qt

Names of packages are `mxe-<target>-<package>`.

Possible targets:

> - i686-w64-mingw32.static
> - x86-64-w64-mingw32.static (note that `_` replaced with `-`)
> - i686-w64-mingw32.shared
> - x86-64-w64-mingw32.shared (note that `_` replaced with `-`)

*(A complete list of packages can be found on the [MXE site](http://mxe.cc/#packages) and in [the build matrix](http://mxe.cc/build-matrix.html).)*

Packages are installed to `/usr/lib/mxe/<target>/`. The location acts as the root of the MXE source tree.

For example, cross-compile CMake project:

    $ target=i686-w64-mingw32.static
    $ mxedir=/usr/lib/mxe/
    $ $mxedir/usr/bin/$target-cmake project-source
    $ make


### Cross-compiling SLIMCoin

    #!/bin/bash

    # Working setup to cross-compile Windows binaries for Slimcoin hosted on a
    # Vagrant Ubuntu 16.04 VM using non-Canonical ppas for MXE and Qt5.7:
    # deb http://pkg.mxe.cc/repos/apt/debian wheezy main

    # Doesn't seem to pass the QT directives through, though. Tough.

    # Basic path bindings
    PATH=/usr/lib/mxe/usr/bin:$PATH
    MXE_PATH=/usr/lib/mxe
    MXE_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
    MXE_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
    # Belt and braces
    CXXFLAGS="-std=gnu++11 -march=i686"
    LDFLAGS="-march=i686"
    target="i686-w64-mingw32.static"

    # Particularise for cross-compiling
    export BOOST_LIB_SUFFIX=-mt
    export BOOST_THREAD_LIB_SUFFIX=_win32-mt
    export BOOST_INCLUDE_PATH=${MXE_INCLUDE_PATH}/boost
    export BOOST_LIB_PATH=${MXE_LIB_PATH}
    export OPENSSL_INCLUDE_PATH=${MXE_INCLUDE_PATH}/openssl
    export OPENSSL_LIB_PATH=${MXE_LIB_PATH}
    export BDB_INCLUDE_PATH=${MXE_INCLUDE_PATH}
    export BDB_LIB_PATH=${MXE_LIB_PATH}
    export MINIUPNPC_INCLUDE_PATH=${MXE_INCLUDE_PATH}
    export MINIUPNPC_LIB_PATH=${MXE_LIB_PATH}
    export QMAKE_LRELEASE=${MXE_PATH}/usr/${target}/qt5/bin/lrelease

    # Call qmake to create Makefile.[Release|Debug]
    ${target}-qmake-qt5 \
        MXE=1 \
        USE_O3=1 \
        USE_QRCODE=1 \
        FIRST_CLASS_MESSAGING=1 \
        RELEASE=1 \
        USE_UPNPC=1 \
        BOOST_LIB_SUFFIX=${BOOST_LIB_SUFFIX} \
        BOOST_THREAD_LIB_SUFFIX=${BOOST_THREAD_LIB_SUFFIX} \
        BOOST_INCLUDE_PATH=${BOOST_INCLUDE_PATH} \
        BOOST_LIB_PATH=${BOOST_LIB_PATH} \
        OPENSSL_INCLUDE_PATH=${OPENSSL_INCLUDE_PATH} \
        OPENSSL_LIB_PATH=${OPENSSL_LIB_PATH} \
        BDB_INCLUDE_PATH=${BDB_INCLUDE_PATH} \
        BDB_LIB_PATH=${BDB_LIB_PATH} \
        MINIUPNPC_INCLUDE_PATH=${MINIUPNPC_INCLUDE_PATH} \
        MINIUPNPC_LIB_PATH=${MINIUPNPC_LIB_PATH} \
        QMAKE_LRELEASE=${QMAKE_LRELEASE} slimcoin-qt.pro

    # Go for it. If successful, Windows binary will be written out to ./release/slimcoin-qt.exe
    make -f Makefile.Release CXXFLAGS="-DQT_GUI -DQT_NO_PRINTER -std=gnu++11 -march=i686" LDFLAGS="-march=i686"

The cross-compilation product is saved in the `release` directory under the name `slimcoin-qt.exe` and can be tested on Ubuntu with:

    $ wine release/slimcoin-qt.exe

*(be prepared for a long wait during the loading of the index, e.g. 15-20 mins on a low-end machine)*


