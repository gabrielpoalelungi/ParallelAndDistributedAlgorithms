#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>
#include <math.h>

int minfnc(int a, int b) {

	if(a > b) {
		return b;
	} else {
		return a;
	}
}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count nr_of_threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
	printf("\n\n\n");

}
void print_generation_start_stop(const individual *generation, int start, int stop)
{
	for (int i = start; i < stop; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
		 	printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int start, int stop, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = start; i < stop; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
				// Instead of counting how many objects are in the sack in the cmpfunc, i did it
				// here in order to optimize the entire algorithm.
				generation[i].objects_count++;
			}
		}
		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	//int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {

		// I don't need the for loop anymore, I calculated first_count and second_count
		// in compute_fitness_function
		res = first->objects_count - second->objects_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void *run_genetic_algorithm_parallel(void *args)
{
	genetic_algorithm_arguments arg1 = *(genetic_algorithm_arguments *) args;
	int count, cursor;

	// set initial generation (composed of object_count individuals with a single item in the sack)
	int start_index = arg1.id * (double)arg1.object_count / arg1.no_threads;
	int stop_index = minfnc(arg1.object_count, (arg1.id + 1) * (double) arg1.object_count/arg1.no_threads);

	for (int i = start_index; i < stop_index; ++i) {
		arg1.current_generation[i].fitness = 0;
		arg1.current_generation[i].chromosomes = (int*) calloc(arg1.object_count, sizeof(int));
		arg1.current_generation[i].chromosomes[i] = 1;
		arg1.current_generation[i].index = i;
		arg1.current_generation[i].chromosome_length = arg1.object_count;

		arg1.next_generation[i].fitness = 0;
		arg1.next_generation[i].chromosomes = (int*) calloc(arg1.object_count, sizeof(int));
		arg1.next_generation[i].index = i;
		arg1.next_generation[i].chromosome_length = arg1.object_count;
	}
	// Barrier in order to wait for all threads to intialize generation
	pthread_barrier_wait(arg1.barrier);

	// iterate for each generation
	for (int k = 0; k < arg1.generations_count; ++k) {
		cursor = 0;
		
		// compute fitness and sort by it
		compute_fitness_function(arg1.objects, arg1.current_generation, start_index, stop_index, arg1.sack_capacity);
		// wait all threads to do compute_fitness_function
		pthread_barrier_wait(arg1.barrier);

		// do the sort just on one thread
		if(arg1.id == 0){
			qsort(arg1.current_generation, arg1.object_count, sizeof(individual), cmpfunc);
		}
		// others should wait until the sort is done
		pthread_barrier_wait(arg1.barrier);
		
		//keep first 30% children (elite children selection)
		count = arg1.object_count * 3 / 10;

		int start_index_copy_indiv = arg1.id * (double)count / arg1.no_threads;
		int stop_index_copy_indiv = minfnc(count, (arg1.id + 1) * (double) count/arg1.no_threads);

		for (int i = start_index_copy_indiv; i < stop_index_copy_indiv; ++i) {
			copy_individual(arg1.current_generation + i, arg1.next_generation + i);
		}

		// wait until all threads are done with copy_individual
		pthread_barrier_wait(arg1.barrier);
		
		// mutate first 20% children with the first version of bit string mutation
		cursor = count;
		count = arg1.object_count * 2 / 10;

		start_index_copy_indiv = arg1.id * (double)count / arg1.no_threads;
		stop_index_copy_indiv = minfnc(count, (arg1.id + 1) * (double) count/arg1.no_threads);

		for (int i = start_index_copy_indiv; i < stop_index_copy_indiv; ++i) {
			copy_individual(arg1.current_generation + i, arg1.next_generation + cursor + i);
			mutate_bit_string_1(arg1.next_generation + cursor + i, k);
		}
		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		count = arg1.object_count * 2 / 10;

		start_index_copy_indiv = arg1.id * (double)count / arg1.no_threads;
		stop_index_copy_indiv = minfnc(count, (arg1.id + 1) * (double) count/arg1.no_threads);

		for (int i = start_index_copy_indiv; i < stop_index_copy_indiv; ++i) {
			copy_individual(arg1.current_generation + i + count, arg1.next_generation + cursor + i);
			mutate_bit_string_2(arg1.next_generation + cursor + i, k);
		}
		
		// wait all threads to finish both mutations
		pthread_barrier_wait(arg1.barrier);

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		cursor += count;
		count = arg1.object_count * 3 / 10;
		
		if (count % 2 == 1) {
			copy_individual(arg1.current_generation + arg1.object_count - 1, arg1.next_generation + cursor + count - 1);
			count--;	
		}

		int start_index_crossover = arg1.id * ((double)count / arg1.no_threads);
		int stop_index_crossover = minfnc(count, (arg1.id + 1) * ((double) count/arg1.no_threads));

		if(start_index_crossover % 2 == 1) {
			start_index_crossover++;
		}

		for (int i = start_index_crossover; i < stop_index_crossover; i += 2) {
			crossover(arg1.current_generation + i, arg1.next_generation + cursor + i, k);
		}

		// Wait all threads to do crossover
		pthread_barrier_wait(arg1.barrier);

		// Only one thread should do this
		if (arg1.id == 0){
			// switch to new generation
			arg1.tmp = arg1.current_generation;
			arg1.current_generation = arg1.next_generation;
			arg1.next_generation = arg1.tmp;

			for (int i = 0; i < arg1.object_count; ++i) {
				arg1.current_generation[i].index = i;
			}
		
			if (k % 5 == 0 && arg1.id == 0) {
				print_best_fitness(arg1.current_generation);
			}
		}
		// Wait all threads to finish their job before going to next generation
		pthread_barrier_wait(arg1.barrier);
	}
	
	compute_fitness_function(arg1.objects, arg1.current_generation, start_index, stop_index,arg1. sack_capacity);
	// Wait for all threads to finish the last compute_fitness_function
	pthread_barrier_wait(arg1.barrier);

	if(arg1.id == 0){
		qsort(arg1.current_generation, arg1.object_count, sizeof(individual), cmpfunc);
	}
	// Only thread 0 does the qsort. The others wait
	pthread_barrier_wait(arg1.barrier);

	if(arg1.id == 0){
		print_best_fitness(arg1.current_generation);
	}
	pthread_exit(NULL);
	return NULL;
}