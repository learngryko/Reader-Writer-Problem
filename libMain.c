
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

struct node {
	pthread_t * thread;
	struct node * next;
};


pthread_mutex_t mutex;

void* czytelnik(void *arg) {
    int *i = arg;
    printf("%d\n",*i);
    free(i);
}

int dodajCzyt(int *a,pthread_t *p){
	if (pthread_create(p, NULL, &czytelnik, a) != 0) {
        free(a);
        return -1;
    }
	return 0;
}

int main(int argc, char* argv[]) {
	if(argc<3) return -1;
	int W = atoi(argv[1]);
	int R = atoi(argv[2]);
    printf("%d \t %d\n",sizeof(pthread_t),sizeof(pthread_t*));
    pthread_t * buff;
	printf("%d \t %d\n",W,R);
    struct node * head = malloc(sizeof(struct node));
    head->thread=NULL;
    head->next=NULL;
	pthread_t p,p2;
    pthread_mutex_init(&mutex, NULL);
	int *x;
	x=malloc(sizeof(int));
	*x=2;
	dodajCzyt(x,&p);
	x=malloc(sizeof(int));
	*x=3;
	dodajCzyt(x,&p2);

    if (pthread_join(p, NULL) != 0) {
        return 5;
    }
    if (pthread_join(p2, NULL) != 0) {
        return 2;
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
