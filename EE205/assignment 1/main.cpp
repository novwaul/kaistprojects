// //****************************************************************
// //                   MAIN FUNCTION IN PROJECT
// //****************************************************************

/* 20160788 InJe Hwang */

#include "student.h"

int main()
{
  Check C;
  char command;
  Manager myman;
  int isgrad;
  string name;
  int stunum;
  string labname;
  int freshmenclass;
  int index;
  string stra, strb, strc;
  char c;

  while(1){
    try{
	std::cout << "\n\n\n*---------------------Welcome to Student Management System---------------------*" << std::endl;

	std::cout << "What do you want to do?\n1. Add student:a\n2. Compare student:c\n3. Find student:f\n4. Delete student:d\n5. Print all student:p" << std::endl;

	std::cin >> stra;
	while(1){
	  c = cin.get();
	  if(c == '\n') break;
	}
	C.IsAlpha(stra);
	if(stra.size() > 1) throw Error("ERR: Invalid argument");
	command = stra[0];
	
	switch(command)
	  {
	  case 'a':{
		std::cout << "Add student executed\n\nType\nUndergraduate:0 Graduate:1"<< std::endl;
		
		std::cin >> stra;
		while(1){
		  c = cin.get();
		  if(c == '\n') break;
		}
		
		isgrad = C.IsDigit(stra);

		if (isgrad == 1){
		  std::cout << "\nFormat: (name, stunum, labname)" << std::endl;
		  
		  std::cin >> name >> strb >> labname;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		    
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  C.IsAlpha(labname);
		  if(myman.getSize() == MAX) throw Error("ERR: Full array");
		  
		  myman.add_student(name, stunum, labname);
		  break;
		}

		else if (isgrad == 0){
		  std::cout << "\nFormat: (name, stunum, freshmenclass) " << std::endl;
		  
		  std::cin >> name >> stra >> strb;
		  while(1){
		    c=cin.get();
		    if(c == '\n') break;
		  }
		  
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(stra)) < 0) throw Error("ERR: Invalid argument");
		  if( (freshmenclass = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  if(myman.getSize() == MAX) throw Error("Full array");
		  
		  myman.add_student(name, stunum, freshmenclass);
		  break;
		}
	        throw Error("ERR: Invalid argument");
	  }

	  case 'c':{
		std::cout << "Compare student executed\n\nType\nUndergraduate:0 Graduate:1" << std::endl;
		
		std::cin >> stra;
		while(1){
		  c = cin.get();
		  if(c == '\n') break;
		}
		
		isgrad = C.IsDigit(stra);
		
		if (isgrad == 1){
		  std::cout << "\nTarget student\nFormat: (index, name, stunum, labname)" << std::endl;
		  
		  std::cin >> stra >> name >> strb >> labname;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		   
		  if( ( (index = C.IsDigit(stra) ) < 0) || (index > MAX) )
		    throw Error("ERR: Invalid Argument");
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(strb)) < 0 )
		    throw Error("ERR: Invalid Argument");
		  C.IsAlpha(labname);
		  if(index > myman.getSize()) throw Error("ERR: Empty index");
		 
		  myman.compare_student(index, name, stunum, labname);
		  break;
		}

		else if (isgrad == 0){
		  std::cout << "\nTarget student\nFormat: (index, name, stunum, freshmenclass) " << std::endl;
		  std::cin >> stra >> name >> strb >> freshmenclass;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		  if( ((index = C.IsDigit(stra)) < 0) || (index > MAX) )
		    throw Error("ERR: Invalid Argument");
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(strb)) < 0 )
		    throw Error("ERR: Invalid Argument");
		  C.IsAlpha(labname);
		  if(index > myman.getSize()) throw Error("ERR: Empty index");
		  
		  myman.compare_student(index, name, stunum, freshmenclass);
		  break;
		}
		throw Error("ERR: Invalid argument");
	  }

	  case 'f':{
		std::cout << "Find student executed\n\nType\nUndergraduate:0 Graduate:1" << std::endl;
		
		std::cin >> stra;
		while(1){
		  c = cin.get();
		  if(c == '\n') break;
		}
		  
		isgrad = C.IsDigit(stra);
		
		if (isgrad == 1){
		  std::cout << "\nFormat: (name, stunum, labname)" << std::endl;

		  std::cin >> name >> strb >> labname;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		  
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  C.IsAlpha(labname);
	  
		  myman.find_student(name, stunum, labname);
		  break;
		}
		else if (isgrad == 0){
		  std::cout << "\nFormat: (name, stunum, freshmenclass)" << std::endl;

		  std::cin >> name >> stra >> strb;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(stra)) < 0) throw Error("ERR: Invalid argument");
		  if( (freshmenclass = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  
		  myman.find_student(name, stunum, freshmenclass);
		  break;
		}
		throw Error("Invalid argument");
	  }


	  case 'd':{
		std::cout << "Delete student executed\n\nType\nUndergraduate:0 Graduate:1" << std::endl;

		std::cin >> stra;
		while(1){
		  c = cin.get();
		  if(c == '\n') break;
		}
		isgrad = C.IsDigit(stra);
		
		if (isgrad == 1){
		  std::cout << "\nFormat: (name, stunum, labname)" << std::endl;
		  
		  std::cin >> name >> strb >> labname;
		  while(1){
		    c = cin.get();
		    if(c == '\n') break;
		  }
		    
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  C.IsAlpha(labname);
	  
		  myman.delete_student(name, stunum, labname);
		  break;
		}
		else if (isgrad == 0){
		  std::cout << "\nFormat: (name, stunum, freshmenclass)" << std::endl;
		  
		  std::cin >> name >> stra >> strb;
		  while(1){
		    c - cin.get();
		    if(c == '\n') break;
		  }
		    
		  C.IsAlpha(name);
		  if( (stunum = C.IsDigit(stra)) < 0) throw Error("ERR: Invalid argument");
		  if( (freshmenclass = C.IsDigit(strb)) < 0) throw Error("ERR: Invalid argument");
		  
		  myman.delete_student(name, stunum, freshmenclass);
		  break;
		}
		throw Error("ERR: Invalid argument");
	  }

	  case 'p':{
		cout << "Print all executed" << endl;
		myman.print_all();
	  }break;

	  default:{
	    throw Error("ERR: Invalid argument");
	  }
	  }
    }
    catch(Error& E){
      cerr << E.geterr() << endl;
    }
  }
  return 0;
}

