

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <queue>
#include <limits>

using namespace std;

enum Trans {TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT, TRANS_TO_DONE};
enum STATE {CREATED, READY, RUNNG, BLOCK, DONE};
bool Verbose = false;
string StateToString(STATE State)
{
    if(State == CREATED)
        return "CREATED";
    else if(State == READY)
        return "READY";
    else if(State == RUNNG)
        return "RUNNG";
    else
        return "BLOCK";

}
bool CALL_SCHEDULER;
int SchedulerMode = 0;      //FIFO by default
int RdsOffset = 1;              // 0 index corresponds to size
vector<int> randValue;

int myRandom(int URange)
{
    int num = 1 + ( (randValue[ (RdsOffset % randValue.size() )]) % URange ) ;
    RdsOffset++;
    return num;
}

class Process
{
    int AT;
    int TC;
    int CB;
    int IO;
    int Pid;
    int RemCPUTime;
    int RemCPUBurst;
    int StaticPriority;
    int DynamicPriority;
    int IOTime;
    static int lastest_Pid;
public:
    STATE State;
    int TimeInPrevState;
    int state_ts;
    double AddedToReadyQueue;
    int FinishingTime;
    int TimeInReadyState;
    int BlockedTime;
    Process(Process* A)
    {
        AT = A->get_AT();
        TC = A->get_TC();
        CB = A->get_CB();
        IO = A->get_IO();
        Pid = A->get_Pid();
        RemCPUTime = A->get_RemCPUTime();
        RemCPUBurst = A->get_RemCPUBurst();
        StaticPriority = A->get_StaticPriority();
        DynamicPriority = A->get_DynamicPriority();
        State = A->State;
        TimeInPrevState = A->TimeInPrevState;
        state_ts = A->state_ts;
        AddedToReadyQueue = A->AddedToReadyQueue;
        IOTime = A->get_IOTime();
        TimeInReadyState = A->TimeInReadyState;
        BlockedTime = A->BlockedTime;
    }
    Process ()
    {
        TimeInReadyState = 0;
        AddedToReadyQueue = -1;
        state_ts = 0;
        TimeInPrevState = -1;
        IOTime = 0;
        RemCPUBurst = 0;
        BlockedTime = 0;
        set_Pid(lastest_Pid);
        lastest_Pid++;
    }
    int get_AT()
    {
        return AT;
    }
    int get_TC()
    {
        return TC;
    }
    int get_CB()
    {
        return CB;
    }
    int get_IO()
    {
        return IO;
    }
    int get_Pid()
    {
        return Pid;
    }
    int get_StaticPriority()
    {
        return StaticPriority;
    }
    int get_DynamicPriority()
    {
        return DynamicPriority;
    }
    int get_RemCPUTime()
    {
        return RemCPUTime;
    }
    int get_RemCPUBurst()
    {
        return RemCPUBurst;
    }
    int get_IOTime()
    {
        return IOTime;
    }
    void set_AT(int new_AT)
    {
        AT = new_AT;
    }
    void set_TC(int new_TC)
    {
        TC = new_TC;
        RemCPUTime = TC;
    }
    void set_CB(int new_CB)
    {
        CB = new_CB;
    }
    void set_IO(int new_IO)
    {
        IO = new_IO;
    }
    void set_Pid(int new_Pid)
    {
        Pid = new_Pid;
    }
    void Dec_RemCPUTime(int CPUBurst)
    {
        RemCPUTime -= CPUBurst;
    }
    void setCPUBurst(int newCPUBurst)
    {
        RemCPUBurst = newCPUBurst;
    }
    void DecreaseCPUBurst(int value)
    {
        RemCPUBurst -= value;
    }
    void setStaticPriority(int newStaticPriority)
    {
        StaticPriority = newStaticPriority;
        DynamicPriority = newStaticPriority - 1;
    }
    void setDynamicPriority(int newDynamicPriority)
    {
        DynamicPriority = newDynamicPriority - 1;
    }
    bool UpdateDynamicPriority()
    {

        DynamicPriority--;
        if(DynamicPriority <0)
        {
            return false;
        }
        else
            return true;
    }
    void IncreaseIO(int amt)
    {
        IOTime += amt;
    }
};

int Process::lastest_Pid = 0;

Process* CURRENT_RUNNING_PROCESS = NULL;

struct OrderProcess
{
    bool operator() (Process* &a, Process* &b)      //is b >(ahead of) a
    {
        if (SchedulerMode == 0)             //FCFS
        {
            return a->AddedToReadyQueue > b->AddedToReadyQueue;
        }
        else if(SchedulerMode == 1)         //LCFS
        {
            return a->AddedToReadyQueue < b->AddedToReadyQueue;
        }

        else if(SchedulerMode == 2)         //SJF
        {
            if(a->get_RemCPUTime() < b->get_RemCPUTime() )
                return false;
            else if(a->get_RemCPUTime() > b->get_RemCPUTime() )
                return true;
            else
            {
                return a->AddedToReadyQueue > b->AddedToReadyQueue;         //If same RemTime, use one added first
            }
        }
        else if(SchedulerMode == 3)         //RR
        {
            return a->AddedToReadyQueue > b->AddedToReadyQueue;
        }
        else if(SchedulerMode == 4)         //PRIO
        {
            if(a->get_DynamicPriority() < b->get_DynamicPriority())
            {
                return true;
            }
            else if (a->get_DynamicPriority() > b->get_DynamicPriority())
                return false;
            else                        //samePriority
                return a->AddedToReadyQueue > b->AddedToReadyQueue;

        }
        else
        {
            cout<<"Error With Scheduler Mode: Mode Not Recognised. Using FCFS\n";
            return a->AddedToReadyQueue < b->AddedToReadyQueue;

        }
    }
};

typedef std::priority_queue<Process*, std::vector<Process*>, OrderProcess> runQueue;

struct OrderCompleted
{
    bool operator() (Process* &a, Process* &b)      //is b >(ahead of) a
    {
        return a->get_Pid() > b->get_Pid();
    }
};

typedef std::priority_queue<Process*, std::vector<Process*>, OrderCompleted> DoneQueue;

DoneQueue CompltedProcesses;


class Scheduler
{
    int SchedulerType;

public:
    runQueue RunQueue;
    runQueue ExpiredQueue;
    Process* get_next_process()
    {
        if(RunQueue.size() ==0)
        {
            SwapQueues();
            if(RunQueue.size() == 0)
                return NULL;

        }
        Process* Next_Process= RunQueue.top();
        RunQueue.pop();
        return Next_Process;
    }
    void addExpiredProcess(Process* NewProcess)
    {
        Process* MyProcess = new Process(NewProcess);
        ExpiredQueue.push(MyProcess);
    }
    void addProcess(Process* NewProcess, Trans From)
    {

        Process* MyProcess = new Process(NewProcess);

        if(SchedulerMode ==4)
        {
            MyProcess->UpdateDynamicPriority();
            if(From == TRANS_TO_READY )
            {
                MyProcess->setDynamicPriority(MyProcess->get_StaticPriority());
                RunQueue.push(MyProcess);
            }
            else if(From == TRANS_TO_PREEMPT && MyProcess->get_DynamicPriority() >= 0)
            {
                RunQueue.push(MyProcess);
            }
            else if(MyProcess->get_DynamicPriority() <0)
            {
                MyProcess->setDynamicPriority(MyProcess->get_StaticPriority());
                addExpiredProcess(MyProcess);
            }
        }
        else
            RunQueue.push(MyProcess);
    }
    void SwapQueues()
    {
        RunQueue = ExpiredQueue;
        while(ExpiredQueue.size()>0)
        {
            ExpiredQueue.pop();
        }
    }
    int getQueueSize()
    {
        return RunQueue.size();
    }

};

Scheduler* THE_SCHEDULER = new Scheduler;

class EVENT
{
    static int LatestEventId ;      //use to update EventId
    public:
    int EvtTimeStamp;
    Trans Transition;
    Process* EvtProcess;
    int EventId;        //ToBeGenerated

    EVENT(int MyTimeStamp, Trans MyTransition, Process* MyProcess)
    {
        EvtTimeStamp = MyTimeStamp;
        Transition = MyTransition;
        EvtProcess = MyProcess;
        EventId = LatestEventId;


        // Increase every time object is created
        LatestEventId++;
    }
    EVENT()
    {

    }
    EVENT (EVENT* A)
    {
        EvtTimeStamp = A->EvtTimeStamp;
        Transition = A->Transition;
        EvtProcess = new Process(A->EvtProcess);
        EventId = A->EventId;
    }

};

int EVENT::LatestEventId = 1;

struct OrderEvent           //TODO: TO CHANGE
{
    bool operator() (EVENT &a, EVENT &b)
    {
        if (a.EvtTimeStamp < b.EvtTimeStamp)
            return false;
        else if (a.EvtTimeStamp > b.EvtTimeStamp)
            return true;
        else    //same event Time, should be impossible
        {
            if(a.EvtProcess->get_Pid() == b.EvtProcess->get_Pid() )
            {
                //TODO
                return false;
            }
            else
            {
                return (a.EventId > b.EventId);
            }
        }

    }
};

typedef std::priority_queue<EVENT, std::vector<EVENT>, OrderEvent> EventQueue;



void AddToEventQueue(EVENT NewEvent, EventQueue EventQUEUE)
{
    EVENT NewEvent2 = EVENT(NewEvent);
    EventQUEUE.push(NewEvent2);
}


EVENT* get_event(EventQueue EventQUEUE)
{
    if(EventQUEUE.size() ==0)
        return NULL;
    EVENT NEvent = EventQUEUE.top();
    EventQUEUE.pop();
    EVENT* Pointer = &NEvent;
    return Pointer;

}

int get_next_event_time(EventQueue EventQUEUE)
{
    if(EventQUEUE.size() ==0)
        return -1;
    EVENT nextEvt = EventQUEUE.top();
    return nextEvt.EvtTimeStamp;
}

void report(int Quantum)
{
    string ModeName[] = {"FCFS ", "LCFS ", "SJF ", "RR ", "PRIO "};
    cout<<ModeName[SchedulerMode];
    if(Quantum != numeric_limits<int>::max())
    {
        cout<<Quantum;
    }
    cout<<endl;

    int    maxfintime = 0;
    double cpu_util = 0;
    double io_util  = 0;
    double avg_turnaround = 0;
    double avg_waittime = 0.0;
    int NumberProcesses =0;

    while(CompltedProcesses.size()>0)
    {
        Process* P = CompltedProcesses.top();

        if(P->FinishingTime > maxfintime)
            maxfintime = P->FinishingTime;
        cpu_util += P->get_TC();
        io_util += P->BlockedTime;
        avg_turnaround += P->FinishingTime - P->get_AT();
        avg_waittime += P->TimeInReadyState;
        NumberProcesses ++;

        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               P->get_Pid(),
               P->get_AT(), P->get_TC() , P->get_CB(), P->get_IO(), P->get_StaticPriority(),
               P->FinishingTime, // last time stamp
               P->FinishingTime- P->get_AT(),
               P->FinishingTime - P->get_AT() - P->get_TC() - P->TimeInReadyState,
               P->TimeInReadyState );

        CompltedProcesses.pop();

    }

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
            maxfintime,
            100*cpu_util/maxfintime,
            100*io_util/maxfintime,
            avg_turnaround/NumberProcesses,
            avg_waittime/NumberProcesses,
            NumberProcesses*100.0/maxfintime);
}

void round_check()
{
    printf("\nCheck correct precision output: should show '0.333 0.667'\n");
    printf("%.3lf %.3lf\n",1.0/3.0, 2.0/3.0);
}

void Simulation(int Quantum, EventQueue EventQUEUE) {
    EVENT* evt;

    int NumBlocked = 0;
    int BlockedTill = 0;
    while( EventQUEUE.size() > 0)
    {
        EVENT NEvent = EventQUEUE.top();
        EventQUEUE.pop();
        evt = &NEvent;
        int CURRENT_TIME = evt->EvtTimeStamp;
        Process* EventProcess = evt->EvtProcess;
        EventProcess->TimeInPrevState = CURRENT_TIME - evt->EvtProcess->state_ts;
        int IOBurst, CPUBurst;
        EVENT* NewEvent = new EVENT;
        EVENT NextEvent;
        STATE prevState;
        switch (evt->Transition)
        {  // which state to transition to?
            case TRANS_TO_READY:
            { // must come from BLOCKED or from PREEMPTION
                EventProcess->state_ts = CURRENT_TIME;
                prevState = EventProcess->State;
                if(prevState == BLOCK)
                    NumBlocked--;

                EventProcess->State = READY;
                EventProcess->AddedToReadyQueue = CURRENT_TIME + evt->EventId/1000.00 ;          //TODO: Check. AddPrecision ?

                THE_SCHEDULER->addProcess(EventProcess, TRANS_TO_READY);
                if(prevState == CREATED)
                {
                    EventProcess->TimeInPrevState = 0;
                    if(Verbose)
                        cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<"\n";
                }
                else
                    if(Verbose)
                        cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<"\n";
                if(CURRENT_RUNNING_PROCESS==NULL)
                    CALL_SCHEDULER = true; // conditional on whether something is run TODO
                break;
            }
            case TRANS_TO_RUN:
            {   // create event for either preemption or blocking
                EventProcess->TimeInReadyState += CURRENT_TIME - EventProcess->state_ts;         //Check
                EventProcess->state_ts = CURRENT_TIME;
                prevState = EventProcess->State;
                EventProcess->State = RUNNG;
                CPUBurst = EventProcess->get_RemCPUBurst();
                if(CPUBurst < 1 )
                {
                    CPUBurst = myRandom(EventProcess->get_CB());
                    EventProcess->setCPUBurst(CPUBurst);
                }
                if(CPUBurst < EventProcess->get_RemCPUTime())
                {
                    if(Verbose)
                        cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<" cb="<<EventProcess->get_RemCPUBurst()<<" rem="<<EventProcess->get_RemCPUTime()<<" prio="<<EventProcess->get_DynamicPriority()<<"\n";
                    if(CPUBurst <= Quantum)
                    {
                        EventProcess->DecreaseCPUBurst(CPUBurst);
                        EventProcess->Dec_RemCPUTime(CPUBurst);
                        NewEvent = new EVENT(CURRENT_TIME + CPUBurst, TRANS_TO_BLOCK, EventProcess);         //TODO: Check
                        NextEvent = new EVENT(CURRENT_TIME + CPUBurst, TRANS_TO_BLOCK, EventProcess);
                        EventQUEUE.push(NextEvent);
                    }
                    else
                    {

                        EventProcess->DecreaseCPUBurst(Quantum);         //UpdateRemCPUBurst
                        EventProcess->Dec_RemCPUTime(Quantum);
                        NewEvent = new EVENT(CURRENT_TIME + Quantum, TRANS_TO_PREEMPT, EventProcess);
                        NextEvent = new EVENT(CURRENT_TIME + Quantum, TRANS_TO_PREEMPT, EventProcess);
                        EventQUEUE.push(NextEvent);
                        CPUBurst = Quantum;
                    }

                }
                else
                {
                    CPUBurst = EventProcess->get_RemCPUTime();
                    EventProcess->setCPUBurst(CPUBurst);
                    if(Quantum < CPUBurst)
                    {
                        if(Verbose)
                            cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<" cb="<<EventProcess->get_RemCPUBurst()<<" rem="<<EventProcess->get_RemCPUTime()<<" prio="<<EventProcess->get_DynamicPriority()<<"\n";

                        EventProcess->DecreaseCPUBurst(Quantum);         //UpdateRemCPUBurst
                        CPUBurst = Quantum;
                        EventProcess->Dec_RemCPUTime(CPUBurst);
                        NewEvent = new EVENT(CURRENT_TIME + Quantum, TRANS_TO_PREEMPT, EventProcess);
                        NextEvent = new EVENT(CURRENT_TIME + Quantum, TRANS_TO_PREEMPT, EventProcess);
                        EventQUEUE.push(NextEvent);
                    }
                    else
                    {
                        //End of Process
                        if(Verbose)
                            cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<" cb="<<EventProcess->get_RemCPUBurst()<<" rem="<<EventProcess->get_RemCPUTime()<<" prio="<<EventProcess->get_DynamicPriority()<<"\n";
                        NextEvent = new EVENT(CURRENT_TIME + EventProcess->get_RemCPUTime(), TRANS_TO_DONE, EventProcess);
                        EventQUEUE.push(NextEvent);

                        EventProcess->FinishingTime = CURRENT_TIME + EventProcess->get_RemCPUTime();
                        CompltedProcesses.push(EventProcess);
                    }
                }

                break;
            }
            case TRANS_TO_BLOCK:
            {   //create an event for when process becomes READY again

                EventProcess->state_ts = CURRENT_TIME;
                prevState = EventProcess->State;
                EventProcess->State = BLOCK;
                IOBurst = myRandom( EventProcess->get_IO() );
                EventProcess->IncreaseIO(IOBurst);
                NumBlocked++;
                if(NumBlocked ==1)
                {
                    EventProcess->BlockedTime+=IOBurst;
                    BlockedTill = CURRENT_TIME + IOBurst;
                }
                else
                {
                    if(CURRENT_TIME + IOBurst > BlockedTill)
                    {

                        EventProcess->BlockedTime += CURRENT_TIME + IOBurst - BlockedTill;
                        BlockedTill = CURRENT_TIME + IOBurst;
                    }
                }
                NextEvent = new EVENT(CURRENT_TIME + IOBurst, TRANS_TO_READY, EventProcess);
                NewEvent = new EVENT(CURRENT_TIME + IOBurst, TRANS_TO_READY, EventProcess);
                EventQUEUE.push(NextEvent);
                if(Verbose)
                    cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<" ib="<<IOBurst<<" rem="<<EventProcess->get_RemCPUTime()<<"\n";
                CURRENT_RUNNING_PROCESS = NULL;
                CALL_SCHEDULER = true;
                break;
            }
            case TRANS_TO_PREEMPT:
            {   // add to runqueue (no event is generated)
                EventProcess->state_ts = CURRENT_TIME;

                CALL_SCHEDULER = true;
                EventProcess->AddedToReadyQueue = CURRENT_TIME + evt->EventId/1000.00;

                /*
                bool Expired;
                if(SchedulerMode ==4)
                    Expired = EventProcess->UpdateDynamicPriority();          //Decrease priority before next cycle
                if(Expired)
                    THE_SCHEDULER->addExpiredProcess(EventProcess);
                else
                    THE_SCHEDULER->addProcess(EventProcess);

                */
                prevState = EventProcess->State;
                EventProcess->State = READY;
                THE_SCHEDULER->addProcess(EventProcess, TRANS_TO_PREEMPT);
                if(Verbose)
                    cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": "<<StateToString(prevState)<<" -> "<<StateToString(EventProcess->State)<<" cb="<<EventProcess->get_RemCPUBurst()<<" rem="<<EventProcess->get_RemCPUTime()<<" prio="<<EventProcess->get_DynamicPriority()<<"\n";
                CURRENT_RUNNING_PROCESS = NULL;
                break;
            }
            case TRANS_TO_DONE:
            {
                EventProcess->state_ts = CURRENT_TIME;
                if(Verbose)
                    cout<<CURRENT_TIME<<" "<<EventProcess->get_Pid()<<" "<<EventProcess->TimeInPrevState<<": Done\n";
                CURRENT_RUNNING_PROCESS = NULL;
                CALL_SCHEDULER = true;
            }
        }
        delete NewEvent;
        if(CALL_SCHEDULER)
        {
            if (get_next_event_time(EventQUEUE) == CURRENT_TIME)
            {
                continue;//process next event from Event queue
            }
            CALL_SCHEDULER = false;
            if (CURRENT_RUNNING_PROCESS == NULL)
            {
                CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == NULL)
                {
                    continue;
                }
            }
            EVENT EventToQueue(CURRENT_TIME, TRANS_TO_RUN, CURRENT_RUNNING_PROCESS);
            EventQUEUE.push(EventToQueue);
        }
    }
}

int main(int argc, const char * argv[]) {

    //string RFilename = "/Users/vipinarora/Desktop/GradSem1/OS/Assignment 2/lab2_assign/rfile";
    int ArgIndex = 1;
    
    /*
     string temp = argv[1];
    if(temp == "-v")
    {
        Verbose = true;
        ArgIndex ++;
    }
    temp = argv[ArgIndex];
    if(temp[0] != 'F' && temp[0] != 'L' && temp[0] != 'S' && temp[0] != 'R' && temp[0] != 'P')
    {
        ArgIndex --;
    }
    */
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "vs:")) != -1)
        switch (c)
        {
            case 'v':
                Verbose = true;
                break;
            case 's':
                //cvalue = optarg;
                Specification = string(optarg);
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                return 1;
            default:
                abort ();
        }
    
    ArgIndex = optind;
    ifstream RFile;
    RFile.open(argv[ArgIndex+1]) ;              //change from 2 to 1
    string RLine, RBuffer = "";
    while( getline(RFile, RLine ))
    {
        int num;
        istringstream ss(RLine);
        while(ss>>num)
        {
            randValue.push_back(num);
        }
    }

    // change string Specification = argv[ArgIndex];
    //change ArgIndex++;
    int Quantum = std::numeric_limits<int>::max();
    if(Specification =="F")
        SchedulerMode = 0;
    else if(Specification =="L")
        SchedulerMode = 1;
    else if(Specification =="S")
        SchedulerMode = 2;
    else if(Specification[0] == 'R')
    {
        SchedulerMode = 3;
        int i = 1;
        Quantum = 0;
        while(i<Specification.length())
        {
            if(isdigit(Specification[i]))
            {
                Quantum = 10 * Quantum + (Specification[i]- '0');
            }
            else
            {
                return -1;
            }
            i++;
        }

    }
    else if(Specification[0] == 'P')
    {
        SchedulerMode = 4;
        int i = 1;
        Quantum = 0;
        while(i<Specification.length())
        {
            if(isdigit(Specification[i]))
            {
                Quantum = 10 * Quantum + (Specification[i]- '0');
            }
            else
            {
                return -1;
            }
            i++;
        }
    }
    else
        SchedulerMode = 0;

    // string filename = "/Users/vipinarora/Desktop/GradSem1/OS/Assignment 2/lab2_assign/input3";
    ifstream file;
    file.open(argv[ArgIndex]) ;
    string line, buffer = "";

    EventQueue EventQUEUE ;
    while( getline(file, line ))
    {
        buffer = buffer + line + "\n";
        istringstream ss(line);
        Process* NewProcess = new Process();

        int num;
        ss>>num;
        NewProcess->set_AT(num);
        ss>>num;
        NewProcess->set_TC(num);
        ss>>num;
        NewProcess->set_CB(num);
        ss>>num;
        NewProcess->set_IO(num);
        NewProcess->setStaticPriority(myRandom(4));
        NewProcess->State = CREATED;
        EVENT NewEvent = EVENT(NewProcess->get_AT(), TRANS_TO_READY, NewProcess);
        EventQUEUE.push(NewEvent);
        EVENT MyEvent2 = (EventQUEUE.top());
        EVENT* MyEvent = &MyEvent2;

    }
    Simulation(Quantum, EventQUEUE);

    report(Quantum);
    return 0;
}






