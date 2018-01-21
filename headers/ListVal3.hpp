#ifndef LISTVAL3_HPP
#define LISTVAL3_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "Journal.hpp"
#include "ExtendableHashtable.hpp"
using namespace std;

class List3 
{
	struct queries{
		uint64_t query[3];
		queries *next;
		
		queries(uint64_t *a,queries* n);
	};
	
	struct exp
	{
		int rid;
		exp		*next;
		queries *query;
		int size;
		
		exp(int a,exp *n);
	};
	
	struct val
	{
		int vid;
		int from;
		int to;
		int exp_size;
		int value;

		val*  next;
		exp*  right;
		val (int a,int b,int c);
	};

	val* _start;
	int   _size;

public:

	List3 ();

	~List3 ();

	List3 (val *n);

	int size ();

	int getfirstvid();

	int push (int vid2,int rid2,int pos,int from,int to,uint64_t *query);

	bool popAt ();

	int popAt2 (Journal *j,ExtendableHashtable *h,int id);

	int return_value();

	val* returnptr(){
		val *t=_start;
		while(t->next!=NULL){
			t=t->next;
		}
		return t;
	}
	
	void printList();

	void nullifier();

};

int popAt2 (List3* l3,Journal *j,ExtendableHashtable *h,int id);

#endif
