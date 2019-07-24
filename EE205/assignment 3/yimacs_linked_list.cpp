/* 20160788 InJe Hwang
     yimacs_linked_list.cpp    */

#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "mylist.h"
#include <list>
#include <cctype>
#include <regex>
#include <cctype>
#include <ctime>

using namespace std;
/*-----------------------------------------------------------------------------------------------------------*/
struct listnode { // for node of linked list
  string key;
  list<int> value;
};
/*-----------------------------------------------------------------------------------------------------------*/

/* replace string "from" that is in the string "str" to  string "to" */
void replace_all(string& str, const string& from, const string& to) {

    std::regex reg1("^" + from + " ");
    str = string(std::regex_replace(str, reg1, to + " "));

    std::regex reg2(" " + from + "$");
    str = string(std::regex_replace(str, reg2, " " + to));

    std::regex reg3(" " + from + " ");
    str = string(std::regex_replace(str, reg3, " " + to + " "));
    str = string(std::regex_replace(str, reg3, " " + to + " "));

    std::regex reg4("^" + from + "$");
    str = string(std::regex_replace(str, reg4, to));
}

/* get word from input string */
void get_word(string& s, list<listnode> &linkedlist, int ln){
  size_t size = s.size();
  string word;
  int checkpoint1 = -1, checkpoint2 = -1; // used as markers : checkpoint1 marks first non-space character & checkpoint2 marks first space character after mark checkpoint1
  list<listnode>::iterator it;
  list<int>::iterator nit;
  for(int i = 0 ; i < size ; i++)
    {
      if(isspace(s[i])) // for space character
	{
	  if(checkpoint1 != -1)// if checkpoint1 is marked
	    {
	      checkpoint2 = i;
	      word = s.substr(checkpoint1, checkpoint2 - checkpoint1);
	      checkpoint1 = checkpoint2 = -1;
	      for(it = linkedlist.begin(); it != linkedlist.end(); it++)
		{
		  if((*it).key == word) break;
		}
	      if(it != linkedlist.end())
		{
		  nit = (*it).value.end();
		  nit--;
		  if((*nit) == ln) continue;
		  else
		    {
		      (*it).value.push_back(ln);
		      continue;
		    }
		}
	      else
		{
		  listnode temp;
		  temp.key= word;
		  temp.value.push_back(ln);
		  linkedlist.push_back(temp);
		  continue;
		}
	    }
	  else continue;
	}
      else// for non-space character
	{
	  if(checkpoint1 == -1) checkpoint1 = i; // if checkpoint1 is not marked
	  if(i == size - 1) checkpoint2 = size;// for last non-space character of the list, mark checkpoint2 to null character
	  if((checkpoint1 != -1)&&(checkpoint2 != -1))// to get last word if list is not ended with space character
	    {
	      word = s.substr(checkpoint1, checkpoint2 - checkpoint1);
	      checkpoint1 = checkpoint2 = -1;
 	      for(it = linkedlist.begin(); it != linkedlist.end(); it++)
		{
		  if((*it).key == word) break;
		}
	      if(it != linkedlist.end())
		{
		  nit = (*it).value.end();
		  nit--;
		  if((*nit) == ln) continue;
		  else
		    {
		      (*it).value.push_back(ln);
		      continue;
		    }
		}
	      else
		{
		  listnode temp;
		  temp.key= word;
		  temp.value.push_back(ln);
		  linkedlist.push_back(temp);
		  continue;
		}
	    }
	  continue;
	}
    }
}

/* get word from input string 
   basically same as above get_word function */
