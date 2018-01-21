#ifndef EXTENDABLE_HASHTABLE_HPP
#define EXTENDABLE_HASHTABLE_HPP
#define HASH_BLOCK 256
#define TRID_BLOCK 10
#define NUM_OF_OFFSETS 2
#define BUCKET_SIZE 1
#include <stdint.h>

class ExtendableHashtable
{
	struct TridsList
	{
		uint32_t* trid;
		unsigned int** offsets; /*offset-1 is the position in journal. offset=0 means no entry*/
		unsigned int size;
		unsigned int totalSize;
	};

	struct Bucket
	{
		uint32_t* keys;
		TridsList* trids;
		unsigned int depth;
		unsigned int currentSize;

		Bucket(unsigned int d);
		~Bucket();

		void insert(uint32_t key, uint32_t trid, unsigned int* offset);
		void insert(uint32_t key, TridsList& trid);
	};

	unsigned int totalSize;  /*total space allocated on index*/
	unsigned int globalDepth;
	unsigned int currentSize; /*space index is currently using*/
	unsigned int bucketSize; /*number of elements one bucket will be able to hold*/

	Bucket** index;

	static unsigned int* values;

	unsigned int hash(uint32_t key);

public:

	unsigned int testVar;

	ExtendableHashtable();
	~ExtendableHashtable();

	void insert(uint32_t key, uint32_t trid, unsigned int* offset);
	/*find returns an array of offsets. Each offset is the position in journal+1.
	the end of the array is marked by the value 0*/
	unsigned int* find(uint32_t key, uint32_t from, uint32_t to);
	/*find2 is a thread safe version of find but the pointer returned must be freed by the calling function*/
	unsigned int* find2(uint32_t key, uint32_t from, uint32_t to);
	unsigned int findLast(uint32_t key);

	/*for debugging purposes*/
	void printData();
};

#endif
