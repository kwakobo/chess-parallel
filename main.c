#include "tree_node.h"
#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define NINF -147483648
#define INF 147483648

#define NODES pow(BRANCH_FACTOR, DEPTH)
#define RANGE 10000

#define THREADS 8
#define BRANCHES_PER_THREAD BRANCH_FACTOR / THREADS

int parallel_alpha = NINF;
int parallel_beta = INF;
pthread_mutex_t lock_alpha;
pthread_mutex_t lock_beta;

int node_cnt[THREADS];

struct thread_data {
	int thread_id;
	struct tree_node *nodes[BRANCHES_PER_THREAD];
	int result;
	int node_cnt;
};

int *rand_gen(int nodes, int range);
void * minimax_parallel_helper(void *threadargs);
int minimax_parallel(struct tree_node *root);
int minimax(struct tree_node *curr_node, int depth, bool max_player, int *count);

int alphabeta(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count);
void * alphabeta_parallel_helper(void *threadargs);
int alphabeta_parallel(struct tree_node *root);
void *pvsplit_parallel_helper(void *threadargs);
int pvsplit_parallel(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count);
int pvsplit(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count);
void alpha_set(int new_alpha);
void beta_set(int new_beta);
int alpha_get();
int beta_get();

int *
rand_gen(int nodes, int range)
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

int
minimax_parallel(struct tree_node *root)
{
	pthread_t *threads = (pthread_t *) malloc(THREADS * sizeof(pthread_t));
	struct thread_data *thread_data_array = (struct thread_data *) malloc(THREADS * sizeof(struct thread_data));


	int i, j = 0, rc;
	for(i = 0; i < THREADS; i++)
	{
		thread_data_array[i].thread_id = i;
		node_cnt[i] = 0;
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
		sub_result = minimax(args->nodes[i], 0, false, &node_cnt[i]);
		if(sub_result > args->result)
			args->result = sub_result;
		//printf("%d ", sub_result);
	}
	//printf("\n");

	pthread_exit(NULL);
}

/* serial minimax algorithm */
int
minimax(struct tree_node *curr_node, int depth, bool max_player, int *count)
{
	(*count)++;

	if (node_is_leaf(curr_node)) 
		return curr_node->weight;
	
	int best_val, i;
	if (max_player) {
		best_val = NINF;
		for (i=0; i<BRANCH_FACTOR; i++)
		{
			int value = minimax(curr_node->children[i], depth + 1, !max_player, count);
			if (best_val < value) best_val = value;
		}
		return best_val;
	}
	else 
	{
		best_val = INF;
		for(i=0; i<BRANCH_FACTOR; i++)
		{
			int value = minimax(curr_node->children[i], depth + 1, !max_player, count);
			if (value < best_val) best_val = value;
		}
		return best_val;
	}
}

/* serial alphabeta algorithm */
int
alphabeta(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count)
{
	(*count)++;

	if (node_is_leaf(curr_node))
		return curr_node->weight;
		
	int val, i;
	if (max_player)
	{
		val = NINF;
		for (i=0; i<BRANCH_FACTOR; i++) 
		{
			int result = alphabeta(curr_node->children[i], depth+1, alpha, beta, !max_player, count);
			if (result > val) val = result;
			if (alpha < val) alpha = val;
			if (beta <= alpha) break;
		}
		return val;
	}
	else
	{
		val = INF;
		for (i=0; i<BRANCH_FACTOR; i++)
		{
			int result = alphabeta(curr_node->children[i], depth+1, alpha, beta, !max_player, count);
			if (result < val) val = result;
			if (beta > val) beta = val;
			if (beta <= alpha) break;
		}
		return val;
	}
}

void *
alphabeta_parallel_helper(void *threadargs)
{
	struct thread_data *args = (struct thread_data *) threadargs;
	//printf("thread id: %d\n", args->thread_id);
	args->result = -1;
	int i, sub_result;
	for(i = 0; i < BRANCHES_PER_THREAD; i++)
	{
		sub_result = alphabeta(args->nodes[i], 0, NINF, INF, false, &node_cnt[i]);
		if(sub_result > args->result)
			args->result = sub_result;
		//printf("%d ", sub_result);
	}
	//printf("\n");

	pthread_exit(NULL);
}

