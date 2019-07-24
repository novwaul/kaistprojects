/* 20160788 InJe Hwang
       ADT_graph.cpp      */

#include "ADT_graph.h"
#include <iostream>

using namespace std;

/*************************** ADT_Node ***************************/

/* Get node name */
template <class T>
string ADT_Node<T>::get_node_name(void) {
	return node_name;
}

/* Get node value */
template <class T>
T ADT_Node<T>::get_node_value(void) {
	return node_value;
}

/* Add new incidence collection */
template <class T>
int ADT_Node<T>::add_incidence(void){
	incidence = new ADT_Incidence<T>;
	return 1;
}

/* Remove incidence */
template <class T>
int ADT_Node<T>::remove_incidence(void){
	delete incidence;	
	return 1;
}

/* erase one edge from incidence list */
template <class T>
int ADT_Node<T>::erase_edge_from_incidence(ADT_Edge<T>* edge) {
	incidence->remove_incidence_list(edge);
	return 1;
}

/* return the begining of incidence list */
template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Node<T>::get_begin(void){
	return incidence->get_begin();
}

/* return the end of incidence list */
template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Node<T>::get_end(void){
	return incidence->get_end();
}

/* Call add_incidence_list in incidence collection */
template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Node<T>::add_incidence_list(ADT_Edge<T>* edge){
	return incidence->add_incidence_list(edge);
}

/* Save the node position information */
template <class T>
int ADT_Node<T>::add_nodelist_info(typename list<ADT_Node<T>*>::iterator node_position){
	this->node_position = node_position;
	return 1;
}

/* Basic functionality of ADT_Node */
template <class T>
list<ADT_Edge<T>*> ADT_Node<T>::incidentEdges(void) { 
  return incidence->incidentEdges();
}

template <class T>
int ADT_Node<T>::isAdjacentTo(string node_name) {
  return incidence->isAdjacentTo(node_name);
}

/* Constructor, initializing the node name and the node value, add new incidence collcetion*/
template <class T>
ADT_Node<T>::ADT_Node(string node_name, T node_value) {

	this->node_name = node_name;
	this->node_value = node_value;
	add_incidence();
}

/*************************** Incidence Collection ***************************/

/* Add new edge connection in the incidence collection */
template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Incidence<T>::add_incidence_list(ADT_Edge<T>* edge) {
	return incidence_list.insert(incidence_list.end(), edge); // pointer will be replaced later	
}

/* Remove an edge connection in the incidence collection */
template <class T>
int ADT_Incidence<T>::remove_incidence_list(ADT_Edge<T>* edge) {
	incidence_list.remove(edge); 
	return 1;
}

template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Incidence<T>::get_begin(void){
	return incidence_list.begin();
}

template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Incidence<T>::get_end(void){
	return incidence_list.end();
}


template <class T>
int ADT_Incidence<T>::isAdjacentTo(string node_name) {
  typename list<ADT_Edge<T>*>::iterator iter;
  for(iter = incidence_list.begin(); iter != incidence_list.end(); ++iter)
    {
      if((*iter)->isIncidentOn(node_name) == 1) return 1;
    }
  return 0;
}

template <class T>
list<ADT_Edge<T>*> ADT_Incidence<T>::incidentEdges(void) {
  return incidence_list;
}
/*************************** ADT_Edge ***************************/

/* Get edge name */
template <class T>
string ADT_Edge<T>::get_edge_name(void) {
	return edge_name;	
}

/* Get edge value */
template <class T>
T ADT_Edge<T>::get_edge_value(void) {
	return edge_value;
}

/* Get first node */
template <class T>
ADT_Node<T> ADT_Edge<T>::get_first_node(void) {
	return *first_node;
}


/* Get second node */
template <class T>
ADT_Node<T> ADT_Edge<T>::get_second_node(void) {
	return *second_node;
}

/* Save pointer to the incidence collection of the first vertex and the second vertex */
template <class T>
int ADT_Edge<T>::add_incidence_info(typename list<ADT_Edge<T>*>::iterator first_incidence_position, typename list<ADT_Edge<T>*>::iterator second_incidence_position) {
	this->first_incidence_position = first_incidence_position;
	this->second_incidence_position = second_incidence_position;
	return 1;
}

/* Save the edge position information */
template <class T>
int ADT_Edge<T>::add_edgelist_info(typename list<ADT_Edge<T>*>::iterator edge_position) {
	this->edge_position = edge_position;
	return 1;
}

template <class T>
list<ADT_Node<T>*> ADT_Edge<T>::endVertices(void) {
  list<ADT_Node<T>*> endV;
  endV.push_back(first_node);
  endV.push_back(second_node);
  return endV;
}


