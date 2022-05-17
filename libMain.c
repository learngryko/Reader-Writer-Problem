
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct node {
	pthread_t * thread;
	struct node * next;
};


pthread_mutex_t mutex;

void* reader(void *arg) {
	int	*i = arg;
    printf("%d\n",*i);
}

void* writer(void *arg) {
	int	*i = arg;
    printf("%d\n",*i);
}

int addReader(int * a,pthread_t *p){
	if (pthread_create(p, NULL, &reader, a) != 0) {
        return -1;
    }
	pthread_join(*p,NULL);
	return 0;
}
int addWriter(int * a,pthread_t *p){
	if (pthread_create(p, NULL, &writer, a) != 0) {
        return -1;
    }
	pthread_join(*p,NULL);
	return 0;
}

int main(int argc, char* argv[]) {
	if(argc<3) return -1;
	int W = atoi(argv[1]);
	int R = atoi(argv[2]);
    pthread_mutex_init(&mutex, NULL);
	pthread_t ** tab_W = (pthread_t **)malloc(sizeof(pthread_t*)*W);
	pthread_t ** tab_R = (pthread_t **)malloc(sizeof(pthread_t*)*R);
	int i=0;
	for(i=0;i<W;i++){
		tab_W[i]=(pthread_t *)malloc(sizeof(pthread_t));
	}
	for(i=0;i<R;i++){
		tab_R[i]=(pthread_t *)malloc(sizeof(pthread_t));
	}

	int ** c = (int**)malloc(sizeof(int*)*(W+R));
	for(i=0;i<W+R;i++){
		c[i]=(int *)malloc(sizeof(int));
		*c[i]=i;
	}

	addReader(c[0],tab_R[0]);
	addReader(c[1],tab_R[1]);
	addReader(c[2],tab_R[2]);
	sleep(2);
	printf("ASDASDSAD\n");







	for(i=0;i<W+R;i++){
		free(c[i]);
	}
	free(c);
	for(i=0;i<W;i++){
		free(tab_W[i]);
	}
	for(i=0;i<R;i++){
		free(tab_R[i]);
	}
	free(tab_W);
	free(tab_R);


    pthread_mutex_destroy(&mutex);
    return 0;
}
