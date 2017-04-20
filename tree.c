#include "tree.h"
#include "tree_node.h"

#include <assert.h>
#include <malloc.h>

void
tree_init(struct tree *tree)
{
	tree->root = malloc(sizeof(struct tree_node));
	node_init(tree->root, BRANCH_FACTOR);
}