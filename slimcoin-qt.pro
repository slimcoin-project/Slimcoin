TEMPLATE = app
TARGET = slimcoin-qt
VERSION = 0.6.4.0
INCLUDEPATH += src src/json src/qt
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE QT_NO_PRINTER
CONFIG += no_include_pwd
CONFIG += thread
CONFIG += debug # release
CONFIG += qt_framework
QT += core gui network sql
CONFIG += link_pkgconfig

isEmpty(BDB_LIB_SUFFIX) {
	# !macx:unix:BDB_LIB_SUFFIX = -5.3
	windows:macx:BDB_LIB_SUFFIX = -4.8
}

exists( /usr/local/Cellar/* ) {
      message( "Configuring for homebrew..." )
      CONFIG += brew
}

!windows:!unix {
    CONFIG += static
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
}

# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively


# winbuild dependencies
windows {
    contains(MXE, 1) {
        BDB_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
        BDB_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        BOOST_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include/boost
        BOOST_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        BOOST_LIB_SUFFIX=-mt
        BOOST_THREAD_LIB_SUFFIX=_win32-mt
        CXXFLAGS=-std=gnu++11 -march=i686
        LDFLAGS=-march=i686
        MINIUPNPC_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
        MINIUPNPC_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        OPENSSL_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include/openssl
        OPENSSL_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
        PATH=/usr/lib/mxe/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
        QMAKE_LRELEASE=/usr/lib/mxe/usr/i686-w64-mingw32.static/qt5/bin/lrelease
        QTDIR=/usr/lib/mxe/usr/i686-w64-mingw32.static/qt5
    }else{
        lessThan(QT_VERSION, 5.4) {
    		BOOST_LIB_SUFFIX=-mgw48-mt-s-1_55
    	} else {
    		BOOST_LIB_SUFFIX=-mgw49-mt-s-1_55
    	}
    	BOOST_INCLUDE_PATH=C:/deps/boost_1_55_0
    	BOOST_LIB_PATH=C:/deps/boost_1_55_0/stage/lib
    	BDB_INCLUDE_PATH=C:/deps/db-4.8.30.NC/build_unix
    	BDB_LIB_PATH=C:/deps/db-4.8.30.NC/build_unix
    	OPENSSL_INCLUDE_PATH=C:/deps/openssl-1.0.1i/include
    	OPENSSL_LIB_PATH=C:/deps/openssl-1.0.1i
    	MINIUPNPC_INCLUDE_PATH=C:/deps
    	MINIUPNPC_LIB_PATH=C:/deps/miniupnpc
    }
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Tested with MacOS Sierra, using QT 5.7.0 because previous versions
    # are not supported. QT 5.7.0 requires C++11 support and in order to
    # build QT 5.7.0 with Homebrew, the full version of Xcode must be
    # installed, not just the command-line tools. Grab Xcode free from
    # the Apple Developer site and install it before continuing with
    # the below. Xcode 8.2_beta is used here.

    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_LFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk

    !windows:!macx {
        # Linux: static link
        LIBS += -Wl,-Bstatic
    }
}

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    isEmpty(QRENCODE_LIB_PATH) {
        # !macx:unix:QRENCODE_LIB_PATH = /usr/lib
        contains(CONFIG, brew) {
            macx:QRENCODE_LIB_PATH = /usr/local/lib
        }else{
            macx:QRENCODE_LIB_PATH = /opt/local/lib
        }
        windows {
            contains(MXE, 1) {
                QRENCODE_LIB_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/lib
            }else{
                QRENCODE_LIB_PATH=C:/deps/qrencode-3.4.4/.libs
            }
        }
    }
    isEmpty(QRENCODE_INCLUDE_PATH) {
        !macx:unix:QRENCODE_INCLUDE_PATH = /usr/include
        contains(CONFIG, brew) {
            macx:QRENCODE_INCLUDE_PATH = /usr/local/include
        }else{
            macx:QRENCODE_INCLUDE_PATH = /opt/local/include
        }
        windows {
            contains(MXE, 1) {
                QRENCODE_INCLUDE_PATH=/usr/lib/mxe/usr/i686-w64-mingw32.static/include
            }else{
                QRENCODE_INCLUDE_PATH=C:/deps/qrencode-3.4.4
                QRENCODE_LIB_PATH=C:/deps/qrencode-3.4.4/.libs
            }
        }
    }
    INCLUDEPATH += $$QRENCODE_INCLUDE_PATH
    LIBS += $$join(QRENCODE_LIB_PATH,,-L,)
    LIBS += -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    isEmpty(MINIUPNPC_LIB_SUFFIX) {
        windows:MINIUPNPC_LIB_SUFFIX=-miniupnpc
    }
    isEmpty(MINIUPNPC_INCLUDE_PATH) {
        contains(CONFIG, brew) {
            macx:MINIUPNPC_INCLUDE_PATH=/usr/local/Cellar/miniupnpc/2.0/include
        }else{
            macx:MINIUPNPC_INCLUDE_PATH=/opt/local/include
        }
        !macx:unix:MINIUPNPC_INCLUDE_PATH=/usr/include/miniupnpc
        windows:MINIUPNPC_INCLUDE_PATH=C:/dev/coindeps32/miniupnpc-1.9
    }
    isEmpty(MINIUPNPC_LIB_PATH) {
        contains(CONFIG, brew) {
            macx:MINIUPNPC_LIB_PATH=/usr/local/Cellar/miniupnpc/2.0/lib
        }else{
            macx:MINIUPNPC_LIB_PATH=/opt/local/lib
        }
        # !macx:unix:MINIUPNPC_INCLUDE_PATH=/usr/lib
        windows:MINIUPNPC_LIB_PATH=C:/dev/coindeps32/miniupnpc-1.9
    }
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    windows:LIBS += -liphlpapi
}

# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "FIRST_CLASS_MESSAGING=1"
contains(FIRST_CLASS_MESSAGING, 1) {
    message(Building with first-class messaging)
    DEFINES += FIRST_CLASS_MESSAGING
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

!windows {
    # for extra security against potential buffer overflows
    QMAKE_CXXFLAGS += -fstack-protector
    QMAKE_LFLAGS += -fstack-protector
    # do not enable this on windows, as it will result in a non-working executable!
}

# regenerate src/build.h
!windows || contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = genbuildhook
    PRE_TARGETDEPS += genbuildhook
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -Wextra \
    -Wformat \
    -Wformat-security \
    -Wno-ignored-qualifiers \
    -Wno-invalid-offsetof \
    -Wno-sign-compare \
    -Wno-unused-parameter \
    -Wno-unused-variable

# this option unrecognized when building on OSX 10.6.8
# TODO: is this still the case with OSX 12.0?
!macx {
    QMAKE_CXXFLAGS_WARN_ON += -fdiagnostics-show-option -fpermissive
}
macx:QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations

contains(USE_O3, 1) {
    message(Building O3 optimization flag)
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS += -O3
    QMAKE_CFLAGS += -O3
}

contains(USE_O0, 1) {
    message(Building O0 optimization flag)
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS += -O0
    QMAKE_CFLAGS += -O0
}

# Input
DEPENDPATH += src src/json src/qt

HEADERS += src/addrman.h \
    src/allocators.h \
    src/base58.h \
    src/bignum.h \
    src/bitcoinrpc.h \
    src/checkpoints.h \
    src/compat.h \
    src/crypter.h \
    src/db.h \
    src/dcrypt.h \
    src/init.h \
    src/irc.h \
    src/json/json_spirit.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_writer_template.h \
    src/kernel.h \
    src/key.h \
    src/keystore.h \
    src/main.h \
    src/mruset.h \
    src/net.h \
    src/protocol.h \
    src/qt/aboutdialog.h \
    src/qt/addressbookpage.h \
    src/qt/addresstablemodel.h \
    src/qt/askpassphrasedialog.h \
    src/qt/bitcoinaddressvalidator.h \
    src/qt/bitcoinamountfield.h \
    src/qt/bitcoingui.h \
    src/qt/bitcoinunits.h \
    src/qt/blockbrowser.h \
    src/qt/burncoinsdialog.h \
    src/qt/burncoinsentry.h \
    src/qt/clientmodel.h \
    src/qt/csvmodelwriter.h \
    src/qt/editaddressdialog.h \
    src/qt/guiconstants.h \
    src/qt/guiutil.h \
    src/qt/inscriptiondialog.h \
    src/qt/messagepage.h \
    src/qt/miningpage.h \
    src/qt/monitoreddatamapper.h \
    src/qt/notificator.h \
    src/qt/optionsdialog.h \
    src/qt/optionsmodel.h \
    src/qt/overviewpage.h \
    src/qt/qcustomplot.h \
    src/qt/qtipcserver.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/qvaluecombobox.h \
    src/qt/rpcconsole.h \
    src/qt/sendcoinsdialog.h \
    src/qt/sendcoinsentry.h \
    src/qt/torrentpage.h \
    src/qt/torrenttablemodel.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionrecord.h \
    src/qt/transactiontablemodel.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/script.h \
    src/serialize.h \
    src/sha256.h \
    src/smalldata.h \
    src/strlcpy.h \
    src/ui_interface.h \
    src/uint256.h \
    src/util.h \
    src/wallet.h \
    src/walletdb.h \
    src/version.h
x
SOURCES += src/addrman.cpp \
    src/bitcoinrpc.cpp \
    src/checkpoints.cpp \
    src/crypter.cpp \
    src/db.cpp \
    src/dcrypt.cpp \
    src/init.cpp \
    src/irc.cpp \
    src/json/json_spirit_reader.cpp \
    src/json/json_spirit_value.cpp \
    src/json/json_spirit_writer.cpp \
    src/kernel.cpp \
    src/key.cpp \
    src/keystore.cpp \
    src/main.cpp \
    src/net.cpp \
    src/netbase.cpp \
    src/protocol.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/qt/bitcoin.cpp \
    src/qt/bitcoingui.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/qt/bitcoinstrings.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/blockbrowser.cpp \
    src/qt/burncoinsdialog.cpp \
    src/qt/burncoinsentry.cpp \
    src/qt/clientmodel.cpp \
    src/qt/csvmodelwriter.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/guiutil.cpp \
    src/qt/inscriptiondialog.cpp \
    src/qt/messagepage.cpp \
    src/qt/miningpage.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/notificator.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/overviewpage.cpp \
    src/qt/qcustomplot.cpp \
    src/qt/qtipcserver.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/torrentpage.cpp \
    src/qt/torrenttablemodel.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/rpcdump.cpp \
    src/script.cpp \
    src/sha256.cpp \
    src/smalldata.cpp \
    src/util.cpp \
    src/version.cpp \
    src/wallet.cpp \
    src/walletdb.cpp

RESOURCES += \
    src/qt/bitcoin.qrc

FORMS += \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/blockbrowser.ui \
    src/qt/forms/burncoinsdialog.ui \
    src/qt/forms/burncoinsentry.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/inscriptiondialog.ui \
    src/qt/forms/messagepage.ui \
    src/qt/forms/miningpage.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/torrentpage.ui \
    src/qt/forms/transactiondescdialog.ui

contains(USE_QRCODE, 1) {
    HEADERS += src/qt/qrcodedialog.h
    SOURCES += src/qt/qrcodedialog.cpp
    FORMS += src/qt/forms/qrcodedialog.ui
}

contains(BITCOIN_QT_TEST, 1) {
    SOURCES += src/qt/test/test_main.cpp \
        src/qt/test/uritests.cpp
    HEADERS += src/qt/test/uritests.h
    DEPENDPATH += src/qt/test
    QT += testlib
    TARGET = bitcoin-qt_test
    DEFINES += BITCOIN_QT_TEST
}

contains(USE_SSE2, 1) {
    DEFINES += USE_SSE2
    gccsse2.input  = SOURCES_SSE2
    gccsse2.output = $$PWD/build/${QMAKE_FILE_BASE}.o
    gccsse2.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME} -msse2 -mstackrealign
    QMAKE_EXTRA_COMPILERS += gccsse2
    SOURCES_SSE2 += src/scrypt-sse2.cpp
}

# Todo: Remove this line when switching to Qt5, as that option was removed
lessThan(QT_MAJOR_VERSION, 5) {
    CODECFORTR = UTF-8
}

# for lrelease/lupdate
# also add new translations to src/qt/bitcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    windows:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(TS_DIR):TS_DIR = src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$TS_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM
PRE_TARGETDEPS += compiler_TSQM_make_all

# "Other files" to show in Qt Creator
OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README README.md res/bitcoin-qt.rc

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mgw44-mt-1_43
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

# isEmpty(BDB_LIB_SUFFIX) {
#     BDB_LIB_SUFFIX = -4.8
# }

isEmpty(BDB_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_INCLUDE_PATH = /usr/local/opt/berkeley-db4/include
        }else{
            macx:BDB_INCLUDE_PATH = /usr/local/bekerley-db/include
        }
    }else{
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_INCLUDE_PATH = /opt/local/berkeley-db4/include
        }else{
            macx:BDB_INCLUDE_PATH = /opt/local/berkeley-db/include
        }
    }
    windows:BDB_INCLUDE_PATH = C:/dev/coindeps32/bdb-4.8/include
    # For backward compatibility specify, else assume currency
    contains(BDB_LIB_SUFFIX, 4.8) {
        !macx:unix:BDB_INCLUDE_PATH = /usr/local/BerkeleyDB/include
    } # else{
      #   !macx:unix:BDB_INCLUDE_PATH = /usr/include
    # }
    INCLUDEPATH += $$BDB_INCLUDE_PATH
}

isEmpty(BDB_LIB_PATH) {
    contains(CONFIG, brew) {
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_LIB_PATH = /usr/local/opt/berkeley-db4/lib
        }else{
            macx:BDB_LIB_PATH = /usr/local/opt/berkeley-db/lib
        }
    }else{
        contains(BDB_LIB_SUFFIX, -4.8) {
            macx:BDB_LIB_PATH = /opt/local/berkeley-db4/lib
        }else{
            macx:BDB_LIB_PATH = /opt/local/berkeleydb/lib
        }
    }
    windows:BDB_LIB_PATH = C:/dev/coindeps32/bdb-4.8/lib
    # For backward compatibility specify, else assume currency
    contains(BDB_LIB_SUFFIX, -4.8) {
        !macx:unix:BDB_LIB_PATH = /usr/local/BerkeleyDB/lib
    } # else{
      #   !macx:unix:BDB_LIB_PATH = /usr/lib/x86_64-linux-gnu/
    # }
    LIBS += $$join(BDB_LIB_PATH,,-L,)
}

isEmpty(BOOST_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        macx:BOOST_INCLUDE_PATH = /usr/local/opt/boost/include
    }else{
        macx:BOOST_INCLUDE_PATH = /opt/local/include
    }
    windows:BOOST_INCLUDE_PATH = C:/dev/coindeps32/boost_1_57_0/include
    !macx:unix:BOOST_INCLUDE_PATH = /usr/include/boost
    INCLUDEPATH += $$BOOST_INCLUDE_PATH
}

isEmpty(BOOST_LIB_PATH) {
    contains(CONFIG, brew) {
        macx:BOOST_LIB_PATH = /usr/local/opt/boost/lib
    }else{
        macx:BOOST_LIB_PATH = /opt/local/lib
    }
    windows:BOOST_LIB_PATH = C:/dev/coindeps32/boost_1_57_0/lib
    # !macx:unix:BOOST_LIB_PATH = /usr/lib
    LIBS += $$join(BOOST_LIB_PATH,,-L,)
}

isEmpty(OPENSSL_INCLUDE_PATH) {
    contains(CONFIG, brew) {
        macx:OPENSSL_INCLUDE_PATH = /usr/local/opt/openssl/include
    }else{
        macx:OPENSSL_INCLUDE_PATH = /opt/local/include
    }
    windows:OPENSSL_INCLUDE_PATH = C:/dev/coindeps32/openssl-1.0.1p/include
    !macx:unix:OPENSSL_INCLUDE_PATH = /usr/include/openssl
    INCLUDEPATH += $$OPENSSL_INCLUDE_PATH
}

isEmpty(OPENSSL_LIB_PATH) {
    contains(CONFIG, brew) {
        macx:OPENSSL_LIB_PATH = /usr/local/opt/openssl/lib
    }else{
        macx:OPENSSL_LIB_PATH = /opt/local/lib
    }
    windows:OPENSSL_LIB_PATH = C:/dev/coindeps32/openssl-1.0.1p/lib
    # !macx:unix:OPENSSL_LIB_PATH = /usr/lib
    LIBS += $$join(OPENSSL_LIB_PATH,,-L,)}

# Force OS X Sierra specifics
macx {
	CONFIG += 11 x86_64
	HEADERS += src/qt/macdockiconhandler.h src/qt/macnotificationhandler.h
	OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm src/qt/macnotificationhandler.mm
	LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
	LIBS += /usr/local/opt/miniupnpc/lib/libminiupnpc.a
	LIBS += /usr/local/opt/berkeley-db/lib/libdb_cxx.a
	LIBS += /usr/local/opt/openssl/lib/libcrypto.a
	LIBS += /usr/local/opt/openssl/lib/libssl.a
	LIBS += /usr/local/opt/boost/lib/libboost_system-mt.a
	LIBS += /usr/local/opt/boost/lib/libboost_filesystem-mt.a
	LIBS += /usr/local/opt/boost/lib/libboost_program_options-mt.a
	LIBS += /usr/local/opt/boost/lib/libboost_thread-mt.a
	DEFINES += MAC_OSX MSG_NOSIGNAL=0
	ICON = src/qt/res/icons/slimcoin.icns
	TARGET = "SLIMCoin-Qt"
	QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
	QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
	QMAKE_MAC_SDK = macosx10.12
	CXXFLAGS += -std=c++11 -march=i686
	QMAKE_INFO_PLIST = share/qt/Info.plist
        CONFIG -= brew
}

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!windows:!mac {
    DEFINES += LINUX
    LIBS += -lrt
}

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += \
    $$SLIMCOIN_SRC_PATH \
    $$SLIMCOIN_SRC_PATH/qt \
    $$QT_INCLUDE_PATH \
    $$QT_INCLUDE_PATH/QtGui

LIBS += $$join(LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
# windows:LIBS += -lws2_32 -lshlwapi
# windows:DEFINES += WIN32
# windows:RC_FILE = src/qt/res/bitcoin-qt.rc
windows:LIBS += -lole32 -luuid -lgdi32 -lwsock32
LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX


# for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
QMAKE_CXXFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
QMAKE_LFLAGS *= -fstack-protector-all --param ssp-buffer-size=1

# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2 # -static -static-libgcc -static-libstdc++
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
windows:QMAKE_LFLAGS *= -D_FORTIFY_SOURCE=2 -static -static-libgcc -static-libstdc++
windows:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat

# on Windows: enable GCC large address aware linker flag
# hack: when compiling 64-bit, pass 64BIT=1 to qmake to avoid incompatible large-address flag
windows:!contains(64BIT, 1) QMAKE_LFLAGS *= -Wl,--large-address-aware

macx:{
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    QMAKE_RPATHDIR += @executable_path/lib
}

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)
