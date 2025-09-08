#!/bin/bash
# Práctica 5: pipeline de 3 etapas

OUT=../data/p5_pipeline.csv
NREP=5
TICKS=1000

echo "ticks,rep,time_s" > $OUT

for REP in $(seq 1 $NREP); do
  START=$(date +%s.%N)
  ../bin/p5_pipeline $TICKS > /dev/null
  END=$(date +%s.%N)
  ELAPSED=$(echo "$END - $START" | bc -l)
  echo "$TICKS,$REP,$ELAPSED" >> $OUT
done

echo "Resultados guardados en $OUT"
