#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>

#include "single_list.h"

using namespace std;

/* DO NOT CHANGE INCLUDED HEADERS */

int main(int argc, char* argv[])
{ 
  if(argc < 3){
    cerr << "error - This program needs two files\n";
    return 1;
  }
  
  ifstream fin(argv[1]);
  ofstream fout(argv[2]);
  time_t start, finish;
  string msg_str, num_str, word, *str, line;
  int c, size, index, msg, num, checker, size_buc = 1, num_buc = 0, count, msg_c, num_c, num_arg, trial = 0;
  list_elem *ptr_sl, *prev;
  single_list **bucket, **temp;

  bucket = new single_list* [1];

  time(&start);
  
  bucket[0] = NULL;  
  while(true){

    trial++; // increase trial num
    
    /* validate input */
    getline(fin, line);
    if(fin.eof()) break;

    size = line.size();
    num_arg = 0; //  set argument num to 0

    if(line[0] == '\0'){
      cerr << "error - in input " << trial << endl;
      cerr << "Input is not exist ... ignore this input\n";
      continue;
    }
    for(int i = 0; i < size; i++){
      if((line[i] == '\t') || (line[i] == ' ')) continue;//ignore space character before the first argument
      else{
	for(int j = i; j <= size; j++){ // if find an character of an argument
	  if((line[j] == '\t') || (line[j] == ' ')){
	    count = j - i;
	    num_arg++; //update 
	    break;
	  }
	  if(line[j] == '\0'){ // reach end
	    count = j - i;
	    num_arg++;
	    break;
	  }
	}
	switch(num_arg){
	case 1:
	  msg_str = line.substr(i, count);
	  i += count;
	  continue;
	case 2:
	  num_str = line.substr(i, count);
	  i += count;
	  continue;
	case 3:
	  word = line.substr(i, count);
	  i += count;
	  continue;
	default:
	  break; // error occur
	}
	break; // get out of loop becasue of error
      }
    }

    if(num_arg < 3){
      cerr << "error - in input " << trial << endl;
      cerr << "Insufficient arguments ... ignore this input\n";
      continue;
    }
    else if(num_arg > 3){
      cerr << "error - in input " << trial << endl;
      cerr << "Excessive arguments ... ignore this input\n";
      continue;
    }
    
    msg = atoi(msg_str.c_str());
    num = atoi(num_str.c_str());

    checker = 1; // used to indicate normal state: 1 for normal state, 0 for error state
    
    for(int i = 0; i < msg_str.size(); i++){
      c = msg_str[i];
      if( (c == '0') || (c == '1') || (c == '2') || (c == '3') || (c == '4') || (c == '5') ||
	  (c == '6') || (c == '7') || (c == '8') || (c == '9')) continue;
      else{
	checker--; //set to the error state
      }
    }

    if((checker != 1) || (msg == 0)){
      cerr << "error - in input " << trial << endl;
      cerr << "Message number is incorrect ... ignore this input\n";
      continue;
    }

    checker = 1; 
    
    for(int i = 0; i < num_str.size(); i++){
      c = num_str[i];
      if( (c == '0') || (c == '1') || (c == '2') || (c == '3') || (c == '4') || (c == '5') ||
	  (c == '6') || (c == '7') || (c == '8') || (c == '9')) continue;
      else{
	checker--;
      }
    }

    if((checker != 1) || (num == 0)){
      cerr << "error - in input " << trial << endl;
      cerr << "Packet number is incorrect ... ignore this input\n";
      continue;
    }
    
    /* insert elemnt in the list */
    if(bucket[0] == NULL){// insert first msg
      bucket[0] = new single_list;
      bucket[0]->list_insert_front(new list_elem(msg, num, word));
      num_buc++;
    }
    else{
      for(index = 0 ; index < num_buc; index++){// check if msg already exist
	ptr_sl = bucket[index]->list_head();
	if(bucket[index]->list_get_data1(ptr_sl) == msg) break;
      }
      if(index == num_buc){// insert new msg
	if(num_buc == size_buc){ //  if bucket is full
	  temp = new single_list*[2*size_buc];
	  for(int i = 0; i < num_buc; i++){
	    temp[i] = bucket[i];
	  }
	  delete[] bucket;
	  bucket = temp;
	  size_buc = 2*size_buc;
	}
	bucket[index] = new single_list;
	bucket[index]->list_insert_front(new list_elem(msg, num, word));
	num_buc++;
      }
      else{// insert new packet of old msg
	ptr_sl = bucket[index]->list_head();
	while(ptr_sl != NULL){
	  if(bucket[index]->list_get_data2(ptr_sl) >=  num){
	    if(bucket[index]->list_get_data2(ptr_sl) == num) bucket[index]->list_replace(ptr_sl, new list_elem(msg, num, word));
	    else bucket[index]->list_insert_before(ptr_sl, new list_elem(msg, num, word));
	    break;
	  }
	  else{
	    prev = ptr_sl;
	    ptr_sl = bucket[index]->list_next(ptr_sl);
	    continue;
	  }
	}
	if(ptr_sl == NULL)
	  bucket[index]->list_insert_after(prev, new list_elem(msg, num, word));
      }
    }
  }
  
  /* print out */
  fin.close();

  for(index = 0; index < num_buc ; index++){
    ptr_sl = bucket[index]->list_head();
    msg_c = bucket[index]->list_get_data1(ptr_sl);
    fout << "- Message " << msg_c << endl;
    num_c = 1; // used for checking packet num
    while(ptr_sl != NULL){
      if(bucket[index]->list_get_data2(ptr_sl) != num_c){
	fout << "WARNING: packet " << num_c << " of message " << msg_c << " is missing" << endl;
	num_c++;
	continue;
      }
      else{
	fout << bucket[index]->list_get_data3(ptr_sl) <<  endl;
	num_c++;
	ptr_sl = bucket[index]->list_next(ptr_sl);
	continue;
      }
      
    }
    fout << "- End Message " << msg_c << endl << endl;
  }
  time(&finish);
  fout << "Running Time: " << difftime(finish, start) << " s." <<endl;

  fout.close();

  for(int i = 0 ; i < num_buc; i++){
    delete bucket[i];
  }
  delete[] bucket;
  return 0;
}
