//
//  main.cpp
//  Linker
//
//  Created by Vipin Arora on 29/09/16.
//  Copyright © 2016 Pranit Arora. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iomanip>

using namespace std;

int Curr_Tok = 0;


bool isWSpace(char C)
{
    return (C== ' ' || C == '\t' || C == '\n');
        
}


vector<string> Tokenizer1(string inp, vector<int> &LineNum, vector<int> &Offset)
{
    int Line = 1;
    int Off = 1, PrevOffset = 1;
    char* tok ;
    tok = &inp[0];
    vector<string> TokenStream;
    string CurrStr ="";
    while (*tok)
    {
        if(isWSpace(*tok))
        {
            if(CurrStr.length()>0)
            {
                LineNum.push_back(Line);
                Offset.push_back(PrevOffset);
                PrevOffset = Off;
                TokenStream.push_back(CurrStr);
                CurrStr = "";
            }
            if(*tok =='\n')
            {
                Line ++;
                PrevOffset = 1;
                Off = 1;
            }
            else if (*tok  == '\t')
            {
                PrevOffset +=1;                // Check;
                Off +=1;
            }
            else
            {
                PrevOffset ++;
                Off ++;
            }
            tok++;
        }
        else
        {
            CurrStr = CurrStr + *tok;
            tok++;
            Off++;
        }
    } // cout<<answer<<"\nOver\n";
    
    return TokenStream;
}


int GetInt(vector<string> TokStr)
{
    int Result = 0;
    //Check
    if(Curr_Tok == TokStr.size())
        return -1;
    string temp = TokStr[Curr_Tok];
    int i = 0;
    while(i<temp.length())
    {
        if(isdigit(temp[i]))
        {
            Result = 10*Result + (temp[i]- '0');
        }
        else
        {
            // Raise Exception
            return -1;
        }
        i++;
    }
    return Result;
}

string GetSymbol(vector<string> TokStr)
{
    string Result = "";
    //Check
    if(Curr_Tok == TokStr.size())
        return "";
    string temp = TokStr[Curr_Tok];
    int i = 0;
    if( isalpha(temp[i]))    //(temp[i]>='a' && temp[i]<='z') || (temp[i]>='A' && temp[i]<='Z') )
        Result +=temp[i];
    else
        return "";
    i++;
    while(i<temp.length())
    {
        if( (isdigit(temp[i])) || isalpha(temp[i]) ) //(temp[i]>='a' && temp[i]<='z') || (temp[i]>='A' && temp[i]<='Z') )
        {
            Result += temp[i];
        }
        else
        {
            return "";
        }
        i++;
    }

    return Result;
}


int ReadDef1(vector<string> TokStr, vector<int> LineNum, vector<int> BufOffset, int ModuleOffset, map<string, int> &SymbolTable, map<string, string> &SymTableError, string &ErrorMsg) //Check
{
    int NumDef = GetInt(TokStr);
    if(NumDef == -1)
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", BufOffset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " +  string(buffer2) + ": NUM_EXPECTED\n";
        //cout<<ErrorMsg;
        return -1;
    }
    if(NumDef>16)
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", BufOffset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": TO_MANY_DEF_IN_MODULE\n";
        //cout<<ErrorMsg;
        return -1;
    }
    Curr_Tok++;
    for(int i=0;i<NumDef;i++)
    {
        string Sym = GetSymbol(TokStr);
        if(SymbolTable.find(Sym)!= SymbolTable.end())
        {
            //Raise Error  Rule 2
            SymTableError[Sym] = " Error: This variable is multiple times defined; first value used";
            Curr_Tok += 2;
            continue;
        }
        if(Sym == "")
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", BufOffset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": SYM_EXPECTED\n";
            //cout<<ErrorMsg;
            return -1;
        }
        if (Sym.length()>16)
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", BufOffset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": SYM_TOLONG\n";
            //cout<<ErrorMsg;
            return -1;
        }
        Curr_Tok++;
        int offset = GetInt(TokStr);
        if(offset== -1)
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", BufOffset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": NUM_EXPECTED\n";
            //cout<<ErrorMsg;
            return -1;
        }
        Curr_Tok++;
        if(SymbolTable.find(Sym)== SymbolTable.end())
            SymbolTable[Sym] = ModuleOffset + offset;
        /*
        else
        {
            //Raise Error  Rule 2
            SymTableError[Sym] = " Error: This variable is multiple times defined; first value used";  
        }
        */
    }
    return 0;
}

