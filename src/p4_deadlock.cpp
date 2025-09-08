/**
 * Universidad del Valle de Guatemala
 * CC3086 Programación de Microprocesadores
 * Laboratorio 6 - Práctica 4: Deadlock Intencional, Diagnóstico y Corrección
 * 
 * Autor: Adrian Penagos
 * Fecha: Septiembre 2025
 * Propósito: Reproducir interbloqueo con dos mutex, diagnosticar y corregir
 *           usando orden total y trylock con backoff
 */
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string> 

pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

// ----------- Versión con deadlock intencional -----------
void* t1_deadlock(void*) {
    pthread_mutex_lock(&A);
    usleep(1000);
    pthread_mutex_lock(&B);
    std::puts("t1 ok");
    return nullptr;
}
void* t2_deadlock(void*) {
    pthread_mutex_lock(&B);
    usleep(1000);
    pthread_mutex_lock(&A);
    std::puts("t2 ok");
    return nullptr;
}

// ----------- Corrección 1: Orden global (A siempre antes que B) -----------
void* t1_order(void*) {
    pthread_mutex_lock(&A);
    usleep(1000);
    pthread_mutex_lock(&B);
    std::puts("t1 ok (ordenado)");
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);
    return nullptr;
}
void* t2_order(void*) {
    // cambia el orden: primero A, luego B
    pthread_mutex_lock(&A);
    usleep(1000);
    pthread_mutex_lock(&B);
    std::puts("t2 ok (ordenado)");
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);
    return nullptr;
}

// ----------- Corrección 2: trylock con backoff -----------
void* t1_trylock(void*) {
    while (true) {
        pthread_mutex_lock(&A);
        if (pthread_mutex_trylock(&B) == 0) {
            std::puts("t1 ok (trylock)");
            pthread_mutex_unlock(&B);
            pthread_mutex_unlock(&A);
            break;
        } else {
            pthread_mutex_unlock(&A);
            usleep(100); // backoff
        }
    }
    return nullptr;
}
void* t2_trylock(void*) {
    while (true) {
        pthread_mutex_lock(&B);
        if (pthread_mutex_trylock(&A) == 0) {
            std::puts("t2 ok (trylock)");
            pthread_mutex_unlock(&A);
            pthread_mutex_unlock(&B);
            break;
        } else {
            pthread_mutex_unlock(&B);
            usleep(100);
        }
    }
    return nullptr;
}

int main(int argc, char** argv) {
    pthread_t x, y;

    if (argc < 2) {
        std::puts("Uso: ./p4_deadlock modo");
        std::puts("modos: deadlock | orden | trylock");
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "deadlock") {
        pthread_create(&x,nullptr,t1_deadlock,nullptr);
        pthread_create(&y,nullptr,t2_deadlock,nullptr);
    } else if (mode == "orden") {
        pthread_create(&x,nullptr,t1_order,nullptr);
        pthread_create(&y,nullptr,t2_order,nullptr);
    } else if (mode == "trylock") {
        pthread_create(&x,nullptr,t1_trylock,nullptr);
        pthread_create(&y,nullptr,t2_trylock,nullptr);
    } else {
        std::puts("Modo inválido.");
        return 1;
    }

    pthread_join(x,nullptr);
    pthread_join(y,nullptr);
    return 0;
}
