#ifndef LISTVAL_HPP
#define LISTVAL_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "Journal.hpp"
#include "ExtendableHashtable.hpp"
using namespace std;

class List 
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

		val*  next;
		exp*  right;
		val (int a,int b,int c);
	};

	val* _start;
	int   _size;

public:

	List ();

	~List ();

	int size ();

	int getfirstvid();

	int push (int vid2,int rid2,int pos,int from,int to,uint64_t *query);

	int popAt2 (Journal *j,ExtendableHashtable *h);

	bool popAt ();
	
	void printList();

};

#endif
