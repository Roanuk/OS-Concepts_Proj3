
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <queue>

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
		string name;
		int arrivalT, durationT, executionT;
		Job(string Name, int ArrivalT, int DurationT) : name(Name), arrivalT(ArrivalT), durationT(DurationT)
		{
			executionT = 0;
		}
		
		//standard version of run continues until completion
		string run()
		{
			return run(durationT);
		}
		
		//Overloaded version of run takes a number of slices
		string run(int slices)
		{
			int runSlices = 0;
			string returner(min(durationT,slices),' ');
			while(executionT < durationT && runSlices < slices)
			{
				returner[runSlices] = name[0];
				executionT++;
				runSlices++;
			}
			return returner;
		}
};

class Scheduler 
{
	public:
		Scheduler(vector<Job>* JobList): jobList(JobList)
		{
			outputGraph.resize(jobList->size()+1); //make atleast as many rows in outputGraph as there are jobs + title row
		}
		
		virtual void processJobs() = 0; //every scheduler must process job output into outputGraph

		void reportOutput()
		{
			for(int outer = 0; outer < outputGraph.size();outer++)
			{
				for(int inner = 0; inner < outputGraph[outer].size(); inner++)
				{
					printf("%c",outputGraph[outer][inner]);
				}
				printf("\n");
			}
		}

	protected:
		vector<Job>* jobList; 
		vector< vector<char> > outputGraph;
};

class scheduleFCFS : public Scheduler
{
	public:
		scheduleFCFS(vector<Job>* JobList) : Scheduler(JobList) {}
		
		void processJobs()
		{
			int inner, totalT = 0;
			string buffer = "First Come First Serve Graph";
			for(int outer = 0; outer < jobList->size()+1;outer++)
			{				
				outputGraph[outer].resize(buffer.size()+totalT,' ');
				for(inner = 0; inner < buffer.size(); inner++)
				{
					outputGraph[outer][inner+totalT] = buffer[inner];
				}								
				buffer = (*jobList)[outer].run();
				totalT += outer == 0 ? 0 : inner;
			}

		}
};

class scheduleRR : public Scheduler
{
	public:
		int quantum;
		scheduleRR(vector<Job>* JobList, int Quantum) : Scheduler(JobList), quantum(Quantum) {}
		
		void processJobs()
		{
			string buffer = "Round Robin Graph";
			outputGraph[0].resize(buffer.size(),' ');
			for(int i = 0; i < buffer.size(); i++)
			{
				outputGraph[0][i] = buffer[i];
			}				
		}
};

class OS
{
	public:
		OS(ifstream* InputFile, SchedulerType SchedulerType, int Quantum) : inputFile(InputFile), schedulerType(SchedulerType), quantum(Quantum) {}

		Scheduler* executeScheduler() //returns Scheduler object
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
			switch(schedulerType)
			{
				case fcfs:
						return new scheduleFCFS(&jobList);
					break;

				case rr:
						return new scheduleRR(&jobList, quantum);
					break;

				case spn:

					break;

				case srt:

					break;

				case hrrn:

					break;

				case fb:

					break;
			}
		}

	protected:
		ifstream* inputFile;
		SchedulerType schedulerType;
		int quantum;
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
	Scheduler* instanceScheduler = instanceOS.executeScheduler();
	instanceScheduler->processJobs();
	instanceScheduler->reportOutput();
	delete(instanceScheduler);
	inputFile.close();
	return 0;
}
