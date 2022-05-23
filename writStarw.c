
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define timeSpeed 0.02// mniej = szybciej


pthread_mutex_t mutexR;
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_mutex_t mutexcheck;
int W=0,R=0;
int * check=NULL;
int coutR = 0;// ilosc czytelnikow w bibliotece
int coutW = 0;// ilosc pisarzy w bibliotece

/*
void sig_handler_sigusr1(int signum){
    for(int i=0;i<X;i++)
        printf("\t%d[%d]",check[i],i);
    printf("\n");
}
*/


void *writer(void *arg) {
    int zarodek;
    time_t tt;
    zarodek = time(&tt);
    srand(zarodek);
    int i = *(int*)arg;
    free(arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        while (coutR > 0)
            pthread_cond_wait(&cond, &mutex);
        coutW++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        usleep((rand()%8000000+500000)*timeSpeed);
        coutW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        usleep((rand()%8000000+5000000)*timeSpeed);
    }
}

void *reader(void *arg) {
    int i= *(int*)arg;
    free(arg);
    int zarodek;
    time_t tt;
    zarodek = time(&tt);
    srand(zarodek);
    while (1) {
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
        coutR++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_unlock(&mutexR);
        usleep((rand()%8000000+500000)*timeSpeed);
        pthread_mutex_lock(&mutexR);
        coutR--;
        pthread_mutex_unlock(&mutexR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        printf("\n");
        usleep((rand()%8000000+5000000)*timeSpeed);

    }
}


int main(int argc, char *argv[]) {
	if(argc<3) return -1;
    int i=0;
    W = atoi(argv[1]);
	R = atoi(argv[2]);
    //signal(SIGUSR1,sig_handler_sigusr1);
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexcheck, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t * tab=(pthread_t*)malloc(sizeof(pthread_t)*(W+R));
    check=(int*)malloc(sizeof(int)*(W+R));

    for ( i = 0; i < W; i++) {
        int *a = (int *) malloc(sizeof(int));
        *a=i;
        if (pthread_create(&tab[i], NULL, &writer, a) != 0) {
            perror("Failed to create thread");
        }
    }
    for ( i = W; i < W+R; i++) {
        int *a = (int *) malloc(sizeof(int));\
        *a=i;
        if (pthread_create(&tab[i], NULL, &reader, a) != 0) {
            perror("Failed to create thread");
        }
    }

    for ( i = 0; i < W+R; i++) {
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }


    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexcheck);
    pthread_cond_destroy(&cond);
    return 0;
}
