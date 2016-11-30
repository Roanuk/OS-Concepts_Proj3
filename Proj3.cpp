#include <iostream>
#include <string>
#include <vector>

#define FCFS "FCFS"
#define RR "RR"
#define SPN "SPN"
#define SRT "SRT"
#define HRRN "HRRN"
#define FB "FB"
using namespace std;
enum SchedulerType { FCFS, RR, SPN, SRT, HRRN, FB};
typedef struct jobStruct{
	char name;
	int arrivalT;
	int durationT;
}job;

class OS{
	
}

class Job{
public:
	Job(job data){
		name = data.name;
		arrivalT = data.arrivalT;
		durationT = data.durationT;
		executionT = 0;
	}
	
	//standard version of run continues until completion
	void run(){
		run(durationT);
	}
	
	//Overloaded version of run takes a number of slices
	void run(int slices){
		int runSlices = 0;
		while(executionT < durationT && runSlices < slices){
			cout << name;
			executionT++;
			runSlices++;
		}
	}
	
protected:
	char name;
	int arrivalT, durationT,executionT;
}

class Scheduler {
public:
	Scheduler(int NumJobs, vector<job> JobList): type(Type),numJobs(NumJobs){
		jobList.reserve(NumJobs);
		for(int loops = 0;loops<numJobs;loops++){
			jobList[loops]=Job(JobList[loops]);
		}
		processJobs();
	}
	void processJobs(){
		
	}
protected:
	int sliceDur; //0 means nonpreemptive, >0 means # of slices per preemption
	int numJobs; //number of jobs in jobList
	vector<job> jobList; //
}
