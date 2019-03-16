#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <cctype>
#include <stdlib.h>
#include <stdlib.h>
#include <iomanip>
#include <vector>
#include <map> 
#include "pin.H"
#include <list>


struct mute
{
	ADDRINT madd;
	int mflag;
};

struct memregion
{
	THREADID tid; //ThreadID of the thread accessing the variable for read or write
	char memop; //'R' for read or 'W' for write
	struct mute mute1;
	struct mute sem;
	struct mute rw;
	ADDRINT bar;
	int mlock;
	int slock;
	int wlock;
	char occurrence; //variable access is 'after' or 'before' barrier instruction
};

struct gl
{
	THREADID gt;
	ADDRINT ga;
	char op;
};

std::map<ADDRINT, std::list<struct memregion> > accessmap; 
std::map<ADDRINT, std::list<struct memregion> >::iterator it;
std::map<ADDRINT, std::list<struct memregion> >::iterator it2;

std::map<THREADID, ADDRINT> barriermap;
std::map<THREADID, ADDRINT>::iterator bit;
std::map<THREADID, ADDRINT>::iterator bit2;

VOID Image(IMG, VOID *);
VOID Instruction(INS, VOID *);
VOID Routine(RTN, VOID *);
VOID BarrierDetect(char *, THREADID, ADDRINT);
VOID RecordMemRead(VOID *, VOID *,CONTEXT *, THREADID, ADDRINT *);
VOID RecordMemWrite(VOID *, VOID *, CONTEXT *, THREADID, ADDRINT *);
VOID RecordLockRoutine(char*, THREADID, ADDRINT );
VOID RecordUnLockRoutine( char *, THREADID, ADDRINT );
VOID sRecordLockRoutine(char*, THREADID, ADDRINT );
VOID sRecordUnLockRoutine( char *, THREADID, ADDRINT );
VOID wRecordLockRoutine(char*, THREADID, ADDRINT );
VOID wRecordUnLockRoutine( char *, THREADID, ADDRINT );
VOID ThreadStart(THREADID, CONTEXT *, INT32, VOID *);
VOID ThreadFini(THREADID, const CONTEXT *, INT32, VOID *);
VOID Fini(INT32, VOID *);
INT32 Usage();
//VOID LockIt(VOID * addr, CONTEXT *ctxt, THREADID id);
//VOID UnlockIt(VOID * addr, CONTEXT *ctxt, THREADID id);
//static std::string TrimWhitespace(const std::string &);
const char * StripPath(const char *);
unsigned found_m1,found_m2,found_s1,found_s2,found_r1,found_w1,found_w2,found_b;
	
PIN_LOCK lock;
ofstream outFile, out;
ADDRINT lowImg,highImg,lowSec,highSec;
static REG RegSkipNextR,RegSkipNextW;
vector<int> thread_lock_flag;
vector<struct gl> globe;
static bool EnableInstructionCount = true;

std::ofstream* outf = 0;

void print()
{

*outf << "\n\t\t\t\t\t\t\tCURRENT MAP:";
for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
	
  		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
			{
//cout <<"IN2"<<endl;
if (tit->tid) 
{

*outf << "\n\t\t\t\t\t\t\tThread" << tit->tid<< "   Read/write->:" <<tit->memop ;
if((tit->mlock==1) )*outf<< "\n\t\t\t\t\t\t\t\t\t\t Lock->:Mutex"<<"\t Lock Address:"<<tit->mute1.madd<< "\tFlag" <<tit->mute1.mflag<<endl;
if((tit->slock==1) )*outf<< "\n\t\t\t\t\t\t\t\t\t\t Lock->:Semaphore"<<"\t Lock Address:"<<tit->sem.madd<< "\tFlag" <<tit->sem.mflag<<endl;
if((tit->wlock==1) )*outf<< "\n\t\t\t\t\t\t\t\t\t\t Lock->:Read_Write"<<"\t Lock Address:"<<tit->rw.madd<< "\tFlag" <<tit->rw.mflag<<endl;
if((tit->bar) )*outf<< "\n\t\t\t\t\t\t\t\t\t\t Barrier->"<<"\t Address:"<<tit->bar;
if((tit->occurrence=='B')) *outf<< "\t Occurrence:"<<tit->occurrence<<endl;
*outf<<"\n";

}
			}
	}

}


