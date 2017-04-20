#ifndef TREE_H
#define TREE_H

#define BRANCH_FACTOR 16
#define DEPTH 3

struct tree {
	struct tree_node *root;
};

void tree_init(struct tree *);
void tree_rand_create(struct tree *);

#endif