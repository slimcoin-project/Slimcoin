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

## Minting with an encrypted wallet

Wallets cannot be unencrypted in Slimcoin. If you encrypt (or “lock”) your wallet, you must thereafter specify how long it is to remain unlocked for minting. The following instructions for minting were copied from peercoin.net's [Minting Guide](https://peercoin.net/minting-guide)

1. Ensure your wallet is encrypted with a good passphrase. Write this down and keep it somewhere safe; if you forget the passphrase you will lose your coins. The wallet encryption option can be found under the settings tab in the Peercoin-Qt wallet program.

2. To start minting go to help -> debug window -> console and enter: `walletpassphrase abc 999999 true` where `abc` is your passphrase and `999999` is the time you want to mint for in seconds. You can change the amount of time to whatever you like, but it is usually easiest just to set it at a very high number. If your passphrase includes spaces, enclose it in quotation marks.

3. Clear your passphrase by pressing `Ctrl-L`.

4. Minting status is indicated by the little padlock in the bottom right corner of the client. After a few moments it should become unlocked. If you hover your mouse over it, it should say "Wallet is encrypted and currently unlocked for block minting only".


## Build on Linux VM

(useful fragment, parked for later inclusion)

    sudo apt-get install libprotoc-dev libprotobuf-dev -y
    sudo apt-get install libpng-dev qrencode -y
    sudo apt-get install libqrencode-dev -y

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

## Technical parameters

pszTimestamp: "RT: 2 southeast Ukranian regions to hold referendum May 11 as 
planned";

strMessageMagic: "SLIMCoin Signed Message:\n"
pchMessageStart: 6e8b92a5
testnet_pchMessageStart: 4d2ae1ab

nBits: 0.00024414
nVersion: 1
nTime: 1399578460
nNonce: 116872

testnetnTime: 1390500425
testnetnNonce: 63626

PORT: 41682
RPC_PORT: 41683
TESTNET_PORT: 41684
TESTNET_RPC_PORT: 41685

PUBKEY_ADDRESS: 63 - slimcoin: addresses begin with 'S'
SCRIPT_ADDRESS: 125 - slimcoin: script addresses begin with 's'
PUBKEY_ADDRESS_TEST: 111
SCRIPT_ADDRESS_TEST: 196

### Status printout on startup...

    Slimcoin Network: 
        genesis=0x00000766be5a4bb74c04 
        nBitsLimit=0x1e0fffff 
        nBitsInitial=0x1e07ffff
        nStakeMinAge=604800 
        nCoinbaseMaturity=500 
        nModifierInterval=21600

## Repository Guidelines

* Developers work in their own forks, then submit pull requests when they think their feature or bug fix is ready.
* If it is a simple/trivial/non-controversial change, then one of the development team members simply pulls it.
* If it is a more complicated or potentially controversial change, then the change may be discussed in the pull request, or the requester may be asked to start a discussion [SlimCoin Talk](<forum link>) for a broader community discussion. 
* The patch will be accepted if there is broad consensus that it is a good thing. Developers should expect to rework and resubmit patches if they don't match the project's coding conventions (see coding.txt) or are controversial.
* From time to time a pull request will become outdated. If this occurs, and the pull is no longer automatically mergeable; a comment on the pull will be used to issue a warning of closure.  Pull requests closed in this manner will have their corresponding issue labeled 'stagnant'.

---
## All cmmand-line options

