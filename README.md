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

## jonnylatte on Dcrypt

While I'm on the topic of RAM, it's possible to completely eliminate the dynamic memory requirements of the dcrypt function:

    #define SHA256_HEX_LEN 64

    uint256 dcrypt_progressive(const uint8_t *data, size_t data_sz)
    {
        //if(fTestNet) return sha256d(data,data_sz);   // for testnet hashes use sha256d

        SHA256_CTX  ctx;   // sha256 context
        uint256     hash_result;    

        uint32_t    index = 0;
        uint8_t     index_values[SHA256_HEX_LEN +1]; 
        uint8_t     scratch_pad[SHA256_HEX_LEN +1];  

        SHA256_Init(&ctx);  // initialize context which will progressively hash the result as data for it is generated in scratch_pad

        sha256_to_str(data, data_sz, index_values);    // initialize index_values with sha256(data) -> ascii/hex
        memset(scratch_pad, 0xff, SHA256_HEX_LEN);     // initialize scratchpad all 0xff 

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

        SHA256_Update(&ctx,  (u8int*)data,data_sz); // write the original data to the sha256 context for the resulting hash
        SHA256_Final((u8int*)hash_result, &ctx);    // finalize the hash and store the result

        return hash_result; // we are done here
    }

Instead of building up a buffer and hashing it at the end, this version progressively hashes the data generated. SHA256 contexts have an internal buffer which is of fixed size. So it seems Dcrypt was never memory hard except for the fact that you can get a minor optimization with hashing by buffering the data instead of hashing progressively and then aborting if the resulting buffer gets too big. The optimization is not that you are saving memory or preventing too much memory use, it's that you are not hashing longer buffers, saving the processing of the large data. This is probably why there isn't a spectacular increase in efficiency by limiting the number of iterations (you are only really saving one SHA256 call on the larger buffer but still wasting many small calls for calculating the scratchpad and index values)

In any case I think this version of Dcrypt algorithm might be better for the main client to prevent too much resource usage while we wait on the possibility of a change in hash functions.

I dont think its bad that Dcrypt has fixed memory usage. I hope my code is a clear implementation that could be of help to someone writing a GPU miner which I now believe should not be too difficult to implement (the problem is now variable computation time instead of variable memory usage) If we have a GPU miner then it will be much harder for a botnet or server farm to compete and if we can do it with dcrypt then changing the hashing function is less important... 

---

