#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define TIMESPEED 0.5                // less = faster

pthread_mutex_t mutexR;            // mutex blocking the number of readers
pthread_mutex_t mutex;             // mutex blocking the library and the number of writers
pthread_mutex_t mutexQW;           // mutex blocking the queue of writers
pthread_mutex_t mutexQR;           // mutex blocking the queue of readers
pthread_mutex_t mutexcheck;        // mutex for checking entries
pthread_cond_t cond;               // condition variable responding to change in number of writers
pthread_cond_t condR;              // condition variable responding to change in number of readers
int W = 0, R = 0;                  // number of writers and readers
int *check = NULL;                 // array to check the number of entries per person
int coutR = 0;                     // number of readers in the library
int coutW = 0;                     // number of writers in the library
int end = 0;                       // variable for safe termination
int queueW = 0;                    // number of threads in the writers' queue
int queueR = 0;                    // number of threads in the readers' queue

void sig_handler_sigusr1(int signum) {
    printf("\n  SAVE TERMINATE \n\n");
    end = 1; // stop the loop upon receiving SIGUSER1
    for (int i = 10; i > 0; i--) {
        sleep(1 * TIMESPEED);
        printf("Terminate in %d\n", i);
        pthread_cond_broadcast(&condR);
        pthread_cond_broadcast(&cond);
    }
    return;
}

void *reader(void *arg) {
    int i = *(int *) arg; // convert and save thread id (void*) as i (int)
    free(arg); // free the space allocated for the arg parameter by main
    int seed;
    time_t tt;
    seed = time(&tt);
    srand(seed);
    while (end == 0) {
        pthread_mutex_lock(&mutexQR);
        queueR++;    // increase the queue of readers
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        pthread_mutex_lock(&mutex); // wait for no writers in the library
        while (queueW > 0 && end == 0)
            pthread_cond_wait(&cond, &mutex);  // wait for change in number of writers in the queue to the library
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
        coutR++; // increase number of readers in the library
        pthread_mutex_unlock(&mutexR);
        pthread_mutex_lock(&mutexQR);
        queueR--; // decrease readers' queue
        pthread_mutex_unlock(&mutexQR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        usleep((rand() % 5000000 + 500000) * TIMESPEED);  // reading
        pthread_mutex_lock(&mutexR);
        coutR--;    // decrease number of readers in the library
        pthread_mutex_unlock(&mutexR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_cond_signal(&condR); // signal about exiting the library
        pthread_mutex_lock(&mutexcheck);
        check[i]++;        // increase number of visits to the library
        pthread_mutex_unlock(&mutexcheck);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED); // wait before re-entering the queue
    }
}

void *writer(void *arg) {
    int i = *(int *) arg;
    free(arg);
    int seed = 0;
    time_t tt = 0;
    seed = time(&tt);
    srand(seed);
    while (end == 0) {
        pthread_mutex_lock(&mutexQW);
        queueW++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutex);
        while (coutR > 0 && end == 0)
            pthread_cond_wait(&condR, &mutex); // wait for signal of change in number of readers in the library
        coutW++;
        pthread_mutex_lock(&mutexQW);
        queueW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);
        coutW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(
                &cond); // send signal to all waiting readers after changing number of writers in the library
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) return -1;
    int i = 0;
    W = atoi(argv[1]);    // read number of writers
    R = atoi(argv[2]);    // read number of readers
    signal(SIGUSR1, sig_handler_sigusr1);    // register signal
    check = (int *) malloc(sizeof(int) * (W + R));
    pthread_t *tab = (pthread_t *) malloc(sizeof(pthread_t) * (W + R));        // array of all threads
    for (i = 0; i < W + R; i++) { // initialize arrays
        check[i] = 0;  // initialize array for checking entries
        tab[i] = 0;        // initialize thread array
    }
    pthread_cond_init(&cond, NULL); // initialize mutexes and condition variables
    pthread_cond_init(&condR, NULL);
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexQW, NULL);
    pthread_mutex_init(&mutexQR, NULL);
    pthread_mutex_init(&mutexcheck, NULL);

    for (i = 0; i < W; i++) { // create writer threads
        int *a = (int *) malloc(sizeof(int)); // allocate memory for thread id
        *a = i; // assign id for thread
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

    for (i = 0; i < W + R; i++) { // start threads
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    for (i = 0; i < W + R; i++) // print number of entries to the library
        printf("%d[%d]", check[i], i);
    printf("\n");

    pthread_cond_destroy(&cond); // destroy condition variables and mutexes
    pthread_cond_destroy(&condR);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexQW);
    pthread_mutex_destroy(&mutexQR);
    pthread_mutex_destroy(&mutexcheck);
    free(check);        // clear occurrences array
    free(tab);            // clear thread array
    return 0;
}