```
Command-line options:

-conf=<file>            Specify configuration file (default: slimcoin.conf)
-pid=<file>             Specify pid file (default: slimcoind.pid)
-gen                    Generate coins
-gen=0                  Don't generate coins
-min                    Start minimized
-splash                 Show splash screen on startup (default: 1)
-datadir=<dir>          Specify data directory
-dbcache=<n>            Set database cache size in megabytes (default: 25)
-dblogsize=<n>          Set database disk log size in megabytes (default: 100)
-timeout=<n>            Specify connection timeout (in milliseconds)
-proxy=<ip:port>        Connect through socks4 proxy
-dns                    Allow DNS lookups for addnode and connect
-port=<port>            Listen for connections on <port> (default: 41682 or testnet: 41684)
-maxconnections=<n>     Maintain at most <n> connections to peers (default: 125)
-addnode=<ip>           Add a node to connect to and attempt to keep the connection open
-connect=<ip>           Connect only to the specified node
-listen                 Accept connections from outside (default: 1)
-lang=<lang>            Set language, for example "de_DE" (default: system locale)
-dnsseed                Find peers using DNS lookup (default: 1)
-banscore=<n>           Threshold for disconnecting misbehaving peers (default: 100)
-bantime=<n>            Number of seconds to keep misbehaving peers from reconnecting (default: 86400)
-maxreceivebuffer=<n>   Maximum per-connection receive buffer, <n>*1000 bytes (default: 10000)
-maxsendbuffer=<n>      Maximum per-connection send buffer, <n>*1000 bytes (default: 10000)
-maxorphanblocks=<n>    Maximum number of orphan blocks to store in the memory (default: 750)
-upnp                   Use Universal Plug and Play to map the listening port (default: 1)
-detachdb               Detach block and address databases. Increases shutdown time (default: 0)
-paytxfee=<amt>         Fee per KB to add to transactions you send
-server                 Accept command line and JSON-RPC commands
-testnet                Use the test network
-debug                  Output extra debugging information
-logtimestamps          Prepend debug output with timestamp
-printtoconsole         Send trace/debug info to console instead of debug.log file
-rpcuser=<user>         Username for JSON-RPC connections
-rpcpassword=<pw>       Password for JSON-RPC connections
-rpcport=<port>         Listen for JSON-RPC connections on <port> (default: 41683)
-rpcallowip=<ip>        Allow JSON-RPC connections from specified IP address
-rpcconnect=<ip>        Send commands to node running on <ip> (default: 127.0.0.1)
-blocknotify=<cmd>      Execute command when the best block changes (%s in cmd is replaced by block hash)
-walletnotify=<cmd>     Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)
-upgradewallet          Upgrade wallet to latest format
-keypool=<n>            Set key pool size to <n> (default: 100)
-rescan                 Rescan the block chain for missing wallet transactions
-checkblocks=<n>        How many blocks to check at startup (default: 2500, 0 = all)
-checklevel=<n>         How thorough the block verification is (0-6, default: 1)

SSL options: (see the Bitcoin Wiki for SSL setup instructions)
-rpcssl                                     Use OpenSSL (https) for JSON-RPC connections
-rpcsslcertificatechainfile=<file.cert>     Server certificate file (default: server.cert)
-rpcsslprivatekeyfile=<file.pem>            Server private key (default: server.pem)
-rpcsslciphers=<ciphers>                    Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)
-?                      This help message

UI options:
-lang=<lang>            Set language, for example "de_DE" (default: system locale)
-min                    Start minimized
-splash                 Show splash screen on startup (default: 1)

Previously undocumented:

-checkpointkey=<key>    Checkpoint master private key, triggers generation of a sync checkpoint
-limitfreerelay=<n>     Limits relay rate, <n> is in units of thousand-bytes-per-minute
-fastindex              Skip certain time-consuming (presumably, optional) checks (default: 1)
-loadblockindextest     Load the block index and print the block tree (default: 0)
-printblockindex        Print the block tree (default: 0)
-printtodebugger        WIN32-specific, print to (presumably) a Windows system debug app (default: 0)
-printstakemodifier     Enables debug output of stake modifier data (default: 0)
-allowreceivebyip       Allow order requests (default: 1)
-dropmessagestest       Causes all received messages to be dropped (default: 0)
-printcoinage           Enables debug log of coin age data (default: 0)
-printcreation          Enables debug log of fee creation and destruction - fees are not collected by miners as in bitcoin
but are destroyed to compensate the entire network.  (default: 0)
-printmessagestart      Enables debug log of message processing statistics (default: 0)
-printpriority          Enables debug log of staking priority statistics (default: 0)
-testsafemode           Disables RPC status warnings (default: 0)
-chart                  Enable chart in overview tab (default: 1)
-avatar                 Disables automatic creation of a transaction change address (default: 0)
-printcoinstake         Enables debug log of stake creation data (default: 0)
-printfee               Enables debug log of coinstake fee data (default: 0)
-printkeypool           Enables debug log of data on keys added to pool (default: 0)
-printselectcoin        Enables debug log of coin staking selection data (default: 0)```


