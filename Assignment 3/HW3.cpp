
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <limits>
#include <string>

using namespace std;
long long int CountMaps =0, CountUnMaps = 0, CountZeros = 0, CountOuts = 0, CountIns = 0;
bool  OptO = false, OptP = false, OptF = false, OptS = false;

unsigned int NewAge = std::numeric_limits<unsigned int>::max()/2 + 1;
int Num_Instr = 0, Num_frames = 64;
int RdsOffset = 1;
vector<int> randValue;
int myRandom(int URange)
{
    int num = ( (randValue[ (RdsOffset % randValue.size() )]) % URange ) ;
    RdsOffset++;
    return num;
}

struct PTE {
    unsigned int present:1;
    unsigned int modified:1;
    unsigned int referenced:1;
    unsigned int pagedout:1;
    unsigned int index:28;
};
struct PTE PageTable[64];

struct Frame
{
    int FrameNumber;
};

struct FT
{
    int Fnum;
    Frame* Next_Frame;
    Frame* Prev_Frame;
} OccupiedList;

struct Node
{
    Frame NodeFrame;
    Node* Next_Frame;
    Node* Prev_Frame;
} ;

vector<int> FrameTable;

void PrintPageTable()
{
    for(int i=0;i<64;i++)
    {
        if(PageTable[i].present)
        {
            cout<<i<<":";
            if(PageTable[i].referenced)
                cout<<"R";
            else
                cout<<"-";
            if(PageTable[i].modified)
                cout<<"M";
            else
                cout<<"-";
            if(PageTable[i].pagedout)
                cout<<"S";
            else
                cout<<"-";
            cout<<" ";
        }
        else
        {
            if(PageTable[i].pagedout)
                cout<<"# ";
            else
                cout<<"* ";
        }
    }
    cout<<endl;
}

void PrintFrameTable()
{
    for(int i =0;i<FrameTable.size();i++)
    {
        if(FrameTable[i]==-1)
        {
            cout<<"* ";
        }
        else
            cout<<FrameTable[i]<<" ";
    }
    cout<<endl;
}

class FrameLL
{
private:
    Node* Head;
    int ListLength;
public:
    int getLength()
    {
        return ListLength;
    }
    FrameLL()
    {
        Frame MyFrame;
        MyFrame.FrameNumber = -1;
        Node* MyNode = new Node;
        MyNode->NodeFrame = MyFrame;
        MyNode->Next_Frame = NULL;
        MyNode->Prev_Frame = NULL;
        Head = MyNode;
        ListLength = 0;
    }
    bool insertNode( Node * newNode, int position )
    {
        if(position<0 || position > ListLength + 1)
            return false;
        if(Head->Next_Frame == NULL)
        {
            Head->Next_Frame = newNode;
            newNode->Prev_Frame = Head;
            ListLength++;
            return true;
        }
        int count = 0;
        Node* P = Head;
        Node* Q = Head;
        while (Q)
        {
            if (count == position)
            {
                P->Next_Frame = newNode;
                newNode->Prev_Frame = P;
                newNode->Next_Frame = Q;
                // if(Q!=NULL)
                    Q->Prev_Frame = newNode;
                ListLength++;
                return true;
            }
            P = Q;
            Q = P->Next_Frame;
            count++;
        }
        if (count == position)      //Q is null
        {
            P->Next_Frame = newNode;
            newNode->Prev_Frame = P;
            newNode -> Next_Frame = Q;
            ListLength++;
            return true;
        }
        return false;
    }
    bool removeNode( int position )
    {
        if ((position <= 0) || (position > ListLength + 1))
        {
            return false;
        }
        if (Head -> Next_Frame == NULL)
        {
            // cout << "nError: there is nothing to remove.n";
            return false;
        }
        int count = 0;
        Node * P = Head;
        Node * Q = Head;
        while (Q)
        {
            if (count == position)
            {
                P -> Next_Frame = Q ->  Next_Frame;
                Node * R = Q->Next_Frame;
                if(R)
                {
                    R->Prev_Frame = Q->Prev_Frame;
                }
                delete Q;
                ListLength--;
                return true;
            }
            P = Q;
            Q = P -> Next_Frame;
            count++;
        }
        return false;
    }
    Frame* GetFrame(int pos)
    {
        if ((pos <= 0) || (pos > ListLength + 1))
        {
            return NULL;
        }
        if (Head -> Next_Frame == NULL)
        {
            return NULL;
        }
        int count = 0;
        Node * P = Head;
        Node * Q = Head;
        
        while (Q)
        {
            if (count == pos)
            {
                P -> Next_Frame = Q ->  Next_Frame;
                Node * R = Q->Next_Frame;
                if(R)
                {
                    R->Prev_Frame = P;
                }
                Frame NewFrame = Q->NodeFrame;
                //removeNode(pos);
                delete Q;
                ListLength--;
                return &(NewFrame);
            }
            P = Q;
            Q = P -> Next_Frame;
            count++;
        }
        return NULL;
    }
    int Get_Fno(int pos)
    {
        if ((pos <= 0) || (pos > ListLength + 1))
        {
            return false;
        }
        if (Head -> Next_Frame == NULL)
        {
            return false;
        }
        int count = 0;
        Node * P = Head;
        Node * Q = Head;
        
        while (Q)
        {
            if (count == pos)
            {
                return Q->NodeFrame.FrameNumber;
            }
            P = Q;
            Q = P -> Next_Frame;
            count++;
        }
        return NULL;
    }
} FreeList;

void MakeList(int size)
{
    for(int i = 0;i<size;i++)
    {
        Frame NewFrame;
        NewFrame.FrameNumber = i;
        //Node NewNode = *new(Node);
        Node * Pointer = new(Node) ;
        //Pointer = NewNode;
        Pointer->NodeFrame = NewFrame;
        Pointer->Next_Frame = NULL;
        Pointer->Prev_Frame = NULL;
        FreeList.insertNode(Pointer, i+1);
    }
}

void ResetReadBits()
{
    for(int i=0;i<64;i++)
    {
        PageTable[i].referenced = 0;
    }
}

class Pager {

public:
    virtual Frame* allocate_frame()
    {
        Frame* myFrame = new Frame;
        myFrame->FrameNumber = 0;
        
        return myFrame;
    }
    virtual void Page_added(Frame MyFrame)
    {}
    virtual void AddAccess(int pte_index)
    {}
    virtual void RemoveAge(int pte_index)
    {}
} ;
Pager* pager;

class FIFO: public Pager
{
    FrameLL OccupiedLL;
public:
    Frame* allocate_frame()
    {
        Frame* myFrame = OccupiedLL.GetFrame(1);
        return myFrame;
    }
    void Page_added(Frame MyFrame)
    {
        Node* newNode= new Node;
        newNode->NodeFrame = MyFrame;
        OccupiedLL.insertNode(newNode, OccupiedLL.getLength()+1);
    }
    void AddAccess(int pte_index)  {}
};

class RandomAlgo: public Pager
{
public:
    Frame* allocate_frame()
    {
        Frame* myFrame = new Frame;
        myFrame->FrameNumber = myRandom(Num_frames);
        return myFrame;
    }
    void Page_added(Frame MyFrame)  {}
    void AddAccess(int pte_index)  {}
};

class SecondChance: public Pager
{
    FrameLL OccupiedLL;
public:
    Frame* allocate_frame()
    {
        //Check
        int i = 1;
        while( true)
        {
            Frame* myFrame = OccupiedLL.GetFrame(1);
            int Fno = myFrame->FrameNumber;
            if (!PageTable[FrameTable[Fno]].referenced)
                return myFrame;
            PageTable[FrameTable[Fno]].referenced = 0;
            Page_added(*myFrame);
            i++;
        }
        return NULL;
    }
    void Page_added(Frame MyFrame)
    {
        Node* newNode= new Node;
        newNode->NodeFrame = MyFrame;
        OccupiedLL.insertNode(newNode, OccupiedLL.getLength()+1);
    }
    void AddAccess(int pte_index)  {}
};
class NRU: public Pager
{
    FrameLL OccupiedLL;
    int Counter ;
public:
    NRU()
    {
        Counter =  0;
    }

    Frame* allocate_frame()
    {
        vector<int> Class0, Class1, Class2, Class3;
        for(int i=0;i<64;i++)
        {
            if(PageTable[i].present)
            {
                if(PageTable[i].referenced)
                {
                    if(PageTable[i].modified)
                    {
                        Class3.push_back(i);
                    }
                    else
                    {
                        Class2.push_back(i);
                    }
                }
                else
                {
                    if(PageTable[i].modified)
                    {
                        Class1.push_back(i);
                    }
                    else
                    {
                        Class0.push_back(i);
                    }
                }
            }
        }
        Counter++;
        if(Counter%10 == 0)
        {
            ResetReadBits();
        }
        if(Class0.size()>0)
        {
            Frame* myFrame = new Frame;
            int Class_index = myRandom(Class0.size());
            myFrame->FrameNumber = PageTable[Class0[Class_index]].index;
            return myFrame;
        }
        else if (Class1.size()>0)
        {
            Frame* myFrame = new Frame;
            int Class_index = myRandom(Class1.size());
            myFrame->FrameNumber = PageTable[Class1[Class_index]].index;
            return myFrame;
        }
        else if (Class2.size()>0)
        {
            Frame* myFrame = new Frame;
            int Class_index = myRandom(Class2.size());
            myFrame->FrameNumber = PageTable[Class2[Class_index]].index;
            return myFrame;
        }
        else
        {
            Frame* myFrame = new Frame;
            int Class_index = myRandom(Class3.size());
            myFrame->FrameNumber = PageTable[Class3[Class_index]].index;
            return myFrame;
        }
    }
    void Page_added(Frame MyFrame)
    {
        Node* newNode= new Node;
        
        newNode->NodeFrame = MyFrame;
        OccupiedLL.insertNode(newNode, OccupiedLL.getLength()+1);
    }
    void AddAccess(int pte_index)  {}
};

class VirtClock: public Pager
{
    FrameLL OccupiedLL;
    int pointer;
public:
    VirtClock()
    {
        pointer = 0;
    }

    Frame* allocate_frame()
    {
        while( true)
        {
            if(!PageTable[pointer].present)
            {
                pointer = (pointer+1)%64;
                continue;
            }
            if(PageTable[pointer].referenced)
            {
                PageTable[pointer].referenced = 0;
            }
            else
            {
                Frame* myFrame = new Frame;
                myFrame->FrameNumber = PageTable[pointer].index;
                pointer = (pointer + 1)%64;
                return myFrame;
            }
            pointer = (pointer + 1)%64;
            continue;
        }
        return NULL;
    }
    void Page_added(Frame MyFrame)  {}
    void AddAccess(int pte_index)  {}
};

class PhyClock: public Pager
{
    FrameLL OccupiedLL;
    int pointer;
public:
    PhyClock()
    {
        pointer = 0;
    }

    Frame* allocate_frame()
    {
        while( true)
        {
            int pte_index = FrameTable[pointer];
            if(PageTable[pte_index].referenced)
            {
                PageTable[pte_index].referenced = 0;
            }
            else
            {
                Frame* myFrame = new Frame;
                myFrame->FrameNumber = PageTable[pte_index].index;
                pointer = (pointer + 1)%Num_frames;
                return myFrame;
            }
            pointer = (pointer + 1)%Num_frames;
            continue;
        }
        return NULL;
    }
    void Page_added(Frame MyFrame)  {}
    void AddAccess(int pte_index)  {}
};

class VirtAging: public Pager
{
    FrameLL newAccess;
    vector<unsigned int> AgeVec;// = *new vector<unsigned int>(64,0);
    int pointer;
public:
    VirtAging()
    {
        pointer = 0;
        AgeVec = vector<unsigned int>(64,0);
    }

    Frame* allocate_frame()
    {
        int i = 0;
        unsigned int min = NewAge + 1;
        int min_ind = 0;
        while(i<64)
        {
            if(!PageTable[i].present)
            {
                i++;
                continue;
            }
            PageTable[i].referenced =0;         //TODO:Check

            if(AgeVec[i]<min)
            {
                min = AgeVec[i];
                min_ind = i;
            }
            AgeVec[i]/=2;
            i++;
        }
        Frame* myFrame = new Frame;
        myFrame->FrameNumber = PageTable[min_ind].index;
        return myFrame;
    }
    void Page_added(Frame MyFrame)  {}
    void AddAccess(int pte_index)
    {
        if(AgeVec[pte_index]<NewAge)
        {
            AgeVec[pte_index] += NewAge;
        }
    }
    void RemoveAge(int pte_index)
    {
        AgeVec[pte_index] = 0;
    }
};

class PhyAging: public Pager
{
    FrameLL newAccess;
    vector<unsigned int> AgeVec;// = *new vector<unsigned int>(64,0);
public:
    PhyAging()
    {
        AgeVec = vector<unsigned int>(64,0);
    }

    Frame* allocate_frame()
    {
        int i = 0;
        unsigned int min = NewAge + 1;
        int min_ind = 0;
        while(i<Num_frames)
        {
            int pte_index = FrameTable[i];
            PageTable[pte_index].referenced =0;         //TODO:Check
            if(AgeVec[i]<min)
            {
                min = AgeVec[i];
                min_ind = i;
            }
            AgeVec[i]/=2;
            i++;
        }
        Frame* myFrame = new Frame;
        myFrame->FrameNumber = min_ind;
        
        return myFrame;
    }
    void Page_added(Frame MyFrame)  {}
    void AddAccess(int pte_index)
    {
        int Fno = PageTable[pte_index].index;
        if(AgeVec[Fno]<NewAge)
        {
            AgeVec[Fno] += NewAge;
        }
    }
};

Frame* allocate_frame_from_free_list( )
{
    Frame* FreeFrame = FreeList.GetFrame(1);
    return FreeFrame;
}

int Get_frame() {
    Frame *frame = allocate_frame_from_free_list();
    int Fno;
    if(frame == NULL)
        frame = pager->allocate_frame();
    //if(frame)
    Fno = frame->FrameNumber;
    
    pager->Page_added(*frame);
    return Fno;
}

int get_PTE_index(Frame* frame, int index)
{   //TODO: implement
    if(FrameTable[(PageTable[index].index)] == index)
        return index;
    return -1;
}

void PageIn( int pte_index, int Fno, bool Write, int Instr_No)
{
    CountIns++;
    PageTable[pte_index].present = 1;
    PageTable[pte_index].modified = Write;
    PageTable[pte_index].referenced = 1;
    PageTable[pte_index].pagedout = 1;
    PageTable[pte_index].index = Fno;
    if(OptO)
        cout<<Instr_No<<": IN     "<<pte_index<<"   "<<Fno<<"\n";
}

void Zero(int pte_index, int Fno, int Instr_No)
{
    //TODO: Check
    CountZeros++;
    FrameTable[Fno] = -1;
    PageTable[pte_index].present = 0;
    PageTable[pte_index].modified = 0;
    PageTable[pte_index].referenced = 0;
    PageTable[pte_index].pagedout = 0;
    PageTable[pte_index].index = 0;
    if(OptO)
        cout<<Instr_No<<": ZERO        "<<Fno<<"\n";
}

void Map(int pte_index, int Fno, int Instr_No)
{
    CountMaps++;
    FrameTable[Fno] = pte_index;
    PageTable[pte_index].index = Fno;
    if(OptO)
        cout<<Instr_No<<": MAP     "<<pte_index<<"   "<<Fno<<"\n";
}

void UnMap(int pte_index, int Fno, int Instr_No)
{
    CountUnMaps++;
    FrameTable[Fno] = -1;
    PageTable[pte_index].index = 0;
    PageTable[pte_index].present =0;
    if(OptO)
        cout<<Instr_No<<": UNMAP   "<<pte_index<<"   "<<Fno<<"\n";
    if(PageTable[pte_index].modified)
    {
        CountOuts++;
        PageTable[pte_index].pagedout = 1;//PageOut(pte_index, Fno);
        if(OptO)
            cout<<Instr_No<<": OUT    "<<pte_index<<"   "<<Fno<<"\n";
    }
    pager->RemoveAge(pte_index);
}

void WriteTo(int pte_index, int Fno)
{
    PageTable[pte_index].modified = 1;
    PageTable[pte_index].present = 1;
    PageTable[pte_index].referenced = 1;
    //TODO: remove? Redundancy?
    PageTable[pte_index].index = Fno;
    FrameTable[Fno] = pte_index;
}

void ReadFrom(int pte_index, int Fno)
{
    PageTable[pte_index].present = 1;
    PageTable[pte_index].referenced = 1;
    //TODO: remove? Redundancy?
    PageTable[pte_index].index = Fno;
    FrameTable[Fno] = pte_index;
}

