#!/bin/bash
# Práctica 4: deadlock y correcciones

OUT=data/p4_deadlock.csv
mkdir -p data
NREP=5
MODES="deadlock orden trylock"

echo "mode,rep,time_s" > $OUT

for M in $MODES; do
  for REP in $(seq 1 $NREP); do
    START=$(date +%s.%N)
    timeout 2 ../bin/p4_deadlock $M > /dev/null 2>&1
    END=$(date +%s.%N)
    ELAPSED=$(echo "$END - $START" | bc -l)
    echo "$M,$REP,$ELAPSED" >> $OUT
  done
done

echo "Resultados guardados en $OUT"
