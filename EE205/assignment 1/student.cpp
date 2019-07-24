// //****************************************************************
// //                   REQUIRED FUNCTIONALITY IN PROJECT
// //****************************************************************

/* 20160788 InJe Hwang */

#include "student.h"

/*-----------------------------------------------------------------------------------------------*/
Error::Error(const string& msg) : err(msg) {}

string Error::geterr(){ return err; }
/*---------------------------------------------------------------------------------------------*/
int Check::IsDigit(string& input) throw(Error){
  int size = input.size();
  for(int i = 0; i < size; i++)
    if(isdigit(input[i]) == 0)
      throw Error("ERR: Invalid argument");
  return atoi(input.c_str());
}

void Check::IsAlpha(string& input) throw(Error){
  int size = input.size();
  for(int i = 0; i < size; i++){
    if(isalpha(input[i]) == 0)
      throw Error("ERR: Invalid argument");
  }
}
/*--------------------------------------------------------------------------------------------------*/
Manager::Manager() : size(0) {}

Manager::~Manager(){
  for( int i = 0; i < size; i++)
    delete studentlist[i];
}

int Manager::getSize() { return size; }

int Manager::add_student(std::string name, int stunum, std::string labname)
{
  studentlist[size] = new Grad_Student(name, stunum, labname);
  size++;
  cout << "add graduate student DONE" << endl;
  return size;
};

int Manager::add_student(std::string name, int stunum, int freshmenclass)
{
  studentlist[size] = new Undergrad_Student(name, stunum, freshmenclass);
  size++;
  cout << "add undergraduate student DONE" << endl;
  return 0;
};


bool Manager::compare_student(int index, std::string name, int stunum, int freshmenclass)
{
  Undergrad_Student U(name, stunum, freshmenclass);
  if(*studentlist[index - 1] == U){
    cout<<"Matched\n" << "compare to undergraduate student DONE" << endl;
    return true;
  }
  cout << "Not matched\n" << "compare to undergraduate student DONE" << endl;
  return false;
};

bool Manager::compare_student(int index, std::string name, int stunum, std::string labname)
{
  Grad_Student G(name, stunum, labname);
  if(*studentlist[index - 1] == G){
    cout<<"Matched\n" << "compare to graduate student DONE" << endl;
    return true;
  }
  cout << "Not matched\n"<<"compare to graduate student DONE" << endl;
  return false;
};

int Manager::find_student(std::string name, int stunum, std::string labname)
{
  int iarray[size], i, index = 0;
  Grad_Student G(name, stunum, labname);
  for(i = 0; i < size; i++){
    if(*studentlist[i] == G){
      iarray[index] = i + 1;
      index++;
      cout<< "*********************************\nIndex: " << i + 1 << endl;
      studentlist[i]->getInfo();
    }
  }
  if(index == 0){
    cout<< "Not found\nfind graduate student DONE" << endl;
    return 0;
  }
  cout << "find graduate student DONE" << endl;
  return iarray[0];
};

int Manager::find_student(std::string name, int stunum, int freshmenclass)
{
  int iarray[size], i, index = 0;
  Undergrad_Student U(name, stunum, freshmenclass);
  for(i = 0; i < size; i++){
    if(*studentlist[i] == U){
      iarray[index] = i + 1;
      index++;
      cout << "*********************************\nIndex: "<< i + 1 << endl;
      studentlist[i]->getInfo();
    }
  }
  if(index == 0){
    cout<<"Not found\nfind undergraduate student DONE" << endl;
    return 0;
  }
  cout << "find undergraduate student DONE" << endl;
  return iarray[0];
};

int Manager::delete_student(std::string name, int stunum, std::string labname)
{
  Grad_Student G(name, stunum, labname);
  int i;
  for( i = 0 ; i < size ; i++){
    if(*studentlist[i] == G)
      break;
  }
  if(i == size){
    cout<< "Not found\n delete graduate student DONE" << endl;
    return size;
  }
  delete studentlist[i];
  size--;
  for( i ; i < size ; i++){
    studentlist[i] = studentlist[i + 1];
  }
  studentlist[i] = NULL;
  cout << "delete graduate student DONE" << endl;
  return size;
};

int Manager::delete_student(std::string name, int stunum, int freshmenclass)
{
  Undergrad_Student U(name, stunum, freshmenclass);
  int i;
  for(i = 0; i < size; i++){
    if(*studentlist[i] == U)
      break;
  }
  if(i == size){
    cout<< "Not found\ndelete undergraduate student DONE" << endl;
    return size;
  }
  delete studentlist[i];
  size--;
  for(i ; i < size; i++){
    studentlist[i] = studentlist[i+1];
  }
  studentlist[i] = NULL;
  cout << "delete undergraduate student DONE" << endl;
  return size;
};

int Manager::print_all()
{
  for(int i = 0 ; i <size ; i++){
    cout <<"*********************************\nIndex: " << i + 1 << endl;
    studentlist[i]->getInfo();
  }
  cout << "print all DONE" << endl;
  return size;
};
/*--------------------------------------------------------------------------------------------------*/
Student::Student(string& Name, int Stunum) : name(Name), stunum(Stunum) {}

Student::~Student() {}

string& Student::getName() { return name; }

int Student::getID() { return stunum; }
/*---------------------------------------------------------------------------------------------------------------*/
Grad_Student::Grad_Student(string& Name, int Stunum, string& Labname) : Student(Name, Stunum), labname(Labname) {}

Grad_Student::~Grad_Student() {}

void Grad_Student::getInfo(){
  cout<< "*********************************\nNAME: " << name << "\nSTUDENT ID: " << stunum <<
    "\nLAB NAME: " << labname << endl;
}

string& Grad_Student::getLab() { return labname; }

bool Grad_Student::operator==(Grad_Student& G){
  if((G.getName() == name) && (G.getID() == stunum) && (G.getLab() == labname))
    return true;
  else return false;
}

bool Grad_Student::operator==(Undergrad_Student& U){
  return false;
}
/*----------------------------------------------------------------------------------------------------------------*/
Undergrad_Student::Undergrad_Student(string& Name, int Stunum, int Freshmenclass) : Student(Name, Stunum), freshmenclass(Freshmenclass) {}

Undergrad_Student::~Undergrad_Student() {}

void Undergrad_Student::getInfo() {
  cout <<"*********************************\nNAME: " << name << "\nSTUDENT ID: " << stunum << "\nFRESHMEN CLASS: " << freshmenclass << endl;
}

int Undergrad_Student::getClass() { return freshmenclass; }

bool Undergrad_Student::operator==(Undergrad_Student& U){
  if((U.getName() == name) && (U.getID() == stunum) && (U.getClass() == freshmenclass))
    return true;
  else return false;
}

bool Undergrad_Student::operator==(Grad_Student& G){
  return false;
}

 