template <class T>
string ADT_Edge<T>::opposite(string node_name) {
  if( node_name == first_node->get_node_name() ) return second_node->get_node_name();
  else if( node_name == second_node->get_node_name() ) return first_node->get_node_name();
  return 0;
}


template <class T>
int ADT_Edge<T>::isAdjacentTo(string edge_name) {

  list<ADT_Edge<T>*> Elist;
  typename list<ADT_Edge<T>*>::iterator iter;

  Elist = first_node->incidentEdges();
  for( iter = Elist.begin(); iter != Elist.end(); ++iter)
    {
      if( edge_name == (*iter)->get_edge_name() ) return 1;
    }
  
  Elist = second_node->incidentEdges();
  for( iter = Elist.begin(); iter != Elist.end(); ++iter)
    {
      if(edge_name == (*iter)->get_edge_name() ) return 1;
    }
  return 0;
}


template <class T>
int ADT_Edge<T>::isIncidentOn(string node_name) {
  if( (node_name == first_node->get_node_name()) ||
      (node_name == second_node->get_node_name()) )
    return 1;
  return 0;
}


/* Constuctor, initializing edge name, edge value, and vertex name */
template <class T>
ADT_Edge<T>::ADT_Edge(string edge_name, T edge_value, ADT_Node<T> *first_node, ADT_Node<T> *second_node) {

	list<ADT_Edge<T>*> temp_list;

	this->edge_name = edge_name;
	this->edge_value = edge_value;
	this->first_node = first_node;
	this->second_node = second_node;	

}

/*************************** ADT_Nodelist ***************************/

/* Add new node in the nodelist */
template <class T>
typename list<ADT_Node<T>*>::iterator ADT_Nodelist<T>::add_node_list(ADT_Node<T> *vertex) {
	return node_list.insert(node_list.end(),vertex);
}

/* Remove a node in the nodelist */
template <class T>
int ADT_Nodelist<T>::remove_node_list(ADT_Node<T> *vertex) {
	node_list.remove(vertex);
	return 1;
}


template <class T>
ADT_Node<T>* ADT_Nodelist<T>::search_vertex(string vertex) {

  typename list<ADT_Node<T>*>::iterator iter;
  for( iter = node_list.begin(); iter != node_list.end(); ++iter)
    {
      if( vertex == (*iter)->get_node_name() ) return (*iter);
    }

  return NULL;
}

template <class T>
int	ADT_Nodelist<T>::print_all_vertices(void){
  typename list<ADT_Node<T>*>::iterator iter;
  for( iter = node_list.begin(); iter != node_list.end(); ++iter)
    {
      cout << (*iter)->get_node_name() << " ";
    }
  cout << endl;
  return 1;
}

/* Return node list */ 
template <class T>
list<ADT_Node<T>*> ADT_Nodelist<T>::get_node_list(void) {
	return node_list;
}
/*************************** ADT_Edgelist ***************************/

/* Add new edge in the edgelist */
template <class T>
typename list<ADT_Edge<T>*>::iterator ADT_Edgelist<T>::add_edge_list(ADT_Edge<T> *edge) {
	return edge_list.insert(edge_list.end(),edge);
}

/* Remove a edge in the edgelist */
template <class T>
int ADT_Edgelist<T>::remove_edge_list(ADT_Edge<T> *edge) {
	edge_list.remove(edge);
	return 1;
}


template <class T>
ADT_Edge<T>* ADT_Edgelist<T>::search_edge(string edge) {
  typename list<ADT_Edge<T>*>::iterator iter;
  for( iter = edge_list.begin(); iter != edge_list.end(); ++iter)
    {
      if( edge == (*iter)->get_edge_name() ) return (*iter);
    }
  return NULL;
}

template <class T>
int	ADT_Edgelist<T>::print_all_edges(void){
  typename list<ADT_Edge<T>*>::iterator iter;
  for( iter = edge_list.begin(); iter != edge_list.end(); ++iter)
    {
      cout << (*iter)->get_edge_name() << " ";
    }
  cout << endl;
  return 1;

}
/* Return edge list */ 
template <class T>
list<ADT_Edge<T>*> ADT_Edgelist<T>::get_edge_list(void) {
	return edge_list;
}
/*************************** ADT_Graph ***************************/

template <class T>
int ADT_Graph<T>::vertices(void) {
	ADT_nodelist->print_all_vertices();
	return 0;
}


template <class T>
int ADT_Graph<T>::edges(void) {
	ADT_edgelist->print_all_edges();
	return 0;
}