void Simulation(vector <pair<bool,int> > InstrSet) {

    for(int i =0;i<InstrSet.size();i++)
    {
        bool write = InstrSet[i].first;
        int pte_index = InstrSet[i].second;
        Frame *frame;
        int Fno;
        if(OptO)
            cout<<"==> inst: "<<int(write)<<" "<<pte_index<<"\n";
        if (PageTable[pte_index].present ==0)
        {                                               //pte not present
            Fno = Get_frame();
            if(FrameTable[Fno]>-1)
                UnMap(FrameTable[Fno], Fno, i);
            if (PageTable[pte_index].pagedout)          //
                PageIn(pte_index, Fno, write, i);
            else
                Zero(pte_index, Fno, i);
            Map(pte_index,Fno, i);
        }
        else
        {
            Fno = PageTable[pte_index].index;
        }
        //Update_pte(read/modify/write) bits based on operations.
        if(write)
        {
            WriteTo(pte_index, Fno);
        }
        else
        {
            ReadFrom(pte_index, Fno);
        }
        pager->AddAccess(pte_index);
    }
}

int main(int argc, char ** argv) {
    
    int c;
    char Algo = 'f';
    string Option="";
    int ArgIndex = 1;
    opterr = 0;
    // Get opt implementation

    while ((c = getopt (argc, argv, "a:o:f:")) != -1)
        switch (c)
    {
        case 'a':
            Algo = char (optarg[0]);
            break;
        case 'o':
            //cvalue = optarg;
            Option = string(optarg);
            break;
        case 'f':
            Num_frames = atoi(optarg);
            
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
    //
    for(int i = 0;i<Option.length();i++)
    {
        if(Option[i] == 'O')
            OptO = true;
        if(Option[i] == 'F')
            OptF = true;
        if(Option[i] == 'P')
            OptP = true;
        if(Option[i] == 'S')
            OptS = true;
        /*
        if(Option[i] == 'f')
        OptO = true;
        if(Option[i] == 'a')
            OptO = true;
        if(Option[i] == 'p')
            OptO = true;
        */
    }
    string RFilename = "/Users/vipinarora/Desktop/GradSem1/OS/Assignmnent 3/lab3_assign/rfile";
    //string filename = argv[1];
    
    ifstream RFile;
    RFile.open(argv[ArgIndex+1]) ;
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
    


    
    ifstream file;
    file.open(argv[ArgIndex]) ;
//    file.open("/Users/vipinarora/Desktop/GradSem1/OS/Assignmnent 3/lab3_assign/in60") ;

    string line, buffer = "";
    vector<pair<bool, int> > Instr_Set;
    
    while( getline(file, line ))
    {
        if(line[0] == '#')                  //Line is a comment
            continue;
        buffer = buffer + line + "\n";
        istringstream ss(line);
        
        Num_Instr++;
        int num;
        bool write;
        ss>>write;
        //Page_use = num;
        ss>>num;
        if(num>64)
        {
            cout<<"You lied :'(\n";     //TODO: Check if correct
            return 0;
        }
        Instr_Set.push_back(make_pair(write, num));
        //Page_vir_loc = num;
    
    }
    FrameTable = *new vector<int>(Num_frames, -1);
   if(Algo == 'f')
        pager = new(FIFO);
    else if(Algo == 'N')
        pager = new(NRU);
    else if(Algo == 'r')
        pager = new(RandomAlgo);
    else if(Algo == 's')
        pager = new(SecondChance);
    else if(Algo == 'c')
        pager = new(PhyClock);
    else if(Algo == 'X')
        pager = new(VirtClock);
    else if(Algo == 'a')
        pager = new(PhyAging);
    else if(Algo == 'Y')
        pager = new(VirtAging);
    

    MakeList(Num_frames);
    
    
    Simulation(Instr_Set);
    
    if(OptP)
    {
        PrintPageTable();
    }
    
    if(OptF)
    {
        PrintFrameTable();
    }
    
    if(OptS)
    {
    long long int TCost = Num_Instr + 400*CountUnMaps + 400*CountMaps + 3000*CountIns + 3000*CountOuts + 150*CountZeros;
    cout<<"SUM "<<Num_Instr<<" U="<<CountUnMaps<<" M="<<CountMaps<<" I="<<CountIns<<" O="<<CountOuts<<" Z="<<CountZeros<<" ===> "<<TCost<<"\n";
    }
    return 0;
}
