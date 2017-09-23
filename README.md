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

https://graphs.coinmarketcap.com/currencies/slimcoin/

https://api.coinmarketcap.com/v1/ticker/slimcoin/

    Donations:

    Slimcoin address SNDbyMt3UPDTyLsSUumqwniJcf2pCtGosP
    Hash    0a23366013d0d1ddd8de1156af972b5f2aeee400
    Number of confirmed transactions    34
    First transaction   1061 days ago
    Last transaction    1007 days ago
    Total Received  15103.56 SLM
    Balance 4755.12 SLM

    Bounty:

    Slimcoin address ShooqRfkshDajLTvYMEKRKxXW7ooZLBZsm
    Hash    e105a5dc4b200fee9e6693746b44803e7be9a949
    Number of confirmed transactions    5
    First transaction   1122 days ago
    Last transaction    1121 days ago
    Total Received  72.66 SLM
    Balance 0 SLM


---
September 07, 2014, 09:58:18 PM - https://bitcointalk.org/index.php?topic=613213.msg8719988#msg8719988

> Lately, something 'bigger' has come along and have decided to lower my activity on Slimcoin. I will do compilations and releases, maintain the master seed node for synchronized checkpointing, and review pull requests on github. Apart from that, I only plan on doing small tweaks here and there if necessary. I am not abandoning Slimcoin, I am merely giving the majority of the development up to the community. That does not mean I am gone, I may return to more development here depending on how the future goes.

---

September 10, 2014, 09:02:13 PM

> I am maintaining this project, just development will be lower and more community based.

October 23, 2014, 05:35:20 PM - https://bitcointalk.org/index.php?topic=613213.msg9306854#msg9306854

> As I have posted before, the reason I "left" this project is some other stuff came up. I say "left" as I still follow its development on the forum. I did state I could do basic maintenance, of which was to hold the central sync node up. I have absolutely no problems with you forking this project, in fact, it would be of great delight to me if you did so. I can offer you assistance in navigating the PoB code and making you a reddit administrator for /r/Slimcoin. As for development, I have nothing much planned in the short termed future.
> 
> -P4Titan


## Member contributions

October 17, 2014, 11:12:26 AM - https://bitcointalk.org/index.php?topic=613213.msg9232920#msg9232920

Quote from: AizenSou on October 17, 2014, 09:12:37 AM
> a123 could you consider to create a new thread if the old dev won't respond ?

I would try to contact him first. This is really mostly his work, still. But if there's no response I can start a new thread.

I propose in the continued absence of the main dev, we do a hard fork once 

1. PoS is removed
2. we obtain a pool operator or two
3. centralised sync checkpointing is removed (anyone has an opinion on this?)

If no one is against PoS removal, I will just code it such that all PoS blocks after the hard fork will be rejected (for instance, a Block's IsProofOfStake() will always resolve to False if height > forkheight.).

For centralised sync checkpointing, I'll just turn the hardcoded public key into maybe all zeroes or even the SLM burning address, and turn the warnings off (or make the last syncpoint's expiry never expire)

Anything else to be done? I remember reading something about P2Pool needing a few fixes to be operable with Slimcoin. Anyone has details on this?

Hard fork will reject all old clients from connecting and be pure PoW/PoB hybrid.

In the meantime, we could do a community wiki to start collating all the SLM information (such as compilation instructions, installation instructions, PoB theory, etc) so as to help, as well as to start looking for issues / improvements for SLM.

I have enabled the Wiki and Issues tracking feature on my Github page for Slimcoin: https://github.com/kryptoslab/slimcoin, but would be open to suggestions as to where else best to host such information (:

---


## Minting with an encrypted wallet

***Once, encrypted, wallets cannot be unencrypted in Slimcoin.*** If you encrypt (or “lock”) your wallet, you must thereafter specify how long it is to remain unlocked for minting. The following instructions for minting were copied from peercoin.net's [Minting Guide](https://peercoin.net/minting-guide)

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

