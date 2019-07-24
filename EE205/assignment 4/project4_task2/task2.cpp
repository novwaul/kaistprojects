/* 20160788 InJe Hwang 
        task2.cpp      */
#include <iostream>
#include <queue>
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include "ADT_graph.h"

/*several classes*/
/*------------------------------------------------------------------------------------------------------*/
class SP_Node // Node for priority queue used for finding shortest path(=SP)
{
public:
  int key;
  string prev;
  ADT_Node<int>* vert;
  int ver;
  SP_Node(int k = 0, string p = " ", ADT_Node<int>* v = NULL, int r = 0);
  int operator= (SP_Node spn);
};

SP_Node::SP_Node(int k, string p, ADT_Node<int>* v, int r) : key(k), prev(p), vert(v), ver(r){}

int SP_Node::operator= (SP_Node spn)
{
  key = spn.key;
  prev = spn.prev;
  vert = spn.vert;
  ver = spn.ver;
  return 1;
}

class CompareSP // comparator for finding SP
{
public:
  int operator() (SP_Node& n1, SP_Node& n2);
};

int CompareSP::operator() (SP_Node& n1, SP_Node& n2)
{
  if(n1.key < n2.key) return false; // because default priority queue is MAX PQ, return reverse result 
  else return true;
}

class MST_SetNode // Node for MST_Set
{
public:
  MST_SetNode* next;
  MST_SetNode* prev;
  ADT_Node<int>* vert;
  MST_SetNode(MST_SetNode* n = NULL, MST_SetNode* p = NULL, ADT_Node<int>* v = NULL);
  
};

MST_SetNode::MST_SetNode(MST_SetNode* n, MST_SetNode* p, ADT_Node<int>* v) : next(n), prev(p), vert(v) {}

class MST_Set // Set for finding MST
{
public:
  MST_SetNode* head;
  MST_SetNode* tail;
  int size;
  MST_Set();
  ~MST_Set();
};

MST_Set::MST_Set()
{
  head = new MST_SetNode;
  tail = new MST_SetNode;
  head->next = tail;
  tail->prev = head;
  size = 0;
}

MST_Set::~MST_Set()
{
  MST_SetNode *cur,*temp;
  cur = head->next;
  while(cur->next != NULL)
    {
      temp = cur->next;
      delete cur;
      cur = temp;
    }
  delete head;
  delete tail;
}

class MST_Node // Node for priority queue for finding MST
{
public:
  int key;
  ADT_Edge<int>* edge;
  MST_Node(int k = 0, ADT_Edge<int>* e = NULL);
  int operator= (MST_Node mstn);
};

MST_Node::MST_Node(int k, ADT_Edge<int>* e) : key(k), edge(e) {}

int MST_Node::operator= (MST_Node mstn)
{
  key = mstn.key;
  edge = mstn.edge;
  return 1;
}

class CompareMST // comparator for priority queue in MST
{
public:
  int operator() (MST_Node& n1, MST_Node& n2);
};

