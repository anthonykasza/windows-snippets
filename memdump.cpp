
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <map>

#include "pin.H"

#ifdef _WIN32
    #include <direct.h> // _mkdir windows
    namespace W{
        #include <windows.h>
    }
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif


KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memdump.log", "specify file name");
ofstream TraceFile;
std::map<VOID *, VOID *> writtenMap;


VOID RecordMemWrite(VOID * ip, VOID * addr) 
{
    map<VOID *, VOID *>::iterator i;

    for(i = writtenMap.begin(); i != writtenMap.end(); ++i) 
    {
        if(addr >= i->first && addr < i->second) 
        {
            return;
        }
    }

    W::MEMORY_BASIC_INFORMATION info;
    // VirtualQuery returns the size of its results, 0 means failure
    if (W::VirtualQuery(addr, &info, sizeof(info)) == 0)
    {
        TraceFile <<  "VirutalQuery failed :(" << endl;
        return;
    }

    writtenMap[info.BaseAddress] = ((UINT8 *)info.BaseAddress) + info.RegionSize;
    return;
}

VOID checkBBL(ADDRINT addr)
{
    //TraceFile <<  "checking BBL with first instruction at: " << addr << endl;

    map<VOID *, VOID *>::iterator i;
    FILE *memdump;
    char fname[30];

    for(i=writtenMap.begin(); i != writtenMap.end(); ++i)
    {
        if(addr >= (ADDRINT)i->first && addr < (ADDRINT)i->second) 
        {      
            sprintf(fname, "dumps\\%p.dump", i->first);
            memdump = fopen(fname, "wb");
            fwrite(i->first, sizeof(char), (size_t)((ADDRINT)i->second - (ADDRINT)i->first), memdump);
            fclose(memdump);

            writtenMap.erase(i->first);
            break;
        }
    }
    return;
}

VOID Instruction(INS ins, VOID *v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_END);
        }
    }
}

void Trace(TRACE trace, void *v) 
{
    // for each bbl in the application trace...
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) 
    {
        // args:    bbl=bbl, IPOINT_BEFORE=insert before bbl, checkBBL=ptr to function to insert, 
        //          IARG_PTR=function argument type, BBL_Address(bbl)=function argument, IARG_END=end of arguement marker
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)checkBBL, IARG_PTR, BBL_Address(bbl), IARG_END);
    }
} 

VOID Fini(INT32 code, VOID *v)
{
    TraceFile <<  "shutting down!" << endl;
    if (TraceFile.is_open()) 
    { 
        TraceFile.close(); 
    }
}

INT32 Usage()
{
    PIN_ERROR( "This Pintool writes memory blocks executed from to disk\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    TraceFile.open(KnobOutputFile.Value().c_str());

    TraceFile <<  "starting up!" << endl;

    // instruction level hooks
    INS_AddInstrumentFunction(Instruction, 0);

    // application trace level hooks
    TRACE_AddInstrumentFunction(Trace, 0); 

    // application exit hooks
    PIN_AddFiniFunction(Fini, 0);

    // start application trace
    PIN_StartProgram();
    
    return 0;
}

