#include<stdio.h>
#include<stdlib.h>
//Holding function for our linked list 
struct node{
	int a;
	struct node *next;
	struct node *prev;
};
	
struct node *head;

//This function returns a plain node
struct node *makenode(int val){
	struct node *temp = (struct node*)malloc(sizeof(struct node)); 
	temp->a =val;
	return temp;
}

//This function inserts a node at the front of the list 
static void insert_front(int val, struct node **head){
	struct node *curr, *temp;
	curr = (*head);
	temp = makenode(val);	
	//If it is the main element, just go ahead and make it the current
	if (!(*head)){
		(*head) = temp;
		return;
	}
	//Otherwise, get to the end of the list 
	else {
		while (curr->next){
			curr = curr->next;;
		}
		curr->next = temp;
	}
}
//This function will traverse the list 
static void traverse(struct node* head){
	struct node *curr = head;
	while(curr){
		printf("The value is %i \nThe memory is %p\n----------------\n",curr->a, curr);
		curr = curr->next;
	}	
}
//This function deletes the entire list 
static void delete_list(struct node **head){
	struct node *curr;
	struct node *prev = (*head);
	while (prev){
		curr = prev->next;
		free(prev);
		prev = curr;
	}
	*head = NULL;
}

//TODO make a function to reverse the list 
/*static void reverse_list(struct node **head) {
	struct node *curr, *prev, *next;
	curr = (*head);
	while(prev){
	*/
	
//TODO make a function to make a circle list

//TODO make a function to mergesort. WITHOUT looking at any solutions 

//TODO make a function to quicksort WITHOUT looking at any solutions 

//This program intakes args integers and makes them elements in a linked list  
int main (int argc, char *args[]){
	struct node *head;
	int val;
	for (int i=1; i<argc; i++){
		sscanf(args[i], "%d", &val);
		insert_front(val, &head); 
 	}
 	traverse(head);
 	delete_list(&head);
 	traverse(head);	
	return 0;
}





