int CompareMST::operator() (MST_Node& n1, MST_Node& n2)
{
  if(n1.key < n2.key) return false;
  else return true;
}
/* Shortest path function and MST function */
/*----------------------------------------------------------------------------------------------------------------*/
bool SP(ADT_Graph<int>& graph, list<SP_Node>& SP_sol, string& src, string& dest, int src_value, int std_value)
{
  priority_queue< SP_Node, vector<SP_Node>, CompareSP > PQ;
  SP_Node spn;
  list<ADT_Node<int>*>::iterator n_iter;
  list<ADT_Edge<int>*>::iterator e_iter;
  list< SP_Node >::iterator p_iter;
  list<ADT_Node<int>*> nodelist;
  list<ADT_Edge<int>*> edgelist;
  list<SP_Node> pre_sol; // temporary solution
  int version = 0;
  const int OUT = -1; // indicate current vertex is out of PQ
  string cur;
  vector<int> update; // used for checking current vertex is recent version

  nodelist = (graph.get_ADT_nodelist()).get_node_list();
  for(n_iter = nodelist.begin(); n_iter != nodelist.end(); ++n_iter)
    {
      if((*n_iter)->get_node_name() == src)
	PQ.push( SP_Node(src_value, "UNKNOWN", (*n_iter), version) );//src_value : source distance
      else
	PQ.push( SP_Node(std_value, "UNKNOWN", (*n_iter), version) );//std_value : first distance of other vertices
      update.push_back(version);
    }

  while(!PQ.empty())
    {
      spn = PQ.top();
      PQ.pop();

      if(spn.key == std_value) return false; //SP fail
      else if(spn.ver != update[spn.vert->get_node_value()]) continue; // outdated data
      pre_sol.push_front(spn);
      update[spn.vert->get_node_value()] = OUT; // now this vertex is not inserted in PQ
      if(spn.vert->get_node_name() == dest) break; //SP is solved
      edgelist = spn.vert->incidentEdges();

      for(e_iter = edgelist.begin(); e_iter != edgelist.end(); ++e_iter)
	{
	  if( ((*e_iter)->endVertices()).front()->get_node_name() != spn.vert->get_node_name() ) //find opposite vertex by comparing names
	    {
	      version = update[(*e_iter)->endVertices().front()->get_node_value()];
	      if(version == OUT) continue;
	      version += 1;
	      update[(*e_iter)->endVertices().front()->get_node_value()] = version;// update version
	      PQ.push( SP_Node( spn.key + (*e_iter)->get_edge_value(), spn.vert->get_node_name(), (*e_iter)->endVertices().front(), version ) );
	    }
	  else
	    {
	      version = update[(*e_iter)->endVertices().back()->get_node_value()];
	      if(version == OUT) continue;
	      version += 1;
	      update[(*e_iter)->endVertices().back()->get_node_value()] = version;
	      PQ.push( SP_Node( spn.key + (*e_iter)->get_edge_value(), spn.vert->get_node_name(), (*e_iter)->endVertices().back(), version ) );
	    }
	}
    }
  cur = dest; //start from dest
  for(p_iter = pre_sol.begin(); p_iter != pre_sol.end(); ++p_iter)
    {
      if((*p_iter).vert->get_node_name() == cur)
	{
	  cur = (*p_iter).prev;
	  SP_sol.push_front((*p_iter)); // order is reversed 
	}
    }
  return true;
}

void MST(ADT_Graph<int>& graph, list<MST_Node>& MST_sol)
{
  priority_queue< MST_Node, vector<MST_Node>, CompareMST > PQ;
  MST_Node mstn;
  list<ADT_Node<int>*>::iterator n_iter;
  list<ADT_Edge<int>*>::iterator e_iter;
  vector<MST_Set*>::iterator v_iter;
  list<ADT_Node<int>*> nodelist;
  list<ADT_Edge<int>*> edgelist;
  vector<MST_Set*> partition; // contain sets
  MST_Set *set;
  MST_SetNode *snode, *cur;
  int set_num, fs, ss;

  nodelist = (graph.get_ADT_nodelist()).get_node_list();
  set_num = 0; // indicate a set where the current vertex is
  for(n_iter = nodelist.begin(); n_iter != nodelist.end(); ++n_iter)// make partition
    {
      set = new MST_Set();
      snode = new MST_SetNode();

      /* basic settings */
      set->head->next = snode;
      snode->prev = set->head;
      set->tail->prev = snode;
      snode->next = set->tail;
      snode->vert = (*n_iter);
      set->size++;
      
      (*n_iter)->set_set(set_num);

      partition.push_back(set);// insert set in the partition
      set_num++;
    }
  edgelist = (graph.get_ADT_edgelist()).get_edge_list();
  for(e_iter = edgelist.begin(); e_iter != edgelist.end(); ++e_iter)// insert edges in priority queue
    {
      PQ.push( MST_Node( (*e_iter)->get_edge_value(), (*e_iter) ) );
    }

  while(!PQ.empty())
    {
      mstn = PQ.top();
      PQ.pop();
      fs = (mstn.edge->get_first_node()).get_set();
      ss = (mstn.edge->get_second_node()).get_set();
      if(fs == ss) continue; // two vertex are already in the same set

      MST_sol.push_back(mstn);
      /* union two set */
      if(partition[ss]->size < partition[fs]->size )// first set size is bigger
	{
	  cur = partition[ss]->head->next;
	  while(cur->next != NULL)
	    {
	      cur->vert->set_set(fs);
	      cur = cur->next;
	    }
	  /* get vertices in second set */
	  partition[fs]->tail->prev->next = partition[ss]->head->next;
	  partition[ss]->head->next->prev = partition[fs]->tail->prev;
	  partition[fs]->tail->prev = partition[ss]->tail->prev;
	  partition[ss]->tail->prev->next = partition[fs]->tail;
	  partition[fs]->size += partition[ss]->size;
	  partition[ss]->size = 0;
	  partition[ss]->head->next = partition[ss]->tail;
	  partition[ss]->tail->prev = partition[ss]->head;
	}
      else // second set size is bigger
	{
	  cur = partition[fs]->head->next;
	  while(cur->next != NULL)
	    {
	      cur->vert->set_set(ss);
	      cur = cur->next;
	    }
	  /* get verticies in first set */
	  partition[ss]->tail->prev->next = partition[fs]->head->next;
	  partition[fs]->head->next->prev = partition[ss]->tail->prev;
	  partition[ss]->tail->prev = partition[fs]->tail->prev;
	  partition[fs]->tail->prev->next = partition[ss]->tail;
	  partition[ss]->size += partition[fs]->size;
	  partition[fs]->size = 0;
	  partition[fs]->head->next = partition[fs]->tail;
	  partition[fs]->tail->prev = partition[fs]->head;
	}
    }

  for(v_iter = partition.begin(); v_iter != partition.end(); ++v_iter)
    {
      delete (*v_iter); // prevent memory leak
    }
}

