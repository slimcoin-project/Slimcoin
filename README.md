## SlimCoin Official Development Repo

### What is SlimCoin?
[Slimcoin](http://www.slimcoin.club/whitepaper.pdf) (abbreviated SLM), is the first known
[cryptocurrency](https://en.wikipedia.org/wiki/Cryptocurrency) designed as
a combined [proof-of-burn](http://en.bitcoin.it/wiki/Proof_of_burn)/[proof-of-stake](https://en.wikipedia.org/wiki/Proof-of-stake)/[proof-of-work](https://en.wikipedia.org/wiki/Proof-of-work_system)
system. It can be mined using a special “Dcrypt” algorithm. 

SlimCoin is unique for using the [Proof of Burn](http://en.bitcoin.it/wiki/Proof_of_burn) algorithm. This algorithm rewards the “destruction” of coins by sending them to an unspendable address. It can be described as a long-term investment rewarding variant of Proof of Stake and has interesting economical implications - see the resources below for details.

The Dcrypt algorithm is extremely anti-ASIC and currently can only be mined with a CPU. It is made so mining with hardware will be inefficient in the future, given the intensity of this algorithm, thus forcing the users to mine through the proof-of-stake and proof-of-burn. Thus, with SlimCoin energy consumption would decrease over time as proof-of-work blocks become less rewarding and coin generation becomes increasingly based on proof-of-stake block generation.

Slimcoin's codebase is built upon [Bitcoin](http://bitcoin.org/en/) and [Peercoin](http://peercoin.net/).

### SlimCoin Resources
* Client and Source:
<add Client Binaries>
[Source Code](https://github.com/slimcoin-project/Slimcoin)
* Documentation: <add documentation link to slim website>
[SlimCoin Whitepaper](http://www.slimcoin.club/whitepaper.pdf), 
[SlimCoin Wiki](https://github.com/slimcoin-project/Slimcoin/wiki)
* Help:
[Slimcoin Reddit](https://www.reddit.com/r/slimcoin/), 
[Bitcointalk official thread](https://bitcointalk.org/index.php?topic=1141676), 
[Old Bitcointalk announcement](https://bitcointalk.org/index.php?topic=613213)
* Other Sites and Links <other sites>
[![Build Status](https://api.travis-ci.org/gjhiggins/SLIMCoin.svg?branch=slimcoin)](https://travis-ci.org/gjhiggins/SLIMCoin),
[Slimcoin Club](https://www.slimcoin.club) (Block explorer, Brain wallet and other informations)

## Build on OS X

### Install Homebrew

The [recommended way of installing the Homebrew OS X package manager](http://brew.sh/) is via Terminal command:

    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

#### About Homebrew

Homebrew is tractable. The standard script installs Homebrew to `/usr/local` so that sudo is not required for `brew install`. It is a careful script, it can be run even if other packages have already been installed to `/usr/local`. It informs the user exactly what it will do before it does it, enabling the user to confirm the details before permitting brew to execute the action.

#### How do I uninstall Homebrew?

To uninstall Homebrew, paste the command below in a terminal prompt.
     
     ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/uninstall)"

By default, this command will uninstall homebrew and remove its footprint, i.e. will remove brew and restore `/usr/local` to its prior state.

### Install SLIMCoin dependencies

    brew install autoconf automake libtool miniupnpc openssl pkg-config
    brew install boost protobuf libevent libqrencode
    brew install berkeley-db4
    brew install berkeley-db
    brew install qt5 --with-dbus
    brew link qt5 --force --overwrite

## Compile OSX app

In a new Terminal window:

    $ git clone https://github.com/slimcoin-project/slimcoin.git
    $ cd slimcoin
    $ /usr/local/opt/qt/bin/qmake RELEASE=1 USE_UPNP=1 USE_QRCODE=1 slimcoin-qt.pro
    $ make
    $ export QTDIR=/usr/local/opt/qt5
    $ T=$(contrib/qt_translations.py $QTDIR/translations src/qt/locale)
    $ python2.7 contrib/macdeploy/macdeployqtplus  -add-qt-tr $T -dmg -fancy contrib/macdeploy/fancy.plist SLIMCoin-Qt.app


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

> i686-w64-mingw32.static
> x86-64-w64-mingw32.static (note that _ replaced with -)
> i686-w64-mingw32.shared
> x86-64-w64-mingw32.shared (note that _ replaced with -)

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


## Repository Guidelines

* Developers work in their own forks, then submit pull requests when they think their feature or bug fix is ready.
* If it is a simple/trivial/non-controversial change, then one of the development team members simply pulls it.
* If it is a more complicated or potentially controversial change, then the change may be discussed in the pull request, or the requester may be asked to start a discussion [SlimCoin Talk](<forum link>) for a broader community discussion. 
* The patch will be accepted if there is broad consensus that it is a good thing. Developers should expect to rework and resubmit patches if they don't match the project's coding conventions (see coding.txt) or are controversial.
* From time to time a pull request will become outdated. If this occurs, and the pull is no longer automatically mergeable; a comment on the pull will be used to issue a warning of closure.  Pull requests closed in this manner will have their corresponding issue labeled 'stagnant'.