int ReadUse1(vector<string> TokStr, vector<int> LineNum, vector<int> Offset, string &ErrorMsg)
{
    int NumSym = GetInt(TokStr);
    if(NumSym == -1)
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", Offset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": NUM_EXPECTED\n";
        //cout<<ErrorMsg;
        return -1;
    }
    if(NumSym>16)
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", Offset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": TO_MANY_USE_IN_MODULE\n";
        //cout<<ErrorMsg;
        return -1;
    }
    Curr_Tok++;
    for(int i=0;i<NumSym;i++)
    {
        string Sym = GetSymbol(TokStr);
        if(Sym == "")
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", Offset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": SYM_EXPECTED\n";
            //cout<<ErrorMsg;
            return -1;
        }
        if (Sym.length()>16)
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", Offset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": SYM_TOLONG\n";
            //cout<<ErrorMsg;
            return -1;
        }

        Curr_Tok++;
        
    }
    return NumSym;

}
int ReadProg1(vector<string> TokStr, int CurrInstr, vector<int> LineNum, vector<int> Offset, int NumSym, string &ErrorMsg)//Add SymTable, OffsetTable
{
    int NumProg = GetInt(TokStr);
    if(NumProg== -1)
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", Offset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + buffer2 + ": NUM_EXPECTED\n";
        //cout<<ErrorMsg;
        return -1;
    }
    if(NumProg + CurrInstr>512) // Check
    {
        const int max_size = 512;
        char buffer1[max_size] = {0};
        sprintf(buffer1, "%d", LineNum[Curr_Tok]);
        char buffer2[max_size] = {0};
        sprintf(buffer2, "%d", Offset[Curr_Tok]);
        ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": TO_MANY_INSTR\n";
        //cout<<ErrorMsg;
        return -1;
    }
    
    Curr_Tok++;
    for(int i=0;i<NumProg;i++)
    {
        string Sym = GetSymbol(TokStr);
        if(Sym == "" || (Sym!= "A" && Sym!= "I" && Sym!= "R" && Sym!= "E") )
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", Offset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": ADDR_EXPECTED\n";
            ////cout<<ErrorMsg;
            return -1;
        }
        if (Sym.length()>16)
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", Offset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": SYM_TOLONG\n";
            //cout<<ErrorMsg;
            return -1;
        }
        Curr_Tok++;
        
        int instr = GetInt(TokStr);
        if(instr== -1)
        {
            const int max_size = 512;
            char buffer1[max_size] = {0};
            sprintf(buffer1, "%d", LineNum[Curr_Tok]);
            char buffer2[max_size] = {0};
            sprintf(buffer2, "%d", Offset[Curr_Tok]);
            ErrorMsg = "Parse Error line " + string(buffer1) + " offset " + string(buffer2) + ": NUM_EXPECTED\n";     //Check if Num or Addr
            //cout<<ErrorMsg;
            return -1;
        }
        //Check if this should be here or not
        /*if(TokStr[Curr_Tok].length()<4)
        {
            cout<<"Parse Error line "<<LineNum[Curr_Tok]<<" offset "<<Offset[Curr_Tok]<<": ADDR_EXPECTED\n";
            return -1;      //CHECK
        }
         */
        // Error 10, Error 11 Way 2
        Curr_Tok++;
        
    
    }
    return NumProg;           // Check
    
}


