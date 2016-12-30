﻿#pragma once
#include "job.h"
#include "jobrecorder.h"
#include "require.h"
#include "app_cfg.h"
#include <list>
#include <vector>
#include <stdexcept>
#include <memory>
#include <iostream>

#include <algorithm>

class Scheduler {
    Scheduler* scheduler; //envelope

    //Prevent copy-construction & operator =
    Scheduler(Scheduler&);
    Scheduler operator=(Scheduler&);
protected:
    Scheduler() {
        scheduler = 0;
        jobNum = 0;
        readyJobNum = 0;
        execableJobNum = 0;
        DEBUG_PRINT("create scheduler");
        jobVec = { readyJobs0, readyJobs1, readyJobs2, readyJobs3, readyJobs4, readyJobs5, readyJobs6, readyJobs7,
                   readyJobs8, readyJobs9, readyJobs10, readyJobs11, readyJobs12, readyJobs13, readyJobs14, readyJobs15 };
    }
public:
    typedef std::shared_ptr<Job> ptr; //smart pointer
    static const size_t MaxPrio = 15;

    void schedule(us16 runtime, JobRecorder &jobRecorder);
    void checkWaitingJob(us16 runtime, JobRecorder &jobRecorder);
    virtual void schedule_NONE(us16 runtime, JobRecorder &jobRecorder);// { DEBUG_PRINT("schedule_NONE"); }
    virtual void schedule_PM(us16 runtime, JobRecorder &jobRecorder) { DEBUG_PRINT("schedule_PM"); }
    virtual void schedule_PSA(us16 runtime, JobRecorder &jobRecorder) { DEBUG_PRINT("schedule_PSA"); }
    virtual void schedule_PM_PSA(us16 runtime, JobRecorder &jobRecorder) { DEBUG_PRINT("schedule_PM_PSA"); }
    virtual void sortJobNone() {}

    ptr &selectFirstJob() { return readyJobs.front(); }	//select the job to run, which is in the front of the readyJobs list
    ptr &selectNextJob();       //return next job, require detect the size of job-list by user
    ptr &selectFirstJobPrio(); 	//select the job to run, which is in the front of the readyJobs0-15 list
    ptr &selectNextJobPrio(ptr &job);   //return next job, require detect the size of job-list by user

    void clearJob(std::list<ptr> &jobs) {
        jobs.clear();
        DEBUG_PRINT("clear all the readyJobs");
    }	//clear all the readyJobs

    void eraseJob(std::list<ptr> &jobs) {
        jobs.pop_front();
        DEBUG_PRINT("remove the finished job");
    }	//remove the finished job

    std::list<ptr> &getReadyJobs() { return readyJobs; }
    std::list<ptr> &getWaitingJobs() { return waitingJobs; }
    std::list<ptr> &getFinishedJobs() { return finishedJobs; }
    std::list<ptr> &getNextJobs() { return nextJobs; }

    //as time goes by, jobs status will change
    bool statusChange(std::list<ptr> &srcJobs, std::list<ptr> &dstJobs, std::list<ptr> &changeJobs, unsigned short runtime);
    bool isJobNone() { return jobNum == 0; }
    void addWaitingJob(ptr &job); //add waiting job
    void addFinishedJob(ptr &jobs, us16 runtime);
    void addReadyJob(ptr &job) { readyJobs.push_back(job); addReadyJobNum(); addExecableJobNum();  DEBUG_PRINT("add ready job" ); }   //add ready job
    void addReadyJobToPrio();   //add ready job to different prio job
    void storeJobs();
    void setAverTurn(JobRecorder &jobRecorder);
    void clearAllJob();
    void setFlag(bool isPSA = false, bool isPM = false) { flag = 0; flag |= isPSA; flag <<= 1; flag |= isPM; }
    void clear() { scheduler->clearAllJob(); }
    void addJobNum() { ++jobNum; }
    void subJobNum() { --jobNum; }
    void addReadyJobNum() { ++readyJobNum; }
    void subReadyJobNum() { --readyJobNum; }
    void addExecableJobNum() { ++execableJobNum; }
    void subExecableJobNum() { --execableJobNum; }


    virtual ~Scheduler () {
        if (scheduler) {
            scheduler->clearAllJob();
        }
    }

public:
    class  BadSchedulerCreation : public std::logic_error
    {
    public:
        BadSchedulerCreation(const std::string& type)
            : logic_error("cannot create type " + type) {}
    };
    Scheduler(const std::string& type) throw(BadSchedulerCreation); //factory method which throw error of undefined type

protected:
    std::list<ptr> waitingJobs;	//list to store jobs whose committing time is older than current time
    std::list<ptr> readyJobs;	//list to store ready jobs
    std::list<ptr> nextJobs;    //list to store next job, though there is only one next job
    std::list<ptr> finishedJobs;//list to store finished jobs

    /* different priority list*/
    std::list<ptr> readyJobs0;
    std::list<ptr> readyJobs1;
    std::list<ptr> readyJobs2;
    std::list<ptr> readyJobs3;
    std::list<ptr> readyJobs4;
    std::list<ptr> readyJobs5;
    std::list<ptr> readyJobs6;
    std::list<ptr> readyJobs7;
    std::list<ptr> readyJobs8;
    std::list<ptr> readyJobs9;
    std::list<ptr> readyJobs10;
    std::list<ptr> readyJobs11;
    std::list<ptr> readyJobs12;
    std::list<ptr> readyJobs13;
    std::list<ptr> readyJobs14;
    std::list<ptr> readyJobs15;
    std::vector<std::list<ptr>> jobVec;

//what flag mean is that :
#define _NONE	0x00
#define _PM		0x01
#define _PSA	0x02
#define _PM_PSA 0x03
    unsigned char flag;
    us16 jobNum;    //record the num of job
    us16 execableJobNum;    //record the num of executable job;
    us16 readyJobNum;//record the num of ready Job
};

/* FCFS, inherit from Scheduler */
class FCFS : public Scheduler
{
    //Prevent copy-construction & operator =
    FCFS(FCFS&);
    FCFS operator=(FCFS&);

    FCFS() { DEBUG_PRINT("create FCFS scheduler"); } //private constructor, prevent to be instanced by other operation
    friend class Scheduler;
public:
    ~FCFS() {}
    void schedule_NONE(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PSA(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM_PSA(us16 runtime, JobRecorder &jobRecorder);
    void sortJobNone() { Scheduler::sortJobNone(); }
};

/* SJF, inherit from Scheduler */
class SJF : public Scheduler
{
    //Prevent copy-construction & operator =
    SJF(SJF&);
    SJF operator=(SJF&);

    SJF() { DEBUG_PRINT("create SJF scheduler"); } //private constructor, prevent to be instanced by other operation
    friend class Scheduler;
public:
    ~SJF() {}
    void schedule_NONE(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PSA(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM_PSA(us16 runtime, JobRecorder &jobRecorder);
    void sortJobNone();
};

/* EDF, inherit from Scheduler */
class EDF : public Scheduler
{
    //Prevent copy-construction & operator =
    EDF(EDF&);
    EDF operator=(EDF&);

    EDF() { DEBUG_PRINT("create EDF scheduler"); } //private constructor, prevent to be instanced by other operation
    friend class Scheduler;
public:
    ~EDF() {}
    void schedule_NONE(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PSA(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM(us16 runtime, JobRecorder &jobRecorder);
    void schedule_PM_PSA(us16 runtime, JobRecorder &jobRecorder);
    void sortJobNone();
};


inline
Scheduler::Scheduler(const std::string &type) {
    if (type == "FCFS")
        scheduler = new FCFS;
    else if(type == "SJF")
        scheduler = new SJF;
    else if(type == "EDF")
        scheduler = new EDF;
    else    throw BadSchedulerCreation(type);
}


