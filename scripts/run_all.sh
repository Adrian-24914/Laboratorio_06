#!/bin/bash
# run_all.sh - Ejecuta todas las prácticas mostrando solo prints en consola

echo "=== P1 Counter ==="
./bin/p1_counter 4 1000000 all
echo ""

echo "=== P2 Ring Buffer ==="
./bin/p2_ring 4 100000
echo ""

echo "=== P3 Readers/Writers ==="
./bin/p3_rw 4 90
./bin/p3_rw 4 70
./bin/p3_rw 4 50
./bin/p3_rw 8 90
./bin/p3_rw 8 70
./bin/p3_rw 8 50
echo ""

echo "=== P4 Deadlock ==="
echo "Nota: La primera ejecución puede quedarse en deadlock. Usa Ctrl+C para interrumpir."
echo "Ejecutando deadlock intencional..."
./bin/p4_deadlock deadlock &   # se ejecuta en background
pid=$!
sleep 2                        # espera 2 segundos para que aparezca el deadlock
echo "Matando proceso deadlock para evitar congelamiento..."
kill $pid                      # mata el proceso bloqueado
wait $pid 2>/dev/null
echo "Deadlock interrumpido. Probando orden seguro..."
./bin/p4_deadlock orden
echo "Probando trylock con backoff..."
./bin/p4_deadlock trylock
echo ""

echo "=== P5 Pipeline ==="
./bin/p5_pipeline 1000
echo ""
