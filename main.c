#include "tree_node.h"
#include "tree.h"
#include "data/4096_1.inc"

#include <stdio.h>
int main(int argc, char **argv)
{
	struct tree minimax;
	tree_init(&minimax);
	tree_create(&minimax, &data);

	tree_print(&minimax);

	return 0;
}