int ReadModule1(vector<string> TokStr, vector<int> MemMap, vector<int> ModList, vector<int> LineNum, vector<int> Offset,  map<string, int> &SymbolTable, map<string, string> &SymTableError, string &ErrorMsg)
{
    if(ReadDef1(TokStr, LineNum, Offset, ModList[ModList.size()-1], SymbolTable, SymTableError, ErrorMsg) == -1)
        return -1;
    int NumSym = ReadUse1(TokStr, LineNum, Offset, ErrorMsg);
    if(NumSym == -1)//, LineNum, Offset);
        return -1;
    return ReadProg1(TokStr, ModList[ModList.size()-1], LineNum, Offset, NumSym, ErrorMsg);    //check
}
//Run 1 end


//Run 2 start

vector<string> Tokenizer2(string inp)
{
    char* tok ;
    tok = &inp[0];
    vector<string> TokenStream;
    string CurrStr ="";
    while (*tok)
    {
        if(isWSpace(*tok))
        {
            if(CurrStr.length()>0)
            {
                TokenStream.push_back(CurrStr);
                CurrStr = "";
            }
            tok++;
        }
        else
        {
            CurrStr = CurrStr + *tok;
            tok++;
        }
    }
    return TokenStream;
}

int ReadDef2(vector<string> TokStr, int ModuleOffset, map<string, int> &SymbolTable, int ModSize, int ModuleNum, map<string, bool> &SymRead)
{
    
    int NumDef = GetInt(TokStr);
    Curr_Tok++;
    for(int i=0;i<NumDef;i++)
    {
        string Sym = GetSymbol(TokStr);
        if(SymRead.find(Sym)!= SymRead.end())
        {
            //Raise Error  Rule 2
            //SymTableError[Sym] = " Error: This variable is multiple times defined; first value used";
            Curr_Tok += 2;
            continue;
        }
        Curr_Tok++;
        int offset = GetInt(TokStr);
        Curr_Tok++;
        // Rule 5
        if(offset>ModSize)      //0 gave problems with 0;
        {
            //Warning Rule 5
            //Check ModNum+1
            //Check Position
            cout<<"Warning: Module "<<ModuleNum + 1<<": "<<Sym<<" to big "<<offset<<" (max="<<ModSize<<") assume zero relative\n";
            offset = 0;
        }
        SymRead[Sym] = true;
        SymbolTable[Sym] = ModuleOffset + offset;
    }
    return 0;

    
    
}

int ReadUse2(vector<string> TokStr, vector<string> &UseSym, map<string, bool> &SymTableUsed)
{
    int NumSym = GetInt(TokStr);
    Curr_Tok++;
    for(int i=0;i<NumSym;i++)
    {
        string Sym = GetSymbol(TokStr);
        UseSym.push_back(Sym);
        
        //For Rule 4
        SymTableUsed[Sym] = true;
        Curr_Tok++;
    }
    return NumSym;
    
}

