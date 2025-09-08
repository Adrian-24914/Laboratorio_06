/**
 * Universidad del Valle de Guatemala
 * CC3086 Programación de Microprocesadores
 * Laboratorio 6 - Práctica 3: Lectores/Escritores con HashMap
 * 
 * Autor: Adrian Penagos
 * Fecha: Septiembre 2025
 * Propósito: Comparar pthread_rwlock_t vs pthread_mutex_t en tabla hash
 *           Evaluar rendimiento bajo diferentes proporciones de lectura/escritura
 */
#include <pthread.h>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <atomic>
#include <unistd.h> // sleep()

constexpr int NBUCKET = 1024;   // número de buckets
constexpr int NHASH = 4096;     // rango de claves posibles
constexpr int NTHREADS = 8;     // total de hilos
constexpr int RUN_SEC = 5;      // segundos de ejecución

// Proporciones de operaciones
double READ_RATIO = 0.9;  // 90% lecturas (puede cambiarse)

struct Node {
    int k, v;
    Node* next;
    Node(int kk, int vv, Node* n=nullptr) : k(kk), v(vv), next(n) {}
};

struct MapMutex {
    Node* b[NBUCKET]{};
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

struct MapRW {
    Node* b[NBUCKET]{};
    pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
};

// ---------------- Operaciones con Mutex ----------------
int map_get(MapMutex* m, int k) {
    pthread_mutex_lock(&m->m);
    int idx = k % NBUCKET;
    Node* cur = m->b[idx];
    while (cur) {
        if (cur->k == k) {
            int val = cur->v;
            pthread_mutex_unlock(&m->m);
            return val;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&m->m);
    return -1; // no encontrado
}

void map_put(MapMutex* m, int k, int v) {
    pthread_mutex_lock(&m->m);
    int idx = k % NBUCKET;
    m->b[idx] = new Node(k, v, m->b[idx]);
    pthread_mutex_unlock(&m->m);
}

// ---------------- Operaciones con RWLock ----------------
int map_get(MapRW* m, int k) {
    pthread_rwlock_rdlock(&m->rw);
    int idx = k % NBUCKET;
    Node* cur = m->b[idx];
    while (cur) {
        if (cur->k == k) {
            int val = cur->v;
            pthread_rwlock_unlock(&m->rw);
            return val;
        }
        cur = cur->next;
    }
    pthread_rwlock_unlock(&m->rw);
    return -1;
}

void map_put(MapRW* m, int k, int v) {
    pthread_rwlock_wrlock(&m->rw);
    int idx = k % NBUCKET;
    m->b[idx] = new Node(k, v, m->b[idx]);
    pthread_rwlock_unlock(&m->rw);
}

// ---------------- Benchmark ----------------
std::atomic<bool> stop_flag(false);
std::atomic<long> total_ops(0);

struct ThreadArg {
    int id;
    bool use_rwlock;
    void* map;
};

void* worker(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    std::mt19937 rng(ta->id + 1234);
    std::uniform_int_distribution<int> keydist(0, NHASH-1);
    std::uniform_real_distribution<double> opdist(0.0, 1.0);

    long ops = 0;
    while (!stop_flag.load()) {
        int k = keydist(rng);
        double r = opdist(rng);

        if (ta->use_rwlock) {
            MapRW* m = (MapRW*)ta->map;
            if (r < READ_RATIO) map_get(m, k);
            else map_put(m, k, k*10);
        } else {
            MapMutex* m = (MapMutex*)ta->map;
            if (r < READ_RATIO) map_get(m, k);
            else map_put(m, k, k*10);
        }
        ops++;
    }
    total_ops.fetch_add(ops);
    return nullptr;
}

// ---------------- Main ----------------
int main() {
    // Configuración: elegir modo
    bool use_rwlock = true; // cambiar a false para probar con mutex

    // Crear el mapa
    MapRW m_rw;
    MapMutex m_mtx;

    void* map = use_rwlock ? (void*)&m_rw : (void*)&m_mtx;

    // Crear hilos
    pthread_t th[NTHREADS];
    ThreadArg args[NTHREADS];
    for (int i=0; i<NTHREADS; i++) {
        args[i] = {i, use_rwlock, map};
        pthread_create(&th[i], nullptr, worker, &args[i]);
    }

    // Dejar correr la prueba
    sleep(RUN_SEC);
    stop_flag.store(true);

    // Esperar hilos
    for (int i=0; i<NTHREADS; i++) {
        pthread_join(th[i], nullptr);
    }

    // Resultados
    double throughput = total_ops.load() / (double)RUN_SEC;
    std::cout << "Modo: " << (use_rwlock ? "RWLock" : "Mutex") << "\n";
    std::cout << "Lectura/Escritura = " << (int)(READ_RATIO*100) 
              << "/" << (100 - (int)(READ_RATIO*100)) << "\n";
    std::cout << "Throughput: " << throughput << " ops/seg\n";
    return 0;
}
