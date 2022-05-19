
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


pthread_mutex_t mutex;


void* reader(void *arg) {
    int r = 1000000+rand() % 500000;
	int	i = *(int*)arg;
    printf("%d\n",i);
    usleep(r);
}

void* writer(void *arg) {
    pthread_mutex_lock(&mutex);
    int r = 1000000+rand() % 500000;
    int	i = *(int*)arg;
    printf("%d\n",i);
    usleep(r);
    pthread_mutex_unlock(&mutex);
}

int addReader(int * a,pthread_t *p){
    int c=*a;
	if (pthread_create(p, NULL, &reader, a) != 0) {
        free(a);
        return -1;
    }
	pthread_join(*p,NULL);
	return c;
}
int addWriter(int * a,pthread_t *p){
    int c=*a;
	if (pthread_create(p, NULL, &writer, a) != 0) {
        free(a);
        return -1;
    }
	pthread_join(*p,NULL);
	return c;
}

int main(int argc, char* argv[]) {
	if(argc<3) return -1;
	int W = atoi(argv[1]);
	int R = atoi(argv[2]);
    pthread_mutex_init(&mutex, NULL);
	pthread_t ** tab = (pthread_t **)malloc(sizeof(pthread_t*)*(W+R));
	int i=0;
	for(i=0;i<W+R;i++){
		tab[i]=(pthread_t *)malloc(sizeof(pthread_t));
	}
    int *a=(int *)malloc(sizeof(int));
    int z=0;
    int * kolejka = (int*)malloc(sizeof(int)*(W+R));
    int * koniec = NULL,* poczatek = NULL;
    koniec = &kolejka[0];
    poczatek = &kolejka[W+R-1];
    for(i=0;i<W+R;i++){
        kolejka[i]=i;
    }
    while(i<10){
        for(poczatek;poczatek==koniec;poczatek++)
        {
            if(poczatek==kolejka[W+R-1])
                poczatek=kolejka[0];




        }
        i++;
    }


	for(i=0;i<W+R;i++){
		free(tab[i]);
	}
	free(tab);


    pthread_mutex_destroy(&mutex);
    return 0;
}
