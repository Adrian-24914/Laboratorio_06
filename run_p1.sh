#!/bin/bash
# Práctica 1: contador global (naive, mutex, sharded, atomic)

OUT=../data/p1_counter.csv
NREP=5
THREADS_LIST="2 4 8"
ITERS=1000000

echo "mode,threads,iters,rep,time_s" > $OUT

for MODE in naive mutex; do
  for T in $THREADS_LIST; do
    for REP in $(seq 1 $NREP); do
      START=$(date +%s.%N)
      ../bin/p1_counter $MODE $T $ITERS > /dev/null
      END=$(date +%s.%N)
      ELAPSED=$(echo "$END - $START" | bc -l)
      echo "$MODE,$T,$ITERS,$REP,$ELAPSED" >> $OUT
    done
  done
done

echo "Resultados guardados en $OUT"