[The missing explanation of Proof of Stake Version 3](http://earlz.net/view/2017/07/27/1904/the-missing-explanation-of-proof-of-stake-version)

In every cryptocurrency there must be some consensus mechanism which keeps the entire distributed network in sync. When Bitcoin first came out, it introduced the Proof of Work (PoW) system. PoW is done by cryptographically hashing a piece of data (the block header) over and over. Because of how one-way hashing works. One tiny change in the data can cause an extremely different hash to come of it. Participants in the network determine if the PoW is valid complete by judging if the final hash meets a certain condition, called difficulty. The difficulty is an ever changing "target" which the hash must meet or exceed. Whenever the network is creating more blocks than scheduled, this target is changed automatically by the network so that the target becomes more and more difficult to meet. And thus, requires more and more computing power to find a hash that matches the target within the target time of 10 minutes.

### Definitions

Some basic definitions might be unfamiliar to some people not familiar with the blockchain code, these are:

- UTXO - Unspent Transaction Output
- vin - In a transaction a 'vin' is a UTXO that is being spent as an "input"
- vout - In a transaction, a 'vout' is the new UTXO that is being created as an "output". The 'vouts' is effectively all of the coins sent after the transaction is complete
- hashing - The process of creating a hash. This takes an arbritrary amount of data as input, and outputs a fixed size "digest" which is impossible to reverse. Additionally, if you change anything about the input data, it drastically changes the output digest.
- hash - The result of a hashing algorithm.
- script - The computer program that determines how a vout/UTXO is spent. 
- [pay-to-pubkeyhash script](https://en.bitcoin.it/wiki/Script#Standard_Transaction_to_Bitcoin_address_.28pay-to-pubkey-hash.29) - The most common script used to send money in Bitcoin and other cryptocurrencies. In order to send money, all you need to know is the hash of their public key (commonly represented as a base58 address), and in order to spend the received money all that is needed is a signature from the public key, and the public key itself. 
- [pay-to-pubkey script](https://en.bitcoin.it/wiki/Script#Obsolete_pay-to-pubkey_transaction) - A very simple script which has very similar functionality to pubkeyhash scripts. However, instead of sending money to the hash of a public key, the money is sent to the public key itself. All that is needed for spending is a cryptographic signature proving ownership of the public key.
- prevout - The vout which is spent (as a vin) in a transaction
- [OP_RETURN script](https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs) - OP_RETURN is an operation used in script which effectively makes an output provably unspendable. It is commonly used to hold small amounts of data on the blockchain without polluting the UTXO set. 

### Proof of Work and Blockchain Consensus Systems

Proof of Work is a proven consensus mechanism that has made Bitcoin secure and trustworthy for 8 years now. However, it is not without it's fair share of problems. PoW's major drawbacks are:

1. PoW wastes a lot of electricity, harming the environment.
2. PoW benefits greatly from economies of scale, so it tends to benefit big players the most, rather than small participants in the network.
3. PoW provides no incentive to use or keep the tokens. 
4. PoW has some centralization risks, because it tends to encourage miners to participate in the biggest mining pool (a group of miners who share the block reward), thus the biggest mining pool operator holds a lot of control over the network. 

Proof of Stake was invented to solve many of these problems by allowing participants to create and mine new blocks (and thus also get a block reward), simply by holding onto coins in their wallet and allowing their wallet to do automatic "staking". Proof Of Stake was originally invented by Sunny King and implemented in [Peercoin](https://peercoin.net/assets/paper/peercoin-paper.pdf). It has since been improved and adapted by many other people. This includes ["Proof of Stake Version 2" by Pavel Vasin](https://blackcoin.co/blackcoin-pos-protocol-v2-whitepaper.pdf), ["Proof of Stake Velocity" by Larry Ren](https://www.reddcoin.com/papers/PoSV.pdf), and most recently [CASPER by Vlad Zamfir](https://blog.ethereum.org/2015/08/01/introducing-casper-friendly-ghost/), as well as countless other experiments and lesser known projects. 

For [Qtum](https://qtum.org) we have decided to build upon "Proof of Stake Version 3", an improvement over version 2 that was also made by Pavel Vasin and implemented in the Blackcoin project. This version of PoS as implemented in Blackcoin is what we will be describing here. Some minor details of it has been modified in [Qtum](https://qtum.org), but the core consensus model is identical.

For many community members and developers alike, proof of stake is a difficult topic, because there has been very little written on how it manages to accomplish keeping the network safe using only proof of ownership of tokens on the network. This blog post will go into fine detail about Proof of Stake Version 3 and how it's blocks are created, validated, and ultimately how a pure Proof of Stake blockchain is possible to secure. This will assume some technical knowledge, but I will try to explain things so that most of the knowledge can be gathered from context. You should at least be familiar with the concept of the [UTXO-based blockchain](http://earlz.net/view/2017/07/27/1820/what-is-a-utxo-and-how-does-it).

Before we talk about PoS, it helps to understand how the much simpler PoW consensus mechanism works. It's mining process can be described in just a few lines of pseudo-code:

    while(blockhash > difficulty) {
        block.nonce = block.nonce + 1
        blockhash = sha256(sha256(block))
    }

A hash is a cryptographic algorithm which takes an arbritrary amount of input data, does a lot of processing of it, and outputs a fixed-size "digest" of that data. It is impossible to figure out the input data with just the digest. So, PoW tends to function like a lottery, where you find out if you won by creating the hash and checking it against the target, and you create another ticket by changing some piece of data in the block. In Bitcoin's case, nonce is used for this, as well as some other fields (usually called "extraNonce"). Once a blockhash is found which is less than the difficulty target, the block is valid, and can be broadcast to the rest of the distributed network. Miners will then see it and start building the next block on top of this block.

### Proof of Stake's Protocol Structures and Rules

Now enter Proof of Stake. We have these goals for PoS:

1. Impossible to counterfeit a block
2. Big players do not get disproportionally bigger rewards
3. More computing power is not useful for creating blocks
4. No one member of the network can control the entire blockchain

The core concept of PoS is very similar to PoW, a lottery. However, the big difference is that it is not possible to "get more tickets" to the lottery by simply changing some data in the block. Instead of the "block hash" being the lottery ticket to judge against a target, PoS invents the notion of a "kernel hash". 

The kernel hash is composed of several pieces of data that are not readily modifiable in the current block. And so, because the miners do not have an easy way to modify the kernel hash, they can not simply iterate through a large amount of hashes like in PoW. 

Proof of Stake blocks add many additional consensus rules in order to realize it's goals. First, unlike in PoW, the coinbase transaction (the first transaction in the block) must be empty and reward 0 tokens. Instead, to reward stakers, there is a special "stake transaction" which must be the 2nd transaction in the block. A stake transaction is defined as [any transaction](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.h#L279) that:

1. Has at least 1 valid `vin`
2. Its first `vout` must be an empty script
3. Its second vout must not be empty

Furthermore, staking transactions must abide by these rules to be valid in a block: </p>

1. The second `vout` must be either a pubkey (not pubkeyhash!) script, or an `OP_RETURN` script that is unspendable (data-only) but stores data for a public key
2. The timestamp in the transaction must be equal to the block timestamp
3. the total output value of a stake transaction must be less than or equal to the total inputs plus the PoS block reward plus the block's total transaction fees. `output <= (input + block_reward + tx_fees)`
4. The first spent vin's output must be confirmed by at least 500 blocks (in otherwords, the coins being spent must be at least 500 blocks old)
5. Though more vins can used and spent in a staking transaction, the first vin is the only one used for consensus parameters.


These rules ensure that the stake transaction is easy to identify, and ensures that it gives enough info to the blockchain to validate the block. The empty vout method is not the only way staking transactions could have been identified, but this was the original design from Sunny King and has worked well enough. 

Now that we understand what a staking transaction is, and what rules they must abide by, the next piece is to cover the rules for PoS blocks:

- Must have exactly 1 staking transaction
- The staking transaction must be the second transaction in the block
- The coinbase transaction must have 0 output value and a single empty vout
- The block timestamp must have it's bottom 4 bits set to 0 [(referred to as a "mask" in the source code)](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/kernel.h#L11). This effectively means the blocktime can only be represented in 16 second intervals, decreasing it's granularity
- [The version of the block must be 7](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.cpp#L2032)
- A block's "kernel hash" must meet the weighted difficulty for PoS 
- The block hash must be signed by the public key in the staking transaction's second vout. The signature data is placed in the block (but is not included in the formal block hash)
- The signature stored in the block must be "LowS", which means consisting only of a single piece of data and must be as compressed as possible (no extra leading 0s in the data, or other opcodes)
- Most other rules for standard PoW blocks apply (valid merkle hash, valid transactions, timestamp is within time drift allowance, etc)

There are a lot of details here that we'll cover in a bit. The most important part that really makes PoS effective lies in the "kernel hash". The kernel hash is used similar to PoW (if hash meets difficulty, then block is valid). However, the kernel hash is not directly modifiable in the context of the current block. We will first cover exactly what goes into these structures and mechanisms, and later explain why this design is exactly this way, and what unexpected consequences can come from minor changes to it. </p>

### The Proof of Stake Kernel Hash

The kernel hash specifically consists of the following exact pieces of data (in order):

- Previous block's "stake modifier" (more detail on this later)
- Timestamp from "prevout" transaction (the transaction output that is spent by the first vin of the staking transaction)
- The hash of the prevout transaction
- The output number of the prevout (ie, which output of the transaction is spent by the staking transaction)
- Current block time, with the bottom 4 bits set to 0 to reduce granularity. This is the only thing that changes during staking process

The stake modifier of a block is a hash of exactly:

- The hash of the prevout transaction in PoS blocks, OR the block hash in PoW blocks.
- The previous block's stake modifier (the genesis block's stake modifier is 0)

The only way to change the current kernel hash (in order to mine a block), is thus to either change your "prevout", or to change the current block time. 

A single wallet typically contains many UTXOs. The balance of the wallet is basically the total amount of all the UTXOs that can be spent by the wallet. This is of course the same in a PoS wallet. This is important though, because any output can be used for staking. One of these outputs are what can become the "prevout" in a staking transaction to form a valid PoS block. 

Finally, there is one more aspect that is changed in the mining process of a PoS block. The difficulty is weighted against the number of coins in the staking transaction. The PoS difficulty ends up being twice as easy to achieve when staking 2 coins, compared to staking just 1 coin. If this were not the case, then it would encourage creating many tiny UTXOs for staking, which would bloat the size of the blockchain and ultimately cause the entire network to require more resources to maintain, as well as potentially compromise the blockchain's overall security. 

So, if we were to show some pseudo-code for finding a valid kernel hash now, it would look like: </p>

    while(true){
        foreach(utxo in wallet){
            blockTime = currentTime - currentTime % 16
            posDifficulty = difficulty * utxo.value
            hash = hash(previousStakeModifier &lt;&lt; utxo.time &lt;&lt; utxo.hash &lt;&lt; utxo.n &lt;&lt; blockTime)
            if(hash &lt; posDifficulty){
                done
            }
        } 
        wait 16s -- wait 16 seconds, until the block time can be changed
    }

This code isn't so easy to understand as our PoW example, so I'll attempt to explain it in plain english:

    Do the following over and over for infinity: 
    Calculate the blockTime to be the current time minus itself modulus 16 (modulus is like dividing by 16, but then only instead of taking the result, taking the remainder)
    Calculate the posDifficulty as the network difficulty, multiplied by the number of coins held by the UTXO. 
    Cycle through each UTXO in the wallet. With each UTXO, calculate a SHA256 hash using the previous block's stake modifier, as well as some data from the the UTXO, and finally the blockTime. Compare this hash to the posDifficulty. If the hash is less than the posDifficulty, then the kernel hash is valid and you can create a new block. 
    After going through all UTXOs, if no hash produced is less than the posDifficulty, then wait 16 seconds and do it all over again. 

Now that we have found a valid kernel hash using one of the UTXOs we can spend, we can create a staking transaction. This staking transaction will have 1 vin, which spends the UTXO we found that has a valid kernel hash. It will have (at least) 2 vouts. The first vout will be empty, identifying to the blockchain that it is a staking transaction. The second vout will either contain an <code>OP_RETURN</code> data transaction that contains a single public key, or it will contain a pay-to-pubkey script. The latter is usually used for simplicity, but using a data transaction for this allows for some advanced use cases (such as a separate block signing machine) without needlessly cluttering the UTXO set. 

Finally, any transactions from the mempool are added to the block. The only thing left to do now is to create a signature, proving that we have approved the otherwise valid PoS block. The signature must use the public key that is encoded (either as pay-pubkey script, or as a data OP_RETURN script) in the second vout of the staking transaction. The actual data signed in the block hash. After the signature is applied, the block can be broadcast to the network. Nodes in the network will then validate the block and if it finds it valid and there is no better blockchain then it will accept it into it's own blockchain and broadcast the block to all the nodes it has connection to. 

Now we have a fully functional and secure PoSv3 blockchain. PoSv3 is what we determined to be most resistant to attack while maintaining a pure decentralized consensus system (ie, without master nodes or currators). To understand why we approached this conclusion however, we must understand it's history.

## PoSv3's History

Proof of Stake has a fairly long history. I won't cover every detail, but cover broadly what was changed between each version to arrive at PoSv3 for historical purposes:

PoSv1 - This version is implemented in Peercoin. It relied heavily on the notion of "coin age", or how long a UTXO has not been spent on the blockchain. It's implementation would basically make it so that the higher the coin age, the more the difficulty is reduced. This had the bad side-effect however of encouraging people to only open their wallet every month or longer for staking. Assuming the coins were all relatively old, they would almost instantaneously produce new staking blocks. This however makes double-spend attacks extremely easy to execute. Peercoin itself is not affected by this because it is a hybrid PoW and PoS blockchain, so the PoW blocks mitigated this effect.

PoSv2 - This version removes coin age completely from consensus, as well as using a completely different stake modifier mechanism from v1. The number of changes are too numerous to list here. All of this was done to remove coin age from consensus and make it a safe consensus mechanism without requiring a PoW/PoS hybrid blockchain to mitigate various attacks.

PoSv3 - PoSv3 is really more of an incremental improvement over PoSv2. In PoSv2 the stake modifier also included the previous block time. This was removed to prevent a "short-range" attack where it was possible to iteratively mine an alternative blockchain by iterating through previous block times. PoSv2 used block and transaction times to determine the age of a UTXO; this is not the same as coin age, but rather is the "minimum confirmations required" before a UTXO can be used for staking. This was changed to a much simpler mechanism where the age of a UTXO is determined by it's depth in the blockchain. This thus doesn't incentivize inaccurate timestamps to be used on the blockchain, and is also more immune to "timewarp" attacks. PoSv3 also added support for OP_RETURN coinstake transactions which allows for a vout to contain the public key for signing the block without requiring a full pay-to-pubkey script.

## References:

[https://peercoin.net/assets/paper/peercoin-paper.pdf](https://peercoin.net/assets/paper/peercoin-paper.pdf)
[https://blackcoin.co/blackcoin-pos-protocol-v2-whitepaper.pdf](https://blackcoin.co/blackcoin-pos-protocol-v2-whitepaper.pdf)
[https://www.reddcoin.com/papers/PoSV.pdf](https://www.reddcoin.com/papers/PoSV.pdf)
[https://blog.ethereum.org/2015/08/01/introducing-casper-friendly-ghost/](https://blog.ethereum.org/2015/08/01/introducing-casper-friendly-ghost/)
[https://github.com/JohnDolittle/blackcoin-old/blob/master/src/kernel.h#L11](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/kernel.h#L11)
[https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.cpp#L2032](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.cpp#L2032)
[https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.h#L279](https://github.com/JohnDolittle/blackcoin-old/blob/master/src/main.h#L279)
[http://earlz.net/view/2017/07/27/1820/what-is-a-utxo-and-how-does-it](http://earlz.net/view/2017/07/27/1820/what-is-a-utxo-and-how-does-it)
[https://en.bitcoin.it/wiki/Script#Obsolete_pay-to-pubkey_transaction](https://en.bitcoin.it/wiki/Script#Obsolete_pay-to-pubkey_transaction
[https://en.bitcoin.it/wiki/Script#Standard_Transaction_to_Bitcoin_address_.28pay-to-pubkey-hash.29](https://en.bitcoin.it/wiki/Script#Standard_Transaction_to_Bitcoin_address_.28pay-to-pubkey-hash.29)
[https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs](https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs
