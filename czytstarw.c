
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define X 400

int check[X];
pthread_mutex_t mutexR;
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_mutex_t mutexcheck;
static volatile sig_atomic_t sig = 0;

int coutR = 0;// ilosc czytelnikow w bibliotece
int coutW = 0;// ilosc pisarzy w bibliotece

void sig_handler_sigusr1(int signum){
    for(int i=0;i<X;i++)
        printf("\t%d[%d]",check[i],i);
    printf("\n");
}

void *writer(void *arg) {
    int i = *(int*)arg;
    free(arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        while (coutR > 0)
            pthread_cond_wait(&cond, &mutex);
        coutW++;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\t%d\n", coutR, coutW,i);
        sleep(1);
        coutW--;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\t%d\n", coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;

        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        sleep(5);
    }
}

void *reader(void *arg) {
    int i= *(int*)arg;
    free(arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
        coutR++;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\t%d\n", coutR, coutW,i);
        pthread_mutex_unlock(&mutexR);
        sleep(1);
        pthread_mutex_lock(&mutexR);
        coutR--;
        pthread_mutex_unlock(&mutexR);
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\t%d\n", coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        printf("\n");
        sleep(5);
    }
}


int main(int argc, char *argv[]) {
    /*
	if(argc<3) return -1;
    int i=0;
	int W = atoi(argv[1]);
	int R = atoi(argv[2]);
     */
    int i=0;
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexcheck, NULL);
    pthread_cond_init(&cond, NULL);
    signal(SIGUSR1,sig_handler_sigusr1);
    pthread_t tab[X];
    for ( i = 0; i < X; i++) {
        int * a = (int*)malloc(sizeof(int));
        *a=i;
        if (i % 4 == 0) {
            if (pthread_create(&tab[i], NULL, &writer, a) != 0) {
                perror("Failed to create thread");
            }
        } else {
            if (pthread_create(&tab[i], NULL, &reader, a) != 0) {
                perror("Failed to create thread");
            }
        }
    }
    for ( i = 0; i < X; i++) {
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }


    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexcheck);
    pthread_cond_destroy(&cond);
	for(int i=0;i<X;i++)
        printf("\t%d[%d]",check[i],i);
    printf("\n");
    return 0;
}
