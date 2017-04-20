#include "tree_node.h"

#include <assert.h>
#include <stdlib.h>

void
node_init(struct tree_node *node, uint8_t branch)
{
	assert(node != NULL);

	if(branch > 0)
	{
		node->children = malloc(branch * sizeof(struct tree_node *));
	}
	else
	{
		node->children = NULL;
	}

	node->capacity = branch;
	node->size = 0;
	node->weight = -1;
}

bool
node_is_leaf(struct tree_node *node)
{
	assert(node != NULL);

	return node->children == NULL;
}

void
node_add_child(struct tree_node *parent, struct tree_node * child)
{
	assert(parent != NULL);
	assert(parent->children != NULL);
	assert(child != NULL);
	assert(parent->size < parent->capacity);

	parent->children[parent->size] = child;
	child->parent = parent;
}

void
node_def_children(struct tree_node *node, uint8_t *data)
{
	assert(node != NULL);
	assert(data != NULL);

	int length = sizeof(data)/sizeof(data[0]);

	assert(length == node->size);

	int i;

	for(i = 0; i < length; i++)
	{
		node->children[i]->weight = data[i];
	}
}