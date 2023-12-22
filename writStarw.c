#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define TIMESPEED 0.5                // less = faster

pthread_mutex_t mutexQR;            // mutex blocking the readers' queue
pthread_mutex_t mutexQW;            // mutex blocking the writers' queue
pthread_mutex_t mutexR;             // mutex blocking the number of readers
pthread_mutex_t mutex;              // mutex blocking the library and the number of writers
pthread_cond_t cond;                // condition variable triggered when the number of people in the library changes
pthread_mutex_t mutexcheck;         // mutex for checking entries
int W = 0, R = 0;                   // number of writers and readers
int *check = NULL;                  // array to check the number of entries per person
int coutR = 0;                      // number of readers in the library
int coutW = 0;                      // number of writers in the library
int end = 0;                        // variable for safe termination
int queueW = 0;                     // writers' queue
int queueR = 0;                     // readers' queue

void sig_handler_sigusr1(int signum) {
    printf("\n  SAVE TERMINATE \n\n");
    end = 1; // stop the loop upon receiving SIGUSER1
    return;
}

void *writer(void *arg) {
    int i = *(int *) arg; // assign thread id value to i
    free(arg);    // clear memory allocated in main
    int seed = 0;
    time_t tt = 0;
    seed = time(&tt);
    srand(seed);
    while (end == 0) {
        pthread_mutex_lock(&mutexQW);        // lock the library
        queueW++;    // increase the queue
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutex);
        while (coutR > 0 && end == 0) // wait for change in number of readers in library
            pthread_cond_wait(&cond, &mutex);
        coutW++;  // increase number of writers in the library 
        pthread_mutex_unlock(&mutexQW);
        queueW--;    // decrease the queue
        pthread_mutex_unlock(&mutexQW);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        usleep((rand() % 1000000 + 500000) * TIMESPEED);    // writing
        coutW--;    // decrease number of writers in the library
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;        // update number of visits
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);        // wait to re-enter the queue
    }
}

void *reader(void *arg) {
    int i = *(int *) arg;
    free(arg);
    int seed = 0;
    time_t tt = 0;
    seed = time(&tt);
    srand(seed);
    while (end == 0) {
        pthread_mutex_lock(&mutexQR);
        queueR++; // add reader to the queue
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        pthread_mutex_lock(&mutex); // wait for no writer in the library
        pthread_mutex_unlock(&mutex); //
        pthread_mutex_lock(&mutexR);
        coutR++;    // increase number of readers in the library
        pthread_mutex_unlock(&mutexR);
        pthread_mutex_lock(&mutexQR);
        queueR--;        // decrease readers' queue
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        usleep((rand() % 8000000 + 500000) * TIMESPEED);  // reading
        pthread_mutex_lock(&mutexR);
        coutR--;    // decrease number of readers in the library
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexR);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;        // increase number of thread visits
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);        // signal about the change in the number of readers in the library
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);    // wait to re-enter the queue
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) return -1; // exit if we don't receive 2 arguments
    int i = 0;
    W = atoi(argv[1]);    // assign number of writers
    R = atoi(argv[2]);    // assign number of readers
    signal(SIGUSR1, sig_handler_sigusr1); // register SIGUSR1 signal for safe exit without losing memory
    check = (int *) malloc(sizeof(int) * (W + R));   // allocate memory for the array counting visits
    pthread_t *tab = (pthread_t *) malloc(sizeof(pthread_t) * (W + R));  // allocate memory for the thread array
    for (i = 0; i < W + R; i++) {  // initialize array values
        check[i] = 0;
        tab[i] = 0;
    }
    pthread_mutex_init(&mutexR, NULL);        // initialize mutexes and condition variables
    pthread_mutex_init(&mutexQR, NULL);
    pthread_mutex_init(&mutexQW, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexcheck, NULL);
    pthread_cond_init(&cond, NULL);

    for (i = 0; i < W; i++) { // create writer threads
        int *a = (int *) malloc(sizeof(int)); // allocate memory for thread id // freed in routine
        *a = i; // assign id to variable a
        if (pthread_create(&tab[i], NULL, &writer, a) != 0) {
            perror("Failed to create thread");
        }
    }
    for (i = W; i < W + R; i++) { // create reader threads
        int *a = (int *) malloc(sizeof(int));
        *a = i;
        if (pthread_create(&tab[i], NULL, &reader, a) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < W + R; i++) { // join all threads
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    for (i = 0; i < W + R; i++)
        printf("%d[%d]", check[i], i);
    printf("\n");
    pthread_mutex_destroy(&mutex);        // remove mutexes and condition variables
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexQR);
    pthread_mutex_destroy(&mutexQW);
    pthread_mutex_destroy(&mutexcheck);
    pthread_cond_destroy(&cond);
    free(check); // free memory
    free(tab);
    return 0;
}
