/***********************************/
/* Assignment 3                    */
/* File name : customer_manager1.c */
/* Made by : In Je Hwang, 20160788 */
/***********************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"
#define UNIT_ARRAY_SIZE 1024

struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
};

struct DB {
  struct UserInfo *pArray;   // poiner to the array,
                             // it will be used in the array
                             // data structure

  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // whether the array should be expanded
			     // or not
};

/*--------------------------------------------------------------------*/
/* CreateCustomerDB                                                   */
/* Create customer database that contains array storing data of user  */
/* informatoin, the number of registered user information number,     */
/* and size of the array. And return pointer that points to database. */
/* Parameter : None                                                   */
/* Return value : d or NULL(if fail to make database)                 */
/* About stream : Prints error messages in stderr when it fails       */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
DB_T
CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T) malloc(sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 1024 elements
  d->numItems = 0;
  d->pArray = (struct UserInfo *)calloc(d->curArrSize,
	     sizeof(struct UserInfo));
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;
}
/*--------------------------------------------------------------------*/
/* DestroyCustomerDB                                                  */
/* Delete customer database completely.                               */
/* Parameter : d(pointer that points to database)                     */
/* Return value : None                                                */
/* About stream : None                                                */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
void
DestroyCustomerDB(DB_T d)
{
  int i;
  
  if(d == NULL)
    return;

  for(i = 0; i < d->numItems ; i++){
    if(d->pArray[i].name == NULL && d->pArray[i].id == NULL)
      continue;
    else{
      free(d->pArray[i].name);
      free(d->pArray[i].id);
    }
  }
  free(d->pArray);
  free(d);

  return;
}
/*--------------------------------------------------------------------*/
/* RegisterCustomer                                                   */
/* Regist new user(customer) information and expand if database is    */
/* full.                                                              */
/* Parameter : d(database pointer),                                   */
/*             id, name, purchase(information of new user)            */
/* Return value : 0(succeed), -1(fail)                                */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  int i, position;
  struct UserInfo *old;
  
  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(id == NULL){
    fprintf(stderr, "ID is invalid\n");
    return -1;
  }
  if(name == NULL){
    fprintf(stderr, "Name is invalid\n");
    return -1;
  }
  if(purchase <= 0){
    fprintf(stderr, "Purchase is invalid\n");
    return -1;
  }
  /*check duplication*/
  for(i = 0; i < (d->numItems); i++){
    if((d->pArray[i].name) == NULL && (d->pArray[i].id) == NULL){
      continue;
    }
    if(strcmp((d->pArray)[i].name, name) == 0){
      fprintf(stderr, "This name is already registered\n");
      return -1;
    }
    if(strcmp((d->pArray)[i].id, id) == 0){
      fprintf(stderr, "This ID is already registered\n");
      return -1;
    }
  }
  /*array expansion*/
  if((d->numItems) == (d->curArrSize)){
    old = d->pArray;
    (d->pArray) =
      (struct UserInfo *)realloc(d->pArray,
	sizeof(struct UserInfo)*(d->curArrSize + UNIT_ARRAY_SIZE));

    if(d->pArray == NULL){
      fprintf(stderr,"Can't allocate a memory for array of size %d\n",
	      d->curArrSize += UNIT_ARRAY_SIZE);
      d->pArray = old;
      return -1;
    }
    old = NULL;
    for(i = (d->curArrSize);i < (d->curArrSize+UNIT_ARRAY_SIZE); i++){
      d->pArray[i].name = NULL;
      d->pArray[i].id = NULL;
      d->pArray[i].purchase = 0;
    }
    d->curArrSize += UNIT_ARRAY_SIZE;
  }
  /*get position to store new data*/
  for(i = 0; i < (d->curArrSize); i++){
    if (d->pArray[i].name == NULL && d->pArray[i].id == NULL){
      position = i;
      break;
    }
    else continue;
  }
  /*make new data*/
  d->pArray[position].name = (char*)malloc(strlen(name)+1);
  if(d->pArray[position].name == NULL){
    fprintf(stderr, "Can't allocate a memory for name\n");
    return -1;
  }
  d->pArray[position].id = (char*)malloc(strlen(id)+1);
  if(d->pArray[position].id == NULL){
    fprintf(stderr, "Can't allocate a memory for ID\n");
    free(d->pArray[position].name);
    return -1;
  }
  /*store new data*/
  strcpy(d->pArray[position].name, name);
  strcpy(d->pArray[position].id, id);
  d->pArray[position].purchase = purchase;

  ++(d->numItems);

  return 0;
}
/*--------------------------------------------------------------------*/
/* UnregisterCustomerByID                                             */
/* Unregister a node by ID.                                           */
/* Parameter : d(database pointer),                                   */
/*             id(id that belongs to the node)                        */
/* Return value : 0(succeed), -1(fail)                                */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
UnregisterCustomerByID(DB_T d, const char *id)
{
  int i, check = 0;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid");
    return -1;
  }
  if(id == NULL){
    fprintf(stderr, "ID is invalid");
    return -1;
  }
  /*unregister by id*/
  for(i = 0; i < d->curArrSize; i++){
    if(d->pArray[i].id == NULL)
      continue;
    if(strcmp(d->pArray[i].id, id) == 0){
      free(d->pArray[i].id);
      free(d->pArray[i].name);
      d->pArray[i].id = NULL;
      d->pArray[i].name = NULL;
      d->pArray[i].purchase = 0;
      --(d->numItems);
      ++check;
    }
  }

  if(check == 0){
    fprintf(stderr, "This ID is not registered\n");
    return -1;
  }

  return 0;
}

