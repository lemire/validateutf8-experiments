#!/usr/bin/env python

import random, copy, sys


table=set()
for i in range(16):
    table.add((0xC,0x0, i))
    table.add((0xC,0x1, i))
    table.add((0xF,0x5, i))
    table.add((0xF,0x6, i))
    table.add((0xF,0x7, i))
    table.add((0xF,0x8, i))
    table.add((0xF,0x9, i))
    table.add((0xF,0xA, i))
    table.add((0xF,0xB, i))
    table.add((0xF,0xC, i))
    table.add((0xF,0xD, i))
    table.add((0xF,0xE, i))
    table.add((0xF,0xF, i))

table.add((0xE,0x0, 0x8))
table.add((0xE,0x0, 0x9))

table.add((0xF,0x0, 0x8))

table.add((0xF,0x4, 0x9))
table.add((0xF,0x4, 0xA))
table.add((0xF,0x4, 0xB))


table.add((0xE,0xD, 0xA))
table.add((0xE,0xD, 0xB))

cont = set((0x8,0x9,0xA,0xB))

for i in range(16):
    if(not i in cont):
        for j in range(16):
          table.add((0xC,j, i))
          table.add((0xD,j, i))
          table.add((0xE,j, i))
          table.add((0xF,j, i))

for j in range(16):
    for i in [0x8,0x9,0xA,0xB]:
        for z in range(8):
          table.add((z,j, i))




def testme_gah(testme):
  for i in range(len(testme)-1):
    byte1 = testme[i]
    byte2 = testme[1+1]
    nibble1 = byte1>>4
    nibble2 = byte1 &0xF
    nibble3 = byte2 >>4
    print('{0:08b}'.format(byte1), '{0:08b}'.format(byte2))
    if((nibble1,nibble2,nibble3) in table):
        print("bug ", hex(byte1), hex(byte2))


matrix = [[[0 for x in range(16)]  for y in range(16)]  for z in range(16)]
for t in table:
    matrix[t[0]][t[1]][t[2]]=1
all_values = set([i for i in range(16)])

def is_ok(range):
    for i in range[0]:
        for j in range[1]:
            for k in range[2]:
                if(matrix[i][j][k] != 1):
                    return False
    return True

def remove_me(uncover, range_coord):
    for i in range_coord[0]:
        for j in range_coord[1]:
            for k in range_coord[2]:
                z = (i,j,k)
                if(z in uncover):
                    uncover.remove(z)

def tryt(myset):
    uncover = set(table)
    solution = []
    access = [0,1,2]
    all_values = [i for i in range(16)]

    while(len(uncover)>0):
        t = random.sample(uncover,1)[0]
        range_coord = [set([t[i]]) for i in range(3)]
        random.shuffle(access)
        for c in access:
            random.shuffle(all_values)
            for i in all_values:
                r = copy.deepcopy(range_coord)
                r[c].add(i)
                if(is_ok(r)):
                  range_coord = r
        remove_me(uncover,range_coord)
        solution.append(range_coord)
    return solution

best = 8
sol = []

def buildmap(sol) :
    x = [0 for i in range(16)]
    y = [0 for i in range(16)]
    z = [0 for i in range(16)]
    bit = 1
    for t in sol:
        for i in t[0]:
            x[i] |= bit
        for i in t[1]:
            y[i] |= bit
        for i in t[2]:
            z[i] |= bit
        bit *= 2
    print(x)
    print(y)
    print(z)

for i in range(10000):
    z = tryt(table)
    ll = len(z)
    if(ll < best):
        sol = z
        best = ll
        print(sol)
        print(buildmap(sol))
        print(best)



print(sol)
print(best)