int main(int argc, char** argv)
{
  list<SP_Node>::iterator iter;
  list<MST_Node>::iterator m_iter;
  list<SP_Node> SP_sol;
  list<MST_Node> MST_sol;
  const int INF = 99999;
  const int SOURCE_DISTANCE = 0;
  int vert_val;
  string str, str2, val, src, dest;
  ADT_Graph<int> graph("mygraph");
  bool checker;

  if(argc != 5)
    {
      cerr << "ERROR: Check argument number again" << endl;
      return 0;
    }
  /* get vertices and edges */
  ifstream fin(argv[1]);
  fin >> str;
  if(str == "Node")
    {
      vert_val = 0;
      while(true) // get vertex info
	{
	  fin >> str;
	  if(str == "Edge") break;
	  graph.insertVertex(str, vert_val);
	  vert_val++;
	}
      while(true) // get edge info
	{
	  fin >> str >> str2 >> val;
	  if(fin.eof()) break;
	  graph.insertEdge(str, str2, val, stoi(val));
	}
    }
  else
    {
      cerr << "ERROR: Please insert Node first" << endl;
      return 0;
    }
  fin.close();

  /* Find shortest path */
  ifstream fin2(argv[2]);
  ofstream fout(argv[3]);

  while(true)
    {
      fin2 >> src >> dest;
      if(fin2.eof()) break;
      checker = SP(graph, SP_sol, src, dest, SOURCE_DISTANCE, INF);
      if(checker == false) fout << "No path" << endl;
      else
	{
	  for(iter = SP_sol.begin(); iter != SP_sol.end(); ++iter)
	    {
	      fout << (*iter).vert->get_node_name() << " ";
	    }
	  --iter;
	  fout << (*iter).key << endl;
	  SP_sol.clear();
	}
    }
  fin2.close();
  fout.close();

  /* Find MST */
  ofstream fout2(argv[4]);
  MST(graph, MST_sol);
  for(m_iter = MST_sol.begin(); m_iter != MST_sol.end(); ++m_iter)
    {
      str = (*m_iter).edge->get_first_node().get_node_name(); // get node name
      str2 = (*m_iter).edge->get_second_node().get_node_name();

      if(str < str2)
	fout2 << str << " " << str2 << " ";
      else
	fout2 << str2 << " " << str << " ";
      fout2 << (*m_iter).edge->get_edge_value() << endl;
    }
  fout2.close();

  return 1; // exit
}
