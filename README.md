## SlimCoin Official Development Repo

### What is SlimCoin?
[Slimcoin](https://slimcoin-project.github.io/whitepaperSLM.pdf) (abbreviated SLM), is the first known
[cryptocurrency](https://en.wikipedia.org/wiki/Cryptocurrency) designed as
a combined [proof-of-burn](http://en.bitcoin.it/wiki/Proof_of_burn)/[proof-of-stake](https://en.wikipedia.org/wiki/Proof-of-stake)/[proof-of-work](https://en.wikipedia.org/wiki/Proof-of-work_system)
system. It can be mined using a special “Dcrypt” algorithm. 

Slimcoin is a energy-saving, fast-confirming and novel cryptocurrency. It is the first cryptocurrency to using *Proof of Burn* for block generation and currency distribution - a distribution mechanism with interesting economic properties. Proof of Burn is combined with Proof of Work and Proof of Stake to increase security. Another highlight is the DCrypt algorithm, which is one of the most difficult algorithms to implement in an ASIC and suitable for CPU and GPU mining.

SlimCoin is unique for using the [Proof of Burn](http://en.bitcoin.it/wiki/Proof_of_burn) algorithm. This algorithm rewards the “destruction” of coins by sending them to an unspendable address. It can be described as a long-term investment rewarding variant of Proof of Stake and has interesting economical implications - see the resources below for details.

The Dcrypt algorithm is extremely anti-ASIC and currently can only be mined with a CPU. It is made so mining with hardware will be inefficient in the future, given the intensity of this algorithm, thus forcing the users to mine through the proof-of-stake and proof-of-burn. Thus, with SlimCoin energy consumption would decrease over time as proof-of-work blocks become less rewarding and coin generation becomes increasingly based on proof-of-stake block generation.

Slimcoin's codebase is built upon [Bitcoin](http://bitcoin.org/en/) and [Peercoin](https://peercoin.net/).

Slimcoin is a P2P project - there is no “official dev”, nor it is managed by any centralized entity or organization. There was no premine at the start. You can contribute with coding, testing, providing Slimcoin-based products & services, use it for trading of virtual or real-world goods or simply “hodling” it.

### Emissions

- Block time is 1.5 minutes (90 seconds)
- Difficulty re-targets continuously
- Block Rewards:
  - Proof-of-Burn blocks: max 250 coins
  - Proof-of-Work blocks: max 50 coins
- Block rewards decrease in value as the difficulty increases
- No hard supply limit, but comparatively low inflation rate. A "250 million coins maximum" is coded into the client, but probability is extremely low this amount will be reached in the next 100 years.
- Proof of Stake minimum coin age: \~1 week

### Parameter bindings

    pszTimestamp: "RT: 2 southeast Ukranian regions to hold referendum May 11 as planned";

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

- Developers work in their own forks, then submit pull requests when they think their feature or bug fix is ready.
- If it is a simple/trivial/non-controversial change, then one of the development team members simply pulls it.
- If it is a more complicated or potentially controversial change, then the change may be discussed in the pull request, or the requester may be asked to start a discussion [Discord](https://discord.gg/ffeDjmV) for a broader community discussion. 
- The patch will be accepted if there is broad consensus that it is a good thing. Developers should expect to rework and resubmit patches if they don't match the project's coding conventions (see coding.txt) or are controversial.
- From time to time a pull request will become outdated. If this occurs, and the pull is no longer automatically mergeable; a comment on the pull will be used to issue a warning of closure.  Pull requests closed in this manner will have their corresponding issue labeled 'stagnant'.

## SlimCoin Resources

* Project website 
  - [https://slimcoin-project.github.io](https://slimcoin-project.github.io)

* Github Project Site 
  - [github.com/slimcoin-project](https://github.com/slimcoin-project)

* Client and Source:
  - [Client Binaries](https://github.com/slimcoin-project/Slimcoin/releases) (see the Assets section of desired version)
  - [Source Code](https://github.com/slimcoin-project/Slimcoin)

* Documentation: <add documentation link to slim website>
  - [SlimCoin Whitepaper](https://slimcoin-project.github.io/whitepaperSLM.pdf), 
  - [SlimCoin Wiki](https://github.com/slimcoin-project/Slimcoin/wiki)

* Help:
  - [Discord](https://discord.gg/ffeDjmV)
  - [Bitcointalk official thread](https://bitcointalk.org/index.php?topic=1141676) 
  - [Slimcoin Reddit](https://www.reddit.com/r/slimcoin/)

[//]: # (- Old Bitcointalk announcement https://bitcointalk.org/index.php?topic=613213)

* Exchanges
    - [FreiExchange](https://freiexchange.com/market/SLM/BTC)
    
    [//]: # (- Discussion forum Reddit** reddit.com/r/slimcoin https://reddit.com/r/slimcoin)
    
* Blockchain explorer
    - [https://slimcoin-explorer.dynv6.net/](https://slimcoin-explorer.dynv6.net/)
    - [alternative link to the same block explorer above](https://slimcoinexplorer.duckdns.org/)
    
    [//]: # (- ACME http://tessier.bel-epa.com:5064/ experimental metadata explorer, currently being upgraded)

* Price tracking
    - [Coinpaprika](https://coinpaprika.com/coin/slm-slimcoin/)
    - [Coingecko](https://www.coingecko.com/en/coins/slimcoin)

* Bitcointalk threads in other languages
    - [German / Deutsch](https://bitcointalk.org/index.php?topic=2040975.0)
    - [Spanish / Español](https://bitcointalk.org/index.php?topic=628902.0)

* Other Resources
    - [YouTube](https://www.youtube.com/channel/UCUXx5ksHhkq6CQwZdlckaeQ/videos)
      
      [//]: # (- Telegram https://t.me/SlimcoinGroup)
      [//]: # (- Facebook https://www.facebook.com/slimcoin.community/)

* Other Sites and Links
    - [Travis CI continuous integration](https://api.travis-ci.org/gjhiggins/Slimcoin) [![Build Status](https://api.travis-ci.org/gjhiggins/Slimcoin.png?branch=master)](https://travis-ci.org/gjhiggins/Slimcoin),


* Marketplace 
  - [Bitcointalk SLM Marketplace thread](https://bitcointalk.org/index.php?topic=2063259.new#new)

* Original announcement 
  - [The Historic Slimcoin ANN thread](https://bitcointalk.org/index.php?topic=613213.0) (outdated)*

[//]: # (* Mining Pool Bazcoin http://slimcoin.bazco.in/ currently inactive)

#### Coinmarketcap.com history 
  - [https://coinmarketcap.com/currencies/slimcoin/](https://coinmarketcap.com/currencies/slimcoin/)

#### Donations:

    Slimcoin address SNDbyMt3UPDTyLsSUumqwniJcf2pCtGosP
    Hash    0a23366013d0d1ddd8de1156af972b5f2aeee400
    Number of confirmed transactions    34
    First transaction   2014-07-29 02:44:43
    Last transaction    2014-09-21 00:52:21
    Total Received  4,709.9 SLM
    Balance 4755.12 SLM

#### Bounty:

    Slimcoin address ShooqRfkshDajLTvYMEKRKxXW7ooZLBZsm
    Hash    e105a5dc4b200fee9e6693746b44803e7be9a949
    Number of confirmed transactions    5
    First transaction   2014-05-29 00:56:28
    Last transaction    2014-05-30 03:29:55
    Total Received  72.66 SLM
    Balance 0 SLM

## What is Proof of Burn?

Proof of burn is an energy-saving decentralized consensus mechanism that rewards long term involvement. It was [proposed in 2012 by Iain Stewart](https://bitcointalk.org/index.php?topic=131139.0) in 2012. Slimcoin is the first cryptocurrency to implement it as a block generation method and was created in May 2014 by an pseudonymous developer *(using the name “slimcoin” on Bitcointalk, “John Smith” on Github and “P4Titan” on Reddit)*.

Proof of Burn has multiple advantages over both Proof of Work and Proof of Stake and some very interesting economic implications.

### Advantages over Proof of Work:

  - Very low energy consumption
  - No need to invest in powerful hardware
  - No artificial price swings because of the "mining hardware" investment cycle or the influence of multipools


### Advantages over Proof of Stake:

  - No simple "rich get richer" mechanism. Proof of burn rewards entrepreneurial risk, not wealth.
  - While "staking" coins can be stolen hacking the private key, burnt coins cannot be "stolen". The most an attacker can get when he hacks a wallet are the PoB rewards, but incentive for this kind of attack is low as the attacker would have to wait a long time until he gets a significant amount of coins - and the legitimate owner of the key can also transfer these coins to another address.

### The economic model

Proof of Burn has at least three very interesting economic implications that can lead to a more stable price with less pump-and-dump bubbles.

  - *It rewards long-term investments:* When you burn coins, you basically trade a short-term loss to a mid/long-term advantage. You will very probably get back your investment via the PoB reward mechanism (and probably even more), but you won't get it back immediately but after a certain amount of time. That means that the *risk* to make a long-term investment is rewarded by the PoB mechanism. Short term Pump and dump scenarios are obviously possible, but long-term involvement is explicitly made more profitable than in other cryptocurrencies. A high *long term investor/short term investor* rate should stabilize the price.
  - *Burnt coins are locked and can't be sold:* After you burn coins, you cannot spend them anymore. So you can't sell them in a panic event and very probably there will be less sellers in a price crash. For this reason the participants in the Proof-of-burn mechanism can be seen as "backers" of the coin price, as the burnt coins are not part of the available supply.
  - *It is easier to burn coins when they are cheap.* When price is low but fundamentals are sound, then the burn rate should be higher than in the times when coins are expensive, simply because it's cheaper to burn coins. Every coin burnt is rested from total supply. So supply tends to be smaller, the lower the price is - and that is a excellent condition to make the price rise again. That means that PoB probably has an inherent *supply-follows-demand* mechanism - something very difficult to implement as a separate "supply-regulating" algorithm.
  - *Proof of burn can work like an integrated "futures" market:* a high burn rate is a sign for optimism.

### How does Proof of Burn work?

Quoting the original Slimcoin developer:

> “Proof-of-Burn mining is different from Proof-of-Work mining. More computers and higher computational power offers **no** advantage over slower computers.” [link](https://bitcointalk.org/index.php?topic=613213.msg6788942#msg6788942)

In summary: when one burns coins, that transaction can be used to calculate burn hashes. There is also a multiplier that is multiplied to the raw burn hash to calculate the final burn hash. The greater amount of coins burnt by a user, the smaller the multiplier will be and the smaller the burn hashes will be. The smaller the burn hash is, the more likely the hash will meet the difficulty target (be accepted by the network as valid). Over time, the multiplier of a single burn transaction increases slowly, lowering the effectiveness of those burn hashes, acting like "decaying burnt coins". Per transaction, only 1 burn hashes is needed to be calculated per ~90 seconds. The reason low power can mine this is because basically almost any machine can hash a few SHA256 hashes in ~90 seconds.

In short: The more coins you destroy by burning, the higher the chances you find Proof-of-Burn blocks. It's not necessary to burn all the coins together: your "score" (probability to find a block) will also rise when you burn coins periodically. See the [Proof of Burn ELI5](https://slimcoin-project.github.io/proof-of-burn-eli5/)

Proof of burn is explained in detail by its original author in [the Proof of Burn Wiki article.](http://en.bitcoin.it/wiki/Proof_of_burn)

### How to burn coins?

In the QT client there is a tab in the main window with the label "Burn". Simply click on it and you will be able to burn coins.

Or, on the JSON-RPC command line, enter the following command:

    slimcoind burncoins <account name> <amount>

View the PoB statistics of the wallet:

    slimcoind getburndata

For further details see the [Proof of Burn minting guide!](https://slimcoin-project.github.io/proof-of-burn-guide/)

#### How many coins have been burned by Slimcoin users?

Using [the block explorer](https://slimcoin-explorer.dynv6.net/) check the balance of the burn address SfSLMCoinMainNetworkBurnAddr1DeTK5.

This amount differs from `nEffectiveBurnCoins` because it returns all coins that had been burnt at that time.

The number is useful to calculate the real available money supply: Deduct the balance of this address from the total money supply.

---

#### P4Titan, original author

**September 07, 2014, 09:58:18 PM**

> Lately, something 'bigger' has come along and have decided to lower my activity on Slimcoin. I will do compilations and releases, maintain the master seed node for synchronized checkpointing, and review pull requests on github. Apart from that, I only plan on doing small tweaks here and there if necessary. I am not abandoning Slimcoin, I am merely giving the majority of the development up to the community. That does not mean I am gone, I may return to more development here depending on how the future goes.[link](https://bitcointalk.org/index.php?topic=613213.msg8719988#msg8719988)

**September 10, 2014, 09:02:13 PM**

> I am maintaining this project, just development will be lower and more community based.

**October 23, 2014, 05:35:20 PM** 

> As I have posted before, the reason I "left" this project is some other stuff came up. I say "left" as I still follow its development on the forum. I did state I could do basic maintenance, of which was to hold the central sync node up. I have absolutely no problems with you forking this project, in fact, it would be of great delight to me if you did so. I can offer you assistance in navigating the PoB code and making you a reddit administrator for /r/Slimcoin. As for development, I have nothing much planned in the short termed future.
> 
> -P4Titan [link](https://bitcointalk.org/index.php?topic=613213.msg9306854#msg9306854)