void plock(THREADID a)
{


for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
	
  		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
			{

if (tit->tid==a) 
{


if((tit->mlock==1) )outFile<< "\n\t\tWith Lock->:Mutex"<<"\t Lock Address:"<<tit->mute1.madd<< "\tFlag" <<tit->mute1.mflag<<endl;
if((tit->slock==1) )outFile<< "\n\t\tWaits on->:Semaphore"<<"\t Lock Address:"<<tit->sem.madd<< "\tFlag" <<tit->sem.mflag<<endl;
if((tit->wlock==1) )outFile<< "\n\t\tWith Lock->:Read_Write"<<"\t Lock Address:"<<tit->rw.madd<< "\tFlag" <<tit->rw.mflag<<endl;
if((tit->bar) )outFile<< "\n\t\tWaits on->:Barrier"<<"\t  Address:"<<tit->bar<<endl;
if((tit->occurrence=='B')) outFile<< "\t Occurrence:"<<tit->occurrence<<endl;
outFile<<"\n";


}
			}
	}

}

static std::string TrimWhitespace(const std::string &inLine)
{
    std::string outLine = inLine;

    bool skipNextSpace = true;
    for (std::string::iterator it = outLine.begin();  it != outLine.end();  ++it)
    {
        if (iswspace(*it))
        {
            if (skipNextSpace)
            {
                it = outLine.erase(it);
                if (it == outLine.end())
                    break;
            }
            else
            {
                *it = ' ';
                skipNextSpace = true;
            }
        }
        else
        {
            skipNextSpace = false;
        }
    }
    if (!outLine.empty())
    {
        std::string::reverse_iterator it = outLine.rbegin();
        if (iswspace(*it))
            outLine.erase(outLine.size()-1);
    }
    return outLine;
}


static BOOL DebugInterpreter(THREADID tid, CONTEXT *ctxt, const string &cmd, string *result, VOID *)
{
    std::string line = TrimWhitespace(cmd);
    *result = "";

    if (line == "help")
    {

       

        result->append("Datarace on -- start datarace detection in a program.\n");
        result->append("Datarace off -- stop datarace detection in a program.\n");
		
		return TRUE;
    }
   


    else if (line == "Datarace off")
    {
        if (EnableInstructionCount)
        {
            PIN_RemoveInstrumentation();
           EnableInstructionCount = false;
            *result ="Datarace detecion disabled.\n";
        }
		else
		{
		    *result = "Datarace detection is already disabled.\n";	
		}
        return TRUE;
    }

  else if (line == "Datarace on")
    {
        if (!EnableInstructionCount)
        {
            PIN_RemoveInstrumentation();
           EnableInstructionCount = true;
            *result ="Datarace detecion enabled.\n";
        }
		else
		{
		    *result = "Datarace detection is already enabled.\n";	
		}
        return TRUE;
    }

    
    return FALSE;   /* Unknown command */
}


