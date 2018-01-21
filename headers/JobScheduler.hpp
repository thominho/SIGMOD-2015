#ifndef JOB_SCHEDULER_HPP
#define JOB_SCHEDULER_HPP
#include <pthread.h>
#include "Journal.hpp"
#include "ExtendableHashtable.hpp"
#include "ListVal3.hpp"

struct SchedulerFunc_t
{
	int (*func)(List3* l3,Journal*,ExtendableHashtable*,int);
	List3* arg1;
	Journal* arg2;
	ExtendableHashtable* arg3;
	int arg4;
};

class JobScheduler
{
	struct Job
	{
		SchedulerFunc_t jobFunc;
		Job* next;
	};

	Job* jobQueueStart;
	Job* jobQueueEnd;
	pthread_t* threadPool;
	unsigned int poolSize;
public:
	pthread_cond_t waitCond;
	pthread_mutex_t waitCondMutex;

	pthread_cond_t barrierCond;
	pthread_mutex_t barrierCondMutex;

	SchedulerFunc_t getNextJob();

	JobScheduler(unsigned int num_of_threads);
	~JobScheduler();

	void addJob(int (*jobFunction)(List3*,Journal*,ExtendableHashtable*,int),List3* param1,Journal* param2,ExtendableHashtable* param3,int param4);

	void barrier();

	bool jobQueueEmpty();
};

#endif