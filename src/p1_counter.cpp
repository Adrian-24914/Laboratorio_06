/**
 * Universidad del Valle de Guatemala
 * CC3086 Programación de Microprocesadores
 * Laboratorio 6 - Práctica 1: Contador con Race Conditions
 * 
 * Autor: Adrian Penagos
 * Fecha: Septiembre 2025
 * Propósito: Demostrar race conditions en contador global y comparar
 *           soluciones: naive, mutex, sharded y atomic
 */
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstring>

struct Args {
    long iters;
    long* global;
    pthread_mutex_t* mtx;
    long thread_id;
    long num_threads;
    long* shards; // for sharded
    std::atomic<long>* atomic_global; // optional
};

void* worker_naive(void* p){
    auto* a = static_cast<Args*>(p);
    for(long i=0;i<a->iters;i++){
        (*a->global)++; // RACE intencional
    }
    return nullptr;
}

void* worker_mutex(void* p){
    auto* a = static_cast<Args*>(p);
    for(long i=0;i<a->iters;i++){
        pthread_mutex_lock(a->mtx);
        (*a->global)++;
        pthread_mutex_unlock(a->mtx);
    }
    return nullptr;
}

void* worker_sharded(void* p){
    auto* a = static_cast<Args*>(p);
    long id = a->thread_id;
    long* shards = a->shards;
    for(long i=0;i<a->iters;i++){
        shards[id]++; // no sincronización entre shards
    }
    return nullptr;
}

void* worker_atomic(void* p){
    auto* a = static_cast<Args*>(p);
    for(long i=0;i<a->iters;i++){
        a->atomic_global->fetch_add(1, std::memory_order_relaxed);
    }
    return nullptr;
}

long run_naive(int T, long iters){
    long global = 0;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    std::vector<pthread_t> th(T);
    Args arg{iters, &global, &mtx, 0, T, nullptr, nullptr};
    for(int i=0;i<T;i++){
        pthread_create(&th[i], nullptr, worker_naive, &arg);
    }
    for(int i=0;i<T;i++) pthread_join(th[i], nullptr);
    return global;
}

double timed_run_naive(int T, long iters, long &out_total){
    auto t0 = std::chrono::steady_clock::now();
    out_total = run_naive(T, iters);
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    return dt.count();
}

long run_mutex(int T, long iters){
    long global = 0;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    std::vector<pthread_t> th(T);
    Args arg{iters, &global, &mtx, 0, T, nullptr, nullptr};
    for(int i=0;i<T;i++){
        pthread_create(&th[i], nullptr, worker_mutex, &arg);
    }
    for(int i=0;i<T;i++) pthread_join(th[i], nullptr);
    return global;
}

double timed_run_mutex(int T, long iters, long &out_total){
    auto t0 = std::chrono::steady_clock::now();
    out_total = run_mutex(T, iters);
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    return dt.count();
}

long run_sharded(int T, long iters){
    std::vector<long> shards(T);
    for(int i=0;i<T;i++) shards[i]=0;
    std::vector<pthread_t> th(T);
    std::vector<Args> args(T);
    for(int i=0;i<T;i++){
        args[i].iters = iters;
        args[i].global = nullptr;
        args[i].mtx = nullptr;
        args[i].thread_id = i;
        args[i].num_threads = T;
        args[i].shards = shards.data();
        args[i].atomic_global = nullptr;
        pthread_create(&th[i], nullptr, worker_sharded, &args[i]);
    }
    for(int i=0;i<T;i++) pthread_join(th[i], nullptr);
    long total = 0;
    for(int i=0;i<T;i++) total += shards[i];
    return total;
}

double timed_run_sharded(int T, long iters, long &out_total, double *reduce_time=nullptr){
    auto t0 = std::chrono::steady_clock::now();
    // launch and join
    std::vector<long> shards(T);
    for(int i=0;i<T;i++) shards[i]=0;
    std::vector<pthread_t> th(T);
    std::vector<Args> args(T);
    for(int i=0;i<T;i++){
        args[i].iters = iters;
        args[i].global = nullptr;
        args[i].mtx = nullptr;
        args[i].thread_id = i;
        args[i].num_threads = T;
        args[i].shards = shards.data();
        args[i].atomic_global = nullptr;
        pthread_create(&th[i], nullptr, worker_sharded, &args[i]);
    }
    for(int i=0;i<T;i++) pthread_join(th[i], nullptr);
    auto t_mid = std::chrono::steady_clock::now();
    long total = 0;
    for(int i=0;i<T;i++) total += shards[i]; // reduce
    auto t1 = std::chrono::steady_clock::now();
    if(reduce_time){
        std::chrono::duration<double> rd = t1 - t_mid;
        *reduce_time = rd.count();
    }
    out_total = total;
    std::chrono::duration<double> dt = t1 - t0;
    return dt.count();
}

long run_atomic(int T, long iters){
    std::atomic<long> atomic_global(0);
    std::vector<pthread_t> th(T);
    std::vector<Args> args(T);
    for(int i=0;i<T;i++){
        args[i].iters = iters;
        args[i].atomic_global = &atomic_global;
        pthread_create(&th[i], nullptr, worker_atomic, &args[i]);
    }
    for(int i=0;i<T;i++) pthread_join(th[i], nullptr);
    return atomic_global.load();
}

double timed_run_atomic(int T, long iters, long &out_total){
    auto t0 = std::chrono::steady_clock::now();
    out_total = run_atomic(T, iters);
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    return dt.count();
}

void usage(const char* name){
    std::printf("Usage: %s [threads] [iters] [mode]\n", name);
    std::printf(" modes: naive | mutex | sharded | atomic | all\n");
}

int main(int argc, char** argv){
    if(argc < 4){ usage(argv[0]); return 1; }
    int T = std::atoi(argv[1]);
    long it = std::atol(argv[2]);
    const char* mode = argv[3];
    if(T<=0 || it<=0){ usage(argv[0]); return 1; }

    if(std::strcmp(mode, "naive")==0 || std::strcmp(mode,"all")==0){
        long total;
        double t = timed_run_naive(T,it,total);
        std::printf("NAIVE: total=%ld expected=%ld time=%.6fs throughput=%.3f Mops/s\n",
            total, (long)T*it, t, ((double)total)/(t*1e6));
    }
    if(std::strcmp(mode, "mutex")==0 || std::strcmp(mode,"all")==0){
        long total;
        double t = timed_run_mutex(T,it,total);
        std::printf("MUTEX: total=%ld expected=%ld time=%.6fs throughput=%.3f Mops/s\n",
            total, (long)T*it, t, ((double)total)/(t*1e6));
    }
    if(std::strcmp(mode, "sharded")==0 || std::strcmp(mode,"all")==0){
        long total;
        double reduce_t;
        double t = timed_run_sharded(T,it,total,&reduce_t);
        std::printf("SHARDED: total=%ld expected=%ld time=%.6fs reduce_time=%.6fs throughput=%.3f Mops/s\n",
            total, (long)T*it, t, reduce_t, ((double)total)/(t*1e6));
    }
    if(std::strcmp(mode, "atomic")==0 || std::strcmp(mode,"all")==0){
        long total;
        double t = timed_run_atomic(T,it,total);
        std::printf("ATOMIC: total=%ld expected=%ld time=%.6fs throughput=%.3f Mops/s\n",
            total, (long)T*it, t, ((double)total)/(t*1e6));
    }

    return 0;
}
