
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <iostream>

#define FCFS "FCFS"
#define RR "RR"
#define SPN "SPN"
#define SRT "SRT"
#define HRRN "HRRN"
#define FB "FB"
#define BUFFER_SIZE 64
#define FB_Queue_Count 3
#define FB_Quantum 1

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
		int arrivalT, durationT, executionT, ID;
		bool acknowledged;
		int* totalTp;
		int nextQue;
		Job(string Name, int ArrivalT, int DurationT) : name(Name), arrivalT(ArrivalT), durationT(DurationT)
		{
			executionT = 0;
			nextQue = 0;
			acknowledged = false;
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
			acknowledged = true;
			string returner(min(durationT-executionT,slices),' ');
			while(executionT < durationT && runSlices < slices)
			{
				returner[runSlices] = name[0];
				executionT++;
				runSlices++;
			}
			return returner;
		}
};

class ShortestNext
{
	public:
		bool operator() (Job* a, Job* b)
		{
			int aRemainingT =  a->durationT-a->executionT;
			int bRemainingT =  b->durationT-b->executionT;
			if(aRemainingT == bRemainingT)
			{
				return a->arrivalT > b->arrivalT; //return longest waiting with priority if remaining time is equal
			}
			else
			{
				return  aRemainingT > bRemainingT;
			}
		}
};

class LargestResponseNext
{
	public:
		bool operator() (Job* a, Job* b)
		{
			float aResponseT =  *(a->totalTp) - a->arrivalT + a->durationT;
			aResponseT /= a->durationT;
			float bResponseT =  *(b->totalTp) - b->arrivalT + b->durationT;	
			bResponseT /= b->durationT;		
			if(aResponseT == bResponseT)
			{
				return a->arrivalT > b->arrivalT; //return longest waiting with priority if remaining time is equal
			}
			else
			{
				return  aResponseT < bResponseT;
			}
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
				totalT += outer == 0 ? 0 : inner;
				totalT += max(0,(*jobList)[outer].arrivalT-totalT); //assumes that jobs arrive in order (mentioned in class)
				(*jobList)[outer].ID = outer;
				buffer = (*jobList)[outer].run();
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
			queue<Job*> jobQue;
			Job* running = NULL;
			int remainingT, totalT = 0;
			do{
				remainingT = 0;
				for(int jobI = 0; jobI < (*jobList).size(); jobI++)
				{
					remainingT += (*jobList)[jobI].durationT;
					remainingT -= (*jobList)[jobI].executionT;
					if(!(*jobList)[jobI].acknowledged && (*jobList)[jobI].arrivalT <= totalT)
					{
						(*jobList)[jobI].ID = jobI;
						jobQue.push(&((*jobList)[jobI])); //ensures job is not added to que until it arrives
						(*jobList)[jobI].acknowledged = true; //ensures job is not added more than once to que
					}
				}
				if(running && running->executionT < running->durationT)
				{
					jobQue.push(running);
				}
				if(!jobQue.empty())
				{
					running = jobQue.front();
					jobQue.pop(); //que is wierd and front is like peek and, pop returns void?					
					buffer = running->run(min(quantum, running->durationT - running->executionT)); //min is redundant verification of not over running
					outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
					for(int inner = 0; inner < buffer.size(); inner++)
					{
						outputGraph[1+running->ID][totalT] = buffer[inner];
						totalT++;
					}
				}
				else
				{
					totalT++;
				}
			}while(remainingT > 0);
		}
};

class scheduleSPN : public Scheduler
{
	public:
		scheduleSPN(vector<Job>* JobList) : Scheduler(JobList) {}

		void processJobs()
		{
			string buffer = "Shortest Process Next Graph";
			outputGraph[0].resize(buffer.size(),' ');
			for(int i = 0; i < buffer.size(); i++)
			{
				outputGraph[0][i] = buffer[i];
			}
			priority_queue<Job*,vector<Job*>,ShortestNext> jobQue;
			Job* running = NULL;
			int remainingT, totalT = 0;				
			do{				
				remainingT = 0;
				for(int jobI = 0; jobI < (*jobList).size(); jobI++)
				{
					remainingT += (*jobList)[jobI].durationT;
					remainingT -= (*jobList)[jobI].executionT;
					if(!(*jobList)[jobI].acknowledged && (*jobList)[jobI].arrivalT <= totalT)
					{
						(*jobList)[jobI].ID = jobI;
						jobQue.push(&((*jobList)[jobI])); //ensures job is not added to que until it arrives
						(*jobList)[jobI].acknowledged = true; //ensures job is not added more than once to que
					}
				}
				if(!jobQue.empty())
				{
					running = jobQue.top();
					jobQue.pop(); //que is wierd and front is like peek and, pop returns void?					
					buffer = running->run(); //run til completion == run(durationT)
					outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
					for(int inner = 0; inner < buffer.size(); inner++)
					{
						outputGraph[1+running->ID][totalT] = buffer[inner];
						totalT++;
					}
				}
				else
				{
					totalT++;
				}
			}while(remainingT > 0);
		}
};

class scheduleSRT : public Scheduler
{
	public:
		scheduleSRT(vector<Job>* JobList) : Scheduler(JobList) {}

