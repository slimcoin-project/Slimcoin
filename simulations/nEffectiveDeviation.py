COIN = 1000000
BURN_CONSTANT = 0.1 * COIN / 100
BURN_HARDER_TARGET = 0.5
START = 1000 * COIN
HALF_LIFE = 350000

def deviation(value):
    return 100 * (value / float(START)) - 100

if __name__ == '__main__':
    decay = 1.00000198
    start1, start2 = START, START

    for i in range(1, HALF_LIFE * 10 + 1):
        #print "Values are %f %f, difference %f" % (start1, start2, start2 - start1)

        if(i % HALF_LIFE == 0):
            half_lives = i / HALF_LIFE
            print "Half lives %d Deviation is %f%% %f%%" % (half_lives, deviation(start1 * 2 ** half_lives), \
                                                            deviation(start2 * 2 ** half_lives))
        
        start1 = int(start1 / decay)
        start2 /= decay
        
    diff = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    diff = (diff * (BURN_CONSTANT / start1)) * BURN_HARDER_TARGET
    print "ACTUAL: 0x%064x" % diff
    
    diff = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    diff = (diff * (BURN_CONSTANT / start2)) * BURN_HARDER_TARGET
    print "  BEST: 0x%064x" % diff

