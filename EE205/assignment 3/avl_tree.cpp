/* 20160788 InJe Hwang
      avl_tree.cpp      */

#include "avl_tree.h"
#include <iostream>
#include <string>

#include "mylist.h"

using namespace std;


/* Return the height of the tree. Remember external nodes are valid nodes. 
* Suppose we have a tree as follows. * means an external node.
*           a
*         /   \
*        b     c
*       / \   / \
*      d   * *   *
*     / \
*    *   *
* Then this tree has the height of 3
*/
template <class T>
int AVLTree<T>::height(AVLNode<T> *n) {
	return n->height;
}

/*
* Assign proper height value after operations on nodes.
* You will need this function in some functions.
* such as rotate_right(), rotate_left(), balance(), etc.
*
* Make sure all children have proper heights before you execute
* this function on a node.
*/
template <class T>
void AVLTree<T>::update_height(AVLNode<T> *n) {
	if (is_external(n)) {
		n->height = 0;
	}
	else {
		int lh = height(n->leftChild);
		int rh = height(n->rightChild);

		n->height = (lh >= rh ? lh : rh) + 1;
	}
}

/* Print keys along inorder traversal. Separate each key by a single space.
* Do not print external node's key, which is an empty string.
* e.g.,
*           a
*         /   \
*        b     c
*       / \   / \
*      d   * *   *
*     / \
*    *   *
* Then this function should print "d b a c " (with no newline character).
*/
template <class T>
void AVLTree<T>::inorder_print(AVLNode<T> *n) {
  if(is_external(n)) return;
  inorder_print(n->leftChild);
  cout << n->key << " ";
  inorder_print(n->rightChild);
  return;
}

/* 
* Print keys along preorder traversal. 
* The rule is the same as inorder_print(), except the order. 
*/
template <class T>
void AVLTree<T>::preorder_print(AVLNode<T> *n) {
  if(is_external(n)) return;
  cout << n->key << " ";
  preorder_print(n->leftChild);
  preorder_print(n->rightChild);
  return;
}


/* rotate right */
template <class T>
AVLNode<T>* AVLTree<T>::rotate_right(AVLNode<T> *n) {
	AVLNode<T> *x = n->leftChild;

	n->leftChild = x->rightChild;
	n->leftChild->parent = n;

	x->rightChild = n;
	x->parent = n->parent;

	if (!is_root(n)) {
		if (n->parent->leftChild == n)
			n->parent->leftChild = x;
		else
			n->parent->rightChild = x;
	}
	n->parent = x;

	/* the order of updates is important */
	update_height(n);
	update_height(x);

	return x;
}

/* rotate left */
template <class T>
AVLNode<T>* AVLTree<T>::rotate_left(AVLNode<T> *n) {
  AVLNode<T> *x = n->rightChild;
  
  n->rightChild = x->leftChild;
  n->rightChild->parent = n;

  x->leftChild = n;
  x->parent = n->parent;

  if(!is_root(n)){
    if(n->parent->leftChild == n)
      n->parent->leftChild = x;
    else
      n->parent->rightChild = x;
  }
  n->parent = x;

  update_height(n);
  update_height(x);

  return x;
}

