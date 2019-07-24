/* 20160788 Inje Hwang
        mylist.cpp     */

#include "mylist.h"
/*-------------------------------------------------------------------------*/
/* constructor of mynode class */
mynode::mynode(int ln){
  line_num = ln;
  next = NULL;
  prev = NULL;
  str = "";
}

mynode::mynode(){
  line_num = -1;
  next = NULL;
  prev = NULL;
  str = "";
}

mynode::mynode(string s){
  line_num = -1;
  next = NULL;
  prev = NULL;
  str = s;
}

mynode::mynode(int ln, string s){
  line_num = ln;
  next = NULL;
  prev = NULL;
  str = s;
}

/*---------------------------------------------------------------------------*/
/* constructor of mylist class */
mylist::mylist(){
  lsize = 0;
  head = new mynode;
  tail = new mynode;
  head->next = tail;
  tail->prev = head;
  nodelist = NULL;
}

/* print all the str & line_num in the list to cout */
void mylist::printall(){
  mynode* temp = head->next;
  while(temp != tail){
    cout << temp->str <<" "  << temp->line_num << endl;
    temp = temp->next;
  }
  return;
}

/* get str in a node */
string& mylist::get_nstr(mynode* n){ return n->str;}

/* get line_num in a node */
int& mylist::get_num(mynode*n){ return n->line_num;}

/* get address of next node */
mynode* mylist::get_next(mynode* n){ return n->next;}

/* get address of previous node */
mynode* mylist::get_prev(mynode* n){ return n->prev;}

/* get address of head */
mynode* mylist::get_head(){ return head; }

/* get address of tail */
mynode* mylist::get_tail(){ return tail; }

/* get the number of nodes */
int mylist::get_lsize() { return lsize; }

/* change the previous node pointer of n to point p */
void mylist::set_prev(mynode*n , mynode* p){ n->prev = p;}

/* change the next node pointer of n to point p */
void mylist::set_next(mynode* n, mynode* p){ n->next = p;}

/* return true if list is empty
   otherwise it returns false  */
bool mylist::isempty(){ return head->next == tail;}

/* change the number of nodes to i */
void mylist::set_lsize(int i) { lsize = i; }

/* give a unique number to each node in the list */
void mylist::numbering()
{
  nodelist = new mynode*[lsize];
  mynode* temp = head;
  for(int i = 0; i < lsize; i++)
    {
      temp = temp->next;
      nodelist[i] = temp;
    }
  return;
}

/* return true if list is numbered 
   otherwise return false          */
bool mylist::is_numbered() { return nodelist != NULL; }

/* access a node by number
   it is assumed that the node has its unique number */
mynode* mylist::operator[](int index)
{
  if(nodelist == NULL)
    {
      cerr << "Error: This function is not numbered\n";
      return NULL;
    }
  else if((index < 1) || (index > lsize))
    {
      cerr << "Error: Exceed proper index\n";
      return NULL;
    }

  return nodelist[index - 1];
}

/* remove all the numbers allocated to each node */
void mylist::unnumbering()
{
  delete[] nodelist;
  nodelist = NULL;
  return;
}

/* insert new node n right before the node p 
   if list is numbered, user cannot use this function 
   to use it, user must unnumber the list first       */
void mylist::insert_mynode(mynode* n, mynode* p){
  if(p == head){
    cerr << "Error: Cannot insert a node before head\n";
    return;
  }
  else if(nodelist != NULL){
    cerr << "Error: Cannot insert a node to numbered list\n";
    return;
  }
  n->next = p;
  n->prev = p->prev;
  p->prev->next = n;
  p->prev = n;
  lsize++;
  return;
}

/* delete node n 
   if list is number, user cannot use this function
   to use it, user must unnumber the list first    */
void mylist::delete_mynode(mynode* n){
  if((n == head) || (n == tail)){
    cerr << "Error: Cannot delete head or tail\n";
    return;
  }
  else if(nodelist != NULL){
    cerr << "Error: Cannot delete a node in numbered list\n";
  }
  n->prev->next = n->next;
  n->next->prev = n->prev;
  delete n;
  lsize--;
  return;
}

/* find a node that has string "s" */
mynode* mylist::seek(string& s){
  mynode* temp = head->next;
  while(temp != tail){
    if(temp->str.compare(s) == 0) return temp;
    else temp = temp->next;
  }
  return NULL;
}

/* find a node that has number i */
mynode* mylist::seek(int i){
  mynode* temp = head->next;
  while(temp != tail){
    if(temp->line_num == i) return temp;
    else temp = temp->next;
  }
  return NULL;
}

/* merge this list with another list 
   this list get all nodes          
   if at least one of the lists is numbered, then user cannot use this function */
void mylist::merge(mynode *n, mylist *list){
  mynode *checkpoint, p;
  if((nodelist != NULL) || (list->is_numbered())){
    cerr << "Error: Cannot merge two nodes in numbered list\n";
    return;
  }
  checkpoint = list->get_head()->next;
  if(checkpoint == list->get_tail()){
    return;
  }
  else if(n == tail){
    checkpoint->prev = tail->prev;
    tail->prev->next = checkpoint;
    list->get_tail()->prev->next = tail;
    tail->prev = list->get_tail()->prev;
    list->get_tail()->prev = list->get_head();
    list->get_head()->next = list->get_tail();
    lsize = lsize + list->get_lsize();
    return;
  }

  if(n->line_num == checkpoint->line_num){
    list->delete_mynode(checkpoint);
    n = n->next;
    merge(n, list);
    return;
  }
  else if(n->line_num > checkpoint->line_num){
    checkpoint->next->prev = list->get_head();
    list->get_head()->next = checkpoint->next;

    checkpoint->next = n;
    n->prev->next = checkpoint;
    checkpoint->prev = n->prev;
    n->prev = checkpoint;

    lsize++;
    list->set_lsize(list->get_lsize() - 1);

    n = n->next;
    merge(n, list);
    return;
  }
  else{
    n = n->next;
    merge(n, list);
    return;
  }

}

/* Destructor of class mylist. */
mylist::~mylist(){
  mynode* temp, *cur = head->next;
  while(cur != tail){
    temp = cur->next;
    delete cur;
    cur = temp;
  }
  delete head;
  delete tail;
  if(nodelist != NULL) delete nodelist;
}

