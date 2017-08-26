#!/usr/bin/python

import random

probs = []
#for c in range(ord('A'), ord('Z')+1):
#    probs.append((1, chr(c)))
for c in "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()-_=+[]{}|;:<>?,./":
    probs.append((1, c))
#random.shuffle(probs)

while len(probs) > 1:
    probs.sort(key=lambda x: x[0])
    a = probs.pop(0)
    b = probs.pop(0)
    probs.append((a[0] + b[0], (a[1], b[1])))

tree = probs[0][1]
#print "Tree for all chars"
#print tree
#print

def extract(node, prefix=[]):
    if len(node) == 1:
        return [(node, prefix)]
    else:
        return extract(node[0], prefix+[0]) \
                + extract(node[1], prefix+[1])

listing = extract(tree)

def bitpack(bits):
    return reduce(lambda a, b: a*2+b, bits)

listing = map(lambda l: (l[1], bitpack(l[1]), len(l[1]), l[0]), listing)
#listing.sort()
print len(listing)

print "Encoding"
for item in listing:
    print item
print

print "Values"
for item in listing:
    print "0x%02x," % item[1],
print

print "Lengths"
for item in listing:
    print "%d," % item[2],
print
