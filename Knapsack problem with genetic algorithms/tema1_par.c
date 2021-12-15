#include <stdlib.h>
#include "genetic_algorithm.h"
#include <stdio.h>
#include <pthread.h>

int main(int argc, char *argv[]) {

	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, argc, argv)) {
		return 0;
	}

	// initializing variable for threads
	int P = atoi(argv[3]);
	pthread_t tid[P];

	genetic_algorithm_arguments arguments[P];
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, P);

	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *tmp = NULL;

	// creating threads
	for(int i = 0; i < P; i++) {
		arguments[i].objects = objects;
		arguments[i].object_count = object_count;
		arguments[i].sack_capacity = sack_capacity;
		arguments[i].generations_count = generations_count;
		arguments[i].id = i;
		arguments[i].no_threads = P;
		arguments[i].current_generation = current_generation;
		arguments[i].next_generation = next_generation;
		arguments[i].tmp = tmp;
		arguments[i].barrier = &barrier;
		int d = pthread_create(&tid[i], NULL, run_genetic_algorithm_parallel, &arguments[i]);
		if (d < 0) {
			printf("Eroare la create\n");
		}
	}

	// waiting threads
	void *status;
	for(int i = 0; i < P; i++) {
		int r = pthread_join(tid[i], &status);
		if (r < 0) {
			printf("Eroare la join\n");
		}
	}
	
	free(objects);

	pthread_barrier_destroy(&barrier);
	// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);
	return 0;
}
