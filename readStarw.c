
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define timeSpeed 0.05				// mniej = szybciej


pthread_mutex_t mutexR;			//zmienna blokujaca liczbe readerow
pthread_mutex_t mutex;			//zmienna blokujaca biblioteke oraz liczbe writerow
pthread_mutex_t mutexQW;			//zmienna blokujaca biblioteke oraz liczbe writerow w kolejce
pthread_cond_t cond;			//zmiena warunkowa wywolywana gdzy zmienie sie liczba osob w bibliotece
pthread_cond_t condW;			//zmiena warunkowa wywolywana gdzy zmienie sie liczba pisarzy w kolejce
pthread_mutex_t mutexcheck;		//mutex do sprawdzania wejsc
int W=0,R=0;					//ilosci writerow i readerow
int * check=NULL;				//tablica do sprawdzania ilosci wejsc osoby
int coutR = 0;					// ilosc czytelnikow w bibliotece
int coutW = 0;					// ilosc pisarzy w bibliotece
int end = 0;					//zmienna bezpiecznego zakonczenia
int queueW = 0;					//ilosc pisarzy w kolejce

void sig_handler_sigusr1(int signum){
	printf("\n  SAVE TERMINATE \n\n");
	end=1; // pootrzymaniu SIGUSER1 zatrzymanie petli
	return;
}



void *writer(void *arg) {
    int i = *(int*)arg;
    free(arg);
    int zarodek;
    time_t tt;
    zarodek = time(&tt);
    srand(zarodek);
    while (end==0) {
        pthread_mutex_lock(&mutexQW);
		queueW++;
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutex);
		while(coutR>0)
			pthread_cond_wait(&cond,&mutex);
        coutW++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        usleep((rand()%1000000+500000)*timeSpeed);
        coutW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        pthread_mutex_lock(&mutexQW);
		queueW++;
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutexQW);
		queueW--;
		pthread_cond_broadcast(&condW);
        pthread_mutex_unlock(&mutexQW);
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
    while (end==0) {
		pthread_mutex_lock(&mutex);
		while(end==0&&queueW>0)
			pthread_cond_wait(&condW,&mutex);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
        coutR++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_unlock(&mutexR);
        usleep((rand()%8000000+500000)*timeSpeed);
        pthread_mutex_lock(&mutexR);
        coutR--;
		pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutexR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n",R,W, coutR, coutW,i);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        usleep((rand()%8000000+5000000)*timeSpeed);
    }
}


int main(int argc, char *argv[]) {
	if(argc<3) return -1;
    int i=0;
    W = atoi(argv[1]);
	R = atoi(argv[2]);
	signal(SIGUSR1,sig_handler_sigusr1);
    check=(int*)malloc(sizeof(int)*(W+R));
    pthread_t * tab=(pthread_t*)malloc(sizeof(pthread_t)*(W+R));
	for(i=0;i<W+R;i++)
	{
		check[i]=0;
		tab[i]=0;

	}
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexQW, NULL);
    pthread_mutex_init(&mutexcheck, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&condW, NULL);

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
	for(i=0;i<W+R;i++)
		printf("%d[%d]",check[i],i);
	printf("\n");

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexQW);
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexcheck);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&condW);
	free(check);
	free(tab);
    return 0;
}
