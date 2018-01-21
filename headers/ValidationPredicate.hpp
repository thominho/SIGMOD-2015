#ifndef VALIDATION_PREDICATE_HPP
#define VALIDATION_PREDICATE_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "Journal.hpp"
#include "ExtendableHashtable.hpp"
#include "TridHash.hpp"
#include "ValidationPredicate.hpp"
#include "Bitmap.hpp"
using namespace std;


struct ValidationPredicate
{
	/*range*/
	unsigned int range_from;
	unsigned int range_to;
	static uint64_t *validay;

	/*a struct which as a bitmap with some many positions as the registers take part in for this predicate*/
	bitmap value;

	/*Predicate condition, for example c0=1*/
	uint16_t column;
	uint8_t condition;
	uint64_t cond_value;

	/*counts how many validations are using this predicate currently*/
	unsigned int in_use_by;

	ValidationPredicate(uint64_t *query,unsigned int from,unsigned int to);
	~ValidationPredicate();
	uint8_t* getValue(Journal *j,ExtendableHashtable *h,TridHash *tid,int rid,int search_by,int until,int size);
	uint64_t* getCond();
	bool isPredicate(unsigned int cfrom,unsigned int cto,uint64_t *condition);
};

#endif