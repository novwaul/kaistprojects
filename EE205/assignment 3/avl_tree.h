/* 20160788 InJe Hwang 
      avl_tree.h       */

#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include <string>
#include <iostream>

using namespace std;

template <class T>
class AVLNode {
public:
	string key;
	T *value;
	int height;

	class AVLNode<T> *leftChild;
	class AVLNode<T> *rightChild;
	class AVLNode<T> *parent;

	AVLNode(string k, AVLNode<T> *p) {
		key = k;
		leftChild = NULL;
		rightChild = NULL;
		parent = p;
		value = new T;
		height = 0;
	}

};


template <class T>
class AVLTree {
public:
	AVLTree();
	void display();

	bool insert(string key);
	bool remove(string key);
	AVLNode<T>* search(string key);

	bool is_external(AVLNode<T> *n);
	bool is_root(AVLNode<T> *n);

private:
	AVLNode<T> *root;
	int height(AVLNode<T> *n);
	void update_height(AVLNode<T> *n);
	int balance_factor(AVLNode<T> *n);
	void balance(AVLNode<T> *n);
	AVLNode<T>* rotate_right(AVLNode<T> *n);
	AVLNode<T>* rotate_left(AVLNode<T> *n);

	void inorder_print(AVLNode<T> *n);
	void preorder_print(AVLNode<T> *n);

};

template <class T>
AVLTree<T>::AVLTree() {
	root = new AVLNode<T>("", NULL);
}

template <class T>
void AVLTree<T>::display() {
	if (is_external(root)) {
		cout << "Tree is empty." << endl;
		return;
	}
	cout << "root: " << root->key << endl;;
	inorder_print(root);
	cout << endl;
	preorder_print(root);
	cout << endl;
}

template <class T>
bool AVLTree<T>::is_external(AVLNode<T> *n) {

	return (n->leftChild == NULL && n->rightChild == NULL);
}

template <class T>
bool AVLTree<T>::is_root(AVLNode<T> *n) {
	
	return n->parent == NULL;
}

template <class T>
int AVLTree<T>::balance_factor(AVLNode<T> *n) {
	if (is_external(n))
		return 0;
	else
		return height(n->leftChild) - height(n->rightChild);
}


#endif /* __AVL_TREE_H__ */
