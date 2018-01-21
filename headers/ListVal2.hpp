#ifndef LISTVAL2_HPP
#define LISTVAL2_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "Journal.hpp"
#include "ExtendableHashtable.hpp"
#include "TridHash.hpp"
#include "ValidationPredicate.hpp"
#include "ValidationHash.hpp"
#include "zombie.hpp"
#include "Bitmap.hpp"
using namespace std;

class List2 
{
	struct queries{
		ValidationPredicate* query;
		queries *next;
		
		queries(ValidationPredicate *a,queries* n);
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
		int exp_size;

		val*  next;
		exp*  right;
		val (int a,int b,int c);
	};

	val* _start;
	int   _size;

public:

	List2 ();

	~List2 ();
	
	int getfirstvid();

	int push (int vid2,int rid2,int pos,int from,int to,ValidationPredicate *query);

	int popAt2 (Journal *j,ExtendableHashtable *h,TridHash *tid,ValidationHash *vh,zombie *l3);

	bool popAt ();

};

#endif
