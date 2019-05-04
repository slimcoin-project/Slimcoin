## Building on Ubuntu Xenix and above

sudo apt install build-essential \
    python3 \
    libtool \
    autotools-dev \
    automake \
    pkg-config \
    bsdmainutils \
    libssl-dev \
    libevent-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-chrono-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libminiupnpc-dev \
    libqt5gui5 \
    libqt5core5a \
    libqt5dbus5 \
    qttools5-dev \
    qttools5-dev-tools \
    libprotobuf-dev \
    protobuf-compiler \
    libqrencode-dev \
    libminiupnpc-dev

Whilst the above packages will install the distro's version of BerkeleyDB and enables a successful compilation with the option `--with-compatible-bdb`, using Berkeley DB 4.8 is recommended. If you have to build it yourself, you can use the installation script [included in [`contrib/`](https://github.com/slimcoin-project/slimcoin/blob/master/contrib/install_db4.sh):

    ./contrib/install_db4.sh `pwd`

from the root of the repository.

    qmake USE_UPNP=1 USE_QRCODE=1 FIRST_CLASS_MESSAGING=1 RELEASE=1 slimcoin-qt.pro
    make
    cd src
    make -f makefile.unix slimcoind
    make -f makefile.unix test_slimcoin
    ./test_slimcoin -r short