/*--------------------------------------------------------------------*/
/* UnregisterCustomerByName                                           */
/* Unregister a node by name.                                         */
/* Parameter : d(database pointer),                                   */
/*             name(name that belongs to the node)                    */
/* Return value : 0(succeed), -1(fail)                                */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  int i, check=0;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid");
    return -1;
  }
  if(name == NULL){
    fprintf(stderr, "Name is invalid");
    return -1;
  }
  /*unregister by name*/
  for(i = 0; i < d->curArrSize; i++){
    if(d->pArray[i].name == NULL){
      continue;
    }
    if(strcmp(d->pArray[i].name, name) == 0){
      free(d->pArray[i].name);
      free(d->pArray[i].id);
      d->pArray[i].name = NULL;
      d->pArray[i].id = NULL;
      d->pArray[i].purchase = 0;
      ++check;
      --(d->numItems);
    }
  }

  if(check == 0){
    fprintf(stderr, "This name is not registered\n");
    return -1;
  }
  
  return 0;
}
/*--------------------------------------------------------------------*/
/* GetPurchaseByID                                                    */
/* Search a node by ID and return its purchase.                       */
/* Parameter : d(database pointer),                                   */
/*             id(id that belongs to the node)                        */
/* Return value : d->pArray[i].purchase(purchase of the node),        */
/*                -1(fail)                                            */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
GetPurchaseByID(DB_T d, const char* id)
{
  int i;
  
  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(id == NULL){
    fprintf(stderr, "ID is invalid\n");
    return -1;
  }

  for(i = 0; i < d->numItems ; i++){
    if(d->pArray[i].id == NULL)
      continue;
    if(strcmp(d->pArray[i].id, id) == 0)
      return d->pArray[i].purchase;
  }

  fprintf(stderr, "This id is not registered\n");
  return -1;
}
/*--------------------------------------------------------------------*/
/* GetPurchaseByName                                                  */
/* Search a node by name and return its purchase                      */
/* Parameter : d(database pointer),                                   */
/*             name(name that belongs to the node)                    */
/* Return value : d->pArray[i].purchase(purchase of the node),        */
/*                -1(fail)                                            */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
GetPurchaseByName(DB_T d, const char* name)
{
  int i;
  
  if(d == NULL){
    fprintf(stderr,"Customer Database is invalid\n");
    return -1;
  }
  if(name == NULL){
    fprintf(stderr, "Name is invalid\n");
    return -1;
  }

  for(i = 0; i < d->numItems; i++){
    if(d->pArray[i].name == NULL)
      continue;
    if(strcmp(d->pArray[i].name, name) == 0)
      return d->pArray[i].purchase;
  }

  fprintf(stderr, "This name is not registered\n");
  return -1;
}
/*--------------------------------------------------------------------*/
/* GetSumCustomerPurchase                                             */
/* Return the total sum of purchase returned by calling fp for every  */
/* users(customers)                                                   */
/* Parameter : d(database pointer),                                   */
/*             fp(function pointer that points to function            */
/*                made by client                                      */
/* Return value : sum, -1(fail)                                       */
/* About stream : Prints error messages in stderr                     */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  int i, sum = 0;

  if(d == NULL){
    fprintf(stderr,"Customer Database is invalid\n");
    return -1;
  }
  if(fp == NULL){
    fprintf(stderr, "Function is invalid\n");
    return -1;
  }

  for(i = 0; i < d->numItems; i++){
    sum += fp(d->pArray[i].id, d->pArray[i].name,
	      d->pArray[i].purchase);
  }

  return sum;
}
