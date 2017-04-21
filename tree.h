#ifndef TREE_H
#define TREE_H

#include <stdint.h>

#define BRANCH_FACTOR 32
#define DEPTH 5

struct tree {
	struct tree_node *root;
};

void tree_init(struct tree *);
void tree_create(struct tree *, int *);
void tree_print(struct tree *tree);

#endif