int ReadProg2(vector<string> TokStr, vector<int> &MemMap, vector<string> &Errs, int CurrInstr, vector<string> UseSym, map<string, int> SymbolTable, vector<string> &Warnings, vector<string> &ModuleWarnings, int ModuleNumber)//Add SymTable, OffsetTable
{
    int NumProg = GetInt(TokStr);
    Curr_Tok++;
    vector<bool> UseSymUsed(UseSym.size(), false);
    
    for(int i=0;i<NumProg;i++)
    {
        string Sym = GetSymbol(TokStr);
        Curr_Tok++;
        // Error 10, Error 11 Way 1
        if(TokStr[Curr_Tok].length()>4)
        {
            //Raise Error
        }
        int instr = GetInt(TokStr);
        // Error 10, Error 11 Way 2
        
        Curr_Tok++;
        
        
        if(Sym == "I")
        {
            if(instr>10000)
            {
                //Rule 10
                Errs[CurrInstr + i] = " Error: Illegal immediate value; treated as 9999";          // Correction: Change index
                instr = 9999;
            }
            MemMap.push_back(instr);
        }
        if(Sym == "A")
        {
            if(instr>10000)
            {
                //Rule 11
                Errs[CurrInstr + i] = " Error: Illegal opcode; treated as 9999";          // Correction: Change index
                instr = 9999;
            }
            else
            {
                int SymIndex = instr%1000;
                if(SymIndex > 512)
                {
                    //Rule 8
                    Errs[CurrInstr+i] = " Error: Absolute address exceeds machine size; zero used";
                    instr = instr - SymIndex;     //Check
                }
            }
            MemMap.push_back(instr);
            
            // Add Check for Error 8
        }
        //Nothing in Run 1, change in Run 2
        if(Sym == "R")
        {
            if(instr>10000)
            {
                //Rule 11
                Errs[CurrInstr + i] = " Error: Illegal opcode; treated as 9999";          // Correction: Change index
                instr = 9999;
            }
            else
            {
                int SymIndex = instr%1000;
                if(SymIndex > NumProg)
                {
                    //Rule 9
                    Errs[CurrInstr+i] = " Error: Relative address exceeds module size; zero used";
                    instr = instr - SymIndex + CurrInstr;     //Check
                }
                else
                {
                    instr += CurrInstr;
                }
                
            }
            MemMap.push_back(instr);
            
            // Add Check for Error 9
        }
        if(Sym == "E")
        {
            //Check Remove redundancy
            if(instr>10000)
            {
                //Rule 11
                Errs[CurrInstr + i] = " Error: Illegal opcode; treated as 9999";          // Correction: Change index
                instr = 9999;
                MemMap.push_back(instr);
            }
            else
            {
                int SymIndex = instr%1000;
                if(SymIndex >= UseSym.size())
                {
                    //Raise Error
                    //Rule 6
                    Errs[CurrInstr+i] = " Error: External address exceeds length of uselist; treated as immediate";
                    
                    MemMap.push_back(instr);    //Check
                }
                else
                {
                    string Sym = UseSym[SymIndex];
                    if(SymbolTable.find(Sym)!=SymbolTable.end())
                    {
                        instr = instr - SymIndex + SymbolTable[Sym];   //Check
                        UseSymUsed[SymIndex] = true;
                    }
                    else
                    {
                        instr = instr - SymIndex;
                        Errs[CurrInstr + i] = " Error: " + Sym +" is not defined; zero used";    //Rule 3
                        UseSymUsed[SymIndex] = true;
                    }
                    
                    MemMap.push_back(instr);
                }
            }
            
        }
        //AddToMemoryMapTable
    }
    //Rule 7
    for(int i = 0;i<UseSymUsed.size();i++)
    {
        if(!UseSymUsed[i])
        {
           // string temp;
            const int max_size = 512;
            char buffer[max_size] = {0};
            sprintf(buffer, "%d", ModuleNumber + 1);
            ModuleWarnings[ModuleNumber] = ("Warning: Module " + string(buffer) + ": "+ UseSym[i] + " appeared in the uselist but was not actually used\n");
            //Check

        }
    }
    
    return NumProg;           // Check
    
}


int ReadModule2(vector<string> TokStr, vector<int> &MemMap, vector<int> ModList, vector<string> &Errs, map<string, int> &SymbolTable, vector<string> &Warnings, vector<string> &ModuleWarnings, map<string, bool> &SymTableUsed, int ModNum, map<string, bool> &SymRead)
{
    ReadDef2(TokStr, ModList[ModNum], SymbolTable, ModList[ModNum+1] - ModList[ModNum] -1, ModNum, SymRead);
    vector<string> UseSym;
    int NumSym = ReadUse2(TokStr, UseSym, SymTableUsed);
    if(NumSym == -1)//, LineNum, Offset);
        return -1;
    
    return ReadProg2(TokStr, MemMap, Errs, ModList[ModNum], UseSym, SymbolTable, Warnings, ModuleWarnings, ModNum);
}


//Run 2 end


