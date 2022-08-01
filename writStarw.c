
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define TIMESPEED 0.5                // mniej = szybciej


pthread_mutex_t mutexQR;            //zmienna blokujaca kolejke readerow
pthread_mutex_t mutexQW;            //zmienna blokujaca kolejke writerow
pthread_mutex_t mutexR;            //zmienna blokujaca liczbe readerow
pthread_mutex_t mutex;            //zmienna blokujaca biblioteke oraz liczbe writerow
pthread_cond_t cond;            //zmiena warunkowa wywolywana gdzy zmienie sie liczba osob w bibliotece
pthread_mutex_t mutexcheck;        //mutex do sprawdzania wejsc
int W = 0, R = 0;                    //ilsoci writerow i readerow
int *check = NULL;                //tablica do sprawdzania ilosci wejsc osoby
int coutR = 0;                    // ilosc czytelnikow w bibliotece
int coutW = 0;                    // ilosc pisarzy w bibliotece
int end = 0;                    //zmienna bezpiecznego zakonczenia
int queueW = 0;                    //kolejka writerow
int queueR = 0;                    //kolejka readerow


void sig_handler_sigusr1(int signum) {
    printf("\n  SAVE TERMINATE \n\n");
    end = 1; // pootrzymaniu SIGUSER1 zatrzymanie petli
    return;
}

void *writer(void *arg) {
    int i = *(int *) arg; // przypisanie wartosci id watku dla i
    free(arg);    //wyczyszczenie pamieci zamalokowanej w main
    int zarodek = 0;
    time_t tt = 0;
    zarodek = time(&tt);
    srand(zarodek);
    while (end == 0) {
        pthread_mutex_lock(&mutexQW);        //zablokowanie biblioteki
        queueW++;    //zwiekszenie kolejki
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQW);
        pthread_mutex_lock(&mutex);
        while (coutR > 0 && end == 0) //oczekiwanie na zmiane ilosci readerow w bibliotece
            pthread_cond_wait(&cond, &mutex);
        coutW++;  //zwiekszenie ilosci writerow w bibliotece 
        pthread_mutex_unlock(&mutexQW);
        queueW--;    //zmniejszenie kolejki
        pthread_mutex_unlock(&mutexQW);
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        usleep((rand() % 1000000 + 500000) * TIMESPEED);    //pisanie
        coutW--;    //zmniejszenie ilosci  writerow w bibliotece
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;        //aktulizacja ilosci odwiedzin
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);        //oczekiwanie na ponowne wejcie do kolejki
    }
}

void *reader(void *arg) {
    int i = *(int *) arg;
    free(arg);
    int zarodek = 0;
    time_t tt = 0;
    zarodek = time(&tt);
    srand(zarodek);
    while (end == 0) {
        pthread_mutex_lock(&mutexQR);
        queueR++; //dodanie readera do kolejki
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        pthread_mutex_lock(&mutex); // oczekiwaie na brak pisarza w bibliotece
        pthread_mutex_unlock(&mutex); //
        pthread_mutex_lock(&mutexR);
        coutR++;    //zwiekszenie readerow w bibliotece
        pthread_mutex_unlock(&mutexR);
        pthread_mutex_lock(&mutexQR);
        queueR--;        //zmniejszenie kolejki readerow
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexQR);
        usleep((rand() % 8000000 + 500000) * TIMESPEED);  //czytanie
        pthread_mutex_lock(&mutexR);
        coutR--;    //zmniejszenie ilosci readerow w bibliotece
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\t%d\n", queueR, queueW, coutR, coutW, i);
        pthread_mutex_unlock(&mutexR);
        pthread_mutex_lock(&mutexcheck);
        check[i]++;        //zwiekszenie ilosci odwiedzin watku
        pthread_mutex_unlock(&mutexcheck);
        pthread_cond_signal(&cond);        //wyslanie sygnalu o zmianie ilosci readerow w bibliotece
        usleep((rand() % 8000000 + 5000000) * TIMESPEED);    //oczekiwanie na ponowne wejscie do kolejki

    }
}


int main(int argc, char *argv[]) {
    if (argc < 3) return -1; // wyjsce jesli nie dostaniemy 2 argumentow
    int i = 0;
    W = atoi(argv[1]);    //przypisanie liczy writerow
    R = atoi(argv[2]);    //przypisanie liczy readerow
    signal(SIGUSR1, sig_handler_sigusr1); //rejestracja sygnalu SIGUSR1 by bezpiecznie wyjsc nie tracac pamieci
    check = (int *) malloc(sizeof(int) * (W + R));   //rezerwacja pamieci dla tablicy zliczajacej ilosc odwiedzin
    pthread_t *tab = (pthread_t *) malloc(sizeof(pthread_t) * (W + R));  //rezerwacja pamieci dla tablicy watkow
    for (i = 0; i < W + R; i++) {  //zainicjalizowanie wartosci tablic
        check[i] = 0;
        tab[i] = 0;
    }
    pthread_mutex_init(&mutexR, NULL);        //inicjalizacja mutexow i zmiennych warunkowych
    pthread_mutex_init(&mutexQR, NULL);
    pthread_mutex_init(&mutexQW, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexcheck, NULL);
    pthread_cond_init(&cond, NULL);

    for (i = 0; i < W; i++) { //utworzenie watkow writerow
        int *a = (int *) malloc(sizeof(int)); //rezerwacja pamieci na id watku // zwolniona w rutynie
        *a = i; //przypisanie id do zmiennaj a
        if (pthread_create(&tab[i], NULL, &writer, a) != 0) {
            perror("Failed to create thread");
        }
    }
    for (i = W; i < W + R; i++) { // utworzenie watkow readerow
        int *a = (int *) malloc(sizeof(int));
        *a = i;
        if (pthread_create(&tab[i], NULL, &reader, a) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < W + R; i++) { // dolaczenie wszystkicj watkow
        if (pthread_join(tab[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    for (i = 0; i < W + R; i++)
        printf("%d[%d]", check[i], i);
    printf("\n");
    pthread_mutex_destroy(&mutex);        //usuniecie mutexow i zmiennych warunkowych
    pthread_mutex_destroy(&mutexR);
    pthread_mutex_destroy(&mutexQR);
    pthread_mutex_destroy(&mutexQW);
    pthread_mutex_destroy(&mutexcheck);
    pthread_cond_destroy(&cond);
    free(check); //zwolnienie pamieci
    free(tab);
    return 0;
}
