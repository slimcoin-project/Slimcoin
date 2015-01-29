SLIMCoin 0.4.0 BETA

Copyright (c) 2011-2014 Peercoin (PPCoin) Developers
Copyright (c) 2013-2014 SLIMCoin Developers
Distributed under the MIT/X11 software license, see the accompanying
file license.txt or http://www.opensource.org/licenses/mit-license.php.
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/).  This product includes
cryptographic software written by Eric Young (eay@cryptsoft.com).


Intro
-----
SLIMCoin is a free open source project derived from PPCoin and Bitcoin, with
the goal of providing a long-term energy-efficient crypto-currency.
Built on the foundation of Bitcoin, innovations such as proof-of-burn
help further advance the field of crypto-currency.


Setup
-----
After completing windows setup then run slimcoin-qt.
Alternatively you can run windows command line (cmd) in slimcoin program dir.
  cd daemon
  slimcoind
You would need to create a configuration file slimcoin.conf in the default
wallet directory. Grant access to slimcoind/slimcoin-qt in anti-virus and firewall
applications if necessary.

The software automatically finds other nodes to connect to.  You can
enable Universal Plug and Play (UPnP) with your router/firewall
or forward port 9901 (TCP) to your computer so you can receive
incoming connections.  SLIMCoin works without incoming connections,
but allowing incoming connections helps the SLIMCoin network.


Upgrade
-------
All your existing coins/transactions should be intact with the upgrade.
To upgrade from 0.3, first backup wallet
slimcoind backupwallet <destination_backup_file>
Then shutdown slimcoind by
slimcoind stop
Uninstall v0.3 client, download and install v0.4 client.
If upgrade is performed after upgrade deadline, blockchain re-download is
required.


See the documentation/wiki at the slimcoin website:
  http://www.slimcoin.org/
for help and more information.


------------------
Bitcoin 0.6.3 BETA

Copyright (c) 2009-2012 Bitcoin Developers
Distributed under the MIT/X11 software license, see the accompanying
file license.txt or http://www.opensource.org/licenses/mit-license.php.
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/).  This product includes
cryptographic software written by Eric Young (eay@cryptsoft.com).


Intro
-----
Bitcoin is a free open source peer-to-peer electronic cash system that is
completely decentralized, without the need for a central server or trusted
parties.  Users hold the crypto keys to their own money and transact directly
with each other, with the help of a P2P network to check for double-spending.


See the bitcoin wiki at:
  https://en.bitcoin.it/wiki/Main_Page
for more help and information.
