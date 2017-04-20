#include "tree_node.h"
#include "tree.h"
#include "data/4096_1.inc"

#include <stdio.h>
int main(int argc, char **argv)
{
	for(int i = 0; i < 4096; i++)
	{
		printf("%d ", data[i]);
	}

	return 0;
}