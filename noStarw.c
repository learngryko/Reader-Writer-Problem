
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define TIMESPEED 0.05                // mniej == szybciej


pthread_mutex_t mutexR;            //zmienna blokujaca liczbe czytajacych
pthread_mutex_t mutex;            //zmienna blokujaca biblioteke oraz liczbe writerow
pthread_mutex_t mutexQW;            //zmienna blokujaca kolejke writerow
pthread_mutex_t mutexQR;            //zmienna blokujaca kolejke readerow
pthread_mutex_t mutexcheck;        //mutex do sprawdzania wejsc
pthread_cond_t cond;			//zmienna warunkowa reagujaca na zmiane ilosci writerow
pthread_cond_t condR;			//zmienna warunkowa reagujaca na zmiane ilosci readerow
int W = 0, R = 0;                    //ilosci writerow i readerow
int *check = NULL;                //tablica do sprawdzania ilosci wejsc osoby
int coutR = 0;                    // ilosc czytelnikow w bibliotece
int coutW = 0;                    // ilosc pisarzy w bibliotece
int end = 0;                    //zmienna bezpiecznego zakonczenia
int queueW=0;					//ilosc watkow w kolejce writerow
int queueR=0;					//ilosc watkow w kolejce readerow
int turn=0;


void sig_handler_sigusr1(int signum) {
    printf("\n  SAVE TERMINATE \n\n");
    end = 1; // pootrzymaniu SIGUSER1 zatrzymanie petli
	turn = 3;
	for(int i=10;i>0;i--){
		sleep(1*TIMESPEED);
		printf("Terminate in %d\n",i);
		pthread_cond_broadcast(&condR);
		pthread_cond_broadcast(&cond);
	}
    return;
}

void *reader(void *arg) {
    int i = *(int *) arg; // przekonwertowanie i zapisanie id(void*) watku na i(int)
    free(arg); //zwolnienie miejsca parametru arg zalokowaniego przez main
    int zarodek;
    time_t tt;
    zarodek = time(&tt);
    srand(zarodek);
    while (end == 0) {
        pthread_mutex_lock(&mutexQR);
	queueR++;	//zwiekszenie koleji readerow
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        pthread_mutex_lock(&mutex); // oczekiwanie na brak writerow w bibliotece
	while((coutW>0&&end==0) || turn==1)
		pthread_cond_wait(&cond,&mutex);  // oczekiwanie na zmiane ilosci writerow w kolejce do biblioteki
	pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexR);
	coutR++; //zwiekszenie ilosci readerow w bibliotece
        pthread_mutex_unlock(&mutexR);
	pthread_mutex_lock(&mutexQR);
	queueR--; //zmniejszenie kolejki readerow
        pthread_mutex_unlock(&mutexQR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        usleep((rand() % 5000000 + 500000) * TIMESPEED);  //czytanie
	pthread_mutex_lock(&mutexR);
	coutR--;	//zmniejszenie ilosci readerow w biliotece
        pthread_mutex_unlock(&mutexR);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
	if(W>0)
		turn = 1;
        pthread_cond_signal(&condR); //wyslanie sygnalu o wyjsciu z biblioteki;
	pthread_mutex_lock(&mutexcheck);
        check[i]++;		//powiekszenie ilosci odwiedzen w bibliotece
        pthread_mutex_unlock(&mutexcheck);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED); //odczekiwanie na ponowne wejsce do kolejki
    pthread_cond_broadcast(&condR);
    pthread_cond_broadcast(&cond);
    }
}


void *writer(void *arg) {
    int i = *(int *) arg;
    free(arg);
    int zarodek=0;
    time_t tt=0;
    zarodek = time(&tt);
    srand(zarodek);
    while (end == 0) {
        pthread_mutex_lock(&mutexQW);
	queueW++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutex);
	while(((coutR > 0  &&  end == 0) || turn == 0)&&R>0)
		pthread_cond_wait(&condR,&mutex); //oczekiwanie na sygnal od zmianie iloci readerow w biliotece
	coutW++;
	pthread_mutex_lock(&mutexQW);
	queueW--;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);
		coutW--;
	printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutex);
	if(R>0)
		turn  = 0;
	pthread_cond_broadcast(&cond); // wyslanie sygnalu do wszystkich oczekujacych readerow po zmianie ilosci writerow w bibliotece
        pthread_mutex_lock(&mutexcheck);
        check[i]++;
        pthread_mutex_unlock(&mutexcheck);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);
    pthread_cond_broadcast(&condR);
    pthread_cond_broadcast(&cond);
    }
}



int main(int argc, char *argv[]) {
    if (argc < 3) return -1;
    int i = 0;
    W = atoi(argv[1]);	//zaczytanie liczby pisarzy
    R = atoi(argv[2]);	//zaczytanie liczby czytaczy
    signal(SIGUSR1, sig_handler_sigusr1);	//zarejestrowanie sygnalu
    check = (int *) malloc(sizeof(int) * (W + R));
    pthread_t *tab = (pthread_t *) malloc(sizeof(pthread_t) * (W + R));		//tablica wszystkich watkow
    for (i = 0; i < W + R; i++) { // zainicjowanie tablic
        check[i] = 0;  //inicjalizacja tablicy sprawdzajacej ilosc wejsc
        tab[i] = 0;		//inicjalizajca tablicy watkow
    }
    pthread_cond_init(&cond, NULL); // inicjalizacje mutexow i zmiennych warunkowych
    pthread_cond_init(&condR, NULL);
    pthread_mutex_init(&mutexR, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexQW, NULL);
    pthread_mutex_init(&mutexQR, NULL);
    pthread_mutex_init(&mutexcheck, NULL);

    for (i = 0; i < W; i++) { //tworzenie watkow writerow
        int *a = (int *) malloc(sizeof(int)); // rezerwacja pamieci na id watku
        *a = i; // przypisanie id dla wadku
        if (pthread_create(&tab[i], NULL, &writer, a) != 0) {
            perror("Failed to create thread");
        }
    }
    for (i = W; i < W + R; i++) { //tworzenie watkow readerow
        int *a = (int *) malloc(sizeof(int));
        *a = i;
        if (pthread_create(&tab[i], NULL, &reader, a) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < W + R; i++) { //rozpoczecie watkow
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    for (i = 0; i < W + R; i++) // wypisanie ilosci wstapen do biblioteki
        printf("%d[%d]", check[i], i);
    printf("\n");

    pthread_cond_destroy(&cond); // zniszczenie zmiennych warunkowych i mutexow
    pthread_cond_destroy(&condR);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexQW);
    pthread_mutex_destroy(&mutexQR);
    pthread_mutex_destroy(&mutexcheck);
    free(check);		//wyczyszczenie tablicy wystopien
    free(tab);			//wyczyszczenie tablicy watkow
    return 0;
}
