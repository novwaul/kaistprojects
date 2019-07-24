// //***************************************************************
// //                   CLASS USED IN PROJECT
// //****************************************************************

/* 20160788 InJe Hwang */

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>

const int MAX = 300;
using namespace std;

class Error{
 public:
  Error(const string& msg);
  string geterr();
 protected:
 private:
  string err;
};

class Check{
 public:
  int IsDigit(string& input) throw(Error);
  void IsAlpha(string& input) throw(Error);
};

class Manager{
 private:
  int size;
  class Student* studentlist[MAX];
 protected:
 public:
  Manager();
  ~Manager();
  int add_student(std::string name, int stunum, std::string labname);
  int add_student(std::string name, int stunum, int freshmenclass);
  bool compare_student(int index, std::string name, int stunum, std::string labname);
  bool compare_student(int index, std::string name, int stunum, int freshmenclass);
  int find_student(std::string name, int stunum, std::string labname);
  int find_student(std::string name, int stunum, int freshemenclass);
  int delete_student(std::string name, int stunum, std::string labname);
  int delete_student(std::string name, int stunum, int freshmenclass);
  int print_all();
  int getSize();
};

class Student{
 private:
 protected:
  string name;
  int stunum;
 public:
  Student(string& Name, int Stunum);
  virtual ~Student();
  virtual void getInfo() = 0;
  virtual bool operator==(class Grad_Student &G) = 0;
  virtual bool operator==(class Undergrad_Student& U) = 0;
  string& getName();
  int getID();
};

class Grad_Student: public Student{
 private:
 protected:
  string labname;
 public:
  Grad_Student(string& Name, int Stunum, string& Labname);
  virtual ~Grad_Student();
  virtual void getInfo();
  virtual bool operator==(Grad_Student& G);
  virtual bool operator==(class Undergrad_Student& U);
  string& getLab();
};

class Undergrad_Student: public Student{
 private:
 protected:
  int freshmenclass;
 public:
  Undergrad_Student(string& Name, int Stunum, int Freshmenclass);
  virtual ~Undergrad_Student();
  virtual void getInfo();
  virtual bool operator==(Grad_Student& G);
  virtual bool operator==(Undergrad_Student& U);
  int getClass();
};
