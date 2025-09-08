#!/bin/bash
# Práctica 2: buffer circular productor/consumidor

OUT=../data/p2_ring.csv
NREP=5
THREADS_LIST="2 4"
WORK=100000

echo "threads,work,rep,time_s" > $OUT

for T in $THREADS_LIST; do
  for REP in $(seq 1 $NREP); do
    START=$(date +%s.%N)
    ../bin/p2_ring $T $WORK > /dev/null
    END=$(date +%s.%N)
    ELAPSED=$(echo "$END - $START" | bc -l)
    echo "$T,$WORK,$REP,$ELAPSED" >> $OUT
  done
done

echo "Resultados guardados en $OUT"