void Image(IMG img, void * v)
{
	if(IMG_IsMainExecutable(img))
	{
	lowImg = IMG_LowAddress(img);
	highImg = IMG_HighAddress(img);
	//outFile<<IMG_Name(img)<< " Starting Address : "<<lowImg<<"  Ending Address : "<<highImg<<endl;
	for(SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
	{
		unsigned found = SEC_Name(sec).compare(".bss");
		if(found==0)
			{
				printf("Starting Address of .bss section: %lu\n",SEC_Address(sec));
				outFile << "Starting Address of .bss section:" << setw(8) << SEC_Address(sec) << " " << SEC_Name(sec) << endl;
				lowSec=SEC_Address(sec);
				highSec=IMG_HighAddress(img);
				outFile << "Ending Address of .bss section: " << highSec << '\n';
				printf("\nEnding Address of .bss section: %lu\n",highSec);
			}	
	}
	}
}

void insg(INS ins)
{
	for(UINT32 mem = 0; mem < INS_MemoryOperandCount(ins); mem++)
	{
		if(INS_MemoryOperandIsRead(ins,mem))
		{
			out<<"TID : "<<PIN_ThreadId()<<"    MEM_OP : READ :  "<<INS_Address(ins)<<endl;
			INS_InsertPredicatedCall(ins,
				IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, mem,
				IARG_CONTEXT,
				IARG_THREAD_ID,
				IARG_REG_REFERENCE, RegSkipNextR,
				IARG_END
				);
		}
		if (INS_MemoryOperandIsWritten(ins, mem))
		{
			out<<"TID : "<<PIN_ThreadId()<<"    MEM_OP : WRITE :  "<<INS_Address(ins)<<endl;
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, mem,
				IARG_CONTEXT,
				IARG_THREAD_ID,
				IARG_REG_REFERENCE, RegSkipNextW,
				IARG_END);
		}
	}
}

VOID Instruction(INS ins, VOID *v)
{
    
if(EnableInstructionCount)
{
void (*fun_ptr)(INS) = &insg; 

(*fun_ptr)(ins);
}
}

void Routine(RTN rtn, void * v)
{
	//unsigned found_m1,found_m2,found_s1,found_s2,found_r1,found_w1,found_w2;
	
	found_m1=RTN_Name(rtn).compare("pthread_mutex_lock");
	if(found_m1==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		

		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RecordLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		
		RTN_Close(rtn);
	}

	found_m2=RTN_Name(rtn).compare("pthread_mutex_unlock");
	if(found_m2==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RecordUnLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
	}

found_s1=RTN_Name(rtn).compare("sem_wait");
	if(found_s1==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		

		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)sRecordLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		
		RTN_Close(rtn);
	}

	found_s2=RTN_Name(rtn).compare("sem_post");
	if(found_s2==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)sRecordUnLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
	}

found_r1=RTN_Name(rtn).compare("__pthread_rwlock_rdlock");
found_w1=RTN_Name(rtn).compare("pthread_rwlock_wrlock");
	if(found_r1==0 || found_w1==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		

		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)wRecordLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		
		RTN_Close(rtn);
	}

	found_w2=RTN_Name(rtn).compare("__pthread_rwlock_unlock");
	if(found_w2==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)wRecordUnLockRoutine,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
	}

	found_b=RTN_Name(rtn).compare("pthread_barrier_wait");
	if(found_b==0)
	{	
		//outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)BarrierDetect,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
	}
}

const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}


VOID RecordLockRoutine(char * rtnName, THREADID id, ADDRINT  addr )
{
   	for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->mute1.madd = addr;
				tit->mute1.mflag = 1;
				tit->mlock=1;
				//outFile << "Thread " << tit->tid << " lock() called, Flag: 1 "<< "ADDR : "<< addr  << endl;
if (tit->tid)
{ 
*outf << "\n\nThread " << tit->tid << " mutex_lock() called, Flag: "<< tit->mute1.mflag << "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " locks mutex()  ADDRESS : "<< addr << endl;

break;
}
			}
		}
	}
    
}

VOID RecordUnLockRoutine(char * rtnName, THREADID id, ADDRINT  addr)
{
    for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->mute1.madd = addr;
				tit->mute1.mflag = 0;
				tit->mlock=0;
				//outFile << "Thread " << tit->tid << " unlock() called, Flag: 0 "<< "ADDR : "<< addr << endl;
if (tit->tid) 
{
*outf << "\n\nThread " << tit->tid << " mutex_unlock() called,Flag: "<< tit->mute1.mflag<< "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " unlocks mutex()  ADDRESS : "<< addr << endl;

break;
}
			}
		}
	}
}

VOID sRecordLockRoutine(char * rtnName, THREADID id, ADDRINT  addr )
{
   	for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->sem.madd = addr;
				tit->sem.mflag = 1;
				tit->slock=1;
				//outFile << "Thread " << tit->tid << " sem_wait() called, Flag: 1 "<< "ADDR : "<< addr  << endl;
if (tit->tid) 
{
*outf << "\n\nThread " << tit->tid << " sem_wait() called, Flag: "<< tit->sem.mflag<< "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " calls sem_wait()  ADDRESS : "<< addr << endl;

break;
}
			}
		}
	}
    
}

VOID sRecordUnLockRoutine(char * rtnName, THREADID id, ADDRINT  addr)
{
    for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->sem.madd = addr;
				tit->sem.mflag = 0;
				tit->slock=0;
				//outFile << "Thread " << tit->tid << " sem_post() called, Flag: 0 "<< "ADDR : "<< addr << endl;
if (tit->tid)
{
*outf << "\n\nThread " << tit->tid << " sem_post() called, Flag: "<< tit->sem.mflag<< "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " calls sem_post()  ADDRESS : "<< addr << endl;
break;
}
			}
		}
	}
}


VOID wRecordLockRoutine(char * rtnName, THREADID id, ADDRINT  addr )
{
   	for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->rw.madd = addr;
				tit->rw.mflag = 1;
				tit->wlock=1;
				//outFile << "Thread " << tit->tid << " rw_lock() called, Flag: 1 "<< "ADDR : "<< addr  << endl;
if (tit->tid)
{
*outf << "\n\nThread " << tit->tid << " rw_lock() called, Flag: "<< tit->rw.mflag<< "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " locks rw_lock()  ADDRESS : "<< addr << endl;
break;
}
			}
		}
	}
    
}

VOID wRecordUnLockRoutine(char * rtnName, THREADID id, ADDRINT  addr)
{
    for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->rw.madd = addr;
				tit->rw.mflag = 0;
				tit->wlock=0;
				//outFile << "Thread " << tit->tid << " rw_unlock() called, Flag: 0 "<< "ADDR : "<< addr << endl;
if (tit->tid)
{
*outf << "\n\nThread " << tit->tid << " rw_unlock() called, Flag: "<< tit->rw.mflag<< "ADDR : "<< addr << endl;
outFile << "\n\nThread " << tit->tid << " unlocks rw_lock()  ADDRESS : "<< addr << endl;
break;
}
			}
		}
	}
}

VOID BarrierDetect(char * rtnName, THREADID id, ADDRINT  addr )
{

 bit=barriermap.find(id);
              

                if(bit==barriermap.end())
                {
                       PIN_GetLock(&lock,id+1);
                        barriermap.insert(std::pair<THREADID,ADDRINT>(id, addr));
                        outFile << "\nBarrierMap Insert: " << addr << "     " << "  TID : " << id <<'\n';
                        PIN_ReleaseLock(&lock);
		}


   	for(it = accessmap.begin(); it!= accessmap.end(); it++)
	{
		std::list<struct memregion> ::iterator tit;
		for(tit = it->second.begin() ; tit != it->second.end() ; tit++)
		{
			if(tit->tid == id)
			{
				tit->bar=addr;
				tit->occurrence='B';
				//outFile << "Thread " << tit->tid << " lock() called, Flag: 1 "<< "ADDR : "<< addr  << endl;
if (tit->tid)
{ 
*outf << "\n\nThread " << tit->tid << " waits on barrier.  ADDR : "<< tit->bar << endl;
outFile << "\n\nThread " << tit->tid << " waits on barrier.  ADDR : "<< tit->bar << endl;
break;
}
			}
		}
	}
    
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    thread_lock_flag.push_back(0);
    outFile << "\nThread " << threadid << " Created" << endl;
}

