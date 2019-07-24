#include <iostream>
#include <string>

#include "single_list.h"

using namespace std;


//////////////////////////////////////////////
// CAUTION: DO NOT CHANGE INCLUDED HEADERS  //
//////////////////////////////////////////////


/* Our singly linked lists have one header elements: the "head" which
   points the first element. The 'next' link of the back header is
   NULL. 
   An empty list looks like this:

                  
          HEAD ---> NULL 
                  

   A list with two elements in it looks like this:
   
                   +-------+    +-------+
          HEAD --->|   1   |--->|   2   |--->NULL
                   +-------+    +-------+

   You should not change the name, input arguments, and
   input/output data types of existing functions. */



/*** COMPLETE FUNCTIONS BELOW ***/

/* SINGLY LINKED LIST 
   - list_elem must already exist */

/* Initializes LIST as an empty list with a head. */
single_list::single_list()
{
  head = NULL;
}

single_list::~single_list() 
{
  list_elem *del_next, *del;
  del = head;
  while( del != NULL ){
    del_next = del->next;
    delete del;
    del = del_next;
  }
  head = NULL;
}

int
single_list::list_get_data1(list_elem *elem) //elem always exist in list
{
  if(head == NULL){
    cerr << "List is empty, list_get_data fail\n";
    return 0;
  }
  return elem->data1;
}

int
single_list::list_get_data2(list_elem *elem) //elem always exist in list
{
  if(head == NULL){
    cerr << "List is empty, list_get_data fail\n";
    return 0;
  }
  return elem->data2;
}

string
single_list::list_get_data3(list_elem *elem) //elem always exist in list
{
  if(head == NULL){
    cerr << "List is empty, list_get_data fail\n";
    return NULL;
  }
  return elem->data3;
}


/* Returns the element after ELEM in its list. */
list_elem *
single_list::list_next (list_elem *elem) //elem always exist in list
{
  if(head == NULL){
    cerr << "List is empty, list_next fail\n";
      return NULL;
  }
  return elem->next;
}


/* Returns LIST's head. */
list_elem *
single_list::list_head (void)
{
  return head;
}

/* Insert ELEM at the beginning of LIST, so that it becomes the head 
   in LIST. */
void 
single_list::list_insert_front (list_elem *elem)
{
  list_elem *ptr = head;
  if(elem == NULL){
    cerr << "Element does not exist, list_insert_front fail\n";
    return;
  }
  while(ptr != NULL){
    if(ptr == elem){
      cerr << "Element already exists, list_insert_front fail\n";
      return;
    }
    else ptr = ptr->next;
  }
  list_elem *temp;
  if(head != NULL) temp = head->next;
  else temp = NULL;
  head = elem;
  elem->next = temp;
  return;
}

/* Insert ELEM just before BEFORE, which may be either an interior
 element or a head. */
void 
single_list::list_insert_before (list_elem *before, list_elem *elem)
{
  list_elem *ptr = head;
  if(elem == NULL){
    cerr << "Element does not exist, list_insert_before fail\n";
    return;
  }
  if(head == NULL){ 
    cerr << "List is empty, list_insesrt_before fail\n";
    return;
  }
  while(ptr != NULL){
    if(ptr == elem){
      cerr << "Element already exists, list_insert_before fail\n";
      return;
    }
    else ptr = ptr->next;
  }

  list_elem *prev = head;
  
  if(prev == before){ 
    list_insert_front(elem);
    return;
  }

  while(prev->next != before) // 'before' always exist in list, no need to handle exception
    prev = prev->next;

  elem->next = before;
  prev->next = elem;
  return;
}

/* Insert ELEM just after AFTER, which may be either an interior
 element or a head. */
void 
single_list::list_insert_after (list_elem *after, list_elem *elem)
{
  list_elem *ptr = head;
  if(elem == NULL){
    cerr << "Element does not exist, list_insert_after fail\n";
    return;
  }
  if(head == NULL){
    cerr << "List is empty, list_insert_after fail\n";
    return;
  }
  
  while(ptr != NULL){
    if(ptr == elem){
      cerr << "Element already exists, list_insert_after fail\n";
      return;
    }
    else ptr = ptr->next;
  }

  elem->next = after-> next;
  after->next = elem;
}

/* Replace FROM with TO and deconstruct FROM */
void 
single_list::list_replace (list_elem *from, list_elem *to)
{
  if(to == NULL){
    cerr << "Element does not exist, list_replace fail\n";
    return;
  }
  else if(head == NULL){
    cerr << "List is empty, list_replace fail\n";
    return;
  }
  
  list_elem *temp = head;

  while(temp != NULL){
    if(temp == to){
      cerr << "Element already exists, list_replace fail\n";
      return;
    }
    else temp = temp->next;
  }
  
  list_insert_before(from, to);
  to->next = from->next;
  delete from;
  return;
}


/* Removes ELEM from its list and deconstructs it. */
void
single_list::list_remove (list_elem *elem)
{
  if(head == NULL){
    cerr << "List is empty, list_remove fail\n";
    return;
  }
  else if(elem == head){
    head = head->next;
    delete elem;
    return;
  }

  list_elem* prev;
  while(prev->next != elem)
    prev = prev->next;
  prev->next = elem->next;
  delete elem;
  return;
  
}

/* Returns true if LIST is empty, false otherwise. */
bool 
single_list::list_empty (void)
{
  /*** MODIFY HERE ***/
  if(head == NULL) return true;
  else return false;
}


