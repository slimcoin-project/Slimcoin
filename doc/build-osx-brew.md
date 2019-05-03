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
    $ cd  slimcoin
    $ /usr/local/opt/qt/bin/qmake RELEASE=1 USE_UPNP=1 USE_QRCODE=1 slimcoin-qt.pro
    $ make

## Cutting a dmg

Due to an infelicity somewhere in the upstream toolchain, one of the necessary boost libs
is inappropriately skipped necessitating a hand-copy of the boost library
`libboost-system-mt.dylib` to `contrib/macdeploy`.

Find the location of the missing library ...

    $ find /usr/local -name libboost_system-mt.dylib -type f

which should result in something similar to:

    -r--r--r--  1 gjh  staff  23368 12 Mar 16:41 /usr/local/Cellar/boost/1.68.0_1/lib/libboost_system-mt.dylib

Entering the following incantations should then result in the creation of a dmg.

    $ cp /usr/local/Cellar/boost/1.68.0_1/lib/libboost_system-mt.dylib contrib/macdeploy
    $ cp -R SLIMCoin.app contrib/macdeploy
    $ python3 `pwd`/contrib/macdeploy/macdeployqtplus SLIMCoin-Qt.app -add-qt-tr da,de,es,hu,ru,uk,zh_CN,zh_TW -dmg -fancy `pwd`/contrib/macdeploy/fancy.plist -verbose 2