[quote author=Slimcoin Community link=topic=1141676.msg12038461#msg12038461 date=1438574772]
[list][center]
[img width=800]http://i.imgur.com/4x849fh.jpg[/img]

[size=14pt]
[b]Project website[/b]
[url=https://slimco.in]slimco.in[/url]
[b]Github Project Site[/b]
[url=https://github.com/slimcoin-project]github.com/slimcoin-project[/url]
[b]Discussion forum (Reddit)[/b]
[url=https://reddit.com/r/slimcoin]reddit.com/r/slimcoin[/url]
[b]Other Resources[/b]
[url=https://t.me/SlimcoinGroup]Telegram[/url] - [url=https://join.slack.com/t/slimcoinproject/invite/MjQzMDU0NTg4MTAyLTE1MDU1NTA4MjAtZmU0Nzc0MDQ0OQ?x=x-239398136118-243256178055]Slack[/url] - [url=https://discord.gg/crMHFRJ]Discord[/url] - [url=https://www.youtube.com/channel/UCUXx5ksHhkq6CQwZdlckaeQ/featured]YouTube[/url] - [url=https://www.facebook.com/slimcoin.community/]Facebook[/url]
[b]Marketplace[/b]
[url=https://bitcointalk.org/index.php?topic=2063259.new#new]Bitcointalk SLM Marketplace thread[/url]
[b]Bitcointalk threads in other languages[/b]
[url=https://bitcointalk.org/index.php?topic=2040975.0]German / Deutsch[/url] - [url=https://bitcointalk.org/index.php?topic=628902.0]Spanish / Español[/url]
[b]Original announcement[/b]
[url=https://bitcointalk.org/index.php?topic=613213.0]The Historic Slimcoin ANN thread[/url] (outdated)*
[b]Exchanges[/b]
[url=https://novaexchange.com/market/BTC_SLM/]Novaexchange[/url]
[url=https://freiexchange.com/market/SLM/BTC]FreiExchange[/url]
[b]Blockchain explorer[/b]
[url=https://chainz.cryptoid.info/slm/]CryptoID[/url]
[s][url=http://tessier.bel-epa.com:5064/]ACME[/url][/s] (experimental metadata explorer, currently being upgraded)
[b]Mining Pool[/b]
[s][url=http://slimcoin.bazco.in/]Bazcoin[/url][/s] (currently inactive)
[b]Price tracking[/b]
[url=https://coinpaprika.com/coin/slm-slimcoin/]Coinpaprika[/url]
[url=https://www.coingecko.com/en/coins/slimcoin]Coingecko[/url]
[/size]


[i][size=12pt]Slimcoin is a [b]energy-saving, fast-confirming and novel cryptocurrency[/b]. It is the first cryptocurrency using [b]Proof of Burn[/b] for block generation and currency distribution - a distribution mechanism with interesting economic properties. Proof of Burn is combined with Proof of Work and Proof of Stake to increase security. Another highlight is the DCrypt algorithm, which is one of the most difficult algorithms to implement in an ASIC and suitable for CPU and GPU mining.

Slimcoin is a P2P project - we have no "official dev", nor it is managed by any centralized entity or organization. There was no premine at the start. You can contribute with coding, testing, providing Slimcoin-based products & services, use it for trading of virtual or real-world goods or simply [i]hodling[/i] it. Be welcome!
[/size][/i]
[/center]

[center][size=14pt][b]Current developments (as of Q1 2019)[/b][/size][/center]

Some exciting experimental developments are currently going on:

- [b]Testing of Slimcoin 0.5.2 has begun[/b]. The client got several improvements like an optimized Proof of Stake algorithm and Coin Control. Check out the [url=https://github.com/slimcoin-project/Slimcoin/tree/develop]Develop branch[/url] to test it.
- [b]Web2Web - Easy Publishing tool:[/b] Create decentralized websites that are updated via the blockchain and [b]can be read without installing any software[/b]! The [url=https://github.com/slimcoin-project/slimweb-publisher]Slimweb Publisher[/url] has now a simple graphical interface and can also be used via the command line.

[b]Coming soon:[/b]

- A [b]second Web2Web node[/b] with a Webtorrent client will come online in Q1-Q2 2019. This node will provide selected Web2Web publications directly. 
- [b]A pool could come soon:[/b] [url=http://slimcoin.bazco.in/]Bazcoin[/url] - donations (BTC): 17evPVkeVJ3H4yPEMpQ6rjyCFEC7FQpb9r  (There are some implementation problems, hopefully to be resolved soon.)
- [b]ACME[/b], a new breed of blockchain explorer, works with [b]Semantic Web technologies[/b] and includes a Web2Web browser. Is already usable. (currently updating). You can find the previous version [url=https://github.com/gjhiggins/acme/tree/slimcoin]here[/url].
- [b]slimvm:[/b] a small virtual machine for Ricardian contracts (currently development is private)

[b]Downloads & Wallet variants[/b]

Current versions from the project repository:

[list]
[li][b][url=https://github.com/slimcoin-project/slimcoin-project.github.io/raw/master/slimcoin-qt.exe]Windows 0.5.1 wallet[/b][/li]
[li][b][url=https://github.com/slimcoin-project/Slimcoin/releases]0.5.0 executables for Windows & MacOS[/url][/b] ([url=https://minkiz.co/noodle/9]alternative source[/url])[/li]

[li][b][url=https://github.com/slimcoin-project/Slimcoin]Source code repository for Linux, MacOS & Windows[/url][/b] (Github, official repository. Current [i]master[/i] branch is roughly equivalent to version 0.5.1). [b]Download the master branch as ZIP file [url=https://github.com/slimcoin-project/Slimcoin/archive/master.zip]here[/url].[/b][/li][/list]

Up-to-date community contributions on other servers (Use at your own risk and scan it for security, but sources seem trustworthy.):

[list]
[li][b][url=https://drive.google.com/open?id=1IDaf4BD2SL0_RJxdp0CZNU7-4aR0EL5x]ARM binaries (Slimcoin 0.5, for ARM v5, v7 and v8)[/url][/b][/li]
[li][b][url=https://github.com/muf18/Slimcoin-aarch64/releases/tag/v0.5]Slimcoin aarch64 wallet[/url] (0.5.1)[/b][/li]
[li][b][url=https://mega.nz/#!YrxlDABb!nXCZD_9ZDc0TLgKzbKNhg3BJ-iYiD8yax_y4XBV2dhM]0.5.1 self-extracting archive[/url] with blockchain snapshot (Updated January 2018).[/b][/li]
[/list]

Experimental versions (still being tested, use at your own risk):

[list]
[li][url=https://minkiz.co/noodlings/slm/slimcoin-qt-v0.5.3-win32.zip]Slimcoin 0.5.3 for Windows[/url] (improved PoS algorithm & Coin control)[/li]
[/list]

Older versions:

[list]
[li][url=https://github.com/kryptoslab/slimcoin/releases/download/v0.3.2.1/slimcoin-qt.7z]Windows QT wallet 0.3.2.1[/url]. Last "stable" 0.3 .exe - Use this version if 0.5+ does not work for you.[/li]
[li][url=https://github.com/slimcoin-project/Slimcoin/tree/fcefbc170c96cb5ee44d69f76024684861e7c112]Last 0.3.2 Source Code[/url][/li]
[/list]

Notes: 

[list][li]Linux executables are not up to date. If you can't compile it yourself, you can ask for help in this thread.[/li]
[li]Instructions on how to install via Homebrew on MacOS and how to cross-compile a Windows version via Ubuntu are [url=https://github.com/slimcoin-project/Slimcoin/tree/master]at Github[/url] (README file on the master branch).[/li]
[li]There is a [b]nightly snapshot of the blockchain[/b] data directory, [b][url=https://minkiz.co/noodlings/slm/slm-datadir-snapshot.zip]download it here[/url][/b].[/li]
[/list]

[b]Test the bleeding-edge source code (master)[/b]

Instructions for Linux/Unix:

[list]
[li]Install git, if you don't have already (For tech-laymen: Git is a version control system. It's not absolutely necessary to build Slimcoin, but it makes updating experimental versions a lot easier).[/li]
[li]Download the source code from [url=https://github.com/slimcoin-project/Slimcoin]Github[/url].[/li]
[li]Open a terminal. Go to the folder with the code and checkout the [b]master[/b] branch:
[font=fixed]git checkout master[/font]
[/li]
[li]Compile it:
QT graphical client (slimcoin-qt):
[font=fixed]qmake
make[/font]
Command line client (slimcoind):
[font=fixed]cd src
make -f makefile.unix[/font]
[/li]
[li]Run it (preferably with a fresh wallet file). We have a testnet for experiments (-testnet option), but it should work on the main net. If not, you are welcome to report bugs.[/li]
[/list]

[center][size=14pt][b]Specifications[/b][/size][/center]

[list]
[li]Tri-Hybrid Blocks: Proof-of-Burn blocks, Proof-of-Stake blocks and Proof-of-Work blocks[/li]
[li]PoW: Dcrypt algorithm, [b]suitable for CPU mining[/b], an algorithm made to be difficult to implement on an ASIC. See [url=https://bitcointalk.org/index.php?topic=1141676.msg13573310#msg13573310]Tutorial for solo CPU mining[/url][/li]
[li]Block time is 1.5 minutes (90 seconds)[/li]
[li]Difficulty re-targets continuously[/li]
[li]Block Rewards:[list][li]Proof-of-Burn blocks: max 250 coins[/li]
[li]Proof-of-Work blocks: max 50 coins[/li]
[li]Proof-of-Stake blocks: 10% of the staking coins per year[/li][/list][/li]
[li]Proof of Work block rewards decrease in value as the difficulty increases[/li]
[li]No hard supply limit, but comparatively low inflation rate. A "250 million coins maximum" is coded into the client, but probability is extremely low this amount will be reached in the next 100 years.[/li]
[li]Proof of Stake minimum coin age: ~1 week[/li]
[/list]

[center][size=14pt][b]What is Proof of Burn?[/b][/size][/center]

Proof of burn is an energy-saving decentralized consensus mechanism that rewards long term involvement. It was [url=https://bitcointalk.org/index.php?topic=131139.0]proposed by Iain Stewart[/url] as early as 2012. Slimcoin is [b]the first cryptocurrency to implement it as a block generation method[/b] and was created in May 2014 by an pseudonymous developer (he used the name [i]slimcoin[/i] in this forum, [i]John Smith[/i] on Github and [i]P4Titan[/i] on Reddit).

[b][size=12pt]How does Proof of Burn work?[/size][/b]

Let's quote the original Slimcoin developer:

[quote author=slimcoin link=topic=613213.msg6788942#msg6788942 date=1400369609]
Proof-of-Burn mining is different from Proof-of-Work mining. More computers and higher computational power offers [b]no[/b] advantage over slower computers.

In short, how this is achieved is: when one burns coins, that transaction can be used to calculate burn hashes. There is also a multiplier that is multiplied to the raw burn hash to calculate the final burn hash. The greater amount of coins burnt by a user, the smaller the multiplier will be and the smaller the burn hashes will be. The smaller the burn hash is, the more likely the hash will meet the difficulty target (be accepted by the network as valid). Over time, the multiplier of a single burn transaction increases slowly, lowering the effectiveness of those burn hashes, acting like "decaying burnt coins". Per transaction, only 1 burn hashes is needed to be calculated per ~90 seconds. The reason low power can mine this is because basically almost any machine can hash a few SHA256 hashes in ~90 seconds. [/quote]

In short: The more coins you destroy by burning, the higher the chances you find Proof-of-Burn blocks. It's not necessary to burn all the coins together: your "score" (probability to find a block) will also rise when you burn coins periodically.

[b][url=http://slimco.in/proof-of-burn-eli5/]Read our Proof of Burn ELI5 here![/url][/b]

Proof of burn is explained in detail by its original author in [url=http://en.bitcoin.it/wiki/Proof_of_burn]the Proof of Burn Wiki article.[/url]

[b][size=12pt]How to burn coins?[/size][/b]

Short explanation to start:

[b]QT client:[/b] There is a tab in the main window with the label "Burn". Simply click on it and you will be able to burn coins.

[b]Command line:[/b] Enter the following command:

[/list][code]slimcoind burncoins <account name> <amount>[/code]

With the following command you see the PoB statistics of the wallet:

[code]slimcoind getburndata[/code]

For more details: [b][url=http://slimco.in/proof-of-burn-guide/]Read the Proof of Burn minting guide![/url][/b]

[size=12pt][b]How many coins are burnt by the Slimcoin users?[/b][/size]

Check the balance of the burn address:

[b][url=https://bchain.info/SLM/addr/SfSLMCoinMainNetworkBurnAddr1DeTK5]SfSLMCoinMainNetworkBurnAddr1DeTK5[/url][/b]

This amount is different from nEffectiveBurnCoins because it returns all coins that were burnt in some moment - also those who already have lost totally their "power".

The number is useful to calculate the real available money supply: You deduct the balance of this address from the total money supply ("moneysupply" parameter when you type "getinfo" in the debug window or with the CLI client).

[center][size=14pt][b]Developers: Bounties & Info[/b][/size][/center]

After the original developer's departure, other developers have made fixes to the code and thanks to them the client is quite stable now. However, we welcome all contributions, as there still are bugs and the code should keep up to date with Bitcoin and Peercoin development.

Slimcoin is unique because it is the first cryptocurrency to use the Proof of Burn consensus method. Because of this [b]first-mover-advantage[/b], Slimcoin has the potential to become one of the leading cryptocurrencies in the future, like it has ocurred with Peercoin (first PoS coin) before. Proof of burn has a [b]very interesting economic model[/b] (see below), which - in a mature market - can lead to a more stable price.

As Slimcoin is based on the Bitcoin code, even [b]crypto 2.0[/b] features can be added relatively easily porting Coloured coins implementations like PeerAssets, Counterparty, OpenAssets or Omni.

To speed development up, a [b]variety of bounties[/b] are offered. There was no premine or ICO and there is no centralized corporation, so these bounties come from donations from individual users.

If you are interested in participating, you can [b]reply to this thread[/b] or start a thread on [b][url=https://www.reddit.com/r/slimcoin/]Reddit[/url][/b], where a more structured discussion is possible. Also Slack is an option where some community members (but not all) are active.

[b]Current bounties:[/b] 

[list]
[li][b]20.000 SLM bounty[/b] for a working pool, offered by gavrilo77 [url=https://bitcointalk.org/index.php?topic=1141676.msg24645842#msg24645842]here[/url]. There may be 20.000 additional SLM which were[url=https://bitcointalk.org/index.php?topic=1141676.msg13114595#msg13114595]announced by ArchitektoR in this thread in 2016, but please confirm first.[/url][/li]
[li][b]5.000 SLM[/b] for watch addresses (announced by aIA in this thread in September 2017).[/li]
[li][b]10.000 SLM[/b] for bugfixes, announced by gavrilo77. Please consult with him.[/li]
[/list]

[b]Older bounties[/b] (please confirm first if they're still offered!):

[list]
[li][b]20.000 SLM bounty[/b] for an "working electrum-like wallet": [url=https://www.reddit.com/r/slimcoin/comments/3fkmpo/development_bounty_thread/ctxvr8n]announced on Reddit by 3429z781678124[/url] (offered in 2016)[/li]
[li][b]20.000 SLM bounty[/b] for an update of the code to the current Bitcoin codebase,  [url=https://bitcointalk.org/index.php?topic=1141676.msg13114595#msg13114595]announced by ArchitektoR in this thread (in 2016).[/url][/li]
[/list]

[b]If you want to apply for a bounty, please reply to this thread.[/b] If you want to stay anonymous, [b][i]before you start development[/i][/b] you can send a PM to the user [url=https://bitcointalk.org/index.php?action=pm;sa=send;u=538351]"d5000"[/url] for more informations. Note that he is [b]not[/b] in control of the bounty amounts, the money for now remains in possession of the donators, but he will contact them and give you contact information.

If you want to donate Slimcoins for development bounties, you can reply to this thread or alternatively open a thread on Reddit (the old bounty thread there was archived).

[b]Resources for developers:[/b]

The official source code repository is [b][url=https://github.com/slimcoin-project/Slimcoin]slimcoin-project/Slimcoin[/url][/b] at Github.

[center][size=14pt][b]The magic of Proof of Burn[/b][/size][/center]

Proof of burn has [b]multiple advantages over both Proof of Work and Proof of Stake[/b] and some very interesting economic implications.

[b]Advantages over Proof of Work:[/b]

[list]
[li]Very low energy consumption[/li]
[li]No need to invest in powerful hardware[/li]
[li]No artificial price swings because of the "mining hardware" investment cycle or the influence of multipools[/li]
[/list]

[b]Advantages over Proof of Stake:[/b]

[list]
[li]No simple "rich get richer" mechanism. Proof of burn rewards entrepreneurial risk, not wealth.[/li]
[li]While "staking" coins can be stolen hacking the private key, burnt coins cannot be "stolen". The most an attacker can get when he hacks a wallet are the PoB rewards, but incentive for this kind of attack is low as the attacker would have to wait a long time until he gets a significant amount of coins - and the legitimate owner of the key can also transfer these coins to another address.[/li]
[/list]

[b]The economic model[/b]

Proof of Burn has at least three very interesting economic implications that can lead to a more stable price with less pump-and-dump bubbles.

[list]
[li][b]It rewards long-term investments:[/b] When you burn coins, you basically trade a short-term loss to a mid/long-term advantage. You will very probably get back your investment via the PoB reward mechanism (and probably even more), but you won't get it back immediately but after a certain amount of time. That means that the [i]risk[/i] to make a long-term investment is rewarded by the PoB mechanism. Short term Pump and dump scenarios are obviously possible, but long-term involvement is explicitly made more profitable than in other cryptocurrencies. A high [i]long term investor/short term investor[/i] rate should stabilize the price.[/li]
[li][b]Burnt coins are locked and can't be sold:[/b] After you burn coins, you cannot spend them anymore. So you can't sell them in a panic event and very probably there will be less sellers in a price crash. For this reason the participants in the Proof-of-burn mechanism can be seen as "backers" of the coin price, as the burnt coins are not part of the available supply.[/li]
[li][b]It is easier to burn coins when they are cheap.[/b] When price is low but fundamentals are sound, then the burn rate should be higher than in the times when coins are expensive, simply because it's cheaper to burn coins. Every coin burnt is rested from total supply. So supply tends to be smaller, the lower the price is - and that is a excellent condition to make the price rise again. That means that PoB probably has an inherent [i]supply-follows-demand[/i] mechanism - something very difficult to implement as a separate "supply-regulating" algorithm.[/li]
[li][b]Proof of burn can work like an integrated "futures" market:[/b] a high burn rate is a sign for optimism.[/li]
[/list]

[size=12pt][b]Bug reports:[/b][/size]

[b]Best practice is to use directly the [url=https://github.com/slimcoin-project/Slimcoin]Github repository[/url] to report bugs.[/b]

[b]How to report a bug on Github:[/b]

[list]
[li]Go to https://github.com/slimcoin-project/Slimcoin. If you haven't registered yet on Github, you should register (it's free).[/li]
[li]In the right column, you will see a menu [b]<> Code[/b] with a item called "[b]Issues[/b]". Click it.[/li]
[li]In the new screen, look at the already posted issues. If your bug isn't reported, click [b]New Issue[/b]. Try to give the most information possible.[/li]
[/list]

If you don't want to use Github, you can use the [url=https://www.reddit.com/r/slimcoin/]Slimcoin subreddit[/url] or reply here in the Bitcointalk thread.

[size=12pt][b]Nodes:[/b][/size]

Add these lines to your slimcoin.conf (in your Application Data/Slimcoin or .slimcoin folder) to connect faster to the network:

[url=https://github.com/slimcoin-project/Slimcoin/wiki/Slimcoin-addnodes]Slimcoin addnodes[/url] (as of 2018-08)

[center][size=14pt][b]More resources[/b][/size][/center]

[size=12pt][b]Technical Whitepaper:[/b][/size]
[size=10pt]An extensive explanation of what Slimcoin has new to offer: [url=https://github.com/slimcoin-project/slimcoin-project.github.io/raw/master/whitepaperSLM.pdf]Slimcoin Whitepaper PDF[/url][/size]

[size=12pt][b]Slimminer Downloads[/b][/size]
[size=10pt][url=https://github.com/kryptoslab/slimminer]CPU miner Source Code[/url] (Kryptoslab's version)[/size]
[size=10pt][url=https://github.com/JonnyLatte/slimminer]CPU miner Source Code[/url] (Johnnylatte's version, experimental!, see [url=https://bitcointalk.or/index.php?topic=1141676.msg12431644#msg12431644]here[/url])
[size=10pt][url=https://github.com/JonnyLatte/slimminerGPU]GPU miner Source Code[/url] (experimental)
[size=10pt][url=http://www37.zippyshare.com/v/42180814/file.html]Windows executable[/url] (original version, outdated)[/size]
[size=10pt][url=https://github.com/slimcoin/slimminer]Source Code[/url] (original version, outdated)[/size]

Tutorial for Linux source code compilation: [url=https://bitcointalk.org/index.php?topic=613213.msg7192912#msg7192912]https://bitcointalk.org/index.php?topic=613213.msg7192912#msg7192912[/url]
Tutorial for Windows source code compilation: [url=https://bitcointalk.org/index.php?topic=613213.msg7090075#msg7090075]https://bitcointalk.org/index.php?topic=613213.msg7090075#msg7090075[/url]

[size=12pt][b]IRC Channel[/b][/size]
[size=10pt]#OfficialSlimcoin on freenode[/size]

[b]Note:[/b] The original Slimcoin developer has not posted in this forum since the end of 2014, and he has stated that for the moment he is too busy to continue maintaining the coin. Some of the links in the release announcement are outdated and the former "official" website is offline now. So some members of the community have decided to restart the thread and build a new ecosystem.
[/quote]

---

## What is coin control?
When you send bitcoins to someone else, the bitcoin client chooses kinda randomly which of your addresses will send the coins. With coin control you can exactly choose, which of your addresses will be the sending addresses. And even more specific which of your unspent outputs will be the sending inputs.
  
## What is an unspent output?
Lets say your bitcoin address is 111. Now someone sends 1 BTC to this address. Now address 111 has one unspent output. Now someone else sends 2 BTC to this address. Now address 111 has two unspent outputs and a balance of 3 BTC in total. Now lets say you want to send someone 0.1 BTC. One could say "ok, simply subtract 0.1 BTC from the first unspent output and send it". But this is not possible, because the bitcoin protocol works different. You always have to spent the whole output. This means in this case the bitcoin client would take the first unspent output, send 0.1 BTC to the other person and 0.9 BTC back to yourself. For this the client creates a new change address in the background and adds this address to your wallet. This address would then have one unspent output of 0.9 BTC. This means every wallet has kinda "hidden" change addresses. For example if you now would loose your wallet, but still know the private key of 111, you would be only able to get the 2 BTC back. The 0.9 BTC from the change address are gone, because you would have to know the private key of the change address.
  
## Why coin control?
The first version of coin control was called "Patching The Bitcoin Client To Make It More Anonymous". Sometimes when you receive bitcoins, the sender or even the public knows that it is you who owns the bitcoin address. Now if you later want to make a real anonymous payment, you shouldnt use those coins. Also you get full control over your unspent outputs and can do things like choosing which address actually owns the coins, clean them up or send all change back to the origin address etc. You can also prevent the bitcoin client from creating a change address by exactly sending the amount of selected unspent outputs minus the fee. Also you get a better impression of whats going on in your wallet and get a better understanding of the bitcoin protocol.

## Changes to the GUI

Screenshots
[screenshot1.png](https://a.fsdn.com/con/app/proj/bitcoincoincont/screenshots/screen_shot1.png)
[screenshot2.png](https://a.fsdn.com/con/app/proj/bitcoincoincont/screenshots/screen_shot2.png)
[screenshot3.png](https://a.fsdn.com/con/app/proj/bitcoincoincont/screenshots/screen_shot3.png)
[screenshot4.png](https://a.fsdn.com/con/app/proj/bitcoincoincont/screenshots/screen_shot4.png)
[screenshot5.png](https://a.fsdn.com/con/app/proj/bitcoincoincont/screenshots/screen_shot5.png)

## Main
Settings checkbox "Display coin control features (experts only!)" (default=no)

## Tab Send coins
Button Inputs
click on this button opens actual coin control dialog. If no Inputs are selected "automatically selected" is shown.
Change Checkbox
checked -> provide custom change address

## Coin Control Dialog

Shows a list of all unspent outputs with two view modes
tree mode: outputs including change are grouped by wallet address
tree can be opened showing the actual outputs for this wallet address including change
if change, the change bitcoin address is shown in column "address", otherwise the column "address" is empty, because its a direct output of the wallet address having the bitcoin address already shown in the parent node (same with label)
list mode: simple list of all unspent outputs
select outputs by checkbox -> only the checked outputs are used when sending a transaction
if none are selected -> coin control inactive (just as normal)
check/uncheck all by clicking on "(Un)select all"
sort colums
tooltip available in column list mode in column label for the change (shows from which address the change came from)
Context menu
Copy to clipboard (amount,label,address,transaction id,lock,unlock)
Labels at the top
Quantity: number of selected outputs
Amount: sum of selected unspent outputs
Fee:   see "Calculation of fee and transaction size" below
minus fee: simply the amount shown is "Selected" minus the amount shown in "Fee"
Bytes: see "Calculation of fee and transaction size" below
Priority: priority = coinage / transactionsize. coinage = value * confirmations.  miners order transactions by priority when selecting which go into a block
Low Output: "yes" if any recipient receives an amount < 0.01BTC
Change: shows the change you get back
direct right click the labels for copy amount to clipboard
   


## Selection

In this version of coin control, all selected outputs are going into the transaction for sure!!
Of course, if you select more than you actually send, the bitcoin core will send the rest back to you as change, just as normal.
And of course, if you select less than you send you will get "The amount exceeds your balance".
And as already mentioned, if none are selected, coin control is inactive, this means everything is just the same as without coin control.
  
## Fee

If the sum of selected outputs minus the amount you are going to send is smaller than the required fee, you will probably get
"The total exceeds your balance when the transaction fee is included"
This is because you didnt select enough outputs to pay the fee.
You always must select enough outputs, so that those outputs can pay the fee.
  
## Calculation of fee and transaction size

The fee is calculated according to the fee set in the Settings menu.
The calculation assumes 2 outputs in total. One for the destination address and one for the change.
The formula is nBytesOutputs + (2 * 34) + 10. nBytesOutputs is the sum of selected outputs, 148 or 180 bytes per output, depending if compressed public key.
Due to the inner workings of bitcoin the size per output is actually +/- 1 byte. Meaning the shown calculation is not always 100% correct. 
If you send exactly "selected minus fee" then you will not have change (1 output only). The transaction will then be 34 bytes smaller as what was calculated before.

## Free Transactions

In order to be able to send a free transaction, you need to follow the rules:
     - transaction size must be < 10000 bytes
     - priority must be at least "medium"
     - any recipient must receive at least 0.01BTC
     - change must be either zero or at least 0.01BTC

If you violate one rule you will see a min-fee and also the labels turn red: Bytes.Priority,Low Output,Change. Depending which rule you violated.  Those 4 labels also have tool tips explaining this.  Also remember that violating one of the first 2 rules means 0.0005 PER kilobyte min-fee, while violating one of the last 2 means 0.0005 min-fee only.

