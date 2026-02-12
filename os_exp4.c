#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define ITERATIONS 500000

int count = 0;
pthread_mutex_t mutex;
sem_t semaphore;

void* producer_no_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        count++;
    }
    return NULL;
}

void* consumer_no_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        count--;
    }
    return NULL;
}

void* producer_with_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer_with_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        count--;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t producer, consumer;
    
    printf("\nPART 1: WITHOUT MUTEX LOCK\n");
    
    count = 0;
    pthread_create(&producer, NULL, producer_no_sync, NULL);
    pthread_create(&consumer, NULL, consumer_no_sync, NULL);
    
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    printf("Final count: %d\n\n", count);
    
    printf("PART 2: WITH MUTEX LOCK\n");
    
    count = 0;
    
    pthread_mutex_init(&mutex, NULL);
    sem_init(&semaphore, 0, 1);
    
    pthread_create(&producer, NULL, producer_with_sync, NULL);
    pthread_create(&consumer, NULL, consumer_with_sync, NULL);
    
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    printf("Final count: %d\n\n", count);
    
    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaphore);
    
    return 0;
}