int main(int argc, const char * argv[]) {
    
    //string filename = "~/Documents/OS/labsamples/input-";
    //string filename = argv[1];
    
    ifstream file;

    //cout<<argv[0]<<" \n Ahama "<<argv[1]<<"\n";

    // "/home/pa1230/Documents/OS/labsamples/input-1"
    file.open(argv[1]);
    string line, buffer = "";
    int FileLength = 0; int LastOff = 0;            //Stored For expectation at end of file
    while( getline(file, line ))
    {
        buffer = buffer + line + "\n";
        FileLength ++;
        LastOff = line.length();
    }
    
    string input = buffer;          //"Hello 123    popo 344, ... \n aa 12\n23";
    
    
    vector<int> Line_Num;
    vector<int> offset;
    vector<string> token_input = Tokenizer1(buffer, Line_Num , offset);
    
    Line_Num.push_back(FileLength);
    offset.push_back(LastOff + 1);
    
    Curr_Tok = 0;
    
    vector<int> MemoryMap;
    //Run 1
    
    int Curr_Instr = 0;
    vector<int> ModuleList;
    int Incr;
    map<string, int> SymbolTable;
    map<string, string> SymTableError;
    string ErrorMsg="";
    while(Curr_Tok<token_input.size())
    {
        ModuleList.push_back(Curr_Instr);
        Incr = ReadModule1(token_input, MemoryMap, ModuleList, Line_Num, offset, SymbolTable, SymTableError, ErrorMsg);
        if(Incr== -1)
        {
            cout<<ErrorMsg;
            return 0;
        }
        Curr_Instr += Incr;
    }
    ModuleList.push_back(Curr_Instr);
    
    vector<string> Errors (Curr_Instr,"");         //stores errors with each instr number
    vector<string> Warnings ;                      //stores warnings with each instr number
    vector<string> ModuleWarnings(ModuleList.size(),"");    //Will append to these
    map<string, bool> SymRead;
    //Run 2
    vector<string> token_input2 = Tokenizer2(buffer);
    Curr_Instr = 0;
    Curr_Tok = 0;
    map<string, bool> SymTableUsed;
    // For Rule 4
    for (map<string,int>::iterator it=SymbolTable.begin(); it!=SymbolTable.end(); ++it)
    {
        SymTableUsed[it->first] = false;
    }
    int ModNum = 0;
    while(Curr_Tok<token_input2.size())
    {
        
        Curr_Instr += ReadModule2(token_input2, MemoryMap, ModuleList, Errors, SymbolTable, Warnings, ModuleWarnings, SymTableUsed, ModNum, SymRead);
        ModNum++;
    }
    cout<<"Symbol Table\n";
    
    for (map<string,int>::iterator it=SymbolTable.begin(); it!=SymbolTable.end(); ++it)
    {
        cout<<it->first<<"="<<it->second;
        if(SymTableError.find(it->first)!=SymTableError.end())
            cout<<SymTableError[it->first];
        cout<<"\n";
    }
    cout<<endl<<"Memory Map\n";
    int Modnumber = 0;
    int i(0);
    for(;i<MemoryMap.size();i++)
    {
        if(i == ModuleList[Modnumber + 1])
        {
            cout<<ModuleWarnings[Modnumber];
            Modnumber++;
        }
        int Addr(MemoryMap[i]);
        cout<<setw(3) <<setfill('0') << i<<": "<<setw(4)<<Addr<<Errors[i]<<"\n";      //Check -> Correct i to make it 3 digits
    }
    //check
    if(i == ModuleList[Modnumber + 1])
    {
        cout<<ModuleWarnings[Modnumber];
        Modnumber++;
    }
    cout<<endl;
    
    //Rule 4
    for (map<string,bool>::iterator it=SymTableUsed.begin(); it!=SymTableUsed.end(); ++it)
    {
        if(it->second == false)
        {
            
            //Check
            int instr = SymbolTable[it->first];   //to get instruction and figure out here itself
            int i =1;
            // cout<<it->first<<" "<<instr<<"\n";       Delete this check
            // cout<<ModuleList[0]<<" ";                Delete this check
            while(i < ModuleList.size() && instr >= ModuleList[i])
            {
                // Delete this check cout<<ModuleList[i]<<" ";
                i++;
            }
            cout<<"Warning: Module "<<i<<": "<<it->first <<" was defined but never used\n";
            //Check
            
        }
    }
    
    file.close();

    
    
    //Check issue with printing instruction number
    //Check issue with printing if add has < 4 digits
    return 0;
}
