#include <stdbool.h>
#ifndef TRIE_H
#define TRIE_H

struct node{
	
	unsigned int ip;
	unsigned int mask;
	bool entry;
	struct node *left;
	struct node *right;
};

typedef struct node *treeNode;
treeNode *root;

unsigned int insert_node(treeNode currentNode,treeNode insertNode, unsigned int numBits);
unsigned int get_node_bit(treeNode node, unsigned int bitPosition);
unsigned int get_num_bits(unsigned int num, unsigned numBitsPositions);
void print_node(treeNode node);


#endif