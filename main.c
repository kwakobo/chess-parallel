#include "tree_node.h"
#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define DEBUG false

#define NINF -147483648
#define INF 147483648


#define NODES pow(BRANCH_FACTOR, DEPTH)
#define RANGE 100

#define THREADS 32
#define BRANCHES_PER_THREAD BRANCH_FACTOR / THREADS

struct thread_data {
	int thread_id;
	struct tree_node *nodes[BRANCHES_PER_THREAD];
	int result;
};

int *rand_gen(int nodes, int range);
void * minimax_parallel_helper(void *threadargs);
int minimax_parallel(struct tree_node *root);
int minimax(struct tree_node *curr_node, int depth, bool max_player);


int *rand_gen(int nodes, int range)
{
	int *data = malloc(nodes * sizeof(int));
	srand(time(NULL));
	int i;
	for (i=0;i<nodes;i++)
	{
		int random = rand() % range;
		data[i] = random;
	}
	return data;
}

int minimax_parallel(struct tree_node *root)
{
	pthread_t *threads = (pthread_t *) malloc(THREADS * sizeof(pthread_t));
	struct thread_data *thread_data_array = (struct thread_data *) malloc(THREADS * sizeof(struct thread_data));


	int i, j = 0, rc;
	for(i = 0; i < THREADS; i++)
	{
		thread_data_array[i].thread_id = i;
		//if (DEBUG) printf("thread id: %d  ", i);
		for(j = 0; j < BRANCHES_PER_THREAD; j++)
		{
			thread_data_array[i].nodes[j] = root->children[i*BRANCHES_PER_THREAD+j];
			//if (DEBUG) printf("%d ", i*BRANCHES_PER_THREAD+j);
		}
		//if (DEBUG) printf("\n");
		rc = pthread_create(&threads[i], NULL, minimax_parallel_helper, (void *) &thread_data_array[i]);
		if(rc) {
			printf("ERROR; Return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	int final_result = -1;

	for(i = 0; i < THREADS; i++)
	{
		rc = pthread_join(threads[i], NULL);
		if(rc)
		{
			printf("ERROR; return code from pthreads_join() is %d\n", rc);
			exit(-1);
		}
		if(thread_data_array[i].result > final_result)
			final_result = thread_data_array[i].result;
	}

	free(thread_data_array);
	free(threads);

	return final_result;
}

void * minimax_parallel_helper(void *threadargs)
{
	struct thread_data *args = (struct thread_data *) threadargs;
	//printf("thread id: %d\n", args->thread_id);
	args->result = -1;
	int i, sub_result;
	for(i = 0; i < BRANCHES_PER_THREAD; i++)
	{
		sub_result = minimax(args->nodes[i], 0, false);
		if(sub_result > args->result)
			args->result = sub_result;
		//printf("%d ", sub_result);
	}
	//printf("\n");

	pthread_exit(NULL);
}


/* serial algorithm */
int minimax(struct tree_node *curr_node, int depth, bool max_player)
{
	if (node_is_leaf(curr_node)) 
		return curr_node->weight;
	
	int best_val, i;
	if (max_player) {
		best_val = NINF;
		for (i=0; i<BRANCH_FACTOR; i++)
		{
			int value = minimax(curr_node->children[i], depth + 1, !max_player);
			if (best_val < value) best_val = value;
		}
		return best_val;
	}
	else 
	{
		best_val = INF;
		for(i=0; i<BRANCH_FACTOR; i++)
		{
			int value = minimax(curr_node->children[i], depth + 1, !max_player);
			if (value < best_val) best_val = value;
		}
		return best_val;
	}
}

int main(int argc, char **argv)
{
	int *data = rand_gen(NODES, RANGE);

	struct tree minimax_tree;
	tree_init(&minimax_tree);
	tree_create(&minimax_tree, data);
	
	
	struct timespec start, stop; 
	double time;
	
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
	
	printf("Minimax: %d\n", minimax_parallel(minimax_tree.root));
	
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Execution time = %f sec\n", time);


	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	printf("Minimax: %d\n", minimax(minimax_tree.root, 0, true));

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Execution time = %f sec\n", time);
	
	free(data);

	return 0;
}