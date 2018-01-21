#include "../headers/JobScheduler.hpp"
#include <stdio.h>
#include <stdlib.h>

void* waitFunction(void* data)
{
	JobScheduler* sch = (JobScheduler*)data;

	while(1)
	{
		pthread_mutex_lock(&(sch->waitCondMutex));
		while(sch->jobQueueEmpty()){
			pthread_cond_wait(&(sch->waitCond),&(sch->waitCondMutex));
		}

		SchedulerFunc_t job = sch->getNextJob();

		pthread_mutex_unlock(&(sch->waitCondMutex));

		if(job.func == NULL) pthread_exit(NULL);

		int retVal = job.func(job.arg1,job.arg2,job.arg3,job.arg4);

		pthread_cond_signal(&(sch->barrierCond));
	}
}

SchedulerFunc_t JobScheduler::getNextJob()
{
	SchedulerFunc_t retVal;

	if(jobQueueEmpty())
	{
		retVal.func = NULL;
		retVal.arg1 = NULL;
		retVal.arg2 = NULL;
		retVal.arg3 = NULL;
		retVal.arg4 = 0;
		return retVal;
	}

	retVal = jobQueueStart->jobFunc;
	Job* temp = jobQueueStart;

	jobQueueStart = jobQueueStart->next;
	if(jobQueueStart == NULL) jobQueueEnd = NULL;

	free(temp);

	return retVal;
}

JobScheduler::JobScheduler(unsigned int num_of_threads)
{
	poolSize = num_of_threads;

	threadPool = (pthread_t*)malloc(poolSize*sizeof(pthread_t));
	if(threadPool == NULL)
	{
		perror("Failed to create Job Scheduler Thread Pool");
		exit(1);
	}

	if(pthread_cond_init(&waitCond,NULL) != 0)
	{
		perror("Could not initialise wait condition variable on Job Scheduler");
		exit(2);
	}
	if(pthread_mutex_init(&waitCondMutex,NULL) != 0)
	{
		perror("Could not initialise mutex for wait condition variable on Job Scheduler");
		exit(3);
	}

	if(pthread_cond_init(&barrierCond,NULL) != 0)
	{
		perror("Could not initialise barrier condition variable on Job Scheduler");
		exit(4);
	}
	if(pthread_mutex_init(&barrierCondMutex,NULL) != 0)
	{
		perror("Could not initialise mutex for barrier condition variable on Job Scheduler");
		exit(5);
	}

	jobQueueStart = NULL;
	jobQueueEnd = NULL;

	for(unsigned int i = 0;i < poolSize;i++)
	{
		if(pthread_create(&threadPool[i],NULL,waitFunction,this) != 0)
		{
			perror("Failed to create new thread in Job Scheduler");
			exit(6);
		}
	}
}

JobScheduler::~JobScheduler()
{
	for(unsigned int i = 0;i < poolSize;i++)
		this->addJob(NULL,NULL,NULL,NULL,0);

	pthread_mutex_destroy(&waitCondMutex);
	pthread_cond_destroy(&waitCond);
	pthread_mutex_destroy(&barrierCondMutex);
	pthread_cond_destroy(&barrierCond);

	void** temp = NULL;

	for(unsigned int i = 0;i < poolSize;i++)
		pthread_join(threadPool[i],temp);
	free(threadPool);

	while(jobQueueStart != NULL)
	{
		Job* temp = jobQueueStart;
		jobQueueStart = jobQueueStart->next;
		free(temp);
	}
}

void JobScheduler::addJob(int (*jobFunction)(List3*,Journal*,ExtendableHashtable*,int),List3* param1,Journal* param2,ExtendableHashtable* param3,int param4)
{
	Job* newJob = (Job*)malloc(sizeof(Job));
	if(newJob == NULL)
	{
		perror("Failed to create new Job on Job Scheduler Queue");
		exit(7);
	}

	SchedulerFunc_t funcData;
	funcData.func = jobFunction;
	funcData.arg1 = param1;
	funcData.arg2 = param2;
	funcData.arg3 = param3;
	funcData.arg4 = param4;

	newJob->jobFunc = funcData;
	newJob->next = NULL;

	if(jobQueueEnd == NULL)
	{
		jobQueueStart = newJob;
		jobQueueEnd = newJob;
	}
	else
	{
		jobQueueEnd->next = newJob;
		jobQueueEnd = newJob;
	}

	pthread_cond_signal(&waitCond);
}

bool JobScheduler::jobQueueEmpty()
{
	if(jobQueueStart == NULL && jobQueueEnd == NULL) return true;
	return false;
}

void JobScheduler::barrier()
{
	//tsekare prosektika mhpws exeis kanena la8os edw
	pthread_mutex_lock(&barrierCondMutex);
	while(!jobQueueEmpty()){
		pthread_cond_wait(&barrierCond,&barrierCondMutex);
	}
	pthread_mutex_unlock(&barrierCondMutex);
}