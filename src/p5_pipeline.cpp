/**
 * Universidad del Valle de Guatemala
 * CC3086 Programación de Microprocesadores
 * Laboratorio 6 - Práctica 5: Pipeline con Barreras y pthread_once
 * 
 * Autor: Adrian Penagos
 * Fecha: Septiembre 2025
 * Propósito: Construir pipeline de 3 etapas sincronizado con barriers
 *           Inicialización única con pthread_once y medición de throughput
 */
#include <pthread.h>
#include <cstdio>
#include <cstdlib>

constexpr int TICKS = 10; // número de rondas

static pthread_barrier_t barrier;
static pthread_once_t once_flag = PTHREAD_ONCE_INIT;

// Inicialización única (se ejecuta solo 1 vez)
static void init_shared() {
    std::puts("[init_shared] Recursos compartidos inicializados.");
}

// Cada hilo representa una etapa del pipeline
void* stage(void* p) {
    long id = (long)p;

    // Inicialización global segura
    pthread_once(&once_flag, init_shared);

    for (int t = 0; t < TICKS; t++) {
        if (id == 1) {
            std::printf("[tick %d] Stage 1: generar dato\n", t);
        } else if (id == 2) {
            std::printf("[tick %d] Stage 2: filtrar dato\n", t);
        } else if (id == 3) {
            std::printf("[tick %d] Stage 3: reducir dato\n", t);
        }

        // Sincronizar al final del tick
        pthread_barrier_wait(&barrier);
    }

    return nullptr;
}

int main() {
    pthread_t h1, h2, h3;
    pthread_barrier_init(&barrier, NULL, 3);

    pthread_create(&h1, 0, stage, (void*)1);
    pthread_create(&h2, 0, stage, (void*)2);
    pthread_create(&h3, 0, stage, (void*)3);

    pthread_join(h1, 0);
    pthread_join(h2, 0);
    pthread_join(h3, 0);

    pthread_barrier_destroy(&barrier);
    return 0;
}