bool get_word(string& s, string sl[3])
{
  size_t size = s.size();
  string word;
  int checkpoint1 = -1, checkpoint2 = -1, count = 0; // count : for checking the number of words in the list 
  for(int i = 0 ; i < size ; i++)
    {
      if(isspace(s[i]))// for space ch
	{
	  if(checkpoint1 != -1)
	    {
	      checkpoint2 = i;
	      word = s.substr(checkpoint1, checkpoint2 - checkpoint1);
	      checkpoint1 = checkpoint2 = -1;
	      sl[count] = word;
	      count++;
	      continue;
	    }
	  else continue;
	}
      else// for non-space ch
	{
	  if(checkpoint1 == -1)// mark checkpoint1
	    {
	      if(count == 3)
		{
		  cerr << "Error: Too many arguments\n" << endl;
		  return false;
		}
	      else checkpoint1 = i;
	    }
	  if(i == size - 1) checkpoint2 = size;
	  if((checkpoint1 != -1)&&(checkpoint2 != -1))
	    {
	      word = s.substr(checkpoint1, checkpoint2 - checkpoint1);
	      checkpoint1 = checkpoint2 = -1;
	      sl[count] = word;
	      count++;
	      continue;
	    }
	  continue;
	}
    }
  if((count == 1) && (sl[0].compare("Q") == 0)) return true;
  else if(count != 3)
    {
      cerr << "Error: Not enough arguments\n" << endl;
      return false;
    }
  return true;
}
/*-----------------------------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  if(argc < 3)
    {
      cerr << "Not enough arguments\n" << endl;
      return 0;
    }
  else if(argc > 3)
    {
      cerr <<"Too much arguments\n" << endl;
      return 0;
    }

  list<listnode> linkedlist;
  list<listnode>::iterator it, zt;
  list<int>::iterator nit;
  mylist storelist;
  mynode *n, *tail;
  int index, line_num = 1;
  string s, sl[3];
  char c;
  bool fac;
  clock_t start, end;

  /* get line and get words */
  ifstream fin(argv[1]);
  while(true)
    {
      getline(fin, s);
      if(fin.eof()) break;

      n = new mynode(s);
      storelist.insert_mynode(n, storelist.get_tail());

      get_word(s, linkedlist, line_num);
      line_num++;
      
    }
  fin.close();

  /* replacing */
  storelist.numbering();
  while(true)
    {
      /* get command */
      getline(cin, s);
      fac = get_word(s, sl);
      if(fac == false) continue;
      /* do command */
      else if(sl[0].compare("Q") == 0) break; // stop
      else if(sl[0].compare("R") == 0 ) // replace
	{
	  start = clock();
	  for(it = linkedlist.begin(); it != linkedlist.end(); it++)
	    {
	      if((*it).key == sl[1]) break;
	    }
	  if(it == linkedlist.end()) // cannot find
	    {
	      cerr << "Error: Cannot find the word\n" << endl;
	      continue;
	    }
	  nit = (*it).value.begin();
	  /* replace */
	  while(nit != (*it).value.end())
	    {
	      index = (*nit);
	      string &temp = storelist.get_nstr(storelist[index]);
	      cout << "\n< " << temp << endl;
	      replace_all(temp, sl[1], sl[2]);
	      cout << "> " << temp << endl << endl;
	      nit++;
	    }
	  /* insert new node */
	  for(zt = linkedlist.begin(); zt != linkedlist.end(); zt++)
	    {
	      if((*zt).key == sl[2]) break;
	    }
	  if(zt == linkedlist.end())
	    {
	      listnode temp;
	      temp.key = sl[2];
	      for(nit = (*it).value.begin(); nit != (*it).value.end(); nit++)
		{
		  temp.value.push_back((*nit));
		}
	    }
	  else (*zt).value.merge((*it).value);

	  linkedlist.erase(it);
	  
	  end = clock();
	  cout << "time: " << float(end - start)/CLOCKS_PER_SEC << endl << endl;
	  continue;
	}
      /* error */
      else
	{
	  cerr << "Error: Invalid command\n" << endl;
	  continue;
	}
    }
  /* write replaced lines in output file */
  ofstream fout(argv[2]);
  n = storelist.get_next(storelist.get_head());
  tail = storelist.get_tail();

  while(n != tail)
    {
      fout << storelist.get_nstr(n) << endl;
      n = storelist.get_next(n);
    }
  fout.close();
  return 0;
}
