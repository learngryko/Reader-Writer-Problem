
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


pthread_mutex_t mutex;



void* czytelnik(void *arg) {
    int *i = arg;
    printf("%d\n",*i);
    free(i);
}


int main(int argc, char* argv[]) {
    pthread_t p1,p2;
    pthread_mutex_init(&mutex, NULL);
    int * x = malloc(sizeof(int));
    *x=2;


    if (pthread_create(&p1, NULL, &czytelnik, x) != 0) {
        free(x);
        return 2;
    }
    x = malloc(sizeof(int));
    *x=3;

    if (pthread_create(&p2, NULL, &czytelnik, x) != 0) {
        free(x);
        return 2;
    }
    if (pthread_join(p1, NULL) != 0) {
        return 5;
    }

    if (pthread_join(p2, NULL) != 0) {
        return 5;
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}