		void processJobs()
		{
			string buffer = "Shortest Remaining Time Graph";
			outputGraph[0].resize(buffer.size(),' ');
			for(int i = 0; i < buffer.size(); i++)
			{
				outputGraph[0][i] = buffer[i];
			}
			priority_queue<Job*,vector<Job*>,ShortestNext> jobQue;
			Job* running = NULL;
			int remainingT, totalT = 0, timeTilNext = 1000000000;				
			do{				
				remainingT = 0;
				for(int jobI = 0; jobI < (*jobList).size(); jobI++)
				{
					remainingT += (*jobList)[jobI].durationT;
					remainingT -= (*jobList)[jobI].executionT;
					if(!(*jobList)[jobI].acknowledged && (*jobList)[jobI].arrivalT <= totalT)
					{
						(*jobList)[jobI].ID = jobI;
						jobQue.push(&((*jobList)[jobI])); //ensures job is not added to que until it arrives
						(*jobList)[jobI].acknowledged = true; //ensures job is not added more than once to que
					}
					else if(!(*jobList)[jobI].acknowledged)
					{
						timeTilNext = min(timeTilNext,(*jobList)[jobI].arrivalT - totalT);
					}
				}
				if(!jobQue.empty())
				{
					running = jobQue.top();
					jobQue.pop(); //que is wierd and front is like peek and, pop returns void?	
					buffer = running->run(min(timeTilNext, running->durationT - running->executionT)); //min is redundant verification of not over running
					outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
					for(int inner = 0; inner < buffer.size(); inner++)
					{
						outputGraph[1+running->ID][totalT] = buffer[inner];
						totalT++;
					}						
					if(running->executionT < running->durationT) //replace job if being preempted
					{
						jobQue.push(running);
					}
				}
				else
				{
					totalT++;
				}
			}while(remainingT > 0);
		}
};

class scheduleHRRN : public Scheduler
{
	public:
		scheduleHRRN(vector<Job>* JobList) : Scheduler(JobList) {}

		void processJobs()
		{
			string buffer = "Highest Response Ratio Next Graph";
			outputGraph[0].resize(buffer.size(),' ');
			for(int i = 0; i < buffer.size(); i++)
			{
				outputGraph[0][i] = buffer[i];
			}
			priority_queue<Job*,vector<Job*>,LargestResponseNext> jobQue;
			Job* running = NULL;
			int remainingT, totalT = 0;				
			do{				
				remainingT = 0;
				for(int jobI = 0; jobI < (*jobList).size(); jobI++)
				{
					if(totalT == 0)
					{
						(*jobList)[jobI].totalTp = &totalT;
					}
					remainingT += (*jobList)[jobI].durationT;
					remainingT -= (*jobList)[jobI].executionT;
					if(!(*jobList)[jobI].acknowledged && (*jobList)[jobI].arrivalT <= totalT)
					{
						(*jobList)[jobI].ID = jobI;
						jobQue.push(&((*jobList)[jobI])); //ensures job is not added to que until it arrives
						(*jobList)[jobI].acknowledged = true; //ensures job is not added more than once to que
					}
				}
				if(!jobQue.empty())
				{
					running = jobQue.top();
					jobQue.pop(); //que is wierd and front is like peek and, pop returns void?					
					buffer = running->run(); //run til completion == run(durationT)
					outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
					for(int inner = 0; inner < buffer.size(); inner++)
					{
						outputGraph[1+running->ID][totalT] = buffer[inner];
						totalT++;
					}
				}
				else
				{
					totalT++;
				}
			}while(remainingT > 0);
		}
};

class scheduleFB : public Scheduler
{
	public:
		scheduleFB(vector<Job>* JobList) : Scheduler(JobList) {}

		void processJobs()
		{
			string buffer = "Feedback Graph";
			outputGraph[0].resize(buffer.size(),' ');
			for(int i = 0; i < buffer.size(); i++)
			{
				outputGraph[0][i] = buffer[i];
			}
			queue<Job*> jobQue[FB_Queue_Count]; 
			Job* running = NULL;
			int remainingT, totalT = 0;				
			do{				
				remainingT = 0;
				for(int jobI = 0; jobI < (*jobList).size(); jobI++)
				{
					remainingT += (*jobList)[jobI].durationT;
					remainingT -= (*jobList)[jobI].executionT;
					if(!(*jobList)[jobI].acknowledged && (*jobList)[jobI].arrivalT <= totalT)
					{
						(*jobList)[jobI].ID = jobI;
						jobQue[0].push(&((*jobList)[jobI])); //ensures job is not added to que until it arrives
						(*jobList)[jobI].acknowledged = true; //ensures job is not added more than once to que
					}
				}
				for(int que = 0; que <= FB_Queue_Count; que++)
				{
					if( que < FB_Queue_Count && !jobQue[que].empty())
					{
						if(running && running->durationT > running->executionT)
						{
							jobQue[running->nextQue].push(running);
						}
						running = jobQue[que].front();
						jobQue[que].pop(); //que is wierd and front is like peek and, pop returns void?					
						running->nextQue = min(FB_Queue_Count - 1, running->nextQue + 1);
						buffer = running->run(FB_Quantum);
						outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
						for(int inner = 0; inner < buffer.size(); inner++)
						{
							outputGraph[1+running->ID][totalT] = buffer[inner];
							totalT++;
						}
						break;
					}
					else if(que == FB_Queue_Count)
					{
						if(running && running->durationT > running->executionT)
						{
							buffer = running->run(FB_Quantum);
							outputGraph[1+running->ID].resize(buffer.size()+totalT,' ');
							for(int inner = 0; inner < buffer.size(); inner++)
							{
								outputGraph[1+running->ID][totalT] = buffer[inner];
								totalT++;
							}
						}
						else
						{
							totalT++;
						}
					}
				}
			}while(remainingT > 0);
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
						return new scheduleSPN(&jobList);
					break;

				case srt:
						return new scheduleSRT(&jobList);
					break;

				case hrrn:
						return new scheduleHRRN(&jobList);
					break;

				case fb:
						return new scheduleFB(&jobList);
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
		if(quantum == 0)
		{
			printf("Round Robin requires a nonzero third argument specifying the quantum size.\n");
			return 1; //failure
		}
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
