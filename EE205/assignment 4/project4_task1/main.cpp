#include "ADT_graph.h"  
/* #include "hello.h"  */
#include <iostream>
#include <string>
using namespace std;

int main() {
	cout<<"Task1: Implement ADT Graph"<<endl;
	cout<<"Evaluation start!"<<endl<<endl;
	ADT_Graph<int> *test_graph = new ADT_Graph<int>("test_graph");  

	test_graph->insertVertex("A", 1);
	test_graph->insertVertex("B", 2);
	test_graph->insertVertex("C", 3);
	test_graph->insertVertex("D", 4);
	test_graph->insertVertex("E", 5);

	test_graph->insertEdge("A", "B", "AB", 1);
	test_graph->insertEdge("A", "C", "AC", 1);
	test_graph->insertEdge("A", "D", "AD", 1);
	test_graph->insertEdge("B", "C", "BC", 1);
	test_graph->insertEdge("B", "D", "BD", 1);
	test_graph->insertEdge("C", "D", "CD", 1);
	test_graph->insertEdge("C", "E", "CE", 1);
	test_graph->insertEdge("D", "E", "DE", 1);

	cout << "##################### vertex test ##########################" <<endl;

	ADT_Node<int>* temp_node = *(test_graph->get_ADT_nodelist().get_node_list().begin());
	cout << "selected node is "<<temp_node->get_node_name()<<endl;
	cout << "Is it adjacenet to node E? " << (temp_node->isAdjacentTo("E") ? "yes":"no") <<endl ; 
	cout << "Is it adjacenet to node C? " << (temp_node->isAdjacentTo("C") ? "yes":"no") << endl; 
	cout << endl << endl; 

	cout << "##################### edge test ##########################" <<endl;
	ADT_Edge<int>* temp_edge = *(test_graph->get_ADT_edgelist().get_edge_list().begin());
	cout << "selected edge is " << temp_edge->get_edge_name() << endl;
	cout << "first node is " << temp_edge->get_first_node().get_node_name() << endl;
	cout << "opposite of first node is " << temp_edge->opposite(temp_edge->get_first_node().get_node_name())<<endl;
	cout << "is it adjacent to AC? " << (temp_edge->isAdjacentTo("AC") ? "yes":"no") <<endl;
	cout << "is it adjacent to DE? " << (temp_edge->isAdjacentTo("DE") ? "yes":"no") <<endl;
	cout << "is it incident on A? " << (temp_edge->isIncidentOn("A") ? "yes":"no") <<endl;
	cout << "is it incident on D? " << (temp_edge->isIncidentOn("D") ? "yes":"no") <<endl;
	cout << endl << endl; 

	cout << "##################### graph test ##########################" <<endl;

	cout << "##################### current graph status 1 ##########################" <<endl;
	test_graph->vertices();
	cout << endl;
	test_graph->edges();

	test_graph->eraseEdge("AB");
	test_graph->eraseEdge("DE");
	cout <<endl<<endl;
	cout << "Test1: remove AB and DE" <<endl;

	cout << "##################### current graph status 2 ##########################" <<endl;
	test_graph->vertices();
	cout << endl;
	test_graph->edges();

	test_graph->insertEdge("A", "B", "AB", 10);
	test_graph->insertEdge("D", "E", "DE", 10);
	cout <<endl<<endl;
	cout << "Test2: reinsert AB and DE" <<endl;

	cout << "##################### current graph status 3 ##########################" <<endl;
	test_graph->vertices();
	cout << endl;
	test_graph->edges();

	test_graph->eraseVertex("E"); 
	cout <<endl<<endl;
	cout << "Test3: remove E" << endl; 

	cout << "##################### current graph status 4 ##########################" <<endl;
	test_graph->vertices();
	cout << endl;
	test_graph->edges();


	return 1;
}