/* keep a balance of avltree */
template <class T>
void AVLTree<T>::balance(AVLNode<T> *n) {

  if(is_root(n)) return;
  
  AVLNode<T> *p = n->parent;
  int rh = height(p->rightChild);
  int lh = height(p->leftChild);
  int oh = height(p);
  int nh, bp, crh, clh;

  nh = (lh >= rh ? lh : rh) + 1;

  if(nh != oh){ // change in height
    if( ( ( bp = balance_factor(p) ) >= 2) || ( bp <= -2)){ // unbalanced
      crh = height(n->rightChild);
      clh = height(n->leftChild);
      /* check cases */
      if((rh > lh) && (crh > clh)) // case1
	{
	  rotate_left(p);
	  if(is_root(p->parent))
	    {
	      root = p->parent;
	      return;
	    }
	  else balance(p);
	}
      else if((rh > lh) && (crh < clh)) // case2
	{
	  rotate_right(p->rightChild);
	  rotate_left(p);
	  if(is_root(p->parent))
	    {
	      root = p->parent;
	      return;
	    }
	  else balance(p);
	}
      else if((rh < lh) && (crh > clh)) //case3
	{
	  rotate_left(p->leftChild);
	  rotate_right(p);
	  if(is_root(p->parent))
	    {
	      root = p->parent;
	      return;
	    }
	  else balance(p);
	}
      else//case4
	{
	  rotate_right(p);
	  if(is_root(p->parent))
	    {
	      root = p->parent;
	      return;
	    }
	  else balance(p);
	}
    }
    else{ // already balanced
      update_height(p);
      if(is_root(p)) return;
      else balance(p);
    }
  }
  else return; // no change in height
}

/* 
* Refer to page 6 of the lecture note 'SearchTrees-BST_AVL'.
* CAUTION: You HAVE TO implement this function the way described in the lecture note.
* Use string::compare function for comparison between two keys.
*/
template <class T>
AVLNode<T>* AVLTree<T>::search(string key) {
  AVLNode<T> *temp = root;
  int i;
  while(!is_external(temp)){
    if( ( i = temp->key.compare(key) ) == 0) return temp;
    else if( i < 0 )
      {
	temp = temp->rightChild;
	continue;
      }
    else
      {
	temp = temp->leftChild;
	continue;
      }
  }
  return temp;
}

/* insert key in avltree
   if already inserted, then do nothing */
template <class T>
bool AVLTree<T>::insert(string key) {
	AVLNode<T> *n = search(key);

	if (is_external(n)) {
		n->key = key;
		n->leftChild = new AVLNode<T>("", n);
		n->rightChild = new AVLNode<T>("", n);
		update_height(n);

		balance(n);

		return true;
	}
	else
		return false; // the key already exists
}

/* 
* Remove a node. Return true if successful(if the key exists), false otherwise. 
*/
template <class T>
bool AVLTree<T>::remove(string key) {
  AVLNode<T> *temp, *temp2, *n = search(key); // search

  if(is_external(n)) return false; // cannot find
  /* check case */
  else if(is_external(n->leftChild)) // left child is exteretnal node 
    {
      if(is_root(n))
	{
	  root = n->rightChild;
	  n->rightChild->parent = NULL;
	}
      else
	{
	  n->rightChild->parent = n->parent;
	  if(n->parent->leftChild == n) n->parent->leftChild = n->rightChild;
	  else n->parent->rightChild = n->rightChild;
	}
      temp = n->rightChild;
      delete n->leftChild;
      delete n;

      balance(temp);

      return true;
    }
  else if(is_external(n->rightChild))// right child is external node
    {
      if(is_root(n))
	{
	  root = n->leftChild;
	  n->leftChild->parent = NULL;
	}
      else
	{
	  n->leftChild->parent = n->parent;
	  if(n->parent->leftChild == n) n->parent->leftChild = n->leftChild;
	  else n->parent->rightChild = n->leftChild;
	}
      temp = n->leftChild;
      delete n->rightChild;
      delete n;

      balance(temp);

      return true;
    }
  else // both child are not external node
    {
      temp = n->rightChild;
      while(!is_external(temp))// find inorder successor
	{
	  temp = temp->leftChild;
	}
      temp = temp->parent;
      
      n->key = temp->key;
      delete n->value;
      n->value = temp->value;

      temp->rightChild->parent = temp->parent;
      if(temp->parent != n)
	temp->parent->leftChild = temp->rightChild;
      else
	n->rightChild = temp->rightChild;

      temp2 = temp->rightChild;
      
      delete(temp->leftChild);
      delete(temp);
      
      balance(temp2);
    }
}


template class AVLNode<mylist>;
template class AVLTree<mylist>;
