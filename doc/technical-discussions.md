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

“The protocol upgrade involves replacing the proof-of-stake difficulty as the hash modifier for proof-of-stake (we call it stake modifier). The new stake modifier is 64 bit and derived from about 9 days worth of blocks after the coin generating the stake. When I get some time over next week I would talk a bit more about how it works.” - Sunny King


## Observations on Dcrypt by jonnylatte

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

## Observations on the miner by jonnylatte (https://bitcointalk.org/index.php?topic=1141676.msg12627802#msg12627802)

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

Even just calculating the first 4 iterations would mean 2\*4 less sha256 blocks per hash with a reasonable amount of memory usage...


---

The build instructions are in the README file: 

Dependencies:
   libcurl-\*-dev      http://curl.haxx.se/libcurl/
   libjansson         http://www.digip.org/jansson/
      autoconf              https://www.gnu.org/software/autoconf/
      (jansson is included in-tree)

Basic \*nix build instructions:
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

