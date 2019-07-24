/***********************************/
/* Assignment 3                    */
/* File name : customer_manager2.c */
/* Made by : In Je Hwang, 20160788 */
/***********************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"
#define UNIT_ARRAY_SIZE 1024
#define HASH_MULTIPLIER 65599
#define MAX_NUMITEMS 1000000

struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
  struct UserInfo *id_key;
  struct UserInfo *name_key;
};

struct DB {
  struct UserInfo **pArrayName; // pointer to the name bucket,
                                // it will be used in Hash table
                                // data structure
  
  struct UserInfo **pArrayID; // pointer to the id bucket,
                              // it will be used in Hash table
                              // data structure
  
  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // whether the array should be expanded
			     // or not
};

/*--------------------------------------------------------------------*/
/* hash_function                                                      */
/* Calculate argumet and return hashnumber.                           */
/* Parameter : pcKey(want to calculate), iBucketCount(size of bucket) */
/* Return value : hashnumber like below                               */ 
/*                (int)(uiHash & ((unsigned int)iBucketCount - 1))    */
/* About Stream : None                                                */
/* About global variable : None                                       */
/*--------------------------------------------------------------------*/
static int hash_function(const char *pcKey, int iBucketCount)
{
   int i;
   unsigned int uiHash = 0U;
   
   for (i = 0; pcKey[i] != '\0'; i++)
      uiHash = uiHash * (unsigned int)HASH_MULTIPLIER
               + (unsigned int)pcKey[i];
   
   return (int)(uiHash & ((unsigned int)iBucketCount - 1));
}
/*--------------------------------------------------------------------*/
/* CreateCustomerDB                                                   */
/* Create customer database that contains two buckets storing pointer */
/* which points to the data of user informatoin by name and id,       */
/* the number of registered user information number, and size         */
/* of buckets.                                                         */
/* Also return pointer that points to database.                       */
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
  d->pArrayName = (struct UserInfo **)calloc(d->curArrSize,
               sizeof(struct UserInfo *));
  if (d->pArrayName == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  d->pArrayID = (struct UserInfo **)calloc(d->curArrSize,
	      sizeof(struct UserInfo *));
  if(d->pArrayID == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);
    free(d->pArrayName);
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
  struct UserInfo *p, *temp;
  
  if(d == NULL)
    return;
  
  for(i = 0; (i < d->curArrSize) && (d->numItems != 0 );i++){
    if(d->pArrayName[i] == NULL)
      continue;
    else{
      for(p = d->pArrayName[i]; p == NULL ; p = temp){
	temp = p->name_key;
	free(p->name);
	free(p->id);
	free(p);
      }
    }
  }
  free(d->pArrayName);
  free(d->pArrayID);
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
  int i, j, count = 0;
  struct UserInfo *p, *tempn = NULL, *tempi = NULL,
    *q, **old_name, **old_id;
  /*check argument is valid*/
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
  /*expansion of hash table*/
  if(d->numItems == (0.75 * d->curArrSize) &&
     (d->numItems <= MAX_NUMITEMS)){
    old_name = d->pArrayName;
    (d->pArrayName) =
      (struct UserInfo **)calloc(d->curArrSize + UNIT_ARRAY_SIZE,
				 sizeof(struct UserInfo*));
    if(d->pArrayName == NULL){
      fprintf(stderr,"Can't allocate a memory for array of size %d\n",
	      d->curArrSize + UNIT_ARRAY_SIZE);
      d->pArrayName = old_name;
      return -1;
    }
    old_id = d->pArrayID;
    (d->pArrayID) =
      (struct UserInfo **)calloc(d->curArrSize + UNIT_ARRAY_SIZE,
				  sizeof(struct UserInfo*));
    if(d->pArrayID == NULL){
      fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	      d->curArrSize + UNIT_ARRAY_SIZE);
      d->pArrayID = old_id;
      return -1;
    }
    d->curArrSize += UNIT_ARRAY_SIZE;
    /*set new bucket*/
    for(i = 0 ; i < (d->curArrSize-UNIT_ARRAY_SIZE); i++){
      /*set name bucket*/
      for(p = old_name[i] ; p != NULL ; tempn = NULL){
	j = hash_function(p->name, d->curArrSize);
	for(q = d->pArrayName[j] ; q != NULL ; q = q->name_key)
	  tempn = q;
	if(tempn == NULL){
	  d->pArrayName[j] = p;
	  tempn = p; //for swap
	  p = p->name_key;
	  tempn->name_key = NULL;
	  ++count;
	}
	else{
	  tempn->name_key = p;
	  tempn = p;
	  p = p->name_key;
	  tempn->name_key = NULL;
	  ++count;
	}
      }
      /*set id bucket*/
      for(p = old_id[i] ; p != NULL; tempi = NULL){
	j = hash_function(p->id, d->curArrSize);
	for(q = d->pArrayID[j] ; q != NULL ; q = q->id_key)
	  tempi = q;
	if(tempi == NULL){
	  d->pArrayID[j] = p;
	  tempi = p;
	  p = p->id_key;
	  tempi->id_key = NULL;
	  ++count;
	}
	else{
	  tempi->id_key = p;
	  tempi = p;
	  p = p->id_key;
	  tempi->id_key = NULL;
	  ++count;
	}
      }
      if((count/2) == d->numItems)
	break;
    }
    /*delete old bucket*/
    free(old_id);
    free(old_name);
  }
  /*check proper position and duplication*/
  i = hash_function(name, d->curArrSize);
  for(p = d->pArrayName[i];
      p != NULL ;p = (tempn->name_key)){
    tempn = p;
    if(strcmp(p->name, name) == 0){
      fprintf(stderr, "This name is already registered\n");
      return -1;
    }
  }
  j = hash_function(id, d->curArrSize);
  for(p = d->pArrayID[j];
      p != NULL ;p = (tempi->id_key)){
    tempi = p;
    if(strcmp(p->id, id) == 0){
      fprintf(stderr, "This ID is already registered\n");
      return -1;
    }
  }
  /*make a node*/
  q = (struct UserInfo *)malloc(sizeof(struct UserInfo));
  if(q == NULL){
    fprintf(stderr, "Can't allocate a memory for new customer Info\n");
    return -1;
  }
  q->name = (char*)malloc(strlen(name)+1);
  if(q->name == NULL){
    fprintf(stderr, "Can't allocate a memory for name\n");
    return -1;
  }
  q->id = (char*)malloc(strlen(id)+1);
  if(q->id == NULL){
    fprintf(stderr, "Can't allocate a memory for ID\n");
    free(q->name);
    return -1;
  }
  strcpy(q->name,name);
  strcpy(q->id, id);
  q->purchase = purchase;
  q->name_key = q->id_key = NULL;
  
  /*insert node in hash table*/
  if((tempn == NULL)&&(tempi == NULL)){
    d->pArrayName[i] = q;
    d->pArrayID[j] = q;
  }
  else if((tempn == NULL)&&(tempi != NULL)){
    d->pArrayName[i] = q;
    tempi->id_key = q;
  }
  else if((tempn != NULL)&&(tempi == NULL)){
    tempn->name_key = q;
    d->pArrayID[j] = q;
  }
  else{
    tempn->name_key = q;
    tempi->id_key = q;
  }

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
  int i, j, k;
  struct UserInfo *p, *q, *tempi=NULL, *tempn=NULL ;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(id == NULL){
    fprintf(stderr, "ID is invalid\n");
    return -1;
  }
  /*get pointer by id(points to unregistered node)*/
  i = hash_function(id, UNIT_ARRAY_SIZE);
  while(i < d->curArrSize){
    for(p = d->pArrayID[i]; p != NULL; p = p->id_key){
      if((k = strcmp(p->id, id)) == 0)
	break;
      else
	tempi = p;
    }
    if(k == 0)
      break;
    else{
      tempi = NULL;
      i+=UNIT_ARRAY_SIZE;
    }
  }
  /*get pointer to unregister node*/
  i = hash_function(id, d->curArrSize);
  for(p = d->pArrayID[i]; p != NULL; p = (p->id_key)){
    if(strcmp(p->id, id) == 0)
      break;
    tempi = p;
  }
  if(p == NULL){
    fprintf(stderr, "This ID is not registered\n");
    return -1;
  }
  j = hash_function(p->name, d->curArrSize);
  for(q = d->pArrayName[j]; q != NULL; q = (q->name_key)){
    if(strcmp(q->name, p->name) == 0)
      break;
    tempn = q;
  }
  /*unregister the node by id*/
  if((tempi == NULL)&&(tempn == NULL)){
    d->pArrayID[i] = p->id_key;
    d->pArrayName[j] = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else if((tempi == NULL)&&(tempn != NULL)){
    d->pArrayID[i] = p->id_key;
    tempn->name_key = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else if((tempi != NULL)&&(tempn == NULL)){
    tempi->id_key = p->id_key;
    d->pArrayName[j] = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else{
    tempi->id_key = p->id_key;
    tempn->name_key = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
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
  int i, j;
  struct UserInfo *p,*q, *tempi = NULL, *tempn = NULL;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(name == NULL){
    fprintf(stderr, "Name is invalid");
    return -1;
  }
  /*get pointer to unregister node*/
  i = hash_function(name, d->curArrSize);
  for(p = d->pArrayName[i]; p != NULL; p = (p->name_key)){
    if(strcmp(p->name, name) == 0){
      break;
    }
    tempn = p;
  }
  if(p == NULL){
    fprintf(stderr, "This name is not registered\n");
    return -1;
  }
  j = hash_function(p->id, d->curArrSize);
  for(q = d->pArrayID[j]; q != NULL; q = (q->id_key)){
    if(strcmp(q->id, p->id) == 0)
      break;
    tempi = q;
  }

  /*unregister node by name*/
  if((tempi == NULL)&&(tempn == NULL)){
    d->pArrayID[j] = p->id_key;
    d->pArrayName[i] = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else if((tempi == NULL)&&(tempn != NULL)){
    d->pArrayID[j] = p->id_key;
    tempn->name_key = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else if((tempi != NULL)&&(tempn == NULL)){
    tempi->id_key = p->id_key;
    d->pArrayName[i] = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
  else{
    tempi->id_key = p->id_key;
    tempn->name_key = p->name_key;
    free(p->name);
    free(p->id);
    free(p);
    --(d->numItems);
    return 0;
  }
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
  struct UserInfo *p;
  
  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(id == NULL){
    fprintf(stderr, "ID is invalid\n");
    return -1;
  }
  /*search by id*/
  i = hash_function(id, d->curArrSize);
  for(p = d->pArrayID[i]; p != NULL ;p = (p->id_key) ){
    if(strcmp(id, p->id) == 0)
      return p->purchase;
    else continue;
  }

  fprintf(stderr, "This ID is not registered\n");
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
  struct UserInfo *p;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(name == NULL){
    fprintf(stderr, "Name is invalid\n");
    return -1;
  }

  i = hash_function(name, d->curArrSize);
  for(p = d->pArrayName[i]; p != NULL ; p = (p->name_key)){
    if(strcmp(name, p->name) == 0)
      return p->purchase;
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
  struct UserInfo *p;
  int i, check = 0, sum = 0;

  if(d == NULL){
    fprintf(stderr, "Customer Database is invalid\n");
    return -1;
  }
  if(fp == NULL){
    fprintf(stderr, "Function is invalid\n");
    return -1;
  }

  for(i = 0; i < d->curArrSize; i++){
    if(check == d->numItems)
      break;
    for(p = d->pArrayID[i]; p != NULL; p = (p->id_key)){
      sum += fp(p->id, p->name,p->purchase);
      ++check;
    }
  }

  return sum;
}
