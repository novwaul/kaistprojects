#ifndef __SINGLE_LIST_H__
#define __SINGLE_LIST_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <string>

using namespace std;

///////////////////////////////////////
// CAUTION: DO NOT CHANGE THIS FILE  //
///////////////////////////////////////

/*
  Glossary of list terms:

  - "front": The first element in a list. NULL in an empty list.

  - "head": The first element in a list. NULL in an empty list.
  Returned by list_head().

  front=head. 
  (Note that front!=head in doubly linked list structure)

*/

class single_list;

/* List element. */
class list_elem{

 friend class single_list;

 private:

  int data1; /* Data of list element. */
  int data2; /* Data of list element. */
  string data3; /* Data of list element. */
  list_elem *next; /* Next list element. */

 public:
  
  /* List element constructor. */
  list_elem (){
    next = NULL;
  }
  
  list_elem (int _data1, int _data2, string _data3, list_elem *_next =NULL){
    data1 = _data1;
    data2 = _data2;
    data3 = _data3;
    next = _next;
  }
  
};


/* Singly Linked List. */
class single_list{
 private:
  list_elem *head; /* List head. */
  
 public:

  /* List constructor. */
  single_list();

  /* List deconstructor. */
  ~single_list();

  /* List data observation */
  int list_get_data1(list_elem *);
  int list_get_data2(list_elem *);
  string list_get_data3(list_elem *);

  /* List traversal. */
  list_elem *list_next (list_elem *);
  list_elem *list_head (void);
  
  /* List insertion. */
  void list_insert_front (list_elem *);
  void list_insert_before (list_elem *, list_elem *);
  void list_insert_after (list_elem *, list_elem *);
  
  /* List replacement. */
  void list_replace(list_elem*, list_elem *);
  
  /* List removal. */
  void list_remove (list_elem *);

  /* List properties. */
  bool list_empty (void);
};

#endif /* __SINGLE_LIST_H__ */