## SPARQL queries for interrogating the RDF graph expression of the blockchain

Maximum block height:

    PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>

    SELECT ?subject ?height
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#Block> .
      ?subject <http://purl.org/net/bel-epa/ccy#height> ?height
    }
    ORDER BY DESC(?height)
    LIMIT 1


List all inscriptions:

    PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>

    SELECT DISTINCT(?inscription as ?inscriptions)
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#TransactionOutput> .
      ?subject <http://purl.org/net/bel-epa/ccy#inscription> ?inscription
    }

## RDF representation

A block:

    :C0000000733567067927d0778bdee1ad3034ecc60e49cfc3ae05dbc3a1eddbb60 a :Block ;
        :difficulty 0.06464541 ;
        :flags "proof-of-work"^^xsd:string ;
        :height 759 ;
        :mint 6.19 ;
        :nextblockhash "None"^^xsd:hexBinary ;
        :previousblockhash :C8fe77bb34c5aec9206a3c73beccaf1fc53e6d4afe297c3782f57c09e4eb9c627 ;
        :size 355 ;
        :time 1492346152 .

A Transaction:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b a :Transaction ;
        :blockhash <C0000002cb1b737ffd109639e3f7e1f81959dffd602e58d1ad6be9720215c5f7b> ;
        :input :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-I-0 ;
        :output :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-O-0 ;
        :time 1492284846 .

A TransactionInput:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-I-0 a :TransactionInput ;
        :coinbase "04a675f2580101062f503253482f"^^xsd:hexBinary .

A TransactionOutput:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-O-0 a :TransactionOutput ;
        :address :mgeUFf7a8dC94yJ5BMbtVKWur1jTrsnR4C ;
        :n 0 ;
        :pkasm "02f07a96082739cb88c5207969d0786771b7a2bcaf3c0fb0c470375d62850a0a5a OP_CHECKSIG"^^xsd:string ;
        :type "pubkey"^^xsd:string ;
        :value 8.34 .

## SPARQL queries

Identify the TransactionOutput for a given inscription:

    SELECT ?subject
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#TransactionOutput> .
      ?subject <http://purl.org/net/bel-epa/ccy#inscription> "Hello world."
    }


results:

    <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25-O-2>

Identify the Transaction containing the TransactionOutput:

    SELECT ?tx
    WHERE {
      ?tx rdf:type <http://purl.org/net/bel-epa/ccy#Transaction> .
      ?tx <http://purl.org/net/bel-epa/ccy#output> <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25-O-2> .
    }

results:

    <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25>

Identify the Block containing the Transaction:

    SELECT ?b
    WHERE {
      <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25> <http://purl.org/net/bel-epa/ccy#blockhash> ?b .
    }

results:

    <http://purl.org/net/bel-epa/ccy#C000000061900945721ae028b98844c8717bed57ab3f10d8720c8f96f9d530e34>


Show all statements about the Block:

    SELECT ?pred ?obj
    WHERE {
      <http://purl.org/net/bel-epa/ccy#C000000061900945721ae028b98844c8717bed57ab3f10d8720c8f96f9d530e34> ?pred ?obj .
    }

results:

    1.  rdf:type                                          <http://purl.org/net/bel-epa/ccy#Block>
    2.  <http://purl.org/net/bel-epa/ccy#difficulty>      0.06490015
    3   <http://purl.org/net/bel-epa/ccy#flags>           "proof-of-work"
    4   <http://purl.org/net/bel-epa/ccy#height>          138
    5   <http://purl.org/net/bel-epa/ccy#mint>            6.19
    6   <http://purl.org/net/bel-epa/ccy#nextblockhash>   <http://purl.org/net/bel-epa/ccy#C7a52155b2a89ae0ce461742013ec43d55b66c11410c4f5094cfeb747622f235a>
    7   <http://purl.org/net/bel-epa/ccy#previousblockhash> <http://purl.org/net/bel-epa/ccy#C3bd837d3371108666e1c9c5967d68f7bb95df597b4d0de9dd754f975d1df32c6>
    8   <http://purl.org/net/bel-epa/ccy#size>            588
    9   <http://purl.org/net/bel-epa/ccy#time>            1492283025
