TEMPLATE = app
TARGET = slimcoin-qt
VERSION = 0.6.4.0
INCLUDEPATH += src src/json src/qt
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE QT_NO_PRINTER
CONFIG += no_include_pwd
CONFIG += thread
CONFIG += static
CONFIG += openssl
CONFIG += release
QT += core gui network widgets

# FIXME: Add macports / brew switch and bind paths as appropriate
# in the interim, this is for brew ... (but atm, see explicit 
# over-riding bindings at bottom of this file)
isEmpty(BDB_LIB_SUFFIX) {
    BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /usr/local/opt/berkeley-db4/lib
}

isEmpty(BDB_INCLUDE_PATH) {
    BDB_INCLUDE_PATH = /usr/local/opt/berkeley-db4/include
}

isEmpty(BOOST_LIB_PATH) {
    BOOST_LIB_PATH = /usr/local/opt/boost/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    BOOST_INCLUDE_PATH = /usr/local/opt/boost/include
}
isEmpty(OPENSSL_INCLUDE_PATH) {
    OPENSSL_INCLUDE_PATH=/usr/local/opt/openssl/include
}

isEmpty(OPENSSL_LIB_PATH) {
     OPENSSL_LIB_PATH=/usr/local/opt/openssl/lib
}

isEmpty(MINIUPNPC_INCLUDE_PATH) {
     MINIUPNPC_INCLUDE_PATH=/usr/local/opt/miniupnpc/include
}

isEmpty(MINIUPNPC_LIB_PATH) {
     MINIUPNPC_LIB_PATH=/usr/local/opt/miniupnpc/lib
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.9, 64-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.12 -arch x86_64 -isysroot /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_LFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
}

# for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
QMAKE_CXXFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
QMAKE_LFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
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
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += /usr/local/opt/miniupnpc/include
    LIBS += /usr/local/opt/miniupnpc/lib/libminiupnpc.a
    win32:LIBS += -liphlpapi
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

QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Wformat -Wformat-security -Wno-invalid-offsetof -Wno-sign-compare -Wno-unused-parameter
# this option unrecognized when building on OSX 10.6.8
!macx {
    QMAKE_CXXFLAGS_WARN_ON += -fdiagnostics-show-option # -fpermissive
}

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

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wno-ignored-qualifiers -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-variable -fpermissive

macx:QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations

# Input
DEPENDPATH += src src/json src/qt
HEADERS += src/qt/bitcoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/sendcoinsdialog.h \
    src/qt/burncoinsdialog.h \
    src/qt/addressbookpage.h \
    src/qt/messagepage.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/bitcoinaddressvalidator.h \
    src/addrman.h \
    src/base58.h \
    src/bignum.h \
    src/checkpoints.h \
    src/compat.h \
    src/util.h \
    src/uint256.h \
    src/serialize.h \
    src/strlcpy.h \
    src/main.h \
    src/net.h \
    src/key.h \
    src/db.h \
    src/walletdb.h \
    src/script.h \
    src/init.h \
    src/irc.h \
    src/mruset.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/bitcoinamountfield.h \
    src/wallet.h \
    src/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/bitcoinrpc.h \
    src/qt/overviewpage.h \
    src/qt/miningpage.h \
    src/qt/csvmodelwriter.h \
    src/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/burncoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/bitcoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/qt/rpcconsole.h \
    src/qt/blockbrowser.h \
    src/protocol.h \
    src/qt/notificator.h \
    src/qt/qtipcserver.h \
    src/allocators.h \
    src/ui_interface.h \
    src/kernel.h \
    src/dcrypt.h \
    src/sha256.h \
    src/qt/macnotificationhandler.h \
    src/qt/qcustomplot.h

SOURCES += src/qt/bitcoin.cpp src/qt/bitcoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/burncoinsdialog.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/messagepage.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/version.cpp \
    src/util.cpp \
    src/netbase.cpp \
    src/key.cpp \
    src/script.cpp \
    src/main.cpp \
    src/init.cpp \
    src/net.cpp \
    src/irc.cpp \
    src/checkpoints.cpp \
    src/addrman.cpp \
    src/db.cpp \
    src/walletdb.cpp \
    src/json/json_spirit_writer.cpp \
    src/json/json_spirit_value.cpp \
    src/json/json_spirit_reader.cpp \
    src/qt/clientmodel.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/bitcoinstrings.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/wallet.cpp \
    src/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/bitcoinrpc.cpp \
    src/rpcdump.cpp \
    src/qt/miningpage.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/burncoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/blockbrowser.cpp \
    src/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/qtipcserver.cpp \
    src/kernel.cpp \
    src/dcrypt.cpp \
    src/sha256.cpp \
    src/qt/qcustomplot.cpp

RESOURCES += \
    src/qt/bitcoin.qrc

FORMS += \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/burncoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/messagepage.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/miningpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/burncoinsentry.ui \
    src/qt/forms/blockbrowser.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui

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
CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/bitcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
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
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

LIBS += /usr/local/Cellar/miniupnpc/2.0/lib/libminiupnpc.a
LIBS += /usr/local/Cellar/berkeley-db4/4.8.30/lib/libdb_cxx.a
LIBS += /usr/local/Cellar/openssl/1.0.2j/lib/libcrypto.a
LIBS += /usr/local/Cellar/openssl/1.0.2j/lib/libssl.a
LIBS += /usr/local/Cellar/boost/1.62.0/lib/libboost_system-mt.a
LIBS += /usr/local/Cellar/boost/1.62.0/lib/libboost_filesystem-mt.a
LIBS += /usr/local/Cellar/boost/1.62.0/lib/libboost_program_options-mt.a
LIBS += /usr/local/Cellar/boost/1.62.0/lib/libboost_thread-mt.a

CONFIG += 11 x86_64
HEADERS += src/qt/macdockiconhandler.h src/qt/macnotificationhandler.h
OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm src/qt/macnotificationhandler.mm
LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
DEFINES += MAC_OSX MSG_NOSIGNAL=0
ICON = src/qt/res/icons/slimcoin.icns
TARGET = "SLIMCoin-Qt"
QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.12
QMAKE_MAC_SDK = macosx10.12
CXXFLAGS += -std=c++11 -march=i686
QMAKE_INFO_PLIST = share/qt/Info.plist

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH \
    $$BDB_INCLUDE_PATH \
    $$MINIUPNPC_INCLUDE_PATH \
    $$OPENSSL_INCLUDE_PATH \
    $$QRENCODE_INCLUDE_PATH \
    $$SLIMCOIN_SRC_PATH \
    $$SLIMCOIN_SRC_PATH/qt \
    $$QT_INCLUDE_PATH \
    $$QT_INCLUDE_PATH/QtGui

LIBS += \
    $$join(LIB_PATH,,-L,) \
    $$join(BOOST_LIB_PATH,,-L,) \
    $$join(BDB_LIB_PATH,,-L,) \
    $$join(OPENSSL_LIB_PATH,,-L,) \
    $$join(MINIUPNPC_LIB_PATH,,-L,) \
    $$join(QRENCODE_LIB_PATH,,-L,)

# LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX -lminiupnpc
# LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX

QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2 # -static -static-libgcc -static-libstdc++

QMAKE_RPATHDIR += @executable_path/../Frameworks
QMAKE_RPATHDIR += @executable_path/lib

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)