// This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    outFile << "\nThread " << threadid << " Destroyed" << endl ;
}

VOID Fini(INT32 code, VOID *v)
{
	//outFile << "\nDatarace Access Map contains :\n" ;
*outf << "\n\n\n\nIn FINI:"<<endl;
for (unsigned int i=0; i< globe.size(); i++)
*outf << "   Global address:"<< globe[i].ga << "\tAccessing thread:"<<globe[i].gt <<"\t Operation"<<globe[i].op<<endl;
*outf << "\n\n\n\n"<<endl;
print();

}

INT32 Usage()
{
    outFile<<"This Pintool detects data race\n" <<endl;
    return -1;
}

VOID RecordMemRead(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID id,ADDRINT *regSkipNextR)
{
	struct memregion mr = {0, 0, 0,0,0,0,0,0,0, 0,0,0}; struct gl g={0,0};
	ADDRINT address=reinterpret_cast<ADDRINT>(addr);

	if (PIN_GetContextReg(ctxt, RegSkipNextR) == FALSE)
	{

		PIN_SetContextReg(ctxt, RegSkipNextR, TRUE);
		std::ostringstream os;
    	if(address <= lowSec && address >= highSec )
		{

if (id) {
*outf <<"\n Variable accessed for read at:" << address<< "\t Thread:" << id<<endl;
outFile << "\n\nThread "<<id<< " accesses variable at "<<address<< " for read";
plock(id);
print();
g.gt=id;
g.ga=address;
g.op='R';
globe.push_back(g);}
		it=accessmap.find(address);
		std::list<struct memregion> tlist;
			mr.tid = id;
			mr.memop = 'R';
			mr.mute1.madd = 0;
			mr.mute1.mflag =  FALSE;
			mr.sem.madd = 0;
			mr.sem.mflag =  FALSE;
			mr.rw.madd = 0;
			mr.rw.mflag =  FALSE;
			mr.occurrence = 'A';

bit=barriermap.find(id);
		if(bit!=barriermap.end()){
			mr.bar = bit->second;
			
		}


		if(it==accessmap.end())
		{
			//insert new memregion for accessed variable in accessmap
			
			tlist.push_back(mr);
			
			//PIN_GetLock(&lock,id+1);
            accessmap.insert( std::pair<ADDRINT,std::list<struct memregion> >(address,tlist));
           // outFile << "AccessMap Insert: " << address << "  TID: " << mr.tid << "  MemOp: " << mr.memop << "  Occurrence: " << mr.occurrence <<"  IS LOCK" << mr.mute1.mflag<<"   Mutex Address" << mr.mute1.madd<<'\n';
            PIN_ReleaseLock(&lock);
		}
		else
		{
			std::list<struct memregion> ::iterator tit;
			int thread_match = 0, memread_exist = 0;
			
			for(tit = it->second.begin();tit != it->second.end(); ++tit)
			{
				if(tit->tid == id)
				{
					thread_match=1; //memory address already accessed by same thread
					if(tit->memop == 'R')
					{
						memread_exist=1; //access by same thread for read operation
						break;
					}
                                }
			}

			if(memread_exist==0)
			    it->second.push_back(mr);

		

			if(thread_match==0 && memread_exist==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->mute1.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;
							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;

							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
				PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}



			if(thread_match==0 && memread_exist==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->sem.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;
							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;

							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
				PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}


			if(thread_match==0 && memread_exist==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->rw.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\n\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;
							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;

							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
				PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}


		}
	}
	*regSkipNextR = FALSE;

	}
}

