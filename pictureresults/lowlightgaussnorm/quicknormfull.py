#!/usr/bin/python

import sys

def main(argv):
    
    f = open(argv[0])
    lines = f.readlines()
    f.close

    nums = []
    for line in lines:
        try:
            num = float(line.split(" ")[1])
            nums.append(num)
        except:
            pass

    maximum = max(nums)
    minimum = min(nums)
    for i, num in enumerate(nums):
        print "%d %.2f" % (i, (num - minimum) / (maximum - minimum))


main(sys.argv[1:])