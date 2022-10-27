#!/bin/bash

for benchmark in blackscholes bodytrack fluidanimate;
do
    for i in 64 128 256 512 1024 2048 4096 8192 16384;
    do
        ./coherence MESIF $benchmark $i 2 32 >> $benchmark-cachesize-MESIF;
    done;
done