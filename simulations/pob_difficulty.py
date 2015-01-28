
from random import random as rand_num
import math

#percentages
PoW = 0.8          #80% of all blocks
PoB = 1 - PoW

#constants
POW = 0
POB = 1

#target
PoB_target = 3

blocks = []
total_coins_burned = 0

class CBlock:
    def __init__(self, block_type):
        self.coins_burned = 10000 * rand_num()

        global total_coins_burned
        total_coins_burned += self.coins_burned

        self.type = block_type
        
        self.diff = calc_PoB_difficulty(self.coins_burned)

    def print_self(self):
        if self.type == POW:
            print "Type: POW"
        elif self.type == POB:
            print "Type: POB"
        else:
            print "Type: Unknown"

        print "Coins Burned: %f" % self.coins_burned

        print "Difficulty: %d" % self.diff

        print "-" * 30

def PoW_blocks_back():
    nPoW = 0
    i = -1

    if len(blocks) == 0:
        return nPoW

    while True:
        if i == -1 * len(blocks) - 1 or blocks[i].type == POB:
            break
        
        nPoW += 1
        i -= 1

    return nPoW

def logistic_curve(x):
    return (2 / (1 + math.e ** (-0.2 * x))) - 1

def calc_PoB_difficulty(cur_blk_coins_burned):
    #genesis block
    if len(blocks) == 0:
        return 10000

    nPoW = PoW_blocks_back()
    offset = PoB_target - nPoW

    #offset > 0, increas diff
    #offset < 0, decrease diff
    #offset == 0, do nothing

    adjust = logistic_curve(offset)

    burn_adjust = cur_blk_coins_burned / total_coins_burned

    last_diff = blocks[-1].diff
    new_diff = last_diff * (1 + adjust - burn_adjust)

    return new_diff
    
def gen_fake_blocks(nBlocks):
    #generate psuedo blocks randomly to fill the blocks list

    #genesis block
    blocks.append(CBlock(POW))

    for n in range(nBlocks):
        multi = blocks[-1].diff / blocks[0].diff

        if rand_num() < multi * PoW or blocks[-1].type == POB:         #make a PoW block
            blocks.append(CBlock(POW))
        else:                        #make a PoB block
            blocks.append(CBlock(POB))

if __name__ == '__main__':
    gen_fake_blocks(100000)

    nPoW = nPoB = 0

    for blk in blocks:
        #blk.print_self()

        if blk.type == POB:
            nPoB += 1
        elif blk.type == POW:
            nPoW += 1

    print total_coins_burned

    print "PoW %d  PoB %d  Ratio: %f" % (nPoW, nPoB, float(nPoB) / nPoW)
