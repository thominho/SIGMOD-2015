#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
using namespace std;

class zombie 
{	
	struct val
	{
		unsigned int offset;
		int from;
		val *next;
		
		val (int temp,int temp2,val *n);
	};

	val* _start;
	int   _size;

public:

	zombie ();

	~zombie ();

	int size ();

	int push (int temp,int temp2);

	unsigned int popAt2 (int temp);

	bool popAt ();
	

};

#endif
