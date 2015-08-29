import os
import sys


spec = open(sys.argv[1], 'r')
all_lines = spec.readlines()

n = int(all_lines[2])
lines = all_lines[3:]

count = 2**n
assert len(lines) == count

table = open(sys.argv[1] + '.table', 'w')
table.write('%dx%dx10\n' % (n, n))

for index in xrange(0, count):
    line = lines[index]
    value = int(line)
    table.write('%d\t=>\t%d\n' % (index, value))
