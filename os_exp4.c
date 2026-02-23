#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define ITERATIONS 500000
#define QUEUE_SIZE 10

int count = 0;
pthread_mutex_t mutex;
pthread_mutex_t queue_mutex;

sem_t empty_slots;
sem_t full_slots;

int queue[QUEUE_SIZE];
int front = 0;
int rear = 0;
int queue_count = 0;

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

void* producer_queue_no_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        if (queue_count < QUEUE_SIZE) {
            queue[rear] = i;
            rear = (rear + 1) % QUEUE_SIZE;
            queue_count++;
            count++;
        }
    }
    return NULL;
}

void* consumer_queue_no_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        if (queue_count > 0) {
            int item = queue[front];
            front = (front + 1) % QUEUE_SIZE;
            queue_count--;
            count--;
        }
    }
    return NULL;
}

void* producer_queue_with_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&empty_slots);
        pthread_mutex_lock(&queue_mutex);
        
        queue[rear] = i;
        rear = (rear + 1) % QUEUE_SIZE;
        queue_count++;
        count++;
        
        pthread_mutex_unlock(&queue_mutex);
        sem_post(&full_slots);
    }
    return NULL;
}

void* consumer_queue_with_sync(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&queue_mutex);
        
        int item = queue[front];
        front = (front + 1) % QUEUE_SIZE;
        queue_count--;
        count--;
        
        pthread_mutex_unlock(&queue_mutex);
        sem_post(&empty_slots);
    }
    return NULL;
}

int main() {
    pthread_t producer, consumer;

    printf("PART 1: WITHOUT MUTEX LOCK\n");
    count = 0;
    pthread_create(&producer, NULL, producer_no_sync, NULL);
    pthread_create(&consumer, NULL, consumer_no_sync, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    printf("Final count (Expected 0, but will vary): %d\n\n", count);

    printf("PART 2: WITH MUTEX LOCK\n");
    count = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&producer, NULL, producer_with_sync, NULL);
    pthread_create(&consumer, NULL, consumer_with_sync, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    printf("Final count (Expected 0): %d\n\n", count);
    pthread_mutex_destroy(&mutex);

    printf("PART 3: QUEUE WITHOUT MUTEX LOCK\n");
    count = 0; front = 0; rear = 0; queue_count = 0;
    pthread_create(&producer, NULL, producer_queue_no_sync, NULL);
    pthread_create(&consumer, NULL, consumer_queue_no_sync, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    printf("Final count: %d\n", count);
    printf("Queue count (Likely corrupted): %d\n\n", queue_count);

    printf("PART 4: QUEUE WITH MUTEX AND SEMAPHORES\n");
    count = 0; front = 0; rear = 0; queue_count = 0;
    
    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&empty_slots, 0, QUEUE_SIZE);
    sem_init(&full_slots, 0, 0);
    
    pthread_create(&producer, NULL, producer_queue_with_sync, NULL);
    pthread_create(&consumer, NULL, consumer_queue_with_sync, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    printf("Final count (Expected 0): %d\n", count);
    printf("Queue count (Should be 0): %d\n\n", queue_count);
    
    pthread_mutex_destroy(&queue_mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);

    return 0;
}