#include "tree.h"
#include "tree_node.h"

#include <assert.h>
#include <malloc.h>

struct tree_node * tree_create_helper(struct tree_node *root, int *data, uint8_t level, uint8_t *assigned);

void
tree_init(struct tree *tree)
{
	tree->root = malloc(sizeof(struct tree_node));
	node_init(tree->root, BRANCH_FACTOR);
}

void
tree_create(struct tree *tree, int *data)
{
	assert(tree != NULL);
	assert(data != NULL);

	uint8_t i = 0;
	tree_create_helper(tree->root, data, 0, &i);
}

struct tree_node *
tree_create_helper(struct tree_node *root, int *data, uint8_t level, uint8_t *assigned)
{
	if(level == 0)
	{
		int i;

		for(i = 0; i < BRANCH_FACTOR; i++)
		{
			root->children[i] = tree_create_helper(NULL, data, level + 1, assigned);
		}

		return root;
	}
	else if(level == DEPTH)
	{
		struct tree_node *leaf_node = malloc(sizeof(struct tree_node));
		node_init(leaf_node, BRANCH_FACTOR);

		leaf_node->weight = data[*assigned];
		(*assigned)++;

		return leaf_node;
	}
	else
	{
		struct tree_node *child_node = malloc(sizeof(struct tree_node));
		node_init(child_node, BRANCH_FACTOR);
		int i;

		for(i = 0; i < BRANCH_FACTOR; i++)
		{
			child_node->children[i] = tree_create_helper(NULL, data, level + 1, assigned);
		}

		return child_node;
	}
}