VOID RecordMemWrite(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID id,ADDRINT *regSkipNextW)
{

	struct memregion mr = {0, 0, 0,0,0,0,0,0,0,0, 0, 0};  struct gl g={0,0};
	ADDRINT address=reinterpret_cast<ADDRINT>(addr);


	if (PIN_GetContextReg(ctxt, RegSkipNextW) == FALSE)
	{

		PIN_SetContextReg(ctxt, RegSkipNextW, TRUE);
		std::ostringstream os;
    	if(address <= lowSec && address >= highSec )
		{
if (id) {
*outf <<"\n Variable accessed for write at:" << address<< "\t Thread:" << id<<endl;
outFile << "\n\nThread "<<id<< " accesses variable at "<<address<< " for write";
 plock(id);
print();
g.gt=id;
g.ga=address;
g.op='W';
globe.push_back(g);}
		it=accessmap.find(address);
		std::list<struct memregion> tlist;
			mr.tid = id;
			mr.memop = 'W';
			mr.mute1.madd = 0;
			mr.mute1.mflag =  FALSE;
			mr.sem.madd = 0;
			mr.sem.mflag =  FALSE;
			mr.rw.madd = 0;
			mr.rw.mflag =  FALSE;
			mr.occurrence = 'A';	

bit=barriermap.find(id);
		if(bit!=barriermap.end()){
			mr.bar = bit->second;
			
		}

	

		if(it==accessmap.end())

		{
			//insert new memregion for accessed variable in accessmap
			
			tlist.push_back(mr);
			
			PIN_GetLock(&lock,id+1);
            accessmap.insert( std::pair<ADDRINT,std::list<struct memregion> >(address,tlist));
           // outFile << "AccessMap Insert: " << address << "  TID: " << mr.tid << "  MemOp: " << mr.memop << "  Occurrence: " << mr.occurrence <<"  IS LOCK" << mr.mute1.mflag<<"  Mutex Address" << mr.mute1.madd<<'\n';
            PIN_ReleaseLock(&lock);
		}
		else
		{
			std::list<struct memregion> ::iterator tit;
			int thread_match = 0, memwrite_exist = 0;
			
			for(tit = it->second.begin();tit != it->second.end(); ++tit)
			{
				if(tit->tid == id)
				{
					thread_match=1; //memory address already accessed by same thread
					if(tit->memop == 'W')
					{
						memwrite_exist=1; //access by same thread for write operation
						break;
					}
                                 }
			}
			if(memwrite_exist==1)
			    it->second.push_back(mr);

		

			if(memwrite_exist==1&& thread_match==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->mute1.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\n\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;

							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
 					PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}

			if(memwrite_exist==1&& thread_match==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->sem.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;

							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
 					PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}

			if(memwrite_exist==1&& thread_match==0)
			{
				for(tit = it->second.begin();tit != it->second.end(); ++tit)
				{
					if(tit->tid!=id)
					{
						if(tit->rw.mflag==0&& (tit)->occurrence=='A')
						{
							outFile<<"\nDatarace is possible at "<< address <<" The threads are "<<tit->tid<<" and "<<id<<endl;

							cout<<"Datarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
							*outf<<"\nDatarace is possible at variable of address:"<< address <<". The threads are "<<tit->tid<<" and "<<id<<endl;
						}
 					PIN_ApplicationBreakpoint(ctxt, id, FALSE, os.str());
					}
				}
			}
	}	
	*regSkipNextW = FALSE;
	}
}
}

int main(int argc, char *argv[])
{
	PIN_InitSymbols();

	outFile.open(StripPath(argv[argc-1]));
	out.open("Thread.out");
	
outf = new std::ofstream("maprint.out");


    if (PIN_Init(argc, argv)) return Usage();
   
	
    PIN_InitLock(&lock);
    
    RegSkipNextR = PIN_ClaimToolRegister();
    if (!REG_valid(RegSkipNextR)){
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
    }
	
    RegSkipNextW = PIN_ClaimToolRegister();
    if (!REG_valid(RegSkipNextW)){
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
    }

    PIN_AddDebugInterpreter(DebugInterpreter, 0);

    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(Routine, 0);
	
    // Register Analysis routines to be called when a thread begins/ends
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
   
    return 0;
}
