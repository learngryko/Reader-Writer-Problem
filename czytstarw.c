
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


pthread_mutex_t mutexR;
pthread_mutex_t mutex;
pthread_cond_t cond;

int coutR = 0;// ilosc czytelnikow w bibliotece
int coutW = 0;// ilosc pisarzy w bibliotece


void *writer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (coutR > 0)
            pthread_cond_wait(&cond, &mutex);
        coutW++;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\n", coutR, coutW);
        sleep(1);
        coutW--;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\n", coutR, coutW);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}

void *reader(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
        coutR++;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\n", coutR, coutW);
        pthread_mutex_unlock(&mutexR);
        sleep(1);
        pthread_mutex_lock(&mutexR);
        coutR--;
        printf("ReaderQ: 4 WriterQ: 2 [in: R:%d W:%d]\n", coutR, coutW);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutexR);
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
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t tab[20];
    for (int i = 0; i < 20; i++) {
        if (i % 4 == 0) {
            if (pthread_create(&tab[i], NULL, &writer, NULL) != 0) {
                perror("Failed to create thread");
            }
        } else {
            if (pthread_create(&tab[i], NULL, &reader, NULL) != 0) {
                perror("Failed to create thread");
            }
        }
    }
    for (int i = 0; i < 20; i++) {
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexR);
    pthread_cond_destroy(&cond);

    return 0;
}
