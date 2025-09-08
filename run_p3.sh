#!/bin/bash
# Práctica 3: mapa lectores/escritores

OUT=../data/p3_rw.csv
NREP=5
THREADS_LIST="4 8"
RATIOS="90 70 50"   # porcentaje de lecturas

echo "threads,ratio,rep,time_s" > $OUT

for T in $THREADS_LIST; do
  for R in $RATIOS; do
    for REP in $(seq 1 $NREP); do
      START=$(date +%s.%N)
      ../bin/p3_rw $T $R > /dev/null
      END=$(date +%s.%N)
      ELAPSED=$(echo "$END - $START" | bc -l)
      echo "$T,$R,$REP,$ELAPSED" >> $OUT
    done
  done
done

echo "Resultados guardados en $OUT"
