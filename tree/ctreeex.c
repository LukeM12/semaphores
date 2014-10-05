#include<stdio.h>
#include<stdlib.h>
//http://www.cprogramming.com/tutorial/c/lesson18.html
struct node 
{
	int key_value;
	char name[10];
	int guid;
	struct node *left;
	struct node *right;
};
// Inserting elements, removing elements, deleting a tree and travesing the tree
//Because binary trees have log (base 2) n layers, the average search time for a binary tree is log (base 2) n
struct node *root = 0;

void destroy_tree(struct node *leaf){
	if (leaf != 0)
	{
		printf("%p\n", leaf);
		destroy_tree(leaf->left);
		destroy_tree(leaf->right);
		free(leaf);
	}
}

void insert(int key, struct node **leaf){
	if( *leaf == 0 )
	{
		*leaf = (struct node*) malloc( sizeof( struct node) );
		(*leaf)->key_value = key;
		/*initialize the children to null*/
		(*leaf)->left = 0;
		(*leaf)->right =0;
	}
	else if( key < (*leaf)->key_value )
	{
		insert ( key, &(*leaf)->left );
	}
	else if( key > (*leaf)->key_value )
	{
		insert( key, &(*leaf)->right );
	}
}

struct node *search(int key, struct node* leaf)
{
	if( leaf !=0 )
	{
		if ( key == leaf->key_value )
		{
			//found it 
			return leaf;
		}
		else if (key < leaf->key_value)
		{
			return search(key, leaf->left);
		}
		else 
		{
			return search(key, leaf->right);
		}
	}
	else return 0;
}

int main(void){
	struct node *leaf;
	int *******p;
	for (int i=0; i<100; i++){
		insert(i, &leaf);
	}
	destroy_tree(leaf);
	return 0;
}
