
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#define FCFS "FCFS"
#define RR "RR"
#define SPN "SPN"
#define SRT "SRT"
#define HRRN "HRRN"
#define FB "FB"
#define BUFFER_SIZE 64
using namespace std;
enum SchedulerType { fcfs, rr, spn, srt, hrrn, fb};
typedef struct jobStruct
{
	char name;
	int arrivalT;
	int durationT;
}job;

void printSchedulers()
{
	printf("<SchedulerType> is one of: \n");
	printf("%s for First Come First Serve\n",FCFS);
	printf("%s for Round Robin (also specify a number for the quantum size)\n",RR);
	printf("%s for Shortest Process Next\n",SPN);
	printf("%s for Shortest Remaining Time\n",SRT);
	printf("%s for Highest Response Ratio Next\n",HRRN);
	printf("%s for Feedback (quantum of 1 used with 3 queues)\n",FB);
}

class Job
{
	public:
		Job(string Name, int ArrivalT, int DurationT) : name(Name), arrivalT(ArrivalT), durationT(DurationT)
		{
			executionT = 0;
		}
		
		//standard version of run continues until completion
		void run()
		{
			run(durationT);
		}
		
		//Overloaded version of run takes a number of slices
		void run(int slices)
		{
			int runSlices = 0;
			while(executionT < durationT && runSlices < slices)
			{
				printf("%s ",name.c_str());
				executionT++;
				runSlices++;
			}
		}
		
	protected:
		string name;
		int arrivalT, durationT, executionT;
};

class Scheduler 
{
	public:
		Scheduler(SchedulerType schedulerType, vector<Job>* JobList)
		{
			jobList = JobList;
			processJobs();
		}
		void processJobs()
		{
			int i;
			for(i = 0; i<jobList->size(); i++)
			{
				(*jobList)[i].run();
				printf("\n");
			}

		}
	protected:
		int sliceDur; //0 means nonpreemptive, >0 means # of slices per preemption
		int numJobs; //number of jobs in jobList
		vector<Job>* jobList; //
		vector< vector<char> > outputGraph;
};

class OS
{
	public:
		OS(ifstream* inputFile, SchedulerType schedulerType, int quantum)
		{
			char* inBuffer = (char*)malloc(BUFFER_SIZE);
			string name;
			char* notNull;
			int arrivalT;
			int durationT;
			while(!inputFile->eof())
			{
				inputFile->getline(inBuffer,BUFFER_SIZE);
				notNull = strtok(inBuffer,"	"); //tokens are space and tab
				if(notNull)
				{
					name = string(notNull);
					notNull = strtok(NULL,"	"); //tokens are space and tab
					if(notNull)
					{
						arrivalT = atoi(notNull);
						notNull = strtok(NULL,"	"); //tokens are space and tab
						if(notNull)
						{
							durationT = atoi(notNull);
							jobList.push_back(Job(name,arrivalT,durationT));
						}
					}
				}
			}
			free(inBuffer);		
			Scheduler instanceScheduler(schedulerType,&jobList);
		}
	protected:
		vector<Job> jobList;
};

int main (int argc, char* argv[])
{
	int quantum = 0;
	SchedulerType schedulerType;

	if(argc==4 && isdigit(argv[3][0]))
	{
		quantum = atoi(argv[3]);
	}
	else if(argc!=3)
	{
		printf("Incorrect command line arguments supplied\nPlease specify: inputFile.txt <SchedulerType>\n");
		printSchedulers();
		return 1; //failure
	}

	string schedulerString = string(argv[2]);
	
	if (schedulerString == FCFS)
	{
		schedulerType = fcfs;
	}
	else if (schedulerString == RR)
	{
			schedulerType = rr;
	}
	else if (schedulerString == SPN)
	{
			schedulerType = spn;
	}
	else if (schedulerString == SRT)
	{
		schedulerType = srt;
	}
	else if (schedulerString == HRRN)
	{
		schedulerType = hrrn;
	}
	else if (schedulerString == FB)
	{
		schedulerType = fb;
	}
	else
	{
		printf("Invalid SchedulerType: %s\n",argv[3]);
		printSchedulers();
		return 1; //error
	}

	ifstream inputFile;
	inputFile.open(argv[1],ifstream::in);

	if(!inputFile.is_open())
	{
		printf("File name %s failed to open\n", argv[1]);
		return 1; //error
	}

	OS instanceOS(&inputFile,schedulerType,quantum);
	inputFile.close();
	return 0;
}
