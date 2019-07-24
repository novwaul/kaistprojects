/* 20160788 InJe Hwnag 
       mylist.h        */
#ifndef __MYLIST_H__
#define __MYLIST_H__

#include <iostream>
#include <string>

using namespace std;

/* mynode class 
   : used in avlnode and storing input lines */
class mynode{
friend class mylist;
private:
  int line_num;
  mynode *next;
  mynode *prev;
  string str; // for storing input lines
public:
  mynode();
  mynode(int ln);
  mynode(string s);
  mynode(int ln, string s);
  
};

/* mylist class 
   : used in avlnode and storing input lines */
class mylist{
private:
  int lsize; // the number of nodes in this list
  mynode* head;
  mynode* tail;
  mynode** nodelist; // for giving a unique number to each node to access immediately
public:
  mylist();
  ~mylist();
  string& get_nstr(mynode* n);
  int& get_num(mynode* n);
  int get_lsize();
  void set_lsize(int i);
  bool isempty();
  bool is_numbered();
  mynode* get_next(mynode* n);
  mynode* get_prev(mynode* n);
  void set_prev(mynode* n, mynode* p);
  void set_next(mynode* n, mynode* p);
  mynode* get_tail();
  mynode* get_head();
  mynode* seek(string& s);
  mynode* seek(int i);
  mynode* operator[](int index);
  void insert_mynode(mynode* n, mynode* p);
  void delete_mynode(mynode* n);
  void merge(mynode*n, mylist *list);
  void printall();
  void numbering();
  void unnumbering();

};

#endif
