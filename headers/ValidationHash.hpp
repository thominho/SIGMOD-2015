#ifndef VALIDATION_HASH_HPP
#define VALIDATION_HASH_HPP
#define VAL_HASH_BUCKET_SIZE 1
#define VAL_HASH_STARTING_SIZE 256
#define VAL_HASH_OVERFLOW_BLOCK 64
#include "ValidationPredicate.hpp"

class ValidationHash
{
	struct Bucket
	{
		ValidationPredicate* valPreds[VAL_HASH_BUCKET_SIZE];
		unsigned int currentSize;
		unsigned int depth;

		Bucket(unsigned int d);
		~Bucket();

		ValidationPredicate* insert(unsigned int from, unsigned int to, uint64_t* condition);
		void insert(ValidationPredicate* pred);
	};

	Bucket** index;

	unsigned int currentSize;
	unsigned int totalSize;
	unsigned int globalDepth;

	Bucket** overflow;

	unsigned int overflowCurrentSize;
	unsigned int overflowTotalSize;

	uint32_t hash(unsigned int from, unsigned int to, uint64_t* condition);

	ValidationPredicate* insertToOverflow(unsigned int from, unsigned int to, uint64_t* condition);

public:

	ValidationHash();
	~ValidationHash();

	ValidationPredicate* insert(unsigned int from, unsigned int to, uint64_t* condition);
	ValidationPredicate* find(unsigned int from, unsigned int to, uint64_t* condition);

	unsigned int getOffset(ValidationPredicate* vp);

	/*Removes vp from the hashtable and returns a pointer to the object removed*/
	void remove(unsigned int offset);
};

#endif