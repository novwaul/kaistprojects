#include <iostream>
#include <string>

#include "double_list.h"

using namespace std;

//////////////////////////////////////////////
// CAUTION: DO NOT CHANGE INCLUDED HEADERS  //
//////////////////////////////////////////////

/* Our doubly linked lists have two header elements: the "head"
   just before the first element and the "tail" just after the
   last element. The 'prev' link of the front header is null, as
   is the 'next' link of the back header. Their other two links
   point toward each other via the interior elements of the list.

   An empty list looks like this:

                      +------+     +------+
                  <---| head |<--->| tail |--->
                      +------+     +------+

   A list with two elements in it looks like this:

        +------+     +-------+     +-------+     +------+
    <---| head |<--->|   1   |<--->|   2   |<--->| tail |<--->
        +------+     +-------+     +-------+     +------+

   The symmetry of this arrangement eliminates lots of special
   cases in list processing. That's a lot simpler than the code 
   would be without header elements.

   (Because only one of the pointers in each header element is used,
   we could in fact combine them into a single header element
   without sacrificing this simplicity.  But using two separate
   elements allows us to do a little bit of checking on some
   operations, which can be valuable.) 

   You SHOULD NOT change the name, input arguments, and
   input/output data types of existing functions. */



/*** COMPLETE FUNCTIONS BELOW ***/

/* Doubly LINKED LIST */

/* Initializes LIST as an empty list. 'prev' of 'head' and 'next' of
   'tail' in LIST have null. 'next' of 'head' points 'tail' and 'prev'
   of 'tail' points 'head'. */
double_list::double_list()
{
  head = new d_list_elem;
  tail = new d_list_elem;
  head->next = tail;
  tail->prev = head;
}

double_list::~double_list()
{
  d_list_elem *del, *del_next;
  del = head;
  while(del != NULL){
    del_next = del->next;
    delete del;
    del = del->next;
  }
}

int
double_list::d_list_get_data1(d_list_elem *elem)
{
  if(head->next == tail){
    cerr << "List is empty, d_list_get_data fail1\n";
   return 0;
  }
  else if((elem == head) || (elem == tail)){
    cerr << "Invalid opertaion, d_list_get_data fail\n";
    return 0;
  }
  return elem->data1; // since we assumed elem exists
}

int
double_list::d_list_get_data2(d_list_elem *elem)
{
  if(head->next == tail){
    cerr << "List is empty, d_list_get_data fail2\n";
   return 0;
  }
  else if((elem == head) || (elem == tail)){
    cerr << "Invalid opertaion, d_list_get_data fail\n";
    return 0;
  }
  return elem->data2;
}

string
double_list::d_list_get_data3(d_list_elem *elem)
{
  if(head->next == tail){
    cerr << "List is empty, d_list_get_data fail3\n";
   return NULL;
  }
  else if((elem == head) || (elem == tail)){
    cerr << "Invalid opertaion, d_list_get_data fail\n";
    return NULL;
  }
  return elem->data3;
}

/* Returns the element before ELEM in its list. If ELEM is the first 
   element in its list, returns the list head. Results are undefined 
   if ELEM is itself a list head. */
d_list_elem *
double_list::d_list_prev (d_list_elem *elem)
{
  return elem->prev;
}

/* Returns the element after ELEM in its list. If ELEM is the last
   element in its list, returns the list tail. Return NULL if ELEM is
   itself a list tail. */
d_list_elem *
double_list::d_list_next (d_list_elem *elem)
{
  return elem->next;
}


/* Returns LIST's head. */
d_list_elem *
double_list::d_list_head (void)
{
  return head;
}

/* Returns LIST's tail. */
d_list_elem *
double_list::d_list_tail (void)
{
  return tail;
}

/* Returns the front element in LIST. Return NULL if LIST is empty. */
d_list_elem *
double_list::d_list_front (void)
{
  if(head->next == tail) return NULL;
  else return head->next;
}

/* Returns the back element in LIST. Return NULL if LIST is empty. */
d_list_elem *
double_list::d_list_back (void)
{
  if(head->next == tail) return NULL;
  else return tail->prev;
}


/* Insert ELEM at the beginning of LIST, so that it becomes the front 
   in LIST. */
void 
double_list::d_list_insert_front (d_list_elem *elem)
{
  if(elem == NULL){ // check elem exist
    cerr << "Element does not exist, d_list_insert_front fail\n";
    return;
  }

  d_list_elem *front = d_list_front(), *temp;

  if(front == NULL){ // first insert
    head->next = elem;
    elem->next = tail;
    tail->prev = elem;
    elem->prev = head;
    return;
  }
  else{ // check duplication
    temp = front;
    while(temp != tail){
      if(temp == elem){
	cerr << "Element already exists, d_list_insert_front fail\n";
	return;
      }
      else temp = temp->next;
    }
  }
  /* non-first insert */
  elem->next = front;
  elem->prev = front->prev;
  front->prev = elem;
  head->next = elem;
  return;
}

/* Insert ELEM at the end of LIST, so that it becomes the back in LIST. */
void 
double_list::d_list_insert_back (d_list_elem *elem)
{
  if(elem == NULL){ // check elem exist
    cerr << "Element does not exist, d_list_insert_back fail\n";
    return;
  }

  d_list_elem *back = d_list_back(), *temp;

  if(back == NULL){ // first insert
    head->next = elem;
    elem->next = tail;
    tail->prev = elem;
    elem->prev = head;
    return;
  }
  else{ // check duplication
    temp = back;
    while(temp != head){
      if(temp == elem){
	cerr << "Element already exists, d_list_insert_back fail\n";
	return;
      }
      else temp = temp->prev;
    }
  }
  /* non-first insert */
  elem->next = tail;
  elem->prev = back;
  back->next = elem;
  tail->prev = elem;
  return;
}

/* Insert ELEM just before BEFORE, which may be either an interior
 element or a tail. The latter case is equivalent to list_insert_back().
 Results are undefined if BEFORE is itself a list head. */
void 
double_list::d_list_insert_before (d_list_elem *before, d_list_elem *elem)
{
  if(elem == NULL){ // check elem exist
    cerr << "Element does not exist, d_list_insert_before fail\n";
    return;
  }

  d_list_elem *temp = d_list_head();

  if(temp == NULL)
    ;
  else{ // check duplication
    temp = temp->next;
    while(temp != tail){
      if(temp == elem){
	cerr << "Element already exist, d_list_insert_before fail\n";
	return;
      }
      else temp = temp->next;
    }
  }

  d_list_elem *prev = before->prev;

  if(prev == NULL) return; // before is head

  /* insert before */
  prev->next = elem;
  elem->prev = prev;
  elem->next = before;
  before->prev = elem;
  return;
}

/* Insert ELEM just after AFTER, which may be either an interior
 element or a head. The latter case is equivalent to list_insert_front().
 Results are undefined if AFTER is itself a list tail. */
void 
double_list::d_list_insert_after (d_list_elem *after, d_list_elem *elem)
{
  if(elem == NULL){ // check elem exist
    cerr << "Element does not exist, d_list_insert_before fail";
    return;
  }
  
  d_list_elem *temp = d_list_head();

  if(temp == NULL)
    ;
  else{ // check duplication
    temp = temp->next;
    while(temp != tail){
      if(temp == elem){
	cerr << "Element already exist, d_list_insert_after fail\n";
	return;
      }
      else temp = temp->next;
    }
  }
  
  d_list_elem *next = after->next;

  if(next == NULL) return; // after is tail

  /* insert after */
  after->next = elem;
  elem->prev = after;
  elem->next = next;
  next->prev = elem;
  return;
}

/* Replace FROM with TO and deconstruct FROM*/
void 
double_list::d_list_replace (d_list_elem *from, d_list_elem *to)
{
  if(to == NULL){ //check to exist
    cerr << "Element does not exist, d_list_replace fail\n";
    return;
  }
  else if((from == head) || (from == tail)){ // check from is head or tail
    cerr << "Invalid operation, d_list_replace fail\n";
    return;
  }

  d_list_elem *temp = d_list_head();

  if(temp == NULL){ // check list is empty
    cerr << "List is empty, d_list_replace fail\n";
    return;
  }
  else{ // check duplication
    temp = temp->next;
    while(temp != tail){
      if(temp == to){
	cerr << "Element already exist, d_list_replace fail\n";
	return;
      }
      else temp = temp->next;
    }
  }

  /* replace */
  
  d_list_elem *prev, *next;
  prev = from->prev;
  next = from->next;
  
  delete from;
  prev->next = to;
  to->prev = prev;
  next->prev = to;
  to->next = next;
  return;
}


/* Removes ELEM from its list and deconstructu it.
   Undefined behavior if ELEM is not in a list. Results are undefined
   if ELEM is not an interior element of its list.
*/

void
double_list::d_list_remove (d_list_elem *elem)
{
  if((elem == head) || (elem == tail)) return;
  else if(head->next == tail){
    cerr << "List is empty, d_list_remove fail\n";
    return;
  }

  d_list_elem *prev, *next;
  prev = elem->prev;
  next = elem->next;
  delete elem;
  prev->next = next;
  next->prev = prev;
  return;
}

/* Returns true if LIST is empty, false otherwise. */
bool 
double_list::d_list_empty (void)
{
  if(head->next == tail) return true;
  else return false;
}

