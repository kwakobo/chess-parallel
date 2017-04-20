#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <stdbool.h>
#include <stdint.h>

struct tree_node {
	struct tree_node **children;
	struct tree_node *parent;
	uint8_t capacity;
	uint8_t size;
	uint8_t weight;
};

void node_init(struct tree_node *, uint8_t);
bool node_is_leaf(struct tree_node *);
void node_add_child(struct tree_node *, struct tree_node *);
void node_def_children(struct tree_node *, uint8_t *);

#endif