#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*	
Allocate a linked list node with a given key
Allocate a node using malloc(),
initialize the pointers to NULL,
set the key value to the key provided by the argument
 */
struct list_node *allocate_node_with_key(int key)
{
	struct list_node *ptr = (struct list_node *)malloc(sizeof(struct list_node));
	ptr->next = ptr->prev = NULL;
	ptr->value = NULL;
	ptr->key = key;
	return ptr;
}

/*	
Initialize the key values of the head/tail list nodes (I used -1 key values)
Also, link the head and tail to point to each other
 */
void initialize_list_head_tail(struct list_node *head, struct list_node *tail)
{
	tail->next = head;
	tail->prev = head;
	head->prev = tail;
	head->next = tail;
	return;
}

/*	
Insert the *new_node* after the *node*
 */
void insert_node_after (struct list_node *node, struct list_node *new_node)
{
	struct list_node *next_node;
	next_node = node->next;
	node->next = new_node;
	new_node->next = next_node;
	next_node->prev = new_node;
	new_node->prev = node;
	return;
}

/*	
Remove the *node* from the list
 */
void del_node (struct list_node *node)
{
	struct list_node *n_prev, *n_next;
	n_prev = node->prev;
	n_next = node->next;
	n_next->prev = n_prev;
	n_prev->next = n_next;
	free(node);
	return;
}

/*	
Search from the head to the tail (excluding both head and tail,
as they do not hold valid keys), for the node with a given *search_key*
and return the node.
You may assume that the list will only hold nodes with unique key values
(No duplicate keys in the list)
 */
struct list_node *search_list (struct list_node *head, int search_key)
{
	struct list_node *cur = head->next, *tail = head->prev;
	while(cur != tail)
	{
		if(cur->key == search_key) return cur;
		else cur = cur->next;
	}
	return NULL;
}

/*	
Count the number of nodes in the list (excluding head and tail), 
and return the counted value
 */
int count_list_length (struct list_node *head)
{
	struct list_node *cur = head->next, *tail = head->prev;
	int counter = 0;
	while(cur != tail)
	{
		counter++;
		cur = cur->next;
	}
	return counter;
}

/*	
Check if the list is empty (only head and tail exist in the list)
Return 1 if empty. Return 0 if list is not empty.
 */
int is_list_empty (struct list_node *head)
{
	if(head->next == head->prev) return 1;
	else return 0;
}

/*	
Loop through the list and print the key values
This function will not be tested, but it will aid you in debugging your list.
You may add calls to the *iterate_print_keys* function in the test.c
at points where you need to debug your list.
But make sure to test your final version with the original test.c code.
 */
void iterate_print_keys (struct list_node *head)
{
	struct list_node *cur = head->next, *tail = head->prev;
	int counter = 0;
	while(cur != tail)
	{
		counter++;
		printf("Node%d key value: %d\n",counter,cur->key);
		cur = cur->next;
	}
	return;
}

/*	
Insert a *new_node* at the sorted position so that the keys of the nodes of the
list (including the key of the *new_node*) is always sorted (increasing order)
 */
int insert_sorted_by_key (struct list_node *head, struct list_node *new_node)
{
	struct list_node *cur = head->next, *tail = head->prev;
	if(count_list_length(head) == 0)
	{
		insert_node_after(head,new_node);
		return 1;
	}
	while(cur != tail)
	{
		if(new_node->key <= cur->key)
		{
			insert_node_after(cur->prev, new_node);
			return 1;
		}
		else cur = cur->next;
	}
	insert_node_after(tail->prev, new_node);
	return 1;
}
