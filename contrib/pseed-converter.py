foo = [
['0x'+''.join([hex(149)[2:],hex(239)[2:],hex(189)[2:],hex(106)[2:]])],
['0x'+''.join([hex(185)[2:],hex(68)[2:],hex(67)[2:],hex(37)[2:]])],
['0x'+''.join([hex(5)[2:],hex(9)[2:],hex(39)[2:],hex(9)[2:]])],
['0x'+''.join([hex(144)[2:],hex(76)[2:],hex(64)[2:],hex(49)[2:]])],
]
print(', '.join([f[0] for f in foo]))