- Block time is 1.5 minutes (90 seconds)
- Difficulty re-targets continuously
- Block Rewards:
  - Proof-of-Burn blocks: max 250 coins
  - Proof-of-Work blocks: max 50 coins
- Block rewards decrease in value as the difficulty increases
- No hard supply limit, but comparatively low inflation rate. A "250 million coins maximum" is coded into the client, but probability is extremely low this amount will be reached in the next 100 years.
- Proof of Stake minimum coin age: ~1 week

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
-fastindex              Skip certain time-consuming steps (default: 1)
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
```

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


Get input and output tx for an address:

    PREFIX ccy: <http://purl.org/net/bel-epa/ccy#>
    SELECT DISTINCT ?txo ?datetime ?value ?stype WHERE {
     ?txo ccy:address ccy:VFouUCS3gM3yrTdKHkyKoLGbPcWC8mp4A4 .
     ?tx ccy:output ?txo . ?tx ccy:time ?datetime .
     ?txo ccy:value ?value . ?txo ccy:type ?stype
    } ORDER BY DESC(?datetime)


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

---


### Observations on the miner by jonnylatte (https://bitcointalk.org/index.php?topic=1141676.msg12627802#msg12627802)


A little more fiddling around and I have gotten about a 20% speed improvement by aborting the hash function at count = 120. What makes my eye twitch a little is that I can get a 30% speed improvement on top of that by re-writing 

    void digest_to_string(u8int *hash_digest, u8int *string)

in

https://github.com/slimcoin/slimminer/blob/master/dcrypt_sha256.c

to 

    char * byte_to_hex =
            "000102030405060708090a0b0c0d0e0f"
            "101112131415161718191a1b1c1d1e1f"
            "202122232425262728292a2b2c2d2e2f"
            "303132333435363738393a3b3c3d3e3f"
            "404142434445464748494a4b4c4d4e4f"
            "505152535455565758595a5b5c5d5e5f"
            "606162636465666768696a6b6c6d6e6f"
            "707172737475767778797a7b7c7d7e7f"
            "808182838485868788898a8b8c8d8e8f"
            "909192939495969798999a9b9c9d9e9f"
            "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
            "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
            "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
            "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
            "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
            "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";


    void digest_to_string(u8int *hash_digest, u8int *str)
    {
      register int si = 0;
      register int i = 0;
      for(; i < SHA256_DIGEST_LENGTH; i++)
      {
        memcpy(str+si,byte_to_hex + hash_digest[i]*2,2);
        si+=2;
      }
      str[SHA256_LEN] = 0;
      return;
    }


Which gets rid of the expensive branch and shift operations and just replaces it with a lookup / copy. This function converts a sha256 digest (32 bytes) to a lower case hex string. This is called inside the mixer function every time the internal buffer expands because for some reason they couldn't have used just the byte values (wtf: where they intentionally making it resource intensive in a way that can easily be optimized away?) which means tens to thousands of times per hash. 

I'm not a great programmer but thats why I'm a little concerned that I could get such a significant speed improvement. I'm sure someone who knows what they are doing could do much better. 

I'm not saying the dcrypt function should be replaced. Thats up to you guys but from my perspective it seems like it was knocked together just to be a different but probably wont stay CPU only if it isn't GPU mined already. I think the important aspect is Proof of Burn not what hashing algorithm you use. As far as a fair distribution is concerned I think dcrypt has done its job and even if an ASIC is made for it eventually it wont be broken completely until sha256 is.

---

The other improvement idea I have is theoretical (have not tried it) but I will throw it out there: when the scratchpad is re-hashed only 1 byte is changed and the scratchpad always starts off filled with 0xFF and it is always set to a byte that came from a ascii hex digit (only 16 combinations). That means for a count of N there are only N^16 possible hashes for the buffer and since you can progressively hash this you can stop at whatever N you please. What I'm saying is that if you have enough memory you can pre-calculate a fair chunk of the possible buffers then use a lookup table to retrieve the pre-calculated hashes to progressively add the rest of the hash. 

Even just calculating the first 4 iterations would mean 2*4 less sha256 blocks per hash with a reasonable amount of memory usage...


---

I wrote a program to examine The Dcrypt Algorithm just out of curiosity. I wanted to know how much memory it uses on average. Internally in the mixing hash function it has a scratch pad that exists as multiples of 64 bytes plus the original size of the data being hashed and some fixed sized scratch pads. The variable size buffer is what is of interest because that what is supposed to fight against GPU and ASIC miners. 

After hashing 100K times I got the following result for the multiple: 

average 1026    <10 0.683007%   <100 8.99509%   <500 38.3704%

So on average the function used 65,664 bytes for the variable sized buffer but 0.68% it used less than 640 bytes.

I though this is interesting and it should be possible to see if anyone has a miner that aborts hashing if the memory size of the hash function is over a certain size. 

---
It's possible to completely eliminate the dynamic memory requirements of the dcrypt function:


    #define SHA256_HEX_LEN 64

    uint256 dcrypt_progressive(const uint8_t *data, size_t data_sz)
    {
        //if(fTestNet) return sha256d(data,data_sz);   // for testnet hashes use sha256d

        SHA256_CTX  ctx;   // sha256 context
        uint256     hash_result;    

        uint32_t    index = 0;
        uint8_t     index_values[SHA256_HEX_LEN +1]; 
        uint8_t     scratch_pad[SHA256_HEX_LEN +1];  

        SHA256_Init(&ctx);  // initialize context which will progressively hash the result as data for it is generated in scratch_pad

        sha256_to_str(data, data_sz, index_values);    // initialize index_values with sha256(data) -> ascii/hex
        memset(scratch_pad, 0xff, SHA256_HEX_LEN);     // initialize scratchpad all 0xff 

        do
        {
            index += hex_char_to_int(index_values[index]) + 1; // increment index by the value of the hex char in index_values and add 1 so index is always increasing

            if(index >= INDEX_BUFFER_LEN) // if index is past index_values size, wrap index around and scramble index_values
            {
                index &= 0x3f;  // wrap index around
                sha256_to_str(index_values, INDEX_BUFFER_LEN, index_values); //rescramble with sha256(index_values) -> ascii/hex
            }

            scratch_pad[SHA256_HEX_LEN] = index_values[index]; //set a byte in scratch_pad to index_values[index]
            sha256_to_str(scratch_pad, SHA256_HEX_LEN + 1, scratch_pad); // sha256 hash 

            SHA256_Update(&ctx,scratch_pad,SHA256_HEX_LEN); // write scratch_pad to the sha256 context that will generate the resulting dcrypt hash
        }
        while( (index != SHA256_HEX_LEN - 1) || (index_values[SHA256_HEX_LEN - 1] != scratch_pad[SHA256_HEX_LEN - 1] )); 
        // loop ends when index is at "SHA256_HEX_LEN - 1" and the value of index_values matches the value of scratch_pad at that location
        // this should have a 1 in 16 chance for every time index happens to hit "SHA256_HEX_LEN - 1" 

        SHA256_Update(&ctx,  (u8int*)data,data_sz); // write the original data to the sha256 context for the resulting hash
        SHA256_Final((u8int*)hash_result, &ctx);    // finalize the hash and store the result

        return hash_result; // we are done here
    }

Instead of building up a buffer and hashing it at the end this version progressively hashes the data generated. Sha256 contexts have an internal buffer which is of fixed size. So it seems dcrypt was never memory hard except for the fact that you can get a minor optimization with hashing by buffering the data instead of hashing progressively and then aborting if the resulting buffer gets too big. The optimization is not that you are saving memory or preventing too much memory use, its that you are not hashing longer buffers saving the processing of the large data. This is probably why there isnt a spectacular increase in efficiency by limiting the number of iterations (you are only really saving one sha256 call on the larger buffer but still wasting many small calls for calculating the scratchpad and index values)

In any case I think this version of Dcrypt algorithm might be better for the main client to prevent too much resource usage while we wait on the possibility of a change in hash functions.

I dont think its bad that Dcrypt has fixed memory usage. I hope my code is a clear implementation that could be of help to someone writing a GPU miner which I now believe should not be too difficult to implement (the problem is now variable computation time instead of variable memory usage) If we have a GPU miner then it will be much harder for a botnet or server farm to compete and if we can do it with dcrypt then changing the hashing function is less important..

I have the dcrypt function working on a GPU already using the pseudocode from wikipedia for sha256 and the buffered method. Its just a matter of taking or writing some network code to feed in a valid block header to use as a template and then scan through the results for found blocks/shares and send the results back. I have network code I understand (because I wrote it) that gets work from jsonrpc so that will get done first and I'm sure Stratum is not difficult. The other option is using the CPU miner code but a quick look at how it mines shows something slightly incompatible with what I think is needed (it loops through nonce individual and returns nonce for found block/share, where as I may need to be able to send back multiple shares so I'll have to learn what to change in code I don't fully understand the process flow of, perhaps someone could help me with that but I dont want to give away the GPU code as anything other than a release of the client for cynical reasons) I also want something that can mine in a pool but then fallback to solo mining if the pool stops working.

---

The build instructions are in the README file: 

Dependencies:
   libcurl-*-dev      http://curl.haxx.se/libcurl/
   libjansson         http://www.digip.org/jansson/
      autoconf              https://www.gnu.org/software/autoconf/
      (jansson is included in-tree)

Basic *nix build instructions:
   ./autogen.sh   # only needed if building from git repo
   ./nomacro.pl   # only needed if building on Mac OS X or with Clang
   ./configure CFLAGS="-O3"
   make

As for whether my version should be official I would say once a few people verify it finds them blocks as it does for me. I would also point out that when you massively increase the mumber of threads the hashrate calculation is way off. At first I assumed this was because the program spends most of its time waiting for the network or for threads to start so I moved the timing code to include the time taken to fetch blocks and then all of the time not calculating the time passed (ie get the start time just after the interval is calculated). That made it more accurate but its still way off. Its still make way more blocks when the thread count is pushed up although I have not tested this objectively. This is the main problem that stops me from recommending it as a default cpu miner option.

---

gavrilo77, one thing you might try is setting the scan time (-s N) to something a little higher than the default 5 seconds, for me 30 seconds gets me a much higher hash rate without a significant number of stale blocks. I think the reason for this is that less time is spent fetching new work which is time that the threads are idle. I dont think this would be a problem with a pool/stratum though. 

EDIT: What I just stated should not work. If you increase the scan time it should not have a significant effect on the hashrate. This has been bugging me since I wrote it and I have figured out what the bug is behind this behaviour: the way this version of slimminer optimizes its dcrypt function is to simply skip over any value that takes too long to hash, it effectively only hashes about 1 in 256 values. The effective hashrate that is displayed only includes completed hashes as it should however what I did not count on was that when work is divided up for each thread the miner doesn’t just split up the ranges of nonce values and pass them to each thread but instead it estimates how much each thread can process for the amount of time it will be running. Since the stored values for the hashrate are used in this calculation those values are 1/256 times what is needed for the right amount of work to be passed to each thread. So increasing the scan time only worked to increase the hash rate because of this bug.  

Anyway I have fixed this problem by recording the number of hashes skipped over and then recalculating the hashrate to include them after the effective hashrate is displayed. This hash brought the hashrate back up to the right level even with the short and appropriate scan time since threads are not starved for work and constantly asking for more. 

TL;DR: updated version of  https://github.com/JonnyLatte/slimminer fixes problem that results from the optimized version of dcrypt having a low effective hashrate but needing to think it has a higher hashrate in order for enough work to be allocated to its threads. This improves the hashrate and lowers the time spent asking for more work. 

EDIT2: ok found a block with it and noticed a miner aesthetic glitch, when a block is reported to be found it displayed the hashrate but since I changed the internal hashrates to include the skipped hashes it is displaying this increased hashrate at this point. That does not really effect anything but I'll get rid of it in another commit. 

EDIT3: hashrate displayed correctly when block found, hashrate and effective hashrate now stored separately. Another thing I noticed is that the hashrate fluctuates quite a bit so I have changed the code to make the new hashrate calculation a weighted average of the previous hashrate so that the hashrate does not fluctuate so much and in turn the work supplied to each thread does not fluctuate much. 

---

## P4Titan on Slimcoin

The Dcrypt algorithm was made so that it would be hard to make an ASIC for it, and by then, Proof of burn would have became much bigger and more predominant. Proof of Burn hashes are completely different, they use SHA256 but in a much more differently, giving Slimcoin its uniqueness. Proof of burn mining can be done on any computer, as faster hardware offers no advantage over slower hardware.

Proof of Burn simply requires less hashes to be calculated. Instead of non-stop mining as in PoW, PoB only allows 1 hash for every burn transaction per 90 seconds. Meaning, if one had 100 burn transactions, any computer could operate that as it would need to do 100 SHA256 hashes per 90 seconds, which almost every computer can handle. If that person was to calculate more than 1 hash per burn transaction, that hash would come out the same as there is no way to modify the hash until a new PoW block is found.

The coins that are burnt are forever gone. They are compensated by the proof of burn blocks that are generated as a result of the burning. That destruction of coins is why the coin cap is set to be as high as it is.

Slimcoin will also play well when there is a hardware shortage due to massive demands (like for GPU's during February of this year). With Slimcoin, there are not pre-order queues nor any backorders. If one wants to mine, they simple burn coins and within minutes, they should be up and running.

If there is the opportunity to generate Slimcoins, people will do it. People can generate Slimcoins through PoB, PoW, and PoS. As for PoB's dependability to PoW, given coins can be generated through PoW, people will enact in classic mining. Now, given the block reward is greater for PoB than PoW, and given PoB is more assessable to the larger public, PoB will most likely be bigger than PoW, but PoW mining will not be nonexistent.

The justification behind the 2 billion coins is since PoB involves the destruction of coins, making them unusable again, if a relatively small limit is set, the coins could get exhausted and then people would not want to burn them anymore, thus making the entire PoB useless.

As time passes, burnt coins do decay. That means that the strength at which they mine at decreases as time passes. Also, by burning coins, they are forever destroyed and in-spendable again. That will tequinally lower the amount of coins, but since the total amount that can be generated is 2 billion, that will compensate for the destroyed coins through burning.

> So after all the coins are generated and coins continue to burn, won't there be less and less?

This is true, but it would take a bit of time until that cap is reached.

> Also, what if someone by extreme luck discovers that address in years away from now?

That extreme luck would really need to be super extreme. Also, Slimcoin could be made to detect any outputs coming from a burn address and have the network deny it, regardless if the sender has the private key.

> So people must first mine coins via PoW to obtain coins. Then these coins can be burned via PoB to obtain more coins than if they were just mining through PoW. If there comes a time when they would get less coins from Proof of Burn than they spent burning them, obviously they should stop burning them.

This is true, it is like PoW, if the incentive for mining becomes small, people won't buy as much hardware as they did when the incentive is larger. The network/users will self-regulate on the amount of coins being burned.

> This has nothing to do with the max amount of coins. It would be exactly the same scenario if there were 200,000 coins rather than 2,000,000,000.

I was not too clear in my last post. By setting the total amount of coins to be that high, it will allow people to burn coins without the fear that there will be no coins left for the actual transactions. 

> Regardless of how many max coins you have, there will come a time when you run out of PoW coins or when the multiplier will require so many PoW coins to be burned that it becomes uneconomical to burn PoW coins for PoB coins.

Yes, there will come a time, it would be best if that time was not 1 year from release, but more time from release allowing the spread of Slimcoin's idea.

> Also, can we get a use-case example for the reduction formula?

I will update the whitepaper on that note, but here is the general formula: The strength of the burnt coins mining halves every 350000 proof of work blocks, that halving is continuous and smooth.

Burn hash calculation directly from the source code documentation:


    /*slimcoin: a burn hash is calculated by:
    * hash = (c / b) * 2 ** ((nPoWBlocks - M) / E) * [Hash]
    *
    * Where: c = BURN_CONSTANT (0.01 * CENT)
    *        b = amount of coins burned
    *        nPoWBlocks = the number of proof of work blocks between (not including)
    *                     the blocks with heights last_BlkNHeight and burned_BlkNHeight
    *                         where
    *                             last_BlkNHeight = the height of the last block in the chain
    *                             burned_BlkNHeight = the height of the block at the time of the burning
    *        M = BURN_MIN_CONFIRMS (6), the required amount of proof of work blocks between (not including)
    *                                   the block at the time of burning and the last block in the chain
    *                                   The offset by M allows for the first burn block the burnt coins
    *                                   can hash to be at 100% strength and decay from there, instead of having
    *                                   the coins slightly decayed from the beginning
    *        E = BURN_HASH_DOUBLE (350000.0), an exponential constant which causes 
    *                                   burnt coins to produce slightly larger hashes as time passes
    *
    *        [Hash] = Hash(burntBlockHash ++ burnWTx.GetHash() ++ hashBestBlock)
    *        Where: burntBlockHash = the hash of the block the transaction is found ing
    *               burnTx.GetHash() = the hash of this transaction
    *               hashBestBlock = the hash of the best proof-of-work block in the chain at the time of hashing
    */

> What is the relationship between number of coins burned and reward or reward schedule?

Larger burn amounts are more likely to generate a proof of burn block because their multiplier is smaller, thus making the final burn hash smaller. It is, again, like PoW, one could mine PoW with a small computer, and if they get lucky, they get awarded. If one mines with a bigger computer, they would get awarded the same amount, but are more likely to find a valid block hash.

After doing some calculations, the total amount of coins will be lowered to 250 million coins.

The release date is on Wednesday, May 28 at 22:00 (10:00pm) UTC

---

I believe I've managed to chase down the bug causing transactions to show up as "Mint by burn" when they actually weren't.

The code that did up the transaction list checked whether the transaction was in a PoB block, but forgot to check whether it was the coinbase transaction, with the result that all transactions in PoB blocks would show up as "Mint by burn". I'm not quite sure why the incorrect amount appeared in the list -- but try my patch and see if it fixes the GUI bug. (Or if you don't have a gui environment to test it on, I can do it if you compile an updated windows client. Haven't got a build environment of my own yet...)

(BTW, it looks like even the check for PoB block is kind of hacky -- the check for PoS blocks calls a wtx.IsCoinStake() function, but the check for PoB is implemented as a boolean flag passed in to the function by the caller... kind of ugly...)

---

Just a musing on the current implementation of Slimcoin.

Based on the burn decay rate of 0.001427 / day, as we approach 1.6 million coins burnt, the rewards tends to zero as the decay rate equals reward rate. The only reward would be from fees.

At such a burn decay rate, 2342 coins will decay, which would equal the PoW generation rate if the difficulty is 10 (3.5 SLM block reward * 720 / day), which is roughly 1Ghs, thus achieving kind of an equilibrium.

It provides quite an interesting dynamic circulation ceiling which tends to 1.6 million SLMs with heavy usage, yet increases gradually to 250 million with low volume.


## TODO

It would be really cool to have the following statistics too:

- a graph of the balance of the burn address (SfSLMCoinMainNetworkBurnAddr1DeTK5)
- a graph with the value of "Formatted nEffectiveBurntCoins" (from "slimcoind getburndata")
- a graph with the "effective supply": total supply - balance of burn address

Perhaps these stats would be best at the main SLM website or at slimcoin.club, but if an external service sets them up, it would be very cool. Any chances?

---

Configured the P2Pool, and added the custom Proof of Burn block headers, but got it stuck at the get_block_header polling phase. Apparently the block header for SLM isn't hashed by sha256 but by Dcrypt? Don't think this was supposed to be the case according to the SLM whitepaper, since Dcrypt for hashing block headers would be unnecessarily intensive, will look into it further and propose changes for the upcoming hard fork if necessary.

Think we need a burn calculator of sorts, to get a sense of the ROI. Will work on it, though anyone is keen or exploring the idea or have already built one?

---

> So I know I could make a spreadsheet to figure this out, but I love the convenience of the burn calc on slimcoin.club  - any chance you could add something that says how many days until you break even and how many effective coins will be left on that date?

A few assumptions made: difficulty unchanged from last PoB, no new burnt coin.

For 10000 SLMs burnt, it will show

    83.025724 SLMS GENERATED PER DAY, 14.27 SLMS DECAYED, 120 DAYS TO BREAKEVEN, 8425.162778 BURNT COINS LEFT AT BREAKEVEN

> Do you know if the rate of decay changes as a function of increased difficulty?

Decay is a fixed constant that depends on the number of PoW blocks generated since burn: 1.00000198 per PoW block, burnCoins = nCoins / pow(BURN_DECAY_RATE, depthInChain);

For the slimcoin.club calculator, I just assumed 720 blocks per day are PoW (i.e. 3 PoW and 1 PoB), and thus I hard-coded the decay per day is 1.00000198 ^ 720 = 1.001427...


---

> I've taken the NOMP pool down - I've confirmed that there's a submitblock issue, slimcoind crashes without any error log, every time a valid submitblock is submitted. 
> 
> Will be working on fixing it on the Slimcoin client side, suspect it's a block header mismatch issue: no burn fields in the block header submitted by NOMP, and it doesn't seem that Slimcoin's block parsing routine handles this. Will investigate further and report back!
> 
> NOMP cannot use getwork to receive jobs so submitblock must be fixed, but think MPOS can receive getwork jobs so that might mean suprnova's one might work.

Isn't MPOS just the front end?  I'm guessing ocminer has added dcrypt to stratum, and it may actually be working, but his server is refusing connections at the moment.  If we can get him to fix it, I will throw some big hashpower at it for a couple hours to see if we can find a a block. 

---

Quote from: almightyruler on January 03, 2015, 06:20:11 AM
I seem to have successfully burned some coins using slimcoind burncoins "" 10

I seem to have gone past the 10 confirmations, but nothing further has happened; "listburnminted" is empty, and there is nothing in the transaction list besides the send of the 10 coins.

Am I missing something? Is there a page that explains the process? I had assumed that PoB mining was automatic. Or did that RPC command really discard those coins for good?

edit: wild guess,

    {
        "Net Burnt Coins" : 10.00000000,
        "Effective Burnt Coins" : 9.99934600,
        "Immature Burnt Coins" : 0.00000000,
        "Decayed Burnt Coins" : 0.00065400
    },


Once Effective Burnt Coins reaches zero, I mint a PoB block?

10 coins is not enough for a regular PoB income. You will probably find some blocks, but don't expect this to occur very fast (in some months, perhaps). The rate of "findings" is random, so it can be today, or in one year.

100 or even better 200 SLM coins, for now, are costing only 10-20 USD cents and you have a real chance to find blocks regularly if you burn them (one in about 1-2 weeks).

Effective Burnt Coins is a mechanism to prevent that if you burn coins you will have income forever, that would disproportionally benefit early adopters. This value will decrease to 0 in the lapse of 1 year and determinates your chances to find blocks. So basically, your chances are smaller every day, but not "significantly" smaller until some months have passed.

I know, it's a pretty complex concept, but it has some interesting properties. If you want to take a read, see http://en.bitcoin.it/Proof_of_burn.

I today have changed some of my coin collection to SLM, I think it has an attractive price now. Even if a123 really has left and dumped his coins (his website is still alive and working), he has provided us a stable client without the memory problems former clients had. So his intervention was productive for SLM

---

I think you could use OP_RETURN to specify the beneficiary's public key hash I don't think anything needs to change for the transaction to happen (other than adding an interface in the clients) but the code that checks to see if a PoB block is valid would need to change to incorporate the change: invalidating PoB blocks that point to burn transactions that have a specified beneficiary but are signed by the sender and considering the ones valid that are signed by the beneficiary as valid. No need for the beneficiary's private key, you would just need their address to get the public key hash. You would also need a way for clients to know they have received burned coins. Its a bit clunky I admit.

---

“The protocol upgrade involves replacing the proof-of-stake difficulty as the hash modifier for proof-of-stake (we call it stake modifier). The new stake modifier is 64 bit and derived from about 9 days worth of blocks after the coin generating the stake. When I get some time over next week I would talk a bit more about how it works.” - Sunny King

---

http://earlz.net/view/2017/07/27/1904/the-missing-explanation-of-proof-of-stake-version

PoSv3's History

Proof of Stake has a fairly long history. I won't cover every detail, but cover broadly what was changed between each version to arrive at PoSv3 for historical purposes:

PoSv1 - This version is implemented in Peercoin. It relied heavily on the notion of "coin age", or how long a UTXO has not been spent on the blockchain. It's implementation would basically make it so that the higher the coin age, the more the difficulty is reduced. This had the bad side-effect however of encouraging people to only open their wallet every month or longer for staking. Assuming the coins were all relatively old, they would almost instantaneously produce new staking blocks. This however makes double-spend attacks extremely easy to execute. Peercoin itself is not affected by this because it is a hybrid PoW and PoS blockchain, so the PoW blocks mitigated this effect.

PoSv2 - This version removes coin age completely from consensus, as well as using a completely different stake modifier mechanism from v1. The number of changes are too numerous to list here. All of this was done to remove coin age from consensus and make it a safe consensus mechanism without requiring a PoW/PoS hybrid blockchain to mitigate various attacks.

PoSv3 - PoSv3 is really more of an incremental improvement over PoSv2. In PoSv2 the stake modifier also included the previous block time. This was removed to prevent a "short-range" attack where it was possible to iteratively mine an alternative blockchain by iterating through previous block times. PoSv2 used block and transaction times to determine the age of a UTXO; this is not the same as coin age, but rather is the "minimum confirmations required" before a UTXO can be used for staking. This was changed to a much simpler mechanism where the age of a UTXO is determined by it's depth in the blockchain. This thus doesn't incentivize inaccurate timestamps to be used on the blockchain, and is also more immune to "timewarp" attacks. PoSv3 also added support for OP_RETURN coinstake transactions which allows for a vout to contain the public key for signing the block without requiring a full pay-to-pubkey script.

References:

https://peercoin.net/assets/paper/peercoin-paper.pdf
https://blackcoin.co/blackcoin-pos-protocol-v2-whitepaper.pdf
https://www.reddcoin.com/papers/PoSV.pdf
https://blog.ethereum.org/2015/08/01/introducing-casper-friendly-ghost/
https://github.com/JohnDolittle/blackcoin-old/blob/master/src/kernel.h#L11
https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.cpp#L2032
https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.h#L279
http://earlz.net/view/2017/07/27/1820/what-is-a-utxo-and-how-does-it
https://en.bitcoin.it/wiki/Script#Obsolete_pay-to-pubkey_transaction
https://en.bitcoin.it/wiki/Script#Standard_Transaction_to_Bitcoin_address_.28pay-to-pubkey-hash.29
https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs