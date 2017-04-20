//
//  main.cpp
//  OS_HW4_IO_Scheduler
//
//  Created by Vipin Arora on 03/12/16.
//  Copyright © 2016 Pranit Arora. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <limits>


using namespace std;

int Total_Time = 0;
int Total_Movement = 0;
double Total_TurnaroundTime = 0;
double Total_WaitTime = 0;
int Max_WaitTime = 0;



class Scheduler {
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
public:
    virtual void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        ActiveQueue.push_back(Request);
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        return ActiveQueue[0];
    }
} ;
Scheduler* scheduler;

class FIFO: public Scheduler
{
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
    vector<int> ArrivalTime;
    
public:
    void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        ActiveQueue.push_back(Request);
        ArrivalTime.push_back(time);
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        
        int index = 0;
        int answertime= ArrivalTime[0];
        for(int i = 1; i <ActiveQueue.size();i++)
        {
            if(ArrivalTime[i] < answertime)
            {
                answertime = ArrivalTime[i];
                index = i;
            }
        }
        pair<int , int > answer = ActiveQueue[index];
        ActiveQueue.erase(ActiveQueue.begin() + index );
        ArrivalTime.erase(ArrivalTime.begin() + index );
        return answer;
    }
};

class SSTF: public Scheduler
{
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
    vector<int> ArrivalTime;
    
public:
    void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        ActiveQueue.push_back(Request);
        ArrivalTime.push_back(time);
        
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        
        int index = 0;
        pair<int , int > answer= ActiveQueue[0];
        int dist = abs(currentpos- ActiveQueue[0].second);
        for(int i = 1; i <ActiveQueue.size();i++)
        {
            int newDist = abs(ActiveQueue[i].second - currentpos);
            if(newDist < dist)
            {
                answer = ActiveQueue[i];
                dist = newDist;
                index = i;
            }
        }
        ActiveQueue.erase(ActiveQueue.begin() + index);
        ArrivalTime.erase(ArrivalTime.begin() + index );
        return answer;
    }
};

class SCAN: public Scheduler
{
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
    vector<int> ArrivalTime;
    bool Increase_Dir = true;
public:
    void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        ActiveQueue.push_back(Request);
        ArrivalTime.push_back(time);
        
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        
        if(Increase_Dir)
        {
            int next = numeric_limits<int>::max();
            int max = ActiveQueue[0].second;
            int index = -1, max_index = 0;
            pair<int , int > answer= ActiveQueue[0];
            //int dist = abs(currentpos- ActiveQueue[0].second);
            
             if( currentpos <= ActiveQueue[0].second)
             {
                 next = ActiveQueue[0].second;
                 index = 0;
             }
            for(int i = 1; i <ActiveQueue.size();i++)
            {
                //int newDist = abs(ActiveQueue[i].second - currentpos);
                if(ActiveQueue[i].second > max)
                {
                    //answer = ActiveQueue[i];
                    max = ActiveQueue[i].second;
                    max_index = i;
                }
                if(ActiveQueue[i].second < next && ActiveQueue[i].second >= currentpos)
                {
                    answer = ActiveQueue[i];
                    next = ActiveQueue[i].second;
                    index = i;
                }
            }
            if(index<0 )     //time to reverse TODO test
            {
                if( currentpos > ActiveQueue[0].second)
                {
                    Increase_Dir = false;
                    answer = ActiveQueue[max_index];
                    index = max_index;
                }
                else
                {
                    answer = ActiveQueue[0];
                    index = 0;
                }
            }
            
            ActiveQueue.erase(ActiveQueue.begin() + index);
            ArrivalTime.erase(ArrivalTime.begin() + index );
            return answer;
        }
        else
        {
            int next = -1;
            int min = ActiveQueue[0].second;
            int index = -1, min_index = 0;
            pair<int , int > answer= ActiveQueue[0];
            //int dist = abs(currentpos- ActiveQueue[0].second);
             if( currentpos >= ActiveQueue[0].second)
             {
                 next = min;
                 index = 0;
             }
            for(int i = 1; i <ActiveQueue.size();i++)
            {
                //int newDist = abs(ActiveQueue[i].second - currentpos);
                if(ActiveQueue[i].second < min)
                {
                    //answer = ActiveQueue[i];
                    min = ActiveQueue[i].second;
                    min_index = i;
                }
                if(ActiveQueue[i].second > next && ActiveQueue[i].second <= currentpos)
                {
                    answer = ActiveQueue[i];
                    next = ActiveQueue[i].second;
                    index = i;
                }
            }
            if(index<0 )     //time to reverse     TODO test
            {
                if( currentpos < ActiveQueue[0].second)
                {
                    Increase_Dir = true;
                    answer = ActiveQueue[min_index];
                    index = min_index;
                }
                else
                {
                    answer = ActiveQueue[0];
                    index = 0;
                }
            }
            
            ActiveQueue.erase(ActiveQueue.begin() + index);
            ArrivalTime.erase(ArrivalTime.begin() + index );
            return answer;
        }
    }
};


class CSCAN: public Scheduler
{
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
    vector<int> ArrivalTime;
public:
    void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        ActiveQueue.push_back(Request);
        ArrivalTime.push_back(time);
        
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        
        
        int next = numeric_limits<int>::max();
        int max = ActiveQueue[0].second;
        int index = -1, max_index = 0, min_index = 0;
        int min = ActiveQueue[0].second;
    
        
        pair<int , int > answer= ActiveQueue[0];
        //int dist = abs(currentpos- ActiveQueue[0].second);
        
        if( currentpos <= ActiveQueue[0].second)
        {
            next = ActiveQueue[0].second;
            index = 0;
        }
        
        for(int i = 1; i <ActiveQueue.size();i++)
        {
            //int newDist = abs(ActiveQueue[i].second - currentpos);
            if(ActiveQueue[i].second > max)
            {
                //answer = ActiveQueue[i];
                max = ActiveQueue[i].second;
                max_index = i;
            }
            if(ActiveQueue[i].second < min)
            {
                //answer = ActiveQueue[i];
                min = ActiveQueue[i].second;
                min_index = i;
            }
            if(ActiveQueue[i].second < next && ActiveQueue[i].second >= currentpos)
            {
                answer = ActiveQueue[i];
                next = ActiveQueue[i].second;
                index = i;
            }
        }
        if(index<0 )     //time to reverse TODO test
        {
            if( currentpos > ActiveQueue[0].second)
            {
                answer = ActiveQueue[min_index];
                index = min_index;
            }
            else
            {
                answer = ActiveQueue[0];
                index = 0;
            }
        }
            
        ActiveQueue.erase(ActiveQueue.begin() + index);
        ArrivalTime.erase(ArrivalTime.begin() + index );
        return answer;
    }
};



class FSCAN: public Scheduler
{
    vector<pair<int, int> > ActiveQueue;        //IO_op and position
    vector<int> ArrivalTime;
    bool Increase_Dir = true;
    vector<pair<int, int> > InActiveQueue;
    vector<int> InActArrivalTime;
    
public:
    void AddRequest(pair<int, int> Request, int time, int CurrIOop)
    {
        if(ActiveQueue.size() == 0 && CurrIOop < 0)
        {
            ActiveQueue.push_back(Request);
            ArrivalTime.push_back(time);
        }
        else
        {
            InActiveQueue.push_back(Request);
            InActArrivalTime.push_back(time);
        }
    }
    virtual pair<int, int> GetRequest(int currentpos = 0)
    {
        if(ActiveQueue.size() == 0)             //SWAP
        {
            ActiveQueue = InActiveQueue;
            ArrivalTime = InActArrivalTime;
            
            InActArrivalTime = vector<int>(0);
            InActiveQueue = vector<pair<int, int> > (0);
            
            Increase_Dir = true;
        }
        
        if(ActiveQueue.size() == 0)
            return make_pair(-1, -1);
        
        if(Increase_Dir)
        {
            int next = numeric_limits<int>::max();
            
            int max = ActiveQueue[0].second;
            int index = -1, max_index = 0;
            pair<int , int > answer= ActiveQueue[0];
            //int dist = abs(currentpos- ActiveQueue[0].second);
    
            if( currentpos <= ActiveQueue[0].second)
            {
                next = ActiveQueue[0].second;
                index = 0;
            }
            for(int i = 1; i <ActiveQueue.size();i++)
            {
                //int newDist = abs(ActiveQueue[i].second - currentpos);
                if(ActiveQueue[i].second > max)
                {
                    //answer = ActiveQueue[i];
                    max = ActiveQueue[i].second;
                    max_index = i;
                }
                if(ActiveQueue[i].second < next && ActiveQueue[i].second >= currentpos)
                {
                    answer = ActiveQueue[i];
                    next = ActiveQueue[i].second;
                    index = i;
                }
            }
            if(index<0 )     //time to reverse TODO test
            {
                if( currentpos > ActiveQueue[0].second)
                {
                    Increase_Dir = false;
                    answer = ActiveQueue[max_index];
                    index = max_index;
                }
                else
                {
                    answer = ActiveQueue[0];
                    index = 0;
                }
            }
            
            ActiveQueue.erase(ActiveQueue.begin() + index);
            ArrivalTime.erase(ArrivalTime.begin() + index );
            return answer;
        }
        else
        {
            int next = -1;
            int min = ActiveQueue[0].second;
            int index = -1, min_index = 0;
            pair<int , int > answer= ActiveQueue[0];
            //int dist = abs(currentpos- ActiveQueue[0].second);
            if( currentpos >= ActiveQueue[0].second)
            {
                next = min;
                index = 0;
            }
            for(int i = 1; i <ActiveQueue.size();i++)
            {
                //int newDist = abs(ActiveQueue[i].second - currentpos);
                if(ActiveQueue[i].second < min)
                {
                    //answer = ActiveQueue[i];
                    min = ActiveQueue[i].second;
                    min_index = i;
                }
                if(ActiveQueue[i].second > next && ActiveQueue[i].second <= currentpos)
                {
                    answer = ActiveQueue[i];
                    next = ActiveQueue[i].second;
                    index = i;
                }
            }
            if(index<0 )     //time to reverse     TODO test
            {
                if( currentpos < ActiveQueue[0].second)
                {
                    Increase_Dir = true;
                    answer = ActiveQueue[min_index];
                    index = min_index;
                }
                else
                {
                    answer = ActiveQueue[0];
                    index = 0;
                }
            }
            
            ActiveQueue.erase(ActiveQueue.begin() + index);
            ArrivalTime.erase(ArrivalTime.begin() + index );
            return answer;
        }
    }
};




void Simulation(vector<pair<int, int> > InstrSet, map<int, vector<int> > TimeToIOop)
{
    int QueueLength = 0;
    int Time = 0;
    int TimeRem_CurrProcess = -1;
    bool Process_Active = false;
    int Curr_IO_op = -1;
    int Destination = 0, CurrHeadPosition = 0;
    int Request_added = 0;
    bool timeBefore = false;
    while (Request_added< InstrSet.size() || QueueLength > 0 || Process_Active)
    {
        //Head_Movement
        if(Process_Active)
        {
            if(Destination>CurrHeadPosition)
                CurrHeadPosition ++;
            else if (Destination < CurrHeadPosition)
                CurrHeadPosition--;
            else
                Total_Movement--;
            Total_Movement++;
        }
        
        //Check for Process Add
        if(TimeToIOop.find(Time) != TimeToIOop.end() && !timeBefore)
        {
            for(int i = 0; i< TimeToIOop[Time].size(); i++)
            {
                int temp_Op = TimeToIOop[Time][i];
                scheduler->AddRequest(make_pair(temp_Op, InstrSet[temp_Op].second), Time, Curr_IO_op);
                Request_added++;
                QueueLength ++;
                
                cout<<Time<<":\t"<<temp_Op<<" add "<<InstrSet[temp_Op].second<<endl;
                
            }
        }
        
        //Check for Process Finish
        if(Process_Active)
        {
            if(Destination == CurrHeadPosition && Process_Active)//TimeRem_CurrProcess == 0)//
            {
                //Process Completed
                Process_Active = false;
                //Add Stats
                cout<<Time<<":\t"<<Curr_IO_op<<" finish "<<Time- InstrSet[Curr_IO_op].first<<endl;
                Total_TurnaroundTime += Time - InstrSet[Curr_IO_op].first;
                
                Curr_IO_op = -1;
            }
        }
        
        
        
        //Check for Process Issue
        if(!Process_Active)
        {
            pair<int, int> IORequest = scheduler->GetRequest(CurrHeadPosition);
            if(IORequest.first >= 0 )
            {
                Destination = IORequest.second;
                Curr_IO_op = IORequest.first;
                Process_Active = true;
                TimeRem_CurrProcess = abs(CurrHeadPosition - Destination);
                QueueLength--;
                cout<<Time<<":\t"<<Curr_IO_op<<" issue "<<Destination<<" "<<CurrHeadPosition<<endl;
                
                int WT = Time - InstrSet[Curr_IO_op].first;
                Total_WaitTime += WT;
                if(WT > Max_WaitTime)
                {
                    Max_WaitTime = WT;
                }

            }
        }
        if(TimeRem_CurrProcess!=0 || !Process_Active)
        {
            Time++;
            TimeRem_CurrProcess--;
            timeBefore = false;
        }
        else
        {
            timeBefore = true;
        }
    }
 
    Total_Time = Time-1;
}

void PrintSummary(int NumOps)
{
    double AvgTurnArd = double(Total_TurnaroundTime)/NumOps;
    
    printf("SUM: %d %d %.2lf %.2lf %d\n",
           Total_Time,
           Total_Movement,
           Total_TurnaroundTime/NumOps,
           Total_WaitTime/NumOps,
           Max_WaitTime);

}


int main(int argc, const char * argv[]) {
    // insert code here...
    

    cout << "Hello, World!\n";
    
    
    ifstream file;
    //file.open(argv[ArgIndex]) ;
    file.open("/Users/vipinarora/Desktop/GradSem1/OS/Assignment 4/TestData/input0") ;
    
    string line, buffer = "";
    map<int, vector<int> > TimeToIOop;
    vector<pair <int, int> > Instr_Set;
    int IO_op = 0;
    while( getline(file, line ))
    {
        if(line[0] == '#')                  //Line is a comment
            continue;
        istringstream ss(line);
        
        int StartTime;
        int Position;
        ss>>StartTime;
        //Page_use = num;
        ss>>Position;
        
        TimeToIOop[StartTime].push_back(IO_op);
        
        Instr_Set.push_back(make_pair(StartTime, Position));
        IO_op++;
    }
    
    scheduler = new(CSCAN);
    
    Simulation(Instr_Set, TimeToIOop);
    
    PrintSummary(Instr_Set.size());
    cout<<"endl";
    
    
    return 0;
}

