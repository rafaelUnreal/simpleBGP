#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "trie.h"



int insert_prefix(treeNode node){
	
	int i=1;
	treeNode currentNode = *root;
	if(node == NULL){
		return 1;
	}
	node->mask;
	while(i <= node->mask){
	
	if(insert_node(currentNode,node,i) == 0){
		
			currentNode = currentNode->left;
	}
	else{
			currentNode = currentNode->right;
	}

	i++;
	}
		
}

int init_trie(){

	root =  malloc(sizeof(treeNode));
	treeNode head_node = malloc(sizeof(struct node));
	head_node->ip = 0;
	head_node->mask = 0;
	head_node->left = NULL;
	head_node->right = NULL;
	*root = head_node;
	
}

unsigned int insert_node(treeNode currentNode,treeNode insertNode, unsigned int numBits){
	treeNode temp;
	
	if(get_node_bit(insertNode,numBits)==0){
	temp = currentNode->left;
	
		if(temp == NULL){
			if(numBits == insertNode->mask){
				
				currentNode->left = insertNode;
			}
			else{
			treeNode node = malloc(sizeof(struct node));
			node->ip = get_num_bits(insertNode->ip,numBits);
			node->mask = numBits;
			node->left = NULL;
			node->right = NULL;
			currentNode->left = node;
			}
			
			return 0;
		}
		
		return 0;
		
	}
	
	if (get_node_bit(insertNode,numBits)==1){
	temp = currentNode->right;
	
		if(temp == NULL){
			if(numBits == insertNode->mask){
				
				currentNode->right = insertNode;
			}
			else{
			treeNode node = malloc(sizeof(struct node));
			node->ip = get_num_bits(insertNode->ip,numBits);
			node->mask =  numBits;
			node->left = NULL;
			node->right = NULL;
			currentNode->right = node;
			}             
		}
			return 1;
	}
	
		return 1;
			
}
	




unsigned int search_prefix(treeNode currentNode, treeNode node, unsigned int numBits){

	if (currentNode == NULL){
		return 0;
	}
	
	if(get_num_bits(node->ip,node->mask) == get_num_bits(currentNode->ip,node->mask) && node->mask == currentNode->mask ){
			//printf("Found a specific match\n");
		if(currentNode->entry){
			print_node(currentNode);
			return 1;
		}
	
		return 0;
	}

	
	if(get_node_bit(node,numBits)== 1 ){
		search_prefix(currentNode->right,node,numBits+1);
	}
	
	if(get_node_bit(node,numBits)== 0 ){
		search_prefix(currentNode->left,node,numBits+1);
	}
/*	// Less Specific Match
	if(getNumBits(node->ip,numBits-1) == getNumBits(currentNode->ip,numBits-1)){
			printf("Found a less match=%u\n",numBits-1);
			printNode(currentNode);
			return 1;
	}
*/
		
}

	

	
	

unsigned int get_num_bits(unsigned int num, unsigned numBitsPositions){
	
	if(numBitsPositions == 1){ return  (num &  0x80000000); }
	if(numBitsPositions == 2){ return  (num &  0xC0000000); }
	if(numBitsPositions == 3){ return  (num &  0xE0000000); }
	if(numBitsPositions == 4){ return  (num &  0xF0000000); }
	if(numBitsPositions == 5){ return  (num &  0xF8000000); }
	if(numBitsPositions == 6){ return  (num &  0xFC000000); }
	if(numBitsPositions == 7){ return  (num &  0xFE000000); }
	if(numBitsPositions == 8){ return  (num &  0xFF000000); }
	if(numBitsPositions == 9){ return  (num &  0xFF800000); }
	if(numBitsPositions == 10){ return  (num & 0xFFC00000); }
	if(numBitsPositions == 11){ return  (num & 0xFFE00000); }
	if(numBitsPositions == 12){ return  (num & 0xFFF00000); }
	if(numBitsPositions == 13){ return  (num & 0xFFF80000); }
	if(numBitsPositions == 14){ return  (num & 0xFFFC0000); }
	if(numBitsPositions == 15){ return  (num & 0xFFFE0000); }
	if(numBitsPositions == 16){ return  (num & 0xFFFF0000); }
	if(numBitsPositions == 16){ return  (num & 0xFFFF8000); }
	if(numBitsPositions == 17){ return  (num & 0xFFFFC000); }
	if(numBitsPositions == 18){ return  (num & 0xFFFFE000); }
	if(numBitsPositions == 19){ return  (num & 0xFFFFF000); }
	if(numBitsPositions == 20){ return  (num & 0xFFFFF800); }
	if(numBitsPositions == 21){ return  (num & 0xFFFFFC00); }
	if(numBitsPositions == 22){ return  (num & 0xFFFFFE00); }
	if(numBitsPositions == 23){ return  (num & 0xFFFFFF00); }
	if(numBitsPositions == 24){ return  (num & 0xFFFFFF80); }
	if(numBitsPositions == 25){ return  (num & 0xFFFFFFC0); }
	if(numBitsPositions == 26){ return  (num & 0xFFFFFFE0); }
	if(numBitsPositions == 27){ return  (num & 0xFFFFFFF0); }
	if(numBitsPositions == 28){ return  (num & 0xFFFFFFF8); }
	if(numBitsPositions == 29){ return  (num & 0xFFFFFFFC); }
	if(numBitsPositions == 30){ return  (num & 0xFFFFFFFE); }
	if(numBitsPositions == 31){ return  (num & 0xFFFFFFFF); }
	if(numBitsPositions == 32){ return  num; }
	
	//result = (num >> numBitsPositions) & ((1 << numBitsPositions) - 1);
		
		
}

unsigned int get_node_bit(treeNode node, unsigned int bitPosition){
	
	int position = (32 - bitPosition);

	return ((node->ip >> position) & 1);
		
}




void print_trie(treeNode node){
	treeNode temp = node;
	
	if(temp == NULL){
		
		return;
		
	}
	
	printf("%u.",(node->ip  >> 24) & 0x000000FF);
	printf("%u.",(node->ip  >> 16) & 0x000000FF);
	printf("%u.",(node->ip  >> 8) & 0x000000FF);
	printf("%u", node->ip  & 0x000000FF);
	printf("/%u \n",node->mask); 
			
	print_trie(temp->left);
	print_trie(temp->right);
		
}

void print_routes(treeNode node){
	treeNode temp = node;
	
	if(temp == NULL){
		
		return;
		
	}
	
	if(temp->entry){
		
	printf("%u.",(node->ip  >> 24) & 0x000000FF);
	printf("%u.",(node->ip  >> 16) & 0x000000FF);
	printf("%u.",(node->ip  >> 8) & 0x000000FF);
	printf("%u", node->ip  & 0x000000FF);
	printf("/%u \n",node->mask); 	
	}
	print_routes(temp->left);
	print_routes(temp->right);
}

void print_node(treeNode node){
	
	printf("print node()\n");
	printf("%u.",(node->ip  >> 24) & 0x000000FF);
	printf("%u.",(node->ip  >> 16) & 0x000000FF);
	printf("%u.",(node->ip  >> 8) & 0x000000FF);
	printf("%u", node->ip  & 0x000000FF);
	printf("/%u \n",node->mask); 
	
	
}
