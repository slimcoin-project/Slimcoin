## All command-line options

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
-printcreation          Enables debug log of fee creation and destruction - fees are not collected by miners as in bitcoin but are destroyed to compensate the entire network.  (default: 0)
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
