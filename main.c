#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 2
#define MAX_ITEM_AMOUNT 10

typedef int buffer_item;

buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t empty;
sem_t full;
int inserts = 0;
int removes = 0;
int produced = 0;
int consumed = 0;

void *producer(void *param);
void *consumer(void *param);
int insert_item(buffer_item);
int remove_item(buffer_item*);

int main(int argc, char const *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Useage: <sleep time> <# producer threads> <# consumer threads>\n");
		exit(EXIT_FAILURE);
	}

	int sleepTime = atoi(argv[1]);
	int producerThreadsAmount = atoi(argv[2]);
	int consumerThreadsAmount = atoi(argv[3]);

	pthread_mutexattr_t mutexattr;
	if (pthread_mutexattr_init(&mutexattr)) {
		perror("pthread_mutexattr_init");
		exit(EXIT_FAILURE);
	}
	if (pthread_mutex_init(&mutex, &mutexattr)) {
		perror("pthread_mutex_init");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&empty, 0, 5)) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&full, 0, 0)) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	for (int i = 0; i < producerThreadsAmount; i += 1)  {
		pthread_t tid;
		pthread_attr_t attr;
		if (pthread_attr_init(&attr)) {
			perror("pthread_attr_init");
			exit(EXIT_FAILURE);
		}
		if (pthread_create(&tid, &attr, producer, NULL)) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < consumerThreadsAmount; i += 1)  {
		pthread_t tid;
		pthread_attr_t attr;
		if (pthread_attr_init(&attr)) {
			perror("pthread_attr_init");
			exit(EXIT_FAILURE);
		}
		if (pthread_create(&tid, &attr, consumer, NULL)) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	
	sleep(sleepTime);

	printf("Produced : %d\n", produced);
	printf("Consumed : %d\n", consumed);
	
	exit(EXIT_SUCCESS);
}

void *producer(void *param) {
	buffer_item random;
	int r;
	while (1) {
		r = rand() % 5;
		sleep(r);
		random = rand() % MAX_ITEM_AMOUNT;

		if (insert_item(random)) {
			fprintf(stderr, "insert_item");
			exit(EXIT_FAILURE);
		}
		printf("Producer produced %d \n", random);
		produced += random;
	}
}

void *consumer(void *param) {
	buffer_item random;
	int r;
	while (1) {
		r = rand() % 5;
		sleep(r);

		if (remove_item(&random)) {
			fprintf(stderr, "remove_item");
			exit(EXIT_FAILURE);
		}
		printf("Consumer consumed %d \n", random);
		consumed += random;
	}
}

int insert_item(buffer_item item) {
	if (sem_wait(&empty)) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}
	if (pthread_mutex_lock(&mutex)) {
		perror("pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}

	buffer[inserts++] = item;
	inserts = inserts % BUFFER_SIZE;

	if (pthread_mutex_unlock(&mutex)) {
		perror("pthread_mutex_unlock");
		exit(EXIT_FAILURE);
	}
	if (sem_post(&full)) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int remove_item(buffer_item *item) {
	if (sem_wait(&full)) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}
	if (pthread_mutex_lock(&mutex)) {
		perror("pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}

	*item = buffer[removes];
	buffer[removes++] = 0;
	removes = removes % BUFFER_SIZE;

	if (pthread_mutex_unlock(&mutex)) {
		perror("pthread_mutex_unlock");
		exit(EXIT_FAILURE);
	}
	if (sem_post(&empty)) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}

	return 0;
}
