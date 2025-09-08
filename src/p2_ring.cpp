/**
 * Universidad del Valle de Guatemala
 * CC3086 Programación de Microprocesadores
 * Laboratorio 6 - Práctica 2: Búfer Circular Productor/Consumidor
 * 
 * Autor: Adrian Penagos
 * Fecha: Septiembre 2025
 * Propósito: Implementar cola FIFO thread-safe usando mutex + condition variables
 *           Evitar busy waiting y garantizar no pérdida de datos
 */
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>

constexpr std::size_t Q = 1024;

struct Ring {
    int buf[Q];
    std::size_t head=0, tail=0, count=0;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
    pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
    bool stop=false;
};

void ring_push(Ring* r, int v){
    pthread_mutex_lock(&r->m);
    while(r->count==Q && !r->stop) {
        pthread_cond_wait(&r->not_full,&r->m);
    }
    if(!r->stop){
        r->buf[r->head]=v;
        r->head=(r->head+1)%Q;
        r->count++;
        pthread_cond_signal(&r->not_empty);
    }
    pthread_mutex_unlock(&r->m);
}

bool ring_pop(Ring* r, int* out){
    pthread_mutex_lock(&r->m);
    while(r->count==0 && !r->stop){
        pthread_cond_wait(&r->not_empty,&r->m);
    }
    if(r->count==0 && r->stop){
        pthread_mutex_unlock(&r->m);
        return false;
    }
    *out=r->buf[r->tail];
    r->tail=(r->tail+1)%Q;
    r->count--;
    pthread_cond_signal(&r->not_full);
    pthread_mutex_unlock(&r->m);
    return true;
}

struct ProducerArgs { Ring* r; int N; };
struct ConsumerArgs { Ring* r; int id; };

void* producer(void* p){
    auto* a = static_cast<ProducerArgs*>(p);
    for(int i=0;i<a->N;i++){
        ring_push(a->r, i);
    }
    // al terminar no cierra el ring aquí
    return nullptr;
}

void* consumer(void* p){
    auto* a = static_cast<ConsumerArgs*>(p);
    int v;
    while(ring_pop(a->r,&v)){
        // aquí procesas el valor (ejemplo: imprimir)
        // printf("Consumer %d got %d\n", a->id, v);
    }
    return nullptr;
}

int main(int argc,char**argv){
    int N = (argc>1)? std::atoi(argv[1]) : 10000;
    int C = (argc>2)? std::atoi(argv[2]) : 2;

    Ring r;

    pthread_t prod;
    ProducerArgs pa{&r,N};
    pthread_create(&prod,nullptr,producer,&pa);

    std::vector<pthread_t> cons(C);
    std::vector<ConsumerArgs> ca(C);
    for(int i=0;i<C;i++){
        ca[i].r=&r; ca[i].id=i;
        pthread_create(&cons[i],nullptr,consumer,&ca[i]);
    }

    pthread_join(prod,nullptr);

    // shutdown limpio: marcar stop y despertar a todos
    pthread_mutex_lock(&r.m);
    r.stop=true;
    pthread_cond_broadcast(&r.not_empty);
    pthread_cond_broadcast(&r.not_full);
    pthread_mutex_unlock(&r.m);

    for(int i=0;i<C;i++) pthread_join(cons[i],nullptr);

    printf("Done. Produced %d items\n",N);
    return 0;
}