template <class T>
int ADT_Graph<T>::insertVertex(string node_name, T node_value) {

  ADT_Node<T>* newnode = new ADT_Node<T>(node_name, node_value);
  typename list<ADT_Node<T>*>::iterator iter;
  iter = ADT_nodelist->add_node_list(newnode);
  newnode->add_nodelist_info(iter);
  return 1;
  
}


template <class T>
int ADT_Graph<T>::insertEdge(string vertex_name1, string vertex_name2, string edge_name, T edge_value) {

        ADT_Node<T> *first_node, *second_node;
	ADT_Edge<T> *newedge;
	list<ADT_Node<T>*> n_list;
	typename list<ADT_Edge<T>*>::iterator position, first_edge_iterator, second_edge_iterator;
	typename list<ADT_Node<T>*>::iterator node_iterator, first_node_iterator, second_node_iterator;
	int count = 0;

	n_list = ADT_nodelist->get_node_list();

	for(node_iterator = n_list.begin(); node_iterator != n_list.end(); ++node_iterator)
	  {
	    if((*node_iterator)->get_node_name() == vertex_name1)
	      {
		first_node_iterator = node_iterator;
		count++;
	      }
	    else if((*node_iterator)->get_node_name() == vertex_name2)
	      {
		second_node_iterator = node_iterator;
		count++;
	      }
	    if( count == 2 ) break;
	  }
	if(node_iterator == n_list.end()) return 0;

	newedge = new ADT_Edge<T>(edge_name, edge_value, (*first_node_iterator), (*second_node_iterator));

	first_edge_iterator = (*first_node_iterator)->add_incidence_list(newedge);
	second_edge_iterator = (*second_node_iterator)->add_incidence_list(newedge);

	newedge->add_incidence_info(first_edge_iterator, second_edge_iterator);

	position = ADT_edgelist->add_edge_list(newedge);

	newedge->add_edgelist_info(position);

	return 1;
}


template <class T>
int ADT_Graph<T>::eraseVertex(string node_name) {
  typename list<ADT_Node<T>*>::iterator n_iter;
  typename list<ADT_Edge<T>*>::iterator e_iter, temp;
  list<ADT_Edge<T>*> e_list;
  list<ADT_Node<T>*> n_list;

  n_list = ADT_nodelist->get_node_list();
  for(n_iter = n_list.begin(); n_iter != n_list.end(); ++n_iter)
    {
      if((*n_iter)->get_node_name() == node_name) break;
    }
  if(n_iter == n_list.end()) return 0;

  e_list = (*n_iter)->incidentEdges();
  for(e_iter = e_list.begin(); e_iter != e_list.end(); )
    {
      ADT_edgelist->remove_edge_list(*e_iter);
      if(((*e_iter)->get_first_node().get_node_name()) != node_name)
	{
	  (*e_iter)->get_first_node().erase_edge_from_incidence((*e_iter));
	}
      else
	{
	  (*e_iter)->get_first_node().erase_edge_from_incidence((*e_iter));
	}
      temp = e_iter;
      ++e_iter;
      delete (*temp);
    }
  (*n_iter)->remove_incidence();
  ADT_nodelist->remove_node_list(*n_iter);
  delete (*n_iter);

  return 1;
}


template <class T>
int ADT_Graph<T>::eraseEdge(string edge_name) {
  typename list<ADT_Edge<T>*>::iterator iter;
  list<ADT_Edge<T>*> e_list;
  e_list = ADT_edgelist->get_edge_list();
  for(iter = e_list.begin(); iter != e_list.end(); ++iter)
    {
      if((*iter)->get_edge_name() == edge_name) break;
    }
  if(iter == e_list.end()) return 0;

  
  this->eraseEdge((*iter));
  return 1;
}


template <class T>
int ADT_Graph<T>::eraseEdge(ADT_Edge<T>* erase_edge) {

  erase_edge->get_first_node().erase_edge_from_incidence(erase_edge);
  erase_edge->get_second_node().erase_edge_from_incidence(erase_edge);
  ADT_edgelist->remove_edge_list(erase_edge);
  delete erase_edge;
  
  return 1;
}

template <class T>
ADT_Nodelist<T> ADT_Graph<T>::get_ADT_nodelist(void) {
	return *ADT_nodelist;
}

template <class T>
ADT_Edgelist<T> ADT_Graph<T>::get_ADT_edgelist(void) {
	return *ADT_edgelist;
}

template <class T>
ADT_Graph<T>::ADT_Graph(string graph_name) {
	graph_name = graph_name;
	ADT_nodelist = new ADT_Nodelist<T>;
	ADT_edgelist = new ADT_Edgelist<T>;


}

template class ADT_Graph<int>;
template class ADT_Node<int>;
template class ADT_Edge<int>;
template class ADT_Edgelist<int>;
template class ADT_Nodelist<int>;
