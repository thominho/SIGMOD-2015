#ifndef BITMAP_HPP
#define BITMAP_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

using namespace std;

struct bitmap{

	uint8_t* bits;
	
	bitmap();
	~bitmap();
	void allocation(int length);
};

#endif