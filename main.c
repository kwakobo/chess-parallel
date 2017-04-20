#include "tree_node.h"
#include "tree.h"
#include "data/4096_1.inc"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define NINF -147483648
#define INF 147483648

int minimax(struct tree_node *curr_node, int depth, bool max_player)
{
	if (node_is_leaf(curr_node)) 
		return curr_node->weight;
	
	int best_val, i;
	if (max_player) {
		best_val = NINF;
		for (i=0; i<BRANCH_FACTOR; i++)
		{
			//printf("max\n");
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
			//printf("min\n");
			int value = minimax(curr_node->children[i], depth + 1, !max_player);
			if (value < best_val) best_val = value;
		}
		return best_val;
	}
}

int main(int argc, char **argv)
{
	struct tree minimax_tree;
	tree_init(&minimax_tree);
	tree_create(&minimax_tree, data);
	
	
	struct timespec start, stop; 
	double time;
	
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
	
	printf("Minimax: %d\n", minimax(minimax_tree.root, 0, true));
	
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Execution time = %f sec\n", time);		
	
	
	return 0;
}