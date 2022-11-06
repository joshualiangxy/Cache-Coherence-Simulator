#!/bin/bash

for protocol in MESI MESIF Dragon;
do
    for benchmark in blackscholes bodytrack fluidanimate;
    do
        for i in 64 128 256 512 1024 2048 4096 8192 16384;
        do
            echo "$protocol $benchmark - Cache Size: $i";
            ./coherence $protocol $benchmark $i 2 32 >> logs/$benchmark-cachesize-$protocol;
        done;
    done;
done

for protocol in MESI MESIF Dragon;
do
    for benchmark in blackscholes bodytrack fluidanimate;
    do
        for i in 1 2 4 8 16 32 64 128;
        do
            echo "$protocol $benchmark - Associativity: $i";
            ./coherence $protocol $benchmark 4096 $i 32 >> logs/$benchmark-associativity-$protocol;
        done;
    done;
done

for protocol in MESI MESIF Dragon;
do
    for benchmark in blackscholes bodytrack fluidanimate;
    do
        for i in 4 8 16 32 64 128 256 512 1024 2048;
        do
            echo "$protocol $benchmark - Block Size: $i";
            ./coherence $protocol $benchmark 4096 2 $i >> logs/$benchmark-blocksize-$protocol;
        done;
    done;
done

