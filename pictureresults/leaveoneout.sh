#!/bin/bash
# Perform leave-one-out benchmarks.

cat /dev/null > results.txt

for file in \
    backyard.txt \
    bench.txt \
    book.txt \
    books1.txt \
    books2.txt \
    books3.txt \
    books4.txt \
    bridge.txt \
    building1.txt \
    building2.txt \
    building3.txt \
    cup1.txt \
    cup2.txt \
    cup3.txt \
    cup4.txt \
    fabric.txt \
    flower.txt \
    gametree.txt \
    gorillapod.txt \
    granola.txt \
    interior1.txt \
    interior2.txt \
    lamp.txt \
    landscape1.txt \
    landscape2.txt \
    landscape3.txt \
    screen.txt \
    snails.txt \
    stillLife.txt \
    timbuk.txt \
    ubuntu.txt \
    vase.txt
do
    ./benchmark.sh -lv $file --backlash
done