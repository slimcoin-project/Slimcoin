SlimCoin Official Development Repo
==================================

### What is SlimCoin?
[Slimcoin](http://www.slimcoin.club/whitepaper.pdf) (abbreviated SLM), is the first known
[cryptocurrency](https://en.wikipedia.org/wiki/Cryptocurrency) designed as
a combined [proof-of-burn](http://en.bitcoin.it/wiki/Proof_of_burn)/[proof-of-stake](https://en.wikipedia.org/wiki/Proof-of-stake)/[proof-of-work](https://en.wikipedia.org/wiki/Proof-of-work_system)
system. It can be mined using a special “Dcrypt” algorithm. 

SlimCoin is unique for using the [Proof of Burn](http://en.bitcoin.it/wiki/Proof_of_burn) algorithm. This algorithm rewards the “destruction” of coins by sending them to an unspendable address. It can be described as a long-term investment rewarding variant of Proof of Stake and has interesting economical implications - see the resources below for details.

The Dcrypt algorithm is extremely
anti-ASIC and currently can only be mined with a CPU. It is made so mining
with hardware will be inefficient in the future, given the intensity of this algorithm,
thus forcing the users to mine through the proof-of-stake and proof-of-burn. Thus, with
SlimCoin energy consumption would decrease over time as proof-of-work
blocks become less rewarding and coin generation becomes increasingly
based on proof-of-stake block generation.

Slimcoin's codebase is
build upon [Bitcoin](http://bitcoin.org/en/) and [Peercoin](http://peercoin.net/).

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

Repository Guidelines
=====================

* Developers work in their own forks, then submit pull requests when they think their feature or bug fix is ready.
* If it is a simple/trivial/non-controversial change, then one of the development team members simply pulls it.
* If it is a more complicated or potentially controversial change, then the change may be discussed in the pull request, or the requester may be asked to start a discussion [SlimCoin Talk](<forum link>) for a broader community discussion. 
* The patch will be accepted if there is broad consensus that it is a good thing. Developers should expect to rework and resubmit patches if they don't match the project's coding conventions (see coding.txt) or are controversial.
* From time to time a pull request will become outdated. If this occurs, and the pull is no longer automatically mergeable; a comment on the pull will be used to issue a warning of closure.  Pull requests closed in this manner will have their corresponding issue labeled 'stagnant'.