int alphabeta_parallel(struct tree_node *root)
{
	pthread_t *threads = (pthread_t *) malloc(THREADS * sizeof(pthread_t));
	struct thread_data *thread_data_array = (struct thread_data *) malloc(THREADS * sizeof(struct thread_data));

	int i, j = 0, rc;
	for(i = 0; i < THREADS; i++)
	{
		thread_data_array[i].thread_id = i;
		node_cnt[i] = 0;
		for(j = 0; j < BRANCHES_PER_THREAD; j++)
		{
			thread_data_array[i].nodes[j] = root->children[i*BRANCHES_PER_THREAD+j];
		}
		rc = pthread_create(&threads[i], NULL, alphabeta_parallel_helper, (void *) &thread_data_array[i]);
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

int
alphabeta_lock(struct tree_node *curr_node, int depth, bool max_player, int *count)
{
	(*count)++;

	if (node_is_leaf(curr_node))
		return curr_node->weight;
		
	int val, i;
	if (max_player)
	{
		val = NINF;
		for (i=0; i<BRANCH_FACTOR; i++) 
		{
			int result = alphabeta_lock(curr_node->children[i], depth+1, !max_player, count);
			if (result > val) val = result;
			if (alpha_get() < val) alpha_set(val);
			if (beta_get() <= alpha_get()) break;
		}
		return val;
	}
	else
	{
		val = INF;
		for (i=0; i<BRANCH_FACTOR; i++)
		{
			int result = alphabeta_lock(curr_node->children[i], depth+1, !max_player, count);
			if (result < val) val = result;
			if (beta_get() > val) beta_set(val);
			if (beta_get() <= alpha_get()) break;
		}
		return val;
	}
}

void *
alphabeta_parallel_helper_lock(void *threadargs)
{
	struct thread_data *args = (struct thread_data *) threadargs;
	//printf("thread id: %d\n", args->thread_id);
	args->result = -1;
	int i, sub_result;
	for(i = 0; i < BRANCHES_PER_THREAD; i++)
	{
		sub_result = alphabeta_lock(args->nodes[i], 0, false, &node_cnt[i]);
		if(sub_result > args->result)
			args->result = sub_result;
		//printf("%d ", sub_result);
	}
	//printf("\n");

	pthread_exit(NULL);
}

int alphabeta_parallel_lock(struct tree_node *root)
{
	pthread_t *threads = (pthread_t *) malloc(THREADS * sizeof(pthread_t));
	struct thread_data *thread_data_array = (struct thread_data *) malloc(THREADS * sizeof(struct thread_data));


	int i, j = 0, rc;
	for(i = 0; i < THREADS; i++)
	{
		thread_data_array[i].thread_id = i;
		node_cnt[i] = 0;
		for(j = 0; j < BRANCHES_PER_THREAD; j++)
		{
			thread_data_array[i].nodes[j] = root->children[i*BRANCHES_PER_THREAD+j];
		}
		rc = pthread_create(&threads[i], NULL, alphabeta_parallel_helper, (void *) &thread_data_array[i]);
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

void *
pvsplit_parallel_helper(void *threadargs)
{
	struct thread_data *args = (struct thread_data *) threadargs;

	args->result = alphabeta(args->nodes[0], 0, NINF, INF, false, &args->node_cnt);

	pthread_exit(NULL);
}

int
pvsplit_parallel(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count)
{
	pthread_t *threads = (pthread_t *) malloc((BRANCH_FACTOR - 1) * sizeof(pthread_t));
	struct thread_data *thread_data_array = (struct thread_data *) malloc((BRANCH_FACTOR - 1) * sizeof(struct thread_data));

	int i, rc;
	int final_result = -1;

	for(i = 1; i < BRANCH_FACTOR; i++)
	{
		thread_data_array[i - 1].thread_id = i;
		thread_data_array[i - 1].node_cnt = 0;
		thread_data_array[i - 1].nodes[0] = curr_node->children[i];

		rc = pthread_create(&threads[i - 1], NULL, pvsplit_parallel_helper, (void *) &thread_data_array[i - 1]);
	}

	for(i = 1; i < BRANCH_FACTOR; i++)
	{
		rc = pthread_join(threads[i - 1], NULL);
		if(rc)
		{
			printf("ERROR; return code from pthreads_join() is %d\n", rc);
			exit(-1);
		}
		if(thread_data_array[i - 1].result > final_result)
			final_result = thread_data_array[i - 1].result;
		(*count ) += thread_data_array[i - 1].node_cnt;
	}

	return final_result;
}

int
pvsplit(struct tree_node *curr_node, int depth, int alpha, int beta, bool max_player, int *count)
{
	(*count)++;

	if (node_is_leaf(curr_node))
		return curr_node->weight;

	int result;

	result = pvsplit(curr_node->children[0], depth + 1, alpha, beta, !max_player, count);
	if(max_player)
	{
		if (result >= beta) return beta;
		if (result > alpha) alpha = result;
	}
	else
	{
		if (result <= alpha) return alpha;
		if (result < beta) beta = result;
	}

	return pvsplit_parallel(curr_node, depth + 1, alpha, beta, !max_player, count);
}

int
alpha_get()
{
	int alpha_temp;
	pthread_mutex_lock(&lock_alpha);
	alpha_temp = parallel_alpha;
	pthread_mutex_unlock(&lock_alpha);

	return alpha_temp;
}

int
beta_get()
{
	int beta_temp;
	pthread_mutex_lock(&lock_beta);
	beta_temp = parallel_beta;
	pthread_mutex_unlock(&lock_beta);

	return beta_temp;
}

void
alpha_set(int new_alpha)
{
	pthread_mutex_lock(&lock_alpha);
	parallel_alpha = new_alpha;
	pthread_mutex_unlock(&lock_alpha);
}

void
beta_set(int new_beta)
{
	pthread_mutex_lock(&lock_beta);
	parallel_beta = new_beta;
	pthread_mutex_unlock(&lock_beta);
}


int main(int argc, char **argv)
{
	int *data = rand_gen(NODES, RANGE);

	struct tree minimax_tree;
	tree_init(&minimax_tree);
	tree_create(&minimax_tree, data);


	struct timespec start, stop; 
	double time;

	int i, total, result;
	float avg;

	/* minimax serial */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	node_cnt[0] = 0;
	result = minimax(minimax_tree.root, 0, true, &node_cnt[0]);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	printf("[S]Minimax: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\n", node_cnt[0]);

	/* minimax parallel */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	result = minimax_parallel(minimax_tree.root);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	total = 0;
	avg = 0.0;
	for(i = 0; i < THREADS; i++)
	{
		total += node_cnt[i];
		avg += node_cnt[i];
	}
	avg /= THREADS;
	printf("[P]Minimax: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\tAVG: %f\n", total, avg);

	/* alphabeta serial */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	node_cnt[0] = 0;
	result = alphabeta(minimax_tree.root, 0, NINF, INF, true, &node_cnt[0]);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	printf("[S]Alpha-Beta: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\n", node_cnt[0]);

	/* alphabeta parallel */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	result = alphabeta_parallel(minimax_tree.root);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	total = 0;
	avg = 0.0;
	for(i = 0; i < THREADS; i++)
	{
		total += node_cnt[i];
		avg += node_cnt[i];
	}
	avg /= THREADS;
	printf("[P]Alpha-Beta: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\tAVG: %f\n", total, avg);

	/* alphabeta parallel with lock */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	result = alphabeta_parallel_lock(minimax_tree.root);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	total = 0;
	avg = 0.0;
	for(i = 0; i < THREADS; i++)
	{
		total += node_cnt[i];
		avg += node_cnt[i];
	}
	avg /= THREADS;
	printf("[PL]Alpha-Beta: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\tAVG: %f\n", total, avg);

	/* pvsplit */
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	result = pvsplit(minimax_tree.root, 0, NINF, INF, true, &node_cnt[0]);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("[PV]Alpha-Beta: %d\n", result);
	printf("Execution time = %f sec\n", time);
	printf("Nodes: %d\n", node_cnt[0]);

	free(data);

	return 0;
}