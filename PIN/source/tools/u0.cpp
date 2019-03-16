
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <cctype>
#include <stdlib.h>
#include <stdlib.h>
#include <iomanip>
#include <map> 
#include "pin.H"
#include <list>
#define limit 5
#define NTHREADS 10

FILE * trace;
// Command line switches for this tool.

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "Model_trace.out", "specify output file name");
    
    
    
//KNOB<string> KnobAddrOutputFile(KNOB_MODE_WRITEONCE, "pintool",
  //  "o", "All_address_routines.out", "specify output file name");

KNOB<string> KnobExecutionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ExecFile","Execution_trace.out","specify output file name");

double dataarray[NTHREADS][9];

//memset(dataarray, -1, sizeof(dataarray[8][NTHREADS]) * row * count);

KNOB<string> KnobExceptionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ExceptionFile","Exception_trace.out","specify output file name");
    
    
KNOB<string> KnobTrainFile(KNOB_MODE_WRITEONCE, "pintool",
    "TrainFile","Train.csv","specify output file name");
KNOB<string> KnobModelExecutionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelExecFile","ModelExecution_trace.dot","specify output file name");

KNOB<string> KnobModelMapOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelMapFile","ModelMap_trace.dot","specify output file name");


KNOB<string> KnobDataOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "DataFile","Data_trace.out","specify output file name");


KNOB<string> KnobMutexOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "MutexFile","Mutex_trace.out","specify output file name");


KNOB<string> KnobModelMutexOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelMutexFile","ModelMutex_trace.dot","specify output file name");

KNOB<string> KnobDataSeqOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "DataseqFile","DataSeq_trace.out","specify output file name");

KNOB<string> KnobModelDataSeqOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelDataSeqFile","ModelDataSeq_trace.dot","specify output file name");

KNOB<string> KnobThreadOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ThreadFile","Thread_trace.out","specify output file name");

KNOB<string> KnobModelThreadOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelThreadFile","ModelThread_trace.dot","specify output file name");

KNOB<string> KnobModelDataOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelDataFile","ModelData_trace.dot","specify output file name");

KNOB<string> KnobRoutineFile(KNOB_MODE_WRITEONCE, "pintool",
    "RoutineFile","RouIns_trace.out","specify output file name");
    
KNOB<string> KnobFeatureFile(KNOB_MODE_WRITEONCE, "pintool",
    "FeatureFile","Feature_output.out","specify output file name");


unsigned int secsize;
std::ofstream* outf5 = 0;
std::ofstream* outf4 = 0;
std::ofstream* outf3 = 0;
std::ofstream* outf2 = 0;
std::ofstream* outf1 = 0;

struct threadregion
{
	THREADID tid;
    int node_type;  // 0 for thread, 1 for global variable, 2 for mutexes, 3 for conditions, 11 for semaphore, 13 for rw_lock
	ADDRINT mutex_var;
	ADDRINT sem_var;
	ADDRINT rw_var;

    ADDRINT cond_var;
    ADDRINT global_var;
    char* name;// to specify routine
	string order;
    string cont;
    OS_THREAD_ID oid;
};

THREADID td=0;

struct globalregion
{
	THREADID tid;
  ADDRINT global_var;
	ADDRINT mutex_var;
	ADDRINT sem_var;
	ADDRINT rw_var;
     
	string type;
        string sync;
};

 
struct aglobalinregion
{

ADDRINT var;
THREADID tid[50];
int count;
};

struct bglobalinregion
{

ADDRINT var;
THREADID tid[50];
int count;
};

struct afbfglobalinregion
{

ADDRINT var;
THREADID tid[50];
string occ;
int count;
};
struct globalinregion
{
	

        THREADID tid;
       
	ADDRINT mutex_var;
	ADDRINT sem_var;
	ADDRINT rw_var;
     
	string type;
        string sync;
};

 struct cglobalinregion
{
 int scount;
 int count;
int bcount;
 THREADID tid;
};
struct countmutexregion
{
    THREADID tid;

	int count;
};
struct createregion
{
	THREADID tid;

	THREADID tid1;
    
    
};
struct signalregion
{
	THREADID tid;

	THREADID tid1;
        ADDRINT cond_var;
        ADDRINT mutex_var;
	ADDRINT sem_var;
	ADDRINT rw_var;
        string type;
    
    
};

struct exitregion
{
	THREADID tid;

	string type;
    
    
};

struct mutexregion
{
	THREADID tid;

	int flag;
       ADDRINT addr;
    
};


struct gmutexregion
{
	THREADID tid;

	string occ;
       ADDRINT addr;
    
};

/*struct signalregion
{
	THREADID tid;
  int node_type;  
	THREADID tid1[100];
        int count;
    
};

*/

struct startEnd
{
    THREADID thid;
    int start;
    int end;
    int tFlag;
};


struct join1
{
   
    double join;
    double exit;
   
};

std::list<struct startEnd> timeList;

ADDRINT lowaddr,highaddr;
int manyP;
int counter;
int creatcount=1;
int signalcount=1;
int exitcount=1;
int varcount=1;
int mutcount=1;
//int mutcount,datacount,sigcount;
//creatcount=0;
//mutcount=0;
//datacount=0;
//sigcount=0;
int dcount=0;
THREADID strthread;
THREADID mainthread;

int total_thread = 0;
double start_counter = 0.0;
double exit_counter = 0.0;
static bool EnableModelDetection=false;
static bool EnableExecutionDetection=false;
static bool EnableDataSeqDetection=false;
static bool EnableDataDetection=false;
static bool EnableThreadDetection=false;
static bool EnableMutexDetection=false;
//extern REG RegSkipNextR,RegSkipNextW;

#define PADSIZE 56
std::map<int,std::list<struct threadregion> > accessmap;
//std::map<int,std::list<struct mutexregion> > mutexmap;
//std::map<int,std::list<struct condregion> > condmap;
//std::map<int,std::list<struct globalregion> > globalmap;
//std::map<int,std::list<struct dataregion> > datamap;
//std::map<int,std::list<struct dataregionsync> > datasyncmap;
std::map<THREADID,std::list<THREADID> > createmap;
std::map<THREADID,std::list<THREADID> > signalmap;
std::map<THREADID,std::list<ADDRINT> > condmap;
std::list<THREADID> thlist;
std::list<ADDRINT> glist;
std::list<ADDRINT> mlist;
std::list<ADDRINT> blist;
std::map<ADDRINT,std::list<THREADID> > lmap;
std::map<ADDRINT,std::list<ADDRINT> > checkmap;
std::map<ADDRINT,std::list<struct afbfglobalinregion> > afbfbarrierglobalinmap;

std::map<THREADID,double> startmap;
//std::map<THREADID,int> exit1map;

std::map<THREADID,struct join1> joinmap;

//std::map<THREADID,double> ::iterator jq;
std::map<THREADID,double> ::iterator sq;
//std::map<THREADID,int> ::iterator eq;

std::map<THREADID,struct join1> :: iterator jq;
std::map<ADDRINT,std::list<struct cglobalinregion> > cglobalinmap;
std::map<int,std::list<struct globalregion> > globalmap;
std::map<int,std::list<struct exitregion> > exitmap;
std::map<ADDRINT,std::list<struct globalinregion> > globalinmap;
std::map<ADDRINT,std::list<struct aglobalinregion> > mutexglobalinmap;


std::map<ADDRINT,std::list<struct bglobalinregion> > barrierglobalinmap;
std::map<ADDRINT,std::list<struct mutexregion> > globalmutmap;

std::map<int,std::list<struct gmutexregion> > gglobalmutmap;
std::map<ADDRINT,std::list<struct countmutexregion> > countmap;
std::map<ADDRINT,std::list<THREADID> > globalsyncmap;

std::map<int,std::list<struct createregion> >createseqmap;
std::map<int,std::list<struct signalregion> >signalseqmap;
//std::map<int,std::list<struct signalregion> > signalmap;
//std::map<char*,std::list<struct memregion> >::iterator it;
std::list<THREADID>::iterator itth;
std::map<int , std::list<struct threadregion> >::iterator it2;
//std::map<int , std::list<struct mutexregion> >::iterator itm;
//std::map<int , std::list<struct condregion> >::iterator itco;

std::map<int,std::list<struct createregion> > ::iterator itcrsq;
std::map<THREADID,std::list<THREADID> > ::iterator itcr;
std::map<THREADID, std::list<ADDRINT> >::iterator tempit;

std::map<THREADID, std::list<ADDRINT> > imap;
std::map<THREADID,std::list<ADDRINT> > ::iterator itcc;
std::map<ADDRINT,std::list<ADDRINT> > ::iterator itck;
std::map<int,std::list<struct exitregion> >::iterator itex;

std::list<ADDRINT>::iterator itg;
std::list<ADDRINT>::iterator itm;
std::list<ADDRINT>::iterator itb;

std::map<ADDRINT,std::list<struct aglobalinregion> > :: iterator itmgrin;
std::map<ADDRINT,std::list<struct cglobalinregion> > :: iterator itcgrin;


std::map<ADDRINT,std::list<struct bglobalinregion> >:: iterator itbgrin;
std::map<ADDRINT,std::list<THREADID> > :: iterator itl;

std::map<ADDRINT,std::list<struct afbfglobalinregion> >::iterator itafbfgrin;

std::map<int,std::list<struct signalregion> > ::iterator itsrsq;
std::map<THREADID,std::list<THREADID> > ::iterator itsr;
std::map<int,std::list<struct globalregion> > :: iterator itgr;
std::map<ADDRINT,std::list<struct globalinregion> > :: iterator itgrin;

std::map<ADDRINT,std::list<struct mutexregion> > ::iterator itgrmu;
std::map<int,std::list<struct gmutexregion> > ::iterator itggrmu;
std::map<ADDRINT,std::list<struct countmutexregion> > ::iterator itgrcu;

//std::map<int,std::list<struct signalregion> > ::iterator its;


//std::map<THREADID, std::list<ADDRINT> > barriermap;
//std::map<THREADID, std::list<ADDRINT> >::iterator bit;
//std::map<THREADID, std::list<ADDRINT> >::iterator bit2;

//std::map<THREADID,OS_THREAD_ID> threadmap;
//std::map<THREADID,OS_THREAD_ID> ::iterator tmp;

ofstream OutFile;
ofstream ExecFile;
ofstream TrainFile;
ofstream ExceptionFile;
ofstream DataFile;
ofstream DataseqFile;
ofstream ThreadFile;
ofstream MutexFile;
ofstream ModelMapFile;
ofstream ModelExecFile;
ofstream ModelDataSeqFile;
ofstream ModelThreadFile;
ofstream ModelDataFile;
ofstream ModelMutexFile;
ofstream RoutineFile;
ofstream FeatureFile;
//ofstream AddrOutputFile;
PIN_LOCK lock;

INT32 numThreads = 0;

typedef struct RtnCount
{
    char * _name;
    string _image;
    ADDRINT _address;
    RTN _rtn;
    UINT64 _rtnCount;
        UINT64 _icount;
    struct RtnCount * _next;
} RTN_COUNT;

RTN_COUNT * RtnList = 0;

const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}
class thread_data_tsu
{
  public:
    thread_data_tsu() : _rtncount(0) {}
    UINT64 _rtncount;
    OS_THREAD_ID oid;
   
    char rtnlist[600];
   UINT64 _rdcount;
    UINT64 _wrcount;
    UINT8 _pad[PADSIZE];
   OS_THREAD_ID t;
    UINT64 _fcount;
   
};


static  TLS_KEY tls_key;

thread_data_tsu* get_tls(THREADID threadid)
{
    thread_data_tsu* tdata = 
          static_cast<thread_data_tsu*>(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}


static std::string TrimWhitespace(const std::string &inLine)
{
    std::string outLine = inLine;

    bool skipNextSpace = true;
    for (std::string::iterator it = outLine.begin();  it != outLine.end();  ++it)
    {
        if (std::isspace(*it))
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
        if (std::isspace(*it))
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
        result->append("Model Parameters -- Derive Model Parameters.\n");
 result->append("Complete Execution Sequence -- Derive Execution Snapshot.\n");
result->append("Data Access Sequence -- Derive Data Access Sequence.\n");
result->append("Thread Summary -- Derive Thread Summary with respect to data access and routine Access.\n");
result->append("Data Summary -- Derive Data Summary with respect to access by threads and type of access.\n");
result->append("Mutex Access Summary -- Derive Mutex Access Summary with respect to access by threads.\n");
     
		return TRUE;
    }
    else if (line == "Model Parameters")
    {
   
            EnableModelDetection = true;
            *result = "Model Parameters Derivation on.\n";
       
		
        return TRUE;
    }
    
     else if (line == "Complete Execution Sequence")
    {
   
            EnableExecutionDetection = true;
            *result = "Complete Execution Sequence Capture on.\n";
       
		
        return TRUE;
    }

     else if (line == "Data Access Sequence")
    {
   
            EnableDataSeqDetection = true;
            *result = "Data Access Sequence on .\n";
       
		
        return TRUE;
    }
    else if (line == "Data Summary")
    {
   
            EnableDataDetection = true;
            *result = "Data Summary on.\n";
       
		
        return TRUE;
    }
    


    else if (line == "Thread Summary")
    {
   
            EnableThreadDetection = true;
            *result = "Thread Summary on.\n";
       
		
        return TRUE;
    }

else if (line == "Mutex Access Summary")
    {
   
            EnableMutexDetection = true;
            *result = "Mutex Access Summary on.\n";
       
		
        return TRUE;
    }
    
    return FALSE;   /* Unknown command */
}

VOID start(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"STARTS");
start_counter = start_counter + 0.1;
startmap.insert(std::pair<THREADID,double> (tid, start_counter));



//char * name;


	PIN_GetLock(&lock, tid+1);
       // cout<<"Routine Name is " <<rtnName  ;
        //cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="AFTER";
          tr.cont="STARTS";
          tr.oid=PIN_GetPid();
     

	std::list<struct threadregion> tlist;
	tlist.push_back(tr);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}



VOID create(char* rtnName, THREADID tid, THREADID tid1)
 { 

//cout << "Inside CREATE";
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
tdata->_fcount=(tdata->_fcount)+1;
strcat(tdata->rtnlist,"  CREATES");


	PIN_GetLock(&lock, tid+1);
//PIN_GetLock(&lock, tid1+1);
      // cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';



         tr1.name=rtnName;
         tr1.tid=++td;
//cout << "The TD" << td;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
  tr1.oid=PIN_GetTid();
   tr1.cont="CREATES";
	  tr1.order="AFTER";


         tr.name=rtnName;

//cout << "The TID" << tid;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="CREATES";
  tr.oid=PIN_GetTid();
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
 //PIN_ReleaseLock(&lock);
}

VOID barrier_func(char* rtnName, THREADID tid,ADDRINT adr)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  PUTS MEMORY BAR");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=adr;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Puts Memory bar";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID join(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  JOINS");
//std::map<THREADID,<struct join1>> :: iterator jq;
//std::map<THREADID,<struct join1>> joinmap;


jq=joinmap.find(tid);

  if(jq!=joinmap.end())
  {
      jq->second.join++;
  }
  
  else
  {
  
  struct join1 j1;
  j1.join = 1; j1.exit=0.0;
     joinmap.insert(std::pair<THREADID, struct join1>(tid, j1) ); 
  }
  
 

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';



         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
  tr.oid=PIN_GetPid();
   tr.cont="JOINS";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

  

VOID lock_func_bef(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);

strcat(tdata->rtnlist,"  REQUESTS FOR LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Requests for locking the mutex variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Requests for locking the mutex variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID try_lock_func_bef(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);

strcat(tdata->rtnlist,"  REQUESTS FOR LOCK");

//char * name;


    PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
       tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Requests for try locking the mutex variable whose address is";
  tr1.oid=PIN_GetPid();
      tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
       tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Requests for try locking the mutex variable whose address is";
      tr.order="BEFORE";
  tr.oid=PIN_GetPid();
    std::list<struct threadregion> tlist;
    tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID lock_func_af(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  ACQUIRES LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Acquires the lock for mutex variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Acquires the lock for mutex variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}



VOID unlock_func(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNLOCKS THE MUTEX");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
   tr1.cont="Unlocks the mutex variable whose address is";
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Unlocks the mutex variable whose address is";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}



VOID slock_func_bef(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);

strcat(tdata->rtnlist,"  REQUESTS FOR LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
          tr1.sem_var=addr; 
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=11;
   tr1.cont="Requests for locking the semaphore variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Requests for locking the semaphore variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID slock_func_af(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  ACQUIRES LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.sem_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=11;
   tr1.cont="Acquires the lock for semaphore variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Acquires the lock for semaphore variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID writenew(char* rtnName, THREADID tid,ADDRINT addr)

{

//cout << "writenew";
//cout << "Adress" << addr;
//cout << " Threadid " << tid;

}

VOID sunlock_func(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNLOCKS THE SEMAPHORE");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
   tr1.cont="Unlocks the semaphore variable whose address is";
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=11;
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Unlocks the semaphore variable whose address is";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}



VOID wlock_func_bef(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);

strcat(tdata->rtnlist,"  REQUESTS FOR LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Requests for locking the rw_lock variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Requests for locking the rw_lock variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID wlock_func_af(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  ACQUIRES LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Acquires the lock for rw_lock variable whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Acquires the lock for rw_lock variable whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}



VOID wunlock_func(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNLOCKS THE RW_LOCK");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr; tr1.sem_var=addr; tr1.rw_var=addr;
   tr1.cont="Unlocks the rw_lock variable whose address is";
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Unlocks the rw_lock variable whose address is";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}





VOID main_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  MAIN");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Executes Main";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Executes Main";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}


VOID throw_func(char* rtnName, THREADID tid, ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  THROWS EXCEPTION");

//char * name;


	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=addr;
        tr1.cond_var=00000;
        tr1.node_type=2;
   tr1.cont="Throws Exception";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Throws Exception";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}


VOID cond_wait_func(char* rtnName, THREADID tid, ADDRINT adr1, ADDRINT adr2)
 { 
OutFile<<"Ho";
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
OutFile<<"COUNTER VALUE "<<counter1 <<'\n';

//char * name;

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAITS OVER CONDITION VARIABLE");
	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Waits over condition variable";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";

 
         tr.name=rtnName;
         tr.tid=0;
         tr.mutex_var=adr2; tr.sem_var=adr2; tr.rw_var=adr2;
	   tr.global_var=00;
        tr.cond_var=adr1;
        tr.node_type=3;
	  tr.order="AFTER";
  tr.oid=PIN_GetPid();
   tr.cont="Waits over condition variable";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 OutFile<<"COUNTER VALUE "<<counter1 <<'\n';
                       
 PIN_ReleaseLock(&lock);}



VOID malloc_func(char* rtnName, THREADID tid, ADDRINT adr1)
 { 

struct threadregion tr1;
int counter1;
counter1=counter;
counter++;


//char * name;

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  MALLOC");
	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=adr1;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="MALLOC";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";


	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID cond_signal_func(char* rtnName, THREADID tid, ADDRINT adr1)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;


//char * name;

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  SIGNALS");
	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Signals";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";




 
  for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{


 //cout<<"Event"<<it2->first;				
std::list<struct threadregion>::iterator lit;
				

for(lit=it2->second.begin(); lit!=it2->second.end(); ++lit)
{
//cout<<"1st loop"<<tid<<'\n';
    

   if(lit->cond_var==adr1)
{



//cout<<"cond signal loop "<<tid<<'\n';
//cout << lit->tid;
tr.tid=lit->tid;
tr.mutex_var=lit->mutex_var;
tr.rw_var=lit->rw_var;
tr.sem_var=lit->sem_var;
//lit->node_type=0;
//flag=1;

}
}			
}


         tr.name=rtnName;
         //tr.tid=0;
         //tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;0000;
	   tr.global_var=00000;
        tr.cond_var=adr1;
        tr.node_type=3;
	  tr.order="AFTER";
  tr.oid=PIN_GetPid();
   tr.cont="Signals";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}


VOID child_start(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
THREADID tid2;
int flag;
flag=0;
//char nme[15]="pthread_create";
//char ord[6]="AFTER";


//char * name;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  STARTS");
start_counter = start_counter + 0.1;
startmap.insert(std::pair<THREADID,double> (tid, start_counter));

	PIN_GetLock(&lock, tid+1);
       // cout<<"Routine Name is " <<rtnName  ;
       //cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tid2=PIN_GetParentTid();
         OutFile<<tid2<<" parent"<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=6;
  tr1.oid=PIN_GetPid();
   tr1.cont="STARTS";
	  tr1.order="BEFORE";


         tr.name=rtnName;
         tr.tid=tid;
   tr.cont="STARTS";
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=6;
  tr.oid=PIN_GetPid();
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
       tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       


 
  for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{


 //cout<<"Event"<<it2->first;				
std::list<struct threadregion>::iterator lit;
				

for(lit=it2->second.begin(); lit!=it2->second.end(); ++lit)
{
//cout<<"1st loop"<<tid<<'\n';
    

   if(lit->oid==PIN_GetParentTid() && lit->node_type==12 && flag==0)
{



//cout<<"2nd loop "<<tid<<'\n';
lit->tid=tid;
lit->node_type=0;
flag=1;

}
			
}
 PIN_ReleaseLock(&lock);

}	

   // fprintf(out, "process %d\n",PIN_GetParentTid());

   //fprintf(out, "process %d\n",PIN_GetPid());
  //  fflush(out);
    PIN_ReleaseLock(&lock);
}

VOID exit_func(char* rtnName, THREADID tid, int AGR)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
if(AGR==1)
{

strcat(tdata->rtnlist,"  EXITS DUE TO EXCEPTION");
 tr.cont="EXITS DUE TO EXCEPTION";
}

if(AGR==0)
{

strcat(tdata->rtnlist,"  EXITS NORMALLY");
 tr.cont="EXITS NORMALLY";
}


	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=9;
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID exit_handler_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  RUNS EXIT HANDLERS");

	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="RUNS EXIT HANDLERS";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}
VOID catch_exit(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  EXITS THE CATCH BLOCK");

	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Exits the Catch Block";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID catch_begin(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  CATCHES THE EXCEPTION");
	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Catches the Exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID unregister_fork(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNREGISTERS FORK HANLERS");

	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Unregisters fork handlers";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID exp_begin(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  DETECTS EXCEPTION");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Detects Exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID unwindraise_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Starts Unwinding Stack for finding the exception handler";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID pers_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Runs the personality functions for each routine";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID rethrow_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Rethrows the exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID currentexc_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="checks the current exception type, returns NULL if not handled";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}


VOID term_function(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Calls the default terminate function";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}


VOID set_GR(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Stores the arguments to be passed to next phase of exception hanlding which can be the cleanup phase or the last handler i.e landing pad";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID set_IP(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Sets the instruction pointer";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID kill(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Kills the Process";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID abort_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Aborts the Process which is the action of the default terminate function";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}

VOID urethrow(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Starts the search phase once more after exception is rethrown";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}
VOID exp_end(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  COMPLETES EXCEPTION HANDLING");

	PIN_GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Completed Exception handling";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 PIN_ReleaseLock(&lock);
}
VOID lock_wait_func(char* rtnName, THREADID tid)
 { 

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAITS FOR ACQUIRING LOCK");
struct threadregion tr;
int counter1;

counter1=counter;
counter++;



//char * name;


	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;

     //  cout<<'\n';
         
   


         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
   tr.cont="Waits for acquiring lock";
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;

	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}


VOID docount(VOID * ip)
{

//RoutineFile<< *counter << '\n';
//ADDRINT address=reinterpret_cast<ADDRINT>(ip);
//RoutineFile << ip << '\n';
//RoutineFile << "---------------" << endl;

}

//VOID printip(VOID *ip) { cout << ip; }

//VOID docount1(VOID * ip)
//{

//RoutineFile<< *counter << '\n';
//ADDRINT address=reinterpret_cast<ADDRINT>(ip);
//RoutineFile << ip << '\n';
//RoutineFile << "---------------" << endl;

//}

VOID unlock_wake_func(char* rtnName, THREADID tid)
 { 

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAKE UP WAITING THREADS");
//cout<<tdata->rtnlist<<'\n';
struct threadregion tr;
int counter1;

counter1=counter;
counter++;



//char * name;


	PIN_GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;

     //  cout<<'\n';
         
   


         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
   tr.cont="Wakes up waiting threads";
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;

	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

VOID Routine(RTN rtn, VOID *v)
{
   
  

 //RTN_COUNT * rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
   // rc->_name = const_cast<char*>(RTN_Name(rtn).c_str());
    //rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    //rc->_address = RTN_Address(rtn);
    //rc->_icount = 0;
    //rc->_rtnCount = 0;

    // Add to list of routines
    //rc->_next = RtnList;
   // RtnList = rc;
 
if (RTN_Name(rtn)== "myThreadFun")
{

         
    RTN_Open(rtn);
// cout << "Routine name" << RTN_Name(rtn) <<'\n';
 for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))

 //RoutineFile << "Routine name" << RTN_Name(rtn) <<'\n';
{
        // Insert a call to docount to increment the instruction counter for this rtn
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_INST_PTR, IARG_END);
    }
//(rc->_rtnCount) ++;
 RTN_Close(rtn);

}
  if (RTN_Name(rtn)== "_start")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)start, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "pthread_create")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)create ,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

  RTN_Close(rtn);
}



 if (RTN_Name(rtn)== "pthread_mutex_lock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)lock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__pthread_mutex_trylock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)try_lock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)lock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "pthread_mutex_unlock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "__write")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)writenew, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}

if (RTN_Name(rtn)== "sem_wait")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)slock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)slock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "sem-post")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)sunlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}



 if (RTN_Name(rtn)== "__pthread_rwlock_rdlock" )  
 
{
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}

if (  RTN_Name(rtn)== "pthread_rwlock_wrlock" )


{
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__pthread_rwlock_unlock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wunlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}



if (RTN_Name(rtn)== "12malloc")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)malloc_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_cond_wait")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_FUNCARG_ENTRYPOINT_VALUE, 1,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "pthread_join")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)join, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "main")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)main_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__lll_lock_wait")
{


  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);


     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__lll_unlock_wake")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unlock_wake_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "start_thread")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)child_start, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}
 /*if (RTN_Name(rtn)== "pthread_cond_wait")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_FUNCARG_ENTRYPOINT_VALUE, 1,IARG_END);

    RTN_Close(rtn);
}*/
if (RTN_Name(rtn)== "pthread_cond_signal")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_signal_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "exit")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exit_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}


if (RTN_Name(rtn)== "__cxa_throw")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)throw_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__run_exit_handlers")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exit_handler_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "_Unwind_RaiseException")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unwindraise_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "__cxa_rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)rethrow_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "_ZSt9terminatev")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)term_function, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)rethrow_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__gxx_personality_v0")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)pers_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}



if (RTN_Name(rtn)== "_Unwind_SetIP")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)set_IP, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "_Unwind_SetGR")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)set_GR, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_end_catch")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)catch_exit, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "__cxa_begin_catch")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)catch_begin, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_free_exception")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exp_end, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "gsignal")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)kill, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "abort")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)abort_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "_Unwind_Resume_or_Rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)urethrow, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_current_exception_type")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)currentexc_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__cxa_allocate_exception")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exp_begin, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "__unregister_atfork")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unregister_fork, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_barrier_wait")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)barrier_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}

}


VOID RecordMemWrite(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID tid)
{

//RoutineFile<< "IP" << ip;

ADDRINT address=reinterpret_cast<ADDRINT>(ip);
ADDRINT address1=reinterpret_cast<ADDRINT>(addr);
MutexFile << address << "IP" << "Address" << address1  << endl;
 if( address1 >= lowaddr && address1 <= (lowaddr + ((secsize)*sizeof(int))))
	{
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;

thread_data_tsu* tdata = get_tls(tid);
tdata->_wrcount=(tdata->_wrcount) + 1;
tdata->_rtncount=(tdata->_rtncount)+1;
//char * name;
cout << "WRITE" <<  address <<'\n';
RoutineFile <<"address" << address << "ip"<<ip <<'\n'<< endl;
//RoutineFile <<"address" << address << "ip"<<ip <<'\n'<< endl;


char  nme[8]=" WRITE";

   
	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=nme;
   tr1.cont="Writes to variable whose address is";
         tr1.tid=tid;
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
	  tr1.order="BEFORE";


        tr.name=nme;
         tr.tid=0;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=address;
        tr.cond_var=00000;
        tr.node_type=2;
   tr.cont="Writes to variable whose address is";
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

}

VOID RecordMemRead(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID tid)
{

ADDRINT address=reinterpret_cast<ADDRINT>(addr);
 if(address <= lowaddr && address >= highaddr)
	{
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rdcount=(tdata->_rdcount)+1;
tdata->_rtncount=(tdata->_rtncount)+1;
//cout << "READ" <<  address <<'\n';
char nme[7]= " READ";
        
    
	PIN_GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=nme;
         tr1.tid=tid;
   tr1.cont="Reads from variable whose address is";
         tr1.mutex_var=0; tr1.sem_var=0; tr1.rw_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
	  tr1.order="BEFORE";


         tr.name=nme;
         tr.tid=0;
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=address;
        tr.cond_var=00000;
        tr.node_type=2;
   tr.cont="Reads from variable whose address is";
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);
}

}

VOID Instruction(INS ins, VOID *v)
{
    //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)printip, IARG_INST_PTR, IARG_END);
MutexFile << "    " << setw(8) << hex  << INS_Address(ins) << " " << INS_Disassemble(ins)  << "  " << INS_MemoryOperandCount(ins)<< " "   <<	endl;
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
				IARG_CONST_CONTEXT,
				IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
				IARG_CONST_CONTEXT,
				IARG_THREAD_ID,
                IARG_END);
        }
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
//cout<<" inside thread start";    

PIN_GetLock(&lock, threadid+1);
//OS_THREAD_ID td;
//td=PIN_GetPid();
THREADID th;

    numThreads++;


  // accessmap.insert(std::pair<THREADID,OS_THREAD_ID>(threadid, td) );
    

  thread_data_tsu* tdata = new thread_data_tsu;
   tdata->_rtncount=0;

  
/*for(tmp=threadmap.begin(); tmp!=threadmap.end(); ++tmp)
{
   if (tmp->second==PIN_GetParentTid())
   {
 tdata->oid=tmp->first;
cout<<"DEKH"<<tmp->first;}
   // if (tmp->second==PIN_GetPid())
    //tdata->oid=tmp->first;
}*/

   
    tdata->oid=PIN_GetParentTid();
  tdata->_rdcount=0;
  tdata->_wrcount=0;
   tdata->_fcount=0;
   strcat(tdata->rtnlist," ");
  
  tdata->t=PIN_GetTid();
//cout<< "THREAD OS ID" <<tdata->t;
   tdata->_fcount=0;  
th=threadid;  


	thlist.push_back(th);

    PIN_SetThreadData(tls_key, tdata, threadid);


   PIN_ReleaseLock(&lock);
}

//This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{

struct threadregion tr;
int counter1;
counter1=counter;
counter++;

char nme[8]=" Exits";


jq=joinmap.find(threadid);

  if(jq!=joinmap.end())
  {
  exit_counter = exit_counter + 0.1;
      jq->second.exit = (exit_counter);
   }   
  
  else
  {
  
  struct join1 j1;
  exit_counter = exit_counter + 0.1;
  j1.join = 0; j1.exit=exit_counter;
     joinmap.insert(std::pair<THREADID, struct join1>(threadid, j1) ); 
     cout << " I GOT IN HERE" << threadid <<'\n';
  
  }
  
//char * name;
//exit1map.insert(std::pair<THREADID,int> (threadid, ++exit_counter));


	PIN_GetLock(&lock, threadid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   
        // cout<< nme;
         tr.name=nme;
         tr.tid=threadid;
  tr.oid=PIN_GetPid();
   tr.cont="EXITS";
         tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 PIN_ReleaseLock(&lock);

	

//    fflush(out);
    PIN_ReleaseLock(&lock);
}

VOID Image(IMG Img, VOID *v)
{

    
		unsigned found;
		if(IMG_IsMainExecutable(Img))
		{
			for (SEC sec = IMG_SecHead(Img); SEC_Valid(sec); sec = SEC_Next(sec))
			{ 
				
                                cout << SEC_Name(sec) <<'\n';
                                found=SEC_Name(sec).compare(".bss");
                                
                     
			
				if(found==0)
				{

                                   

					//printf("Address: %lu ",SEC_Address(sec));
					//std::cout << " =>Section: " << setw(8) << SEC_Address(sec) << " " << SEC_Name(sec) << endl;
					lowaddr=SEC_Address(sec);
                                        secsize = SEC_Size(sec);
                                        cout << SEC_Size(sec) <<"SIZE" <<'\n';
					//lowaddr=134519568;
					//lowaddr=134519257;	//global2	
					//lowaddr=IMG_LowAddress(Img);	
					//printf("%lu\n",lowaddr);
					std::cout << "Low addr: " << lowaddr << '\n';
					//OutFile  << "Low addr: " << lowaddr << '\n';
					highaddr=IMG_HighAddress(Img);
					std::cout << "High addr: " << highaddr << '\n';
					//OutFile << "High addr: " << highaddr << '\n';
					//printf("%lu\n",highaddr);
				

                                   }			
			

                           }


	
		}

for (SEC sec = IMG_SecHead(Img); SEC_Valid(sec); sec = SEC_Next(sec))
			{        
			
			ADDRINT low;
			         
			         MutexFile<< " SEC NAME " << SEC_Name(sec);
			         MutexFile<< " SEC ADRESS " << SEC_Address(sec) << '\n';
			         low=SEC_Address(sec);
                                        secsize = SEC_Size(sec); 
                                        for (UINT32 i =0; i < secsize; i+=4)
                                        MutexFile<<low+i << '\n';
                                         
                                        
                                        
                                        
                                        
                                        
                                                         }
			

/*for( RTN rtn= SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn) )

                                        {
                                            
RTN_Open(rtn);
for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        // Insert a call to docount to increment the instruction counter for this rtn
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount1, IARG_INST_PTR, IARG_END);
    }
         RTN_Close(rtn);                                    
                                         }
}*/
	
}

/*Auxillary function to detect whether there are multiple parent threads in a program*/
int many_parents()
{
    int count=0,first =1;
    THREADID thread_identity;
    for(it2=accessmap.begin();it2!=accessmap.end();it2++)
    {
        std::list<struct threadregion>::iterator lit;
        for(lit=it2->second.begin();lit!=it2->second.end();lit++)
        {
            if(lit->cont=="CREATES")
                {
                    if(first==1)
                    {
                        thread_identity = lit->tid;
                        first = 0;
                        count++;
                        cout<<"Count1 : "<<count<<endl;
                    }
                    else
                    {
                        if(thread_identity!=lit->tid)
                        {
                            count++;
                            cout<<"Count : "<<count<<endl;
                        }
                    }
                }
        }
    }
    if(count>1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*Auxillary function to detect whether the threads join in the end or not*/
int is_join()
{
    for(it2=accessmap.begin();it2!=accessmap.end();it2++)
    {
        std::list<struct threadregion>::iterator lit;
        for(lit=it2->second.begin();lit!=it2->second.end();lit++)
        {
            if(lit->cont=="JOINS")
            {
                return 1;
            }
        }
    }
    return 0;
}




double thread_join_counts(THREADID tid) {
double  join = -1;
//std::map<THREADID,int> :: iterator jq;
 for(jq=joinmap.begin(); jq!=joinmap.end(); jq++) {

 if (jq->first == tid)
     
    join= jq->second.join;
 }
  
 if(join > 0)
  return join;
  else
 return 0.15;

}


double thread_cond(THREADID tid) {
double  cond = 0;
//std::map<THREADID,int> :: iterator jq;
 for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)
{


 if (itcc->first == tid) {
     
    std::list<ADDRINT>::iterator cond_2nd_itr;
    for(cond_2nd_itr=itcc->second.begin(); cond_2nd_itr!=itcc->second.end(); ++cond_2nd_itr)
    cond++;
  
    }
}
  
 if(cond> 0)
  return cond;
  else
 return 0.15;




}



double count_globals()

{
double  global=0;

for(itg=glist.begin();itg!=glist.end();++itg)
global++;

if(global> 0)
  return global;
  else
 return 0.15;


}


double thread_create_feature(THREADID tid)
{


double total_count =0, indi_count  = 0;
for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
{

   total_count++;

   if (itcr->first == tid)
   {
               //cout<< "PARENT COUNT"<< itcr->first <<'\n';
       std::list<THREADID>::iterator create_2nd_itr;
               
       for(create_2nd_itr=itcr->second.begin(); create_2nd_itr!=itcr->second.end(); ++create_2nd_itr)
       {
           indi_count ++;
        }

}



}

 // cout << "TID" << tid << "TOTAL"<< indi_count + start_count;

 if(indi_count ==0)
return 0.15;
else
return (indi_count);
  
  //return (start_count);
   //if (indi_count > 0 && total_count > 0)

   //{
   
    
      
      
   // }
  //if(indi_count == 0)
     // return start_count;
   //else return 0;

//return 1;
}



double thread_create()
{


double total_count =0;
for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
{

  std::list<THREADID>::iterator create_2nd_itr;
               
       for(create_2nd_itr=itcr->second.begin(); create_2nd_itr!=itcr->second.end(); ++create_2nd_itr)
       {
           total_count ++;
        }

   

}



cout << "TOTAL COUNT" << total_count;

 // cout << "TID" << tid << "TOTAL"<< indi_count + start_count;

 if(total_count ==0)
return 0.35;
else
return (total_count);
  
  //return (start_count);
   //if (indi_count > 0 && total_count > 0)

   //{
   
    
      
      
   // }
  //if(indi_count == 0)
     // return start_count;
   //else return 0;

//return 1;
}



double thread_signal_feature(THREADID tid)
{

double total_count =0, indi_count  = 0;
for(itsr = signalmap.begin(); itsr!= signalmap.end(); ++itsr)
{

   total_count++;

   if (itsr->first == tid)
   {
              
       std::list<THREADID>::iterator signal_2nd_itr;
               
       for(signal_2nd_itr=itsr->second.begin(); signal_2nd_itr!=itsr->second.end(); ++signal_2nd_itr)
         {
           indi_count ++;
        }

}



}


 if(indi_count ==0)
return 0.15;
else
return (indi_count);
}



int total_thread_counter()
{



for(itth=thlist.begin();itth!=thlist.end();++itth)
{

   total_thread++;
   }
 return  total_thread;


}
   
double total_start_time(THREADID tid)
{

double start_count = 0;

for(sq = startmap.begin(); sq!= startmap.end(); ++sq)
{

    if (sq->first == tid)
    start_count = sq->second;
}

if(start_count ==0)
return 0.15;
else
return (start_count);



}


double total_exit_time(THREADID tid)
{

double join = 0;
//std::map<THREADID,int> :: iterator jq;
 for(jq=joinmap.begin(); jq!=joinmap.end(); jq++) {

 if (jq->first == tid)
     
    join = jq->second.exit;
 }
  
 if(join==0)
  return 0.15;
  else
 return join;



}






/*Auxillary function to detect whether there are any updates of global variables by the threads*/
int does_update()
{
    //int count=0,first =1;
    //THREADID thread_identity;

    
    std::list<THREADID>parentList;
    for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
    {

      
            
           
                   parentList.push_back(itcr->first);
              // cout<< "Parents " <<  itcr->first << '\n';
              
       
    }
    /*std::list<THREADID>::iterator plit;
    for(plit=parentList.begin();plit!=parentList.end();plit++)
    {
        cout<<"pid : "<<*plit;
    }*/
    for(it2=accessmap.begin();it2!=accessmap.end();it2++)
    {
        std::list<struct threadregion>::iterator lit;
        for(lit=it2->second.begin();lit!=it2->second.end();lit++)
        {
            if(lit->cont=="Writes to variable whose address is")
                {
                   cout << lit->global_var << "global_var"<<'\n';
                   cout << lit->tid << "tid"<<'\n';


                   std::list<THREADID>::iterator itpl;
                   itpl = find (parentList.begin(), parentList.end(), lit->tid);
                 // cout<<"Parent thread : "<<lit->tid;
                    if(itpl!=parentList.end())
                     {
                         
                           
                        /*std::map<ADDRINT,std::list<struct globalinregion> >::iterator tempit;
                        //tempit = globalinmap.find(lit->global_var);
                        cout<<"I am here"<<endl;
                        //if(tempit!=globalinmap.end())
                            //return 1;
                        for(tempit=globalinmap.begin();tempit!=globalinmap.end();tempit++)
                        {
                            if(tempit->first==lit->global_var)
                                return 1;
                        }*/
                        for(itg=glist.begin();itg!=glist.end();++itg)
                        { 
                            for(itgrin = globalinmap.begin(); itgrin!= globalinmap.end(); ++itgrin)
                              {


                                if(*itg==itgrin->first)
                                {
                                
                                    std::list<struct globalinregion>::iterator globalin_2nd_itr;
                                    for(globalin_2nd_itr=itgrin->second.begin(); globalin_2nd_itr!=itgrin->second.end(); ++globalin_2nd_itr)
                                   {
                                  
                                    if(*itpl==globalin_2nd_itr->tid) {
                                                                       
                                                                       
                                                                        
                                        return 1;}
                                        }
                                }
                            }
                        }   
                   }
                }
        }
    }
    return 0;
}





double global_updates_thread(THREADID tid)
{
    //int count=0,first =1;
    //THREADID thread_identity;

    double access_count = 0;
  
    /*std::list<THREADID>::iterator plit;
    for(plit=parentList.begin();plit!=parentList.end();plit++)
    {
        cout<<"pid : "<<*plit;
    }*/
    
                           
                        /*std::map<ADDRINT,std::list<struct globalinregion> >::iterator tempit;
                        //tempit = globalinmap.find(lit->global_var);
                        cout<<"I am here"<<endl;
                        //if(tempit!=globalinmap.end())
                            //return 1;
                        for(tempit=globalinmap.begin();tempit!=globalinmap.end();tempit++)
                        {
                            if(tempit->first==lit->global_var)
                                return 1;
                        }*/
                       
                        
                            for(itgrin = globalinmap.begin(); itgrin!= globalinmap.end(); ++itgrin)
                              {

                                int flag = 0;
                                for(itg = glist.begin();itg!=glist.end();++itg)
                                { 
                                
                                  if (itgrin->first == *itg) {flag++;}
                                }
                                
                                if (flag != 0)
                                {
                                  //cout << "GLOABL VARIABLE" << *itg << '\n';
                                    std::list<struct globalinregion>::iterator globalin_2nd_itr;
                                   for(globalin_2nd_itr=itgrin->second.begin(); globalin_2nd_itr!=itgrin->second.end(); ++globalin_2nd_itr)
                                 {
                                  
                                    if(tid==globalin_2nd_itr->tid) {
                                                                       
                                                                       
                                                                        
                                        access_count = access_count +1;
                                        //cout << "ACCESS COUNT" << access_count << '\n';
                                     }
                                  }
                                }
                           }
   
   // cout << "ACCESS COUNT" << access_count;
     if(access_count==0)
return 0.15;
else
return (access_count);
}


/*Auxillary function to detect whether there are any global variables in the program or not*/
int global_present()
{
    int flag=0;
    for(itg=glist.begin();itg!=glist.end();++itg)
    { 

    flag++;

    }
    if(flag==0)
        return 0;
    else 
        return 1;
}

/*Auxillary functio to decide the level of thread safety of the program
  Returns 1 : when totally unsafe
          2 : when partially safe
          3 : when totally safe*/
int safety_decider() {

    int partFlag = 0;
    int totalFlag = 1;
    int g_present=global_present();
    if(g_present==1)
    {
        cout<<"\nGlobal variables are present"<<endl;
	*outf2<<"Global variables are present"<<endl;
	
        for(itg=glist.begin();itg!=glist.end();++itg)
        {
            for(itgrin=globalinmap.begin();itgrin!=globalinmap.end();itgrin++)
            {
                if(*itg==itgrin->first)
                {
                    std::list<struct globalinregion>::iterator l3;
                    for(l3=itgrin->second.begin();l3!=itgrin->second.end();l3++)
                    {

//cout << "l3->sync" <<l3->sync;
                        if(l3->sync=="MUTEX"||l3->sync=="BARRIER" ||l3->sync=="SEMAPHORE" ||l3->sync=="RW_LOCK")
                        {
                            partFlag = 1;
//cout <<"Part flag " << partFlag;
                        }
                        if(l3->sync==" ")
                        {
                            totalFlag = 0;
                        }
                    }
                }
            }
        }
        if(totalFlag==1)
            return 3;
        else if(partFlag==1)
            return 2;
        else if(partFlag==0)
            return 1;
    }
    else
        cout<<"No global variables are present and the program is fully thread safe."<<endl;
	*outf2<<"No global variables are present and the program is fully thread safe."<<endl;
    
    return 0;    
}

/*Auxillary function for deciding starvation among member threads
  Returns 0: when some threads are starving
          1: when majority threads are starving
          2: when none of the threads are starving*/
int starvation_decider()
{
    int count=0;
    std::list<struct startEnd>::iterator sit;
    for(sit=timeList.begin();sit!=timeList.end();sit++)
    {
        if(sit->end-sit->start>limit)
        {
            count++;
		*outf3<<"\n\n\nThread "<<sit->thid<<" requests for lock at event "<<sit->start<<endl;
		*outf3<<"Thread "<<sit->thid<<" acquires lock at event "<<sit->end<<endl;
            cout<<"Thread "<<sit->thid<<" has been starving for "<<sit->end-sit->start<<" trials"<<endl;
	    *outf3<<"\n\nThread "<<sit->thid<<" has been starving for "<<sit->end-sit->start<<" trials"<<endl;
	    
        }
    }
    if(count>2)
        return 1;
    else if(count==0)
        return 2;
    else
        return 0;
}

/*Auxiliary function to decide whether there is live lock in the program
  Returns 0: When there is no chance of any live lock
          1: When there is a potential for live lock*/
int live_lock_decider()
{
    int count=0;
    std::list<struct startEnd>::iterator sit;
    for(sit=timeList.begin();sit!=timeList.end();sit++)
    {
        if(sit->tFlag==1&&sit->end-sit->start>limit)
        {
            count++;
            *outf4<<"\n\n\nThread "<<sit->thid<<" requests for lock at event "<<sit->start<<endl;
		*outf4<<"Thread "<<sit->thid<<" acquires lock at event "<<sit->end<<endl;
            cout<<"Thread "<<sit->thid<<" has been starving for "<<sit->end-sit->start<<" trials"<<endl;
	    *outf4<<"\n\nThread "<<sit->thid<<" has been starving for "<<sit->end-sit->start<<" trials"<<endl;
        }
    }
    if(count>0)
        return 1;
    else
        return 0;
}

void data_access ()
{

  *outf5<<"\n\n............Printing sequence ............"<<endl;
  for(tempit= imap.begin();tempit!=imap.end(); tempit++)
  {
    *outf5<<"\nThread Id : "<<tempit->first<<endl;
    *outf5<<"\nThe globals accessed by the above thread are in the sequence "<<endl;
    std::list<ADDRINT>::iterator litero;
    char a='a';
    for(litero=tempit->second.begin();litero!=tempit->second.end();litero++, a++)
    {
      *outf5<<a<<": "<<*litero<<endl;
    }
  }
int g=global_present();
int flag=0;

if (g==1)
	{

	for(itg=glist.begin();itg!=glist.end();++itg)
	{ 
		int tcount=0;
		for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
		{
			if(*itg==itgrin->first)
			{
				std::list<struct globalinregion>::iterator littgrin3;
				
				for(littgrin3=itgrin->second.begin(); littgrin3!=itgrin->second.end(); ++littgrin3)
				{
				tcount++;
				}
			}
			//cout<<"\n\n Global Variable "<<itgrin->first<<""<<"   Accessed by "<< tcount<< " threads\n"<<'\n';
			if (tcount>=2)
			{ flag=1; break; }
		}

	}


	}
else if (g==0)
	
	{
	 cout <<"\n\n Only local variables"<<endl;
	 return;
	}

if (flag==1)

	{
	cout <<"\n\nOverlapped access"<<endl;
	return;
	}

else

	{
	cout <<"\n\nPartitioned access"<<endl;
	return;
	}
}


VOID Fini(INT32 code, VOID *v)
{
//int i;
for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{

 //cout<<'\n'<<"Event"<<" "<<it2->first<<" : ";				
std::list<struct threadregion>::iterator lit;
std::list<struct threadregion>::reverse_iterator lit1;
lit=it2->second.begin();
lit1=it2->second.rbegin();
//cout<<lit->cont;
//cout<<lit1->cont;
if(lit->order=="AFTER" && lit->cont=="STARTS" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';
 strthread=lit->tid;
}


if(lit->order=="BEFORE" && lit->cont=="STARTS" and lit->node_type==6)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n'
;


if(lit->order=="BEFORE" && lit->cont=="Puts Memory bar" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var<<'\n';

struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=9;
tr.addr=lit->global_var;


itgrmu=globalmutmap.find(lit->global_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit->global_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}




for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

				
std::list<struct globalregion>::iterator littgr2;
				
for(littgr2=itgr->second.begin(); littgr2!=itgr->second.end(); ++littgr2)

{
if(littgr2->tid==lit->tid)

{
littgr2->mutex_var=lit->global_var;
littgr2->type="BEFORE";
littgr2->sync="BARRIER";

}
}


}

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

				
std::list<struct globalinregion>::iterator littgrin2;
				
for(littgrin2=itgrin->second.begin(); littgrin2!=itgrin->second.end(); ++littgrin2)

{
if(littgrin2->tid==lit->tid)

{
littgrin2->mutex_var=lit->global_var;
littgrin2->type="BEFORE";
littgrin2->sync="BARRIER";

}
}


}

for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

				
std::list<struct cglobalinregion>::iterator littcgrin2;
				
for(littcgrin2=itcgrin->second.begin(); littcgrin2!=itcgrin->second.end(); ++littcgrin2)

{
if(littcgrin2->tid==lit->tid)

{
littcgrin2->bcount=(littcgrin2->bcount)+1;

}
}


}

}



if(lit->order=="BEFORE" && lit->cont=="Requests for locking the mutex variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=2;
//cout << " I am in tr.flag" << tr.flag;
tr.addr=lit1->mutex_var;
struct startEnd nStart;
nStart.thid = lit->tid;
nStart.start = it2->first;
nStart.tFlag = 0;
timeList.push_back(nStart);



itgrmu=globalmutmap.find(lit1->mutex_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit1->mutex_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}

}

if(lit->order=="BEFORE" && lit->cont=="Requests for try locking the mutex variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
*outf4<<"\nThread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<" at event "<< it2->first <<'\n';
struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=2;
tr.addr=lit1->mutex_var;
struct startEnd nStart;
nStart.thid = lit->tid;
nStart.start = it2->first;
nStart.tFlag = 1;
timeList.push_back(nStart);



itgrmu=globalmutmap.find(lit1->mutex_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
    tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit1->mutex_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}

}




if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for mutex variable whose address is" and lit->node_type==0)

{

std::list<struct startEnd>::iterator sit;
for(sit=timeList.begin();sit!=timeList.end();sit++)
{
    if(sit->thid==lit->tid)
    {
        sit->end = it2->first;
        //cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<"after "<<sit->end<<"-"<<sit->start<<" trials "<<'\n';
        break;
    }
}


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==2)


littgrmu->flag=1;
//cout << " I am in tr.flag" << littgrmu->flag;
}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="LOCKS";
tr.addr=lit1->mutex_var;
int counterm=mutcount;
mutcount++;



itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}

}





if(lit->order=="BEFORE" && lit->cont=="Requests for locking the semaphore variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->sem_var <<"  "<<'\n';
struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=2;
tr.addr=lit1->sem_var;




itgrmu=globalmutmap.find(lit1->sem_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit1->sem_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}

}




if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for semaphore variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->sem_var <<"  "<<'\n';
for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->sem_var && littgrmu->flag==2)
littgrmu->flag=11;
//cout << "\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t\t\tYOLO FLAG "<< littgrmu->flag<<"\n\n\n\n";
}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="LOCKS";
tr.addr=lit1->sem_var;
int counterm=mutcount;
mutcount++;



itggrmu=gglobalmutmap.find(lit1->sem_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}

}



if(lit->order=="BEFORE" && lit->cont=="Requests for locking the rw_lock variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=2;
tr.addr=lit1->mutex_var;




itgrmu=globalmutmap.find(lit1->mutex_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit1->mutex_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}

}




if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for rw_lock variable whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
//cout << "\n\n\n\n\n\n\n\n\t\t\t\t\t\tNEED FLAG  2 "<<"\t\t ADDRESS "<<lit1->mutex_var<< "\t\t TID "<<lit->tid<<" \n\n\n\n";
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==2)
{
littgrmu->flag=7;
//cout << "\n\n\\t\t\t\t\t\tYOLO FLAG "<< littgrmu->flag<<"\t\t ADDRESS "<<littgrmu->addr<< "\t\t TID "<<littgrmu->tid<<" \n\n\n\n";
}
}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="LOCKS";
tr.addr=lit1->mutex_var;
int counterm=mutcount;
mutcount++;



itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}

}



if(lit->order=="BEFORE" && lit->cont=="Executes Main" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
 mainthread=lit->tid;
}


if(lit->order=="BEFORE" && lit->cont=="Wakes up waiting threads" and lit->node_type==0)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n'
;


if(lit->order=="BEFORE" && lit->cont=="Waits for acquiring lock" and lit->node_type==0)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n'
;

if(lit->order=="BEFORE" && lit->cont=="JOINS" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

struct exitregion tr;
int count;
count=exitcount;
exitcount++;
tr.tid=lit->tid;
tr.type="JOINS";

std::list<struct exitregion> tlist1;
	tlist1.push_back(tr);
 exitmap.insert(std::pair<int, std::list<struct exitregion> >(count, tlist1) );

}


if(lit->order=="BEFORE" && lit->cont=="Unlocks the mutex variable whose address is" and lit->node_type==0)
{


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==1)
littgrmu->flag=0;

}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="UNLOCKS";
tr.addr=lit1->mutex_var;


int counterm=mutcount;
mutcount++;

itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}




//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}


if(lit->order=="BEFORE" && lit->cont=="Unlocks the semaphore lock whose address is" and lit->node_type==0)
{


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->sem_var && littgrmu->flag==1)
littgrmu->flag=0;

}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="UNLOCKS";
tr.addr=lit1->sem_var;


int counterm=mutcount;
mutcount++;

itggrmu=gglobalmutmap.find(lit1->sem_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}




//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->sem_var <<"  "<<'\n';
}




if(lit->order=="BEFORE" && lit->cont=="Unlocks the rw_lock variable whose address is" and lit->node_type==0)
{


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==1)
littgrmu->flag=0;

}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="UNLOCKS";
tr.addr=lit1->mutex_var;


int counterm=mutcount;
mutcount++;

itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}




//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}




if(lit->order=="BEFORE" && lit->cont=="CREATES" and lit->node_type==0)
{
struct createregion tr;
int count;
count=creatcount;
creatcount++;
tr.tid=lit->tid;
//cout << "TR.TID" << tr.tid;
tr.tid1=lit1->tid;
//cout << "TR.TID1" << tr.tid1;

std::list<struct createregion> tlist1;
	tlist1.push_back(tr);
 createseqmap.insert(std::pair<int, std::list<struct createregion> >(count, tlist1) );

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

 itcr= createmap.find(lit->tid);
 if(itcr==createmap.end())
 {
//cout << "Did not find it first " <<'\n';
std::list<THREADID> tlist;
tlist.push_back(lit1->tid);
 createmap.insert(std::pair<THREADID,std::list<THREADID> >(lit->tid, tlist) );
 }

else
{
//cout << "found" <<'\n';
 itcr->second.push_back(lit1->tid);
}


//cout << " *Displaying Create Map* " << '\n'<< '\n';
//cout << "-------------------------------" << '\n';


/*for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
{

cout << " First Element of Create Map " << '\n' << itcr->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<THREADID>::iterator create_2nd_itr;
               
for(create_2nd_itr=itcr->second.begin(); create_2nd_itr!=itcr->second.end(); ++create_2nd_itr)
{

cout<<"Thread "<<*create_2nd_itr<<'\n';
}

}*/


}

if(lit->order=="BEFORE" && lit->cont=="Waits over condition variable" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->cond_var <<"  "<<'\n';
struct signalregion tr1;
int count;
count=signalcount;
signalcount++;
tr1.tid=lit->tid;
tr1.tid1=0;
tr1.type="WAITS";
tr1.cond_var=lit1->cond_var;
tr1.mutex_var=lit1->mutex_var;

std::list<struct signalregion> tlist1;
	tlist1.push_back(tr1);
 signalseqmap.insert(std::pair<int, std::list<struct signalregion> >(count, tlist1) );


itcc=condmap.find(lit->tid);
 if(itcc==condmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(lit1->cond_var);
 condmap.insert(std::pair<THREADID,std::list<ADDRINT> >(lit->tid, tlist) );
 }

else
{
 itcc->second.push_back(lit1->cond_var);
}




for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu1;				
for(litgrmu1=itgrmu->second.begin(); litgrmu1!=itgrmu->second.end(); ++litgrmu1)
{
 
if(litgrmu1->addr==lit1->mutex_var && litgrmu1->flag==1)
{
 
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{
//OutFile<<"IST LOOP E DHUKEHCEH";			
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
//OutFile<<"2ST LOOP E DHUKEHCEH";
if(littgr1->mutex_var==litgrmu1->addr)
{
//OutFile<<"3ST LOOP E DHUKEHCEH";

itck=checkmap.find(lit1->cond_var);
 if(itck==checkmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(littgr1->global_var);
 checkmap.insert(std::pair<ADDRINT,std::list<ADDRINT> >(lit1->cond_var, tlist) );
 }

else
{
 itck->second.push_back(littgr1->global_var);
}

}
}
}
}
}
}

}




if(lit->order=="BEFORE" && lit->cont=="Signals" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

{
dcount++;
struct signalregion tr;
int count;
count=signalcount;
signalcount++;
tr.tid=lit->tid;
tr.tid1=lit1->tid;
tr.type="SIGNALS";
tr.cond_var=0;
tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;

std::list<struct signalregion> tlist1;
	tlist1.push_back(tr);
 signalseqmap.insert(std::pair<int, std::list<struct signalregion> >(count, tlist1) );

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

 itsr=signalmap.find(lit->tid);
 if(itsr==signalmap.end())
 {
std::list<THREADID> tlist;
tlist.push_back(lit1->tid);
 signalmap.insert(std::pair<THREADID,std::list<THREADID> >(lit->tid, tlist) );
 }

else
{
 itsr->second.push_back(lit1->tid);
}

}



for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu1;				
for(litgrmu1=itgrmu->second.begin(); litgrmu1!=itgrmu->second.end(); ++litgrmu1)
{
 
if(litgrmu1->addr==lit1->mutex_var && litgrmu1->flag==1)
{
 
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{
//OutFile<<"IST LOOP E DHUKEHCEH";			
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
//OutFile<<"2ST LOOP E DHUKEHCEH";
if(littgr1->mutex_var==litgrmu1->addr)
{
//OutFile<<"3ST LOOP E DHUKEHCEH";

itck=checkmap.find(lit1->cond_var);
 if(itck==checkmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(littgr1->global_var);
 checkmap.insert(std::pair<ADDRINT,std::list<ADDRINT> >(lit1->cond_var, tlist) );
 }

else
{
 itck->second.push_back(littgr1->global_var);
}

}
}
}
}
}
}

}

if(lit->order=="BEFORE" && lit->cont=="EXITS" and lit->node_type==0)
{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
struct exitregion tr1;
int count;
count=exitcount;
exitcount++;
tr1.tid=lit->tid;
tr1.type="EXITS";

std::list<struct exitregion> tlist1;
	tlist1.push_back(tr1);
 exitmap.insert(std::pair<int, std::list<struct exitregion> >(count, tlist1) );
}

if(lit->order=="BEFORE" && lit->cont=="Writes to variable whose address is" and lit->node_type==0)

{


//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';



 itgrmu=globalmutmap.find(lit1->global_var);
 if(itgrmu==globalmutmap.end())
 {

int count;
count=varcount;
varcount++;
struct globalregion tr;
tr.tid=lit->tid;
tr.type=" ";
 tr.mutex_var=0; tr.sem_var=0; tr.rw_var=0;;
  tr.sync=" ";
tr.global_var=lit1->global_var;

struct globalinregion tr2;
tr2.tid=lit->tid;
tr2.type=" ";
 tr2.mutex_var=0;
  tr2.sync=" ";

struct cglobalinregion tr3;
tr3.tid=lit->tid;
tr3.count=0;
tr3.scount=0;
tr3.bcount=0;
//int sflag=0;
int flag=0;
int bflag=0;
int mflag=0;


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu;				
for(litgrmu=itgrmu->second.begin(); litgrmu!=itgrmu->second.end(); ++litgrmu)
{
 
if(litgrmu->tid==lit->tid && litgrmu->flag==2)
{

//cout << " I am in";
 tr.mutex_var=litgrmu->addr;
  tr.sync="MUTEX";

 tr2.mutex_var=litgrmu->addr;
  tr2.sync="MUTEX";

  tr3.scount=1;
mflag=1;
}

if(litgrmu->tid==lit->tid && litgrmu->flag==11)
{
 tr.sem_var=litgrmu->addr;
  tr.sync="SEMAPHORE";

 tr2.sem_var=litgrmu->addr;
  tr2.sync="SEMAPHORE";

  tr3.scount=1;
mflag=1;

}


if(litgrmu->tid==lit->tid && litgrmu->flag==7)
{
 tr.mutex_var=litgrmu->addr;
  tr.sync="RW_LOCK";

 tr2.mutex_var=litgrmu->addr;
  tr2.sync="RW_LOCK";

  tr3.scount=1;
mflag=1;

//cout <<"\n\n\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tYOLO update\n\n\n\n";
}

else if(litgrmu->tid==lit->tid && litgrmu->flag==9)
{
 tr.mutex_var=litgrmu->addr;
  tr.sync="BARRIER";
tr.type="AFTER";


 tr2.mutex_var=litgrmu->addr;
  tr2.sync="BARRIER";
tr2.type="AFTER";

tr3.bcount=1;
bflag=1;
  
}

}


}

std::list<struct globalregion> tlist;
tlist.push_back(tr);
globalmap.insert(std::pair<int,std::list<struct globalregion> >(count, tlist) );

itcgrin=cglobalinmap.find(lit1->global_var);

  if(itcgrin==cglobalinmap.end())
{
std::list<struct cglobalinregion> tlist3;
tlist3.push_back(tr3);
cglobalinmap.insert(std::pair<ADDRINT,std::list<struct cglobalinregion> >(lit1->global_var, tlist3) );


}
else
{
std::list<struct cglobalinregion> :: iterator litcgrin;
for(litcgrin=itcgrin->second.begin(); litcgrin!=itcgrin->second.end(); ++litcgrin)
{
if(litcgrin->tid==lit->tid)
{

if(mflag==1)

litcgrin->scount=(litcgrin->scount)+1;


else if(bflag==1)
litcgrin->bcount=(litcgrin->bcount)+1;

else
litcgrin->count=(litcgrin->count)+1;
flag=1;
}

}

if(flag==0)
itcgrin->second.push_back(tr3);

}




itgrin=globalinmap.find(lit1->global_var);
 if(itgrin==globalinmap.end())
{
std::list<struct globalinregion> tlist2;
tlist2.push_back(tr2);
globalinmap.insert(std::pair<ADDRINT,std::list<struct globalinregion> >(lit1->global_var, tlist2) );
}
else
{
itgrin->second.push_back(tr2);
}
 
}
else
{

 itgrcu=countmap.find(lit1->global_var);
if(itgrcu==countmap.end())
 {
   
 struct countmutexregion tr;
 tr.tid=lit->tid;
 tr.count=0;
std::list<struct countmutexregion> tlist;
tlist.push_back(tr);
countmap.insert(std::pair<ADDRINT,std::list<struct countmutexregion> >(lit1->global_var, tlist) );

}
else
{
int f=0;
 std::list<struct countmutexregion>::iterator litgrcu;

 for(litgrcu=itgrcu->second.begin(); litgrcu!=itgrcu->second.end(); ++litgrcu)
{
 if(litgrcu->tid==lit->tid)
{
  litgrcu->count=(litgrcu->count) +1;
  f=1;
}
}
if(f==0)
{
struct countmutexregion tr1;
 tr1.tid=lit->tid;
 tr1.count=0;
 itgrcu->second.push_back(tr1);
}
  
}

}
}





if(lit->order=="BEFORE" && lit->cont=="Reads from variable whose address is" and lit->node_type==0)



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n'
;




if(lit->order=="BEFORE" && lit->node_type==9)



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n'
;


if(lit->order=="BEFORE" && lit->cont=="RUNS EXIT HANDLERS" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




if(lit->order=="BEFORE" && lit->cont=="Exits the Catch Block" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Starts Unwinding Stack for finding the exception handler" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Runs the personality functions for each routine" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Catches the Exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="Rethrows the exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="checks the current exception type, returns NULL if not handled" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="Calls the default terminate function" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Stores the arguments to be passed to next phase of exception hanlding which can be the cleanup phase or the last handler i.e landing pad" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Sets the instruction pointer" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Kills the Process" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Aborts the Process which is the action of the default terminate function" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Starts the search phase once more after exception is rethrown" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Unregisters fork handlers" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Completed Exception handling" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Detects Exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Throws Exception" and lit->node_type==0)



ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="MALLOC" and lit->node_type==0) 



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var <<"  "<<'\n'
;

if(1==1)
ExceptionFile<<endl;
}

				


	



//Population of Global Variables List and Local Variables List


for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

int flag=0;
THREADID td;

 //OutFile<<"Global Variable "<<itgrin->first<<"being accessed without any synchronisation by threads  " <<'\n';
			
std::list<struct globalinregion>::iterator littgrin1;
		
std::list<struct globalinregion>::iterator littgrin4;
littgrin4=itgrin->second.begin();
td=littgrin4->tid;
for(littgrin1=itgrin->second.begin(); littgrin1!=itgrin->second.end(); ++littgrin1)
{


if(td!=littgrin1->tid)
{
flag=1;
cout<<"TIDDD"<<littgrin1->tid <<'\n';
cout<<"TID"<<td <<'\n';
}

 td=littgrin1->tid;
//cout<<" "<<littgrin1->tid<<"THREADS   ";

}
if(flag==1)

glist.push_back(itgrin->first);
//cout<<"GLOBALLL"<<itgrin->first<<'\n';
if(flag==0)
{
std::list<THREADID> tlist;
tlist.push_back(littgrin4->tid);

lmap.insert(std::pair<ADDRINT,std::list<THREADID> >(itgrin->first, tlist) );

}


}


//Population of GlobalVars-Mutex and GlobalVars-Barrier maps


for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{


int mflag=0;
int bflag=0;
	
//int afbflag=0;
// OutFile<<"Global Variable "<<itgrin->first<<"being accessed without any synchronisation by threads  " <<'\n';
		
std::list<struct aglobalinregion> tlist1;
std::list<struct aglobalinregion> :: iterator limt;
std::list<struct bglobalinregion> tlist2;
std::list<struct bglobalinregion> :: iterator libt;
std::list<struct afbfglobalinregion> tlist3;
std::list<struct afbfglobalinregion> :: iterator lifft;

std::list<struct globalinregion>::iterator littgrin1;
		

for(littgrin1=itgrin->second.begin(); littgrin1!=itgrin->second.end(); ++littgrin1)
{
if(littgrin1->sync=="MUTEX")
{
 //mlist.push_back(littgrin1->mutex_var);

int e=0;
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 if((*itm)==littgrin1->mutex_var)
{
  e=1;
//cout<<"NEXT TIME";
}
}
if (e==0)
{
//cout<<"MUTEXESS"<<littgrin1->mutex_var<<"  ";
mlist.push_back(littgrin1->mutex_var);
}

int p=0;
for(limt=tlist1.begin();limt!=tlist1.end();++limt)
{
   if(limt->var==littgrin1->mutex_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(limt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      limt->count=(limt->count)+1;
      limt->tid[limt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
mflag=1;
struct aglobalinregion tr1;
tr1.var=littgrin1->mutex_var;
tr1.count=0;
tr1.tid[tr1.count]=littgrin1->tid;
tlist1.push_back(tr1);
mflag=1;

}

}



if(littgrin1->sync=="SEMAPHORE")
{
 //mlist.push_back(littgrin1->mutex_var);

int e=0;
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 if((*itm)==littgrin1->sem_var)
{
  e=1;
//cout<<"NEXT TIME";
}
}
if (e==0)
{
//cout<<"MUTEXESS"<<littgrin1->sem_var<<"  ";
mlist.push_back(littgrin1->sem_var);
}

int p=0;
for(limt=tlist1.begin();limt!=tlist1.end();++limt)
{
   if(limt->var==littgrin1->sem_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(limt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      limt->count=(limt->count)+1;
      limt->tid[limt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
mflag=1;
struct aglobalinregion tr1;
tr1.var=littgrin1->sem_var;
tr1.count=0;
tr1.tid[tr1.count]=littgrin1->tid;
tlist1.push_back(tr1);
mflag=1;

}

}


if(littgrin1->sync=="RW_LOCK")
{
 //mlist.push_back(littgrin1->mutex_var);

int e=0;
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 if((*itm)==littgrin1->mutex_var)
{
  e=1;
//cout<<"NEXT TIME";
}
}
if (e==0)
{
//cout<<"MUTEXESS"<<littgrin1->mutex_var<<"  ";
mlist.push_back(littgrin1->mutex_var);
}

int p=0;
for(limt=tlist1.begin();limt!=tlist1.end();++limt)
{
   if(limt->var==littgrin1->mutex_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(limt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      limt->count=(limt->count)+1;
      limt->tid[limt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
mflag=1;
struct aglobalinregion tr1;
tr1.var=littgrin1->mutex_var;
tr1.count=0;
tr1.tid[tr1.count]=littgrin1->tid;
tlist1.push_back(tr1);
mflag=1;

}

}





if(littgrin1->sync=="BARRIER" and littgrin1->type=="AFTER")
{

int p=0;
for(lifft=tlist3.begin();lifft!=tlist3.end();++lifft)
{
   if(lifft->var==littgrin1->mutex_var and lifft->occ=="AFTER")
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(lifft->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{    
  lifft->count=(lifft->count)+1;
      lifft->tid[lifft->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct afbfglobalinregion tr3;
tr3.var=littgrin1->mutex_var;
tr3.count=0;
tr3.tid[tr3.count]=littgrin1->tid;
tr3.occ="AFTER";
tlist3.push_back(tr3);
//afbflag=1;

}
}





if(littgrin1->sync=="BARRIER" and littgrin1->type=="BEFORE")
{

int p=0;
for(lifft=tlist3.begin();lifft!=tlist3.end();++lifft)
{
   if(lifft->var==littgrin1->mutex_var and lifft->occ=="BEFORE")
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(lifft->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      lifft->count=(lifft->count)+1;
      lifft->tid[lifft->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct afbfglobalinregion tr3;
tr3.var=littgrin1->mutex_var;
tr3.count=0;
tr3.tid[tr3.count]=littgrin1->tid;
tr3.occ="BEFORE";
tlist3.push_back(tr3);
//afbflag=1;
}

}



if(littgrin1->sync=="BARRIER" )
{

int e=0;
for(itb=blist.begin();itb!=blist.end();++itb)
{
 if(*itb==littgrin1->mutex_var)
  e=1;
}


if (e==0)
blist.push_back(littgrin1->mutex_var);

int p=0;
for(libt=tlist2.begin();libt!=tlist2.end();++libt)
{
   if(libt->var==littgrin1->mutex_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(libt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      libt->count=(libt->count)+1;
      libt->tid[libt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct bglobalinregion tr2;
tr2.var=littgrin1->mutex_var;
tr2.count=0;
tr2.tid[tr2.count]=littgrin1->tid;
tlist2.push_back(tr2);
bflag=1;
}

}







}


if(mflag==1)
mutexglobalinmap.insert(std::pair<ADDRINT,std::list<struct aglobalinregion> >(itgrin->first, tlist1) );

if(bflag==1)
{
barrierglobalinmap.insert(std::pair<ADDRINT,std::list<struct bglobalinregion> >(itgrin->first, tlist2) );

afbfbarrierglobalinmap.insert(std::pair<ADDRINT,std::list<struct afbfglobalinregion> >(itgrin->first, tlist3) );
}



}

//cout<<"Global mtex "<<'\n';
for(itmgrin=mutexglobalinmap.begin(); itmgrin!=mutexglobalinmap.end(); ++itmgrin)
{
std::list<struct aglobalinregion> :: iterator limtt;
//cout<<" VAR IABLE"<<" "<<itmgrin->first<<'\n';
	for(limtt=itmgrin->second.begin();limtt!=itmgrin->second.end();++limtt)
{
//cout<<"mtex"<<limtt->var<<"   ";
int c=0;
for (c=0;c<=limtt->count;c++)
{
//cout<<limtt->tid[c]<<"    "<<'\n';
}


}


}



//cout<<"Global barrier"<<'\n';



for(itbgrin=barrierglobalinmap.begin(); itbgrin!=barrierglobalinmap.end(); ++itbgrin)
{
std::list<struct bglobalinregion> :: iterator libtt;
//cout<<" VAR IABLE"<<" "<<itbgrin->first<<'\n';
	for(libtt=itbgrin->second.begin();libtt!=itbgrin->second.end();++libtt)
{
//cout<<"mtex"<<libtt->var<<"   ";
int c=0;
for (c=0;c<=libtt->count;c++)
{
//cout<<libtt->tid[c]<<"    "<<'\n';
}


}
}

//cout<<"Global mtex before after"<<'\n';


for(itafbfgrin=afbfbarrierglobalinmap.begin(); itafbfgrin!=afbfbarrierglobalinmap.end(); ++itafbfgrin)
{
std::list<struct afbfglobalinregion> :: iterator lifftt;
//cout<<" VAR IABLE"<<" "<<itafbfgrin->first<<'\n';
	for(lifftt=itafbfgrin->second.begin();lifftt!=itafbfgrin->second.end();++lifftt)
{
//cout<<"mtex"<<lifftt->var<<"   ";
int c=0;
for (c=0;c<=lifftt->count;c++)
{
cout<<lifftt->tid[c]<<"    "<<'\n';

}


//cout<<"OCCURANCE"<<lifftt->occ<<"    "<<'\n';
}
}








//cout<<"Global varibales list"<<'\n';

for(itg=glist.begin();itg!=glist.end();++itg)
{
 //cout<<"M"<<*itg<<";"<<'\n';
}



//cout<<"barrier varibales list"<<'\n';

for(itb=blist.begin();itb!=blist.end();++itb)
{
 //cout<<"B"<<*itb<<";"<<'\n';
}


//cout<<"mutex varibales list"<<'\n';

for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 //cout<<"M"<<*itm<<";"<<'\n';
}


//cout<<"localvaribales list"<<'\n';

for(itl=lmap.begin();itl!=lmap.end();++itl)
{
 //cout<<"M"<<itl->first<<";"<<'\n';

std::list<THREADID> :: iterator litl;
for(litl=itl->second.begin();litl!=itl->second.end();++litl)
{
//cout <<"THREADLOCALLLLLLL"<<*litl<<"  ";
}
}


//cout<<"cglobalin map"<<'\n';


//for(itg=glist.begin();itg!=glist.end();++itg)
//{


for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

//if(*itg==itcgrin->first)
//{
 //cout<<'\n'<<"Global Variables "<<itcgrin->first<<"being accessed with synchronisation by threads "<<'\n';
				
std::list<struct cglobalinregion>::iterator littcgrin1;
				
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{
 
//cout<<"scount"<<littcgrin1->scount<<'\n';
//cout<<"bcount"<<littcgrin1->bcount<<'\n';
//cout<<"count"<<littcgrin1->count<<'\n';
if(littcgrin1->scount>0)
{
//cout<<"THREAD "<<littcgrin1->tid<<" with the help of Mutex Locks ";
//cout<<"    scount" <<littcgrin1->scount;	
}

if(littcgrin1->bcount>0)
{
//cout<<" "<<littcgrin1->tid<<" with memory barriers ";	


//cout<<"    bcount" <<littcgrin1->bcount;
}

if(littcgrin1->bcount>0 && littcgrin1->scount>0)
{
//cout<<" "<<littcgrin1->tid<<" with out synchronisation ";	


//cout<<"    count" <<littcgrin1->count;
}
}
}
//}
//}

//cout<<"GLOBALIN map"<<'\n';
int icount=1;
for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

 //cout<<"\n\n\n"<<icount<< " : "<<"Global Variable "<<itgrin->first<<""<<"   Access Sequence Order "<<'\n'<<'\n';
//*outf2<<"\n\n\n"<<icount<< " : "<<"Global Variable "<<itgrin->first<<""<<"   Access Sequence Order "<<'\n'<<endl;
	icount++;
std::list<struct globalinregion>::iterator littgrin3;
	char lcount='a';			
for(littgrin3=itgrin->second.begin(); littgrin3!=itgrin->second.end(); ++littgrin3)
{
 if(littgrin3->sync=="MUTEX")
{
 //cout<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of mutex " <<littgrin3->mutex_var <<'\n';
//*outf2<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of mutex " <<littgrin3->mutex_var <<endl;
lcount++;
}

 if(littgrin3->sync=="SEMAPHORE")
{
 //cout<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of semaphore " <<littgrin3->sem_var <<'\n';
 //*outf2<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of semaphore " <<littgrin3->sem_var <<endl;
lcount++;
}

 if(littgrin3->sync=="RW_LOCK")
{
 //cout<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of rw_lock " <<littgrin3->mutex_var <<'\n';
//*outf2<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with the help of rw_lock " <<littgrin3->rw_var <<endl;
lcount++;
}

if(littgrin3->sync==" ")
{
 //cout<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" without any synchronisation "<<'\n';
//*outf2<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" without any synchronisation "<<endl;
lcount++;
}
if(littgrin3->sync=="BARRIER")
{
//cout<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with memory barrier  "<<littgrin3->mutex_var<<"   "<<littgrin3->type<<'\n';
//*outf2<<"\n"<<lcount<<" : "<<" Accessed by thread "<<littgrin3->tid <<" with memory barrier  "<<littgrin3->mutex_var<<"   "<<littgrin3->type<<endl;
lcount++;
}				
}


}		


/*//cout << " *Displaying Create Map* " << '\n'<< '\n';
//cout << "-------------------------------" << '\n';


for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
{

cout << " First Element of Create Map " << '\n' << itcr->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<THREADID>::iterator create_2nd_itr;
               
for(create_2nd_itr=itcr->second.begin(); create_2nd_itr!=itcr->second.end(); ++create_2nd_itr)
{

cout<<"Thread "<<*create_2nd_itr<<'\n';
}

}

*/

/*cout << " *Displaying globalin Map* " << '\n';
cout << "-------------------------------" << '\n';


for(itgrin = globalinmap.begin(); itgrin!= globalinmap.end(); ++itgrin)
{

cout << " First Element of globalin Map " << '\n' << itgrin->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<struct globalinregion>::iterator globalin_2nd_itr;
               
for(globalin_2nd_itr=itgrin->second.begin(); globalin_2nd_itr!=itgrin->second.end(); ++globalin_2nd_itr)
{

cout<<"Thread "<<globalin_2nd_itr->tid<<'\n';
cout<<"mutex_variable "<<globalin_2nd_itr->mutex_var<<'\n';
cout<<"sem_variable "<<globalin_2nd_itr->sem_var<<'\n';
cout<<"rw_variable "<<globalin_2nd_itr->rw_var<<'\n';
cout<<"sync "<<globalin_2nd_itr->sync<<'\n';
cout<<"type "<<globalin_2nd_itr->type<<'\n';
}

}

*/



/*cout << " *Displaying globalmut Map* " << '\n';
cout << "-------------------------------" <<'\n';


for(itgrmu = globalmutmap.begin(); itgrmu!= globalmutmap.end(); ++itgrmu)
{

cout << " First Element of globalmut Map " << '\n' << itgrmu->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<struct mutexregion>::iterator globalmut_2nd_itr;
               
for(globalmut_2nd_itr=itgrmu->second.begin(); globalmut_2nd_itr!=itgrmu->second.end(); ++globalmut_2nd_itr)
{

cout<<"flag "<<globalmut_2nd_itr->flag<<'\n';
cout<<"addr "<<globalmut_2nd_itr->addr<<'\n';
cout<<"thread "<<globalmut_2nd_itr->tid<<'\n';


}

}
*/

/*for(itg=glist.begin();itg!=glist.end();++itg)
{

DataFile<<"global "<<*itg << '\n';

}*/


	if (!EnableModelDetection)
{

OutFile<<"The Start Thread is :"<<strthread<<'\n'<<'\n';

OutFile<<"The Main Thread is :"<<mainthread<<'\n'<<'\n';

OutFile <<"The Create Sequence :"<<'\n'<<'\n';


*outf1<<"The Start Thread is :"<<strthread<<'\n'<<endl;

*outf1<<"The Main Thread is :"<<mainthread<<'\n'<<endl;

*outf1 <<"The Create Sequence :"<<'\n'<<endl;
ModelMapFile<<"digraph G {"<<'\n';

	

for(itcrsq=createseqmap.begin(); itcrsq!=createseqmap.end(); ++itcrsq)
{

 OutFile<<" "<<itcrsq->first<<" : ";	
*outf1<<" "<<itcrsq->first<<" : ";			
std::list<struct createregion>::iterator litcrsq;
				
for(litcrsq=itcrsq->second.begin(); litcrsq!=itcrsq->second.end(); ++litcrsq)
{

OutFile<<"Thread "<<litcrsq->tid <<" Creates "<<"Thread "<<litcrsq->tid1<<'\n';
*outf1<<"Thread "<<litcrsq->tid <<" Creates "<<"Thread "<<litcrsq->tid1<<'\n';
				
				
}
			
}

OutFile <<'\n'<<"The Create Summary:"<<'\n'<<'\n';
*outf1 <<'\n'<<"The Create Summary:"<<'\n'<<'\n';

		ModelMapFile<<"node [style=filled,color=blue];"<<'\n';
           
            ModelMapFile<<"edge[label=creates]"<<'\n';


	int count=0,first =1;
    THREADID thread_identity;	

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{

 OutFile<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';
*outf1<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';

if(first==1)
                    {
                        thread_identity = itcr->first;
                        first = 0;
                        count++;
                        
                    }
                    else
                    {
                        if(thread_identity!=itcr->first)
                        {
                            count++;
                           
                        }
                    }


if(count>1)
    {
        manyP=1;

    }
    else
    {
        manyP=0;

    }
				
std::list<THREADID>::iterator litcr;
				
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{

OutFile<<"Thread "<<*litcr<<'\n';
*outf1<<"Thread "<<*litcr<<'\n';
ModelMapFile<<"T"<<itcr->first<<"->"<<"T"<<*litcr<<";"<<'\n';	
		
}	


}


if(dcount>0)
{

OutFile<<'\n'<<"The communication sequence among threads using condition variables "<<'\n'<<'\n';



		ModelMapFile<<"node [style=filled,color=red];"<<'\n';
           
        


for(itsrsq=signalseqmap.begin(); itsrsq!=signalseqmap.end(); ++itsrsq)
{

 OutFile<<" "<<itsrsq->first<<" : " <<'\n';
				
std::list<struct signalregion>::iterator litsrsq;
				
for(litsrsq=itsrsq->second.begin(); litsrsq!=itsrsq->second.end(); ++litsrsq)
{

if(litsrsq->type=="WAITS")
 OutFile<<"Thread "<<litsrsq->tid <<"Waits over condition variable "<<litsrsq->cond_var<<"  protected by mutex "<<litsrsq->mutex_var<<'\n';
else
OutFile<<"Thread "<<litsrsq->tid <<"Signals Thread "<<litsrsq->tid1<<" "<<'\n';			
}
			
}

OutFile<<'\n'<<"The communication summary among threads using condition variables "<<'\n';


for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)
{

 OutFile<<"Thread"<<"    "<<itcc->first<<"is associated with condition variable ";				
std::list<ADDRINT>::iterator litcc;
				
for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

OutFile<<"  "<<*litcc<<'\n';
ModelMapFile<<"C"<<*litcc<<";"<<'\n';

				
}
			
}



for(itsr=signalmap.begin(); itsr!=signalmap.end(); ++itsr)
{

 OutFile<<"Thread"<<" "<<itsr->first<<"signals"<<'\n';				
std::list<THREADID>::iterator litsr;
				
for(litsr=itsr->second.begin(); litsr!=itsr->second.end(); ++litsr)
{

OutFile<<"Thread "<<*litsr<<'\n';
				
}
			
}

}
/*for(itck=checkmap.begin(); itck!=checkmap.end(); ++itck)
{

 //OutFile<<"Waiting and Signaling on the condition variable"<<"    "<<itcc->first<<"are controlled by global variables "<<'\n';

 cout<<"Waiting and Signaling on the condition variable"<<"    "<<itcc->first<<"are controlled by global variables "<<'\n';
std::list<ADDRINT>::iterator litck;
				
for(litck=itck->second.begin(); litck!=itck->second.end(); ++litck)
{

//OutFile<<"  "<<*litck<<'\n';

cout<<"  "<<*litck<<'\n';
		
}
			
}
}
*/
int flag=0;
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

flag++;

}
if(flag==0)
OutFile<<'\n'<<"There are no global variables in this program hence no communication via global variables"<<'\n';


if(flag>0)
{


OutFile<<'\n'<<"The communication summary among threads using Global variables "<<'\n'<<'\n';



ModelMapFile<<"style=filled;"<<'\n';
	
		ModelMapFile<<"node [style=filled,color=green];"<<'\n';
          
            


for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

if(*itg==itcgrin->first)
{

 OutFile<<"Global Variable "<<itcgrin->first<<"being accessed by   " <<'\n';


	ModelMapFile<<"D"<<itcgrin->first<<";"<<'\n';			
std::list<struct cglobalinregion>::iterator littcgrin1;
	int flag =0;			
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{
 
if(littcgrin1->count>0)
{
OutFile<<" "<<littcgrin1->tid<<" "<<"without synchronisation"<<'\n';
flag=1;
}
}
if(flag==0)
OutFile<< "No threads without Synchronisation "<<'\n';
}

}
}





		ModelMapFile<<"node [style=filled,color=yellow];"<<'\n';
            
          
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelMapFile<<"M"<<*itm<<";"<<'\n';
}




		ModelMapFile<<"node [style=filled,color=orange];"<<'\n';
            
          
for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelMapFile<<"B"<<*itb<<";"<<'\n';
}

//mutex
for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itmgrin=mutexglobalinmap.begin(); itmgrin!=mutexglobalinmap.end(); ++itmgrin)
{

if(*itg==itmgrin->first)
{

std::list<struct aglobalinregion>::iterator littm;
OutFile<<'\n'<<"Global Variable "<<itmgrin->first<<" accessed with  mutex	";			
for(littm=itmgrin->second.begin(); littm!=itmgrin->second.end(); ++littm)
{

OutFile<<littm->var<<"  by threads"<<'\n';
int i=0;
for(i=0;i<=littm->count;i++)
{
OutFile<<"Thread "<<littm->tid[i]<<'\n';
ModelMapFile<<"edge[label=locks];"<<'\n';
ModelMapFile<<"T"<<littm->tid[i]<<"->"<<"M"<<littm->var<<";"<<'\n';
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"M"<<littm->var<<"->"<<"D"<<itmgrin->first<<";"<<'\n';

}

}

}
}

}
//var
for(itg=glist.begin();itg!=glist.end();++itg)
{

for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

if(*itg==itcgrin->first)
{

 		
std::list<struct cglobalinregion>::iterator littcgrin1;
				
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{
 
if(littcgrin1->count>0)
{
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"T"<<littcgrin1->tid<<"->"<<"D"<<itcgrin->first<<";"<<'\n';
}
}
}

}
}

//barrier


for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itafbfgrin=afbfbarrierglobalinmap.begin(); itafbfgrin!=afbfbarrierglobalinmap.end(); ++itafbfgrin)
{

if(*itg==itafbfgrin->first)
{

std::list<struct afbfglobalinregion>::iterator lifftt;
OutFile<<'\n'<<"Global Variable accessed with  memory barriers	";			
for(lifftt=itafbfgrin->second.begin(); lifftt!=itafbfgrin->second.end(); ++lifftt)
{

OutFile<<lifftt->var<<"  by threads"<<"  "<<lifftt->occ<<"  its occurance "<<'\n';
int i=0;
for(i=0;i<=lifftt->count;i++)
{
OutFile<<"Thread "<<lifftt->tid[i]<<'\n';
if(lifftt->occ=="AFTER")
{
ModelMapFile<<"edge[label=barrier];"<<'\n';
ModelMapFile<<"T"<<lifftt->tid[i]<<"->"<<"B"<<lifftt->var<<";"<<'\n';
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"B"<<lifftt->var<<"->"<<"D"<<itafbfgrin->first<<";"<<'\n';
}
if(lifftt->occ=="BEFORE")
{

ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"T"<<lifftt->tid[i]<<"->"<<"D"<<itafbfgrin->first<<";"<<'\n';

ModelMapFile<<"edge[label=barrier];"<<'\n';
ModelMapFile<<"D"<<itafbfgrin->first<<"->"<<"B"<<lifftt->var<<";"<<'\n';
}

}

}

}
}

}

}



for(itsr=signalmap.begin(); itsr!=signalmap.end(); ++itsr)
{

 //OutFile<<"Thread"<<" "<<itsr->first<<"signals"<<'\n';				
std::list<THREADID>::iterator litsr;
				
for(litsr=itsr->second.begin(); litsr!=itsr->second.end(); ++litsr)
{
ModelMapFile<<"edge[label=signals];"<<'\n';

ModelMapFile<<"T"<<itsr->first<<"->"<<"T"<<*litsr<<";"<<'\n';
//OutFile<<"Thread "<<*litsr<<'\n';
				
}
			
}

for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)

{

// OutFile<<"Thread"<<"    "<<itcc->first<<"is associated with condition variable ";				
std::list<ADDRINT>::iterator litcc;
				
for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

//OutFile<<"  "<<*litcc<<'\n';
ModelMapFile<<"edge[label=waits];"<<'\n';
ModelMapFile<<"T"<<itcc->first<<"->"<<"C"<<*litcc<<";"<<'\n';
				
}
			
}







ModelMapFile<<"}"<<endl<<'\n';
OutFile <<'\n'<<"The Exit Sequence:"<<'\n'<<'\n';
*outf1 <<'\n'<<"The Exit Sequence:"<<'\n'<<'\n';


for(itex=exitmap.begin(); itex!=exitmap.end(); ++itex)
{

 OutFile<<" "<<itex->first<<" : ";	

 *outf1<<" "<<itex->first<<" : ";					
std::list<struct exitregion>::iterator litex;
				
for(litex=itex->second.begin(); litex!=itex->second.end(); ++litex)
{

OutFile<<"Thread :"<<litex->tid <<"   "<<litex->type<<'\n'<<endl;
*outf1<<"Thread :"<<litex->tid <<"   "<<litex->type<<'\n'<<endl;
				
}
			
}








}






if (!EnableDataDetection)
{


ModelDataFile<<"digraph G {"<<'\n';	

/*ModelDataFile<<"node [style=filled,color=blue];"<<'\n';

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{ 
//OutFile<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';		
std::list<THREADID>::iterator litcr;		
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{
//OutFile<<"Thread "<<*litcr<<'\n';
ModelDataFile<<"T"<<itcr->first<<"->"<<"T"<<*litcr<<";"<<'\n';

}	
}




  ModelDataFile<<"node [style=filled,color=green];"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{
for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{
if(*itg==itcgrin->first)
{ 
//OutFile<<"Global Variable "<<itcgrin->first<<"being accessed by   " <<'\n';
ModelDataFile<<"D"<<itcgrin->first<<";"<<'\n';			
std::list<struct cglobalinregion>::iterator littcgrin1;	
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{ 
//if(littcgrin1->count>0)
//OutFile<<" "<<littcgrin1->tid<<" "<<"without synchronisation"<<'\n';
}
}
}
}

ModelDataFile<<"node [style=filled,color=brown];"<<'\n';
          for(itl=lmap.begin();itl!=lmap.end();++itl)
{
 ModelDataFile<<"DL"<<itl->first<<";"<<'\n';
}


ModelDataFile<<"node [style=filled,color=yellow];"<<'\n';
          for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelDataFile<<"M"<<*itm<<";"<<'\n';
}

		
ModelDataFile<<"node [style=filled,color=orange];"<<'\n';     
     for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelDataFile<<"B"<<*itb<<";"<<'\n';
}
*/
DataFile<<" Data  and their Access History "<<'\n'<<'\n';

int gcount=0;
int scount=0;
int sflag=0;
//int icount=1;
int flag=0;
int lfag=0;

    for(itl=lmap.begin();itl!=lmap.end();++itl)
{
 lfag++;
}
if(lfag>0)
{

DataFile<<'\n'<<" Local variables Access History"<<'\n';
for(itth=thlist.begin();itth!=thlist.end();++itth)
{
    DataFile<<"Thread "<<*itth<<" Uses following local variables "<<'\n';
int flag = 0;
   for(itl=lmap.begin();itl!=lmap.end();++itl)
{
std::list< THREADID>::iterator litl;

for(litl=itl->second.begin();litl!=itl->second.end();++litl)
{
if(*litl==*itth)
{
flag=1;
 DataFile<<itl->first<<'\n';
 }
}

}
if(flag==0)
DataFile<<"No Local variables";
}
}

for(itg=glist.begin();itg!=glist.end();++itg)
{ 

flag++;

}
if(flag==0)
OutFile<<'\n'<<"There are no global variables in this program "<<'\n';


if(flag>0)
{
DataFile <<'\n'<<" Total List of Global Variables in this program are "<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{
if(*itg==itgrin->first)
{
 DataFile<<itgrin->first<<'\n';
	gcount++;
}
}
}			
DataFile<<'\n'<<" Total Count is : "<<gcount<<'\n'<<'\n'<<'\n';


DataFile <<" Total List of Global Variables in this program which are used among threads with synchronisation"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

 if(*itg==itgrin->first)
{
	
std::list<struct globalinregion>::iterator littgrin2;
				
for(littgrin2=itgrin->second.begin(); littgrin2!=itgrin->second.end(); ++littgrin2)
{
 if(littgrin2->sync=="MUTEX" || littgrin2->sync=="BARRIER" || littgrin2->sync=="SEMAPHORE"|| littgrin2->sync=="RW_LOCK")
 sflag=1;
				
}
if(sflag==1)
{
DataFile<<itgrin->first<<'\n';
scount++;
}

}
}
}			
DataFile<<'\n'<<" Total Count is : "<<scount<<'\n'<<'\n';

DataFile<<" The sequence and mode of access of each global variable w.r.t to the active threads in the system "<<'\n'<<'\n';

int f=0;
int i=1;

for(itg=glist.begin();itg!=glist.end();++itg,i++)
{ 
int icount=0;
for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{
char l='a';
if(*itg==itgrin->first)
{
f++;

icount++;
// DataFile<<icount<< " : "<<"Global Variable "<<itgrin->first<<" Access Sequence Order "<<'\n'<<'\n';

DataFile<<"Global Variable "<<itgrin->first<<" Access Sequence Order "<<'\n'<<'\n';
*outf2<<"\n\n\n"<<i<< " : "<<"Global Variable "<<itgrin->first<<""<<"   Access Sequence Order "<<'\n'<<endl;
	
	

ModelDataFile<<"subgraph cluster_"<<f<<" {"<<'\n';
 ModelDataFile<<"label="<<"D"<<itgrin->first<<'\n';
std::list<struct globalinregion>::iterator littgrin3;
				
for(littgrin3=itgrin->second.begin(); littgrin3!=itgrin->second.end(); ++littgrin3)
{


THREADID temp = littgrin3->tid;
  ADDRINT tempadd = itgrin->first;

  tempit = imap.find(temp);

  if(tempit==imap.end())
  {
    std::list<ADDRINT> tli;
    tli.push_back(tempadd);
    imap.insert(std::pair<THREADID, std::list<ADDRINT> >(temp,tli));
  }  
  else
  {
    for(tempit= imap.begin(); tempit!=imap.end(); tempit++)
    {
      //cout<<temp<<" : "<<tempadd<<endl;
        if(tempit->first==temp)
          {
              tempit->second.push_back(tempadd);
              break;
          }
    }
  }


 if(littgrin3->sync=="MUTEX")
{


ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"M"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';



ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"M"<<littgrin3->mutex_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" with the help of mutex " <<littgrin3->mutex_var <<'\n'<<'\n';
*outf2<<"\n"<< l<<": is accessed by thread "<<littgrin3->tid <<" with the help of mutex " <<littgrin3->mutex_var <<'\n'<<endl;
 l++;

}

 if(littgrin3->sync=="SEMAPHORE")
{


ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"S"<<littgrin3->sem_var<<"_"<<f<<";"<<'\n';



ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"S"<<littgrin3->sem_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" with the help of semaphore " <<littgrin3->sem_var <<'\n'<<'\n';
*outf2<<"\n"<< l<<": is accessed by thread "<<littgrin3->tid <<" with the help of semaphore " <<littgrin3->sem_var <<'\n'<<endl;
 l++;

}

if(littgrin3->sync=="RW_LOCK")
{


ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"R"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';



ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"R"<<littgrin3->mutex_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" with the help of rw_lock" <<littgrin3->mutex_var <<'\n'<<'\n';
*outf2<<"\n"<< l<<": is accessed by thread "<<littgrin3->tid <<" with the help of rw_lock " <<littgrin3->mutex_var <<'\n'<<endl;
 l++;

}

if(littgrin3->sync==" ")
{

ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" without any synchronisation"<<'\n'<<'\n';
*outf2<<"\n"<< l<<": is accessed by thread "<<littgrin3->tid <<" without any synchronisation" <<'\n'<<endl;
l++;
}
if(littgrin3->sync=="BARRIER")
{
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" "<<littgrin3->type<<" the occurance with the help of barrier " <<littgrin3->mutex_var <<'\n'<<'\n';
*outf2<<"\n"<< l<<": is accessed by thread "<<littgrin3->tid <<" with the help of barrier" <<littgrin3->mutex_var <<'\n'<<endl;
if(littgrin3->type=="AFTER")
{
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
}

if(littgrin3->type=="BEFORE")
{
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"B"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataFile<<"B"<<littgrin3->mutex_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
}
l++;
}
				
}

ModelDataFile<<"}"<<'\n';
}
}			
}

}

ModelDataFile<<"}"<<endl;
DataFile<<endl;


}




if (!EnableDataSeqDetection)
{
DataseqFile<<" The Sequence of Data Access of all the global variables in the system "<<'\n'<<'\n';
ModelDataSeqFile<<"digraph G {"<<'\n';	

ModelDataSeqFile<<"node [style=filled,color=blue];"<<'\n';

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{ 
//OutFile<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';		
std::list<THREADID>::iterator litcr;		
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{
//OutFile<<"Thread "<<*litcr<<'\n';
ModelDataSeqFile<<"T"<<itcr->first<<"->"<<"T"<<*litcr<<";"<<'\n';

}	
}




  ModelDataSeqFile<<"node [style=filled,color=green];"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{
for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{
if(*itg==itcgrin->first)
{ 
//OutFile<<"Global Variable "<<itcgrin->first<<"being accessed by   " <<'\n';
ModelDataSeqFile<<"D"<<itcgrin->first<<";"<<'\n';			
std::list<struct cglobalinregion>::iterator littcgrin1;	
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{ 
//if(littcgrin1->count>0)
//OutFile<<" "<<littcgrin1->tid<<" "<<"without synchronisation"<<'\n';
}
}
}
}



ModelDataSeqFile<<"node [style=filled,color=yellow];"<<'\n';
          for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelDataSeqFile<<"M"<<*itm<<";"<<'\n';
}

		
ModelDataSeqFile<<"node [style=filled,color=orange];"<<'\n';     
     for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelDataSeqFile<<"B"<<*itb<<";"<<'\n';
}
int i=1;
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

 DataseqFile<<itgr->first<<" : ";
				
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
 
 
if(littgr1->sync=="MUTEX")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"with the help of mutex "<< littgr1->mutex_var<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';

ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"M"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';

ModelDataSeqFile<<"M"<<littgr1->mutex_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync=="SEMAPHORE")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"with the help of semaphore "<< littgr1->sem_var<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';

ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"M"<<littgr1->sem_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';

ModelDataSeqFile<<"M"<<littgr1->sem_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync=="RW_LOCK")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"with the help of rw_lock "<< littgr1->mutex_var<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';

ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"M"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';

ModelDataSeqFile<<"M"<<littgr1->mutex_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync==" ")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"without synchronisation"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync=="BARRIER")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid << " "
<<littgr1->type<<" the occurance of memory barrier "<< littgr1->mutex_var<<'\n';
if(littgr1->type=="AFTER")
{
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataSeqFile<<"D"<<littgr1->global_var<<"->"<<"B"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->type=="BEFORE")
{
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"B"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"B"<<littgr1->mutex_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

}
				
}
i++;			
}

DataseqFile<<'\n'<<'\n'<<" The waiting time of each thread to acquire a mutex "<<'\n'<<'\n';
ModelDataSeqFile<<"}"<<'\n'<<endl;	

for(itgrcu=countmap.begin(); itgrcu!=countmap.end(); ++itgrcu)

{
int mcount=0;
int lowest=0;
THREADID td;
 DataseqFile<<"Mutex "<<" " <<itgrcu->first<<"has been acquired by "<<'\n';			
std::list<struct countmutexregion>::iterator litttgrcu;

			
for(litttgrcu=itgrcu->second.begin(); litttgrcu!=itgrcu->second.end(); ++litttgrcu)
{


DataseqFile<< "Thread "<<litttgrcu->tid<<"after "<< litttgrcu->count <<" trials "<<'\n';


mcount=litttgrcu->count;

if(lowest==0)
{
lowest=mcount;
td=litttgrcu->tid;
}
else
{
if(mcount<lowest)
{
lowest=mcount;
td=litttgrcu->tid;
}
}


}

DataseqFile<<" This mutex was first acquired by "<<td <<" After "<< lowest <<"trials "<<'\n'<<'\n'; 

}


if(1==1)
DataseqFile<<endl;
}




	if (!EnableThreadDetection)
{

ThreadFile<<" Thread Summary "<<'\n'<<'\n';

ThreadFile<<" The Sequence of Global Variable Access per thread is given below "<<'\n'<<'\n';

ModelThreadFile<<"digraph G {"<<'\n';

int f=0;
for (itth=thlist.begin(); itth!=thlist.end(); ++itth)
{
  ThreadFile<<" Thread " <<*itth <<'\n';
f++;
int i=1;
ModelThreadFile<<"subgraph cluster_"<<f<<" {"<<'\n';
 ModelThreadFile<<"label="<<"T"<<*itth<<'\n';

for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

				
std::list<struct globalregion>::iterator littgr2;
				
for(littgr2=itgr->second.begin(); littgr2!=itgr->second.end(); ++littgr2)
{


if(littgr2->tid==*itth && littgr2->sync=="MUTEX")
{

for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses Global Variable "<<littgr2->global_var<<" with mutex "<<littgr2->mutex_var <<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"M"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';

ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"M"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DG"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
 
i++;
}
}

for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{
ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<" with mutex "<<littgr2->mutex_var <<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"M"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';

ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"M"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
 
i++;
}
}
}




if(littgr2->tid== *itth && littgr2->sync==" ")
{
for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{

ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<"  "<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
i++;
}
}

for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses global Variable "<<littgr2->global_var<<" without synchronisation"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DG"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
i++;
}
}
}


if(littgr2->tid== *itth && littgr2->sync=="BARRIER")
{

for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{
ThreadFile<<i<<" : "<<"Accesses global Variable "<<littgr2->global_var<<"  "<<littgr2->type <<" the occurance of memory barrier "<<littgr2->mutex_var<<"   "<<'\n';
if(littgr2->type=="AFTER")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
if(littgr2->type=="BEFORE")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"B"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
i++;
}
}
for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<"  "<<littgr2->type <<" the occurance of memory barrier "<<littgr2->mutex_var<<"   "<<'\n';
if(littgr2->type=="AFTER")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
if(littgr2->type=="BEFORE")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"B"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
i++;
}
}

}



}
}
ModelThreadFile<<"}";
}

ModelThreadFile<<"}";
ModelThreadFile<<endl;

ThreadFile<<'\n'<<'\n'<<"Thread Statistics"<<'\n'<<'\n';
for (itth=thlist.begin(); itth!=thlist.end(); ++itth)
    {
        thread_data_tsu* tdata = get_tls(*itth);


    ThreadFile<< '\n'<<" Thread   "<<*itth<<'\n';

    ThreadFile<< '\n'<<" OS Thread id "<<tdata->t<<'\n';
   // ThreadFile<<"  The number of routines executed by the thread (only required routines for capturing execution snapshot) are  "<< tdata->_rtncount<<'\n';
    ThreadFile<<" The Routines are(apart from read or write)"<<tdata->rtnlist<<'\n';
   
   ThreadFile<<" This thread was created by  thread "<<tdata->oid<<'\n';
    ThreadFile<<" The number of Memory Reads by this thread are  " <<tdata->_rdcount<<'\n';
    ThreadFile<<" The number of Memory Writes by this thread are  " <<tdata->_wrcount<<'\n';
    ThreadFile<<" The number of child threads created by this thread are  "<<tdata->_fcount<<'\n'<< '\n'<<'\n'; 
         
    }

ThreadFile<<endl;

}
			
if (!EnableMutexDetection)

{
ModelMutexFile<<"digraph G {"<<'\n';

MutexFile<<"Mutex Lock UnLock Sequence "<<'\n'<<'\n';
for(itggrmu=gglobalmutmap.begin(); itggrmu!=gglobalmutmap.end(); ++itggrmu)
{

 				
std::list<struct gmutexregion>::iterator littggrmu;
	MutexFile<<itggrmu->first<<" : ";			
for(littggrmu=itggrmu->second.begin(); littggrmu!=itggrmu->second.end(); ++littggrmu)

{

MutexFile<<"Thread "<<littggrmu->tid <<" "<<littggrmu->occ<<" mutex "<<littggrmu->addr<<'\n';
if(littggrmu->occ=="LOCKS")
{
ModelMutexFile<<"edge[label="<<"E"<<itggrmu->first<<"_"<<"locks]"<<";"<<'\n';
ModelMutexFile<<"T"<<littggrmu->tid<<"->"<<"M"<<littggrmu->addr<<";"<<'\n';

}

if(littggrmu->occ=="UNLOCKS")
{
ModelMutexFile<<"edge[label="<<"E"<<itggrmu->first<<"_"<<"unlocks]"<<";"<<'\n';
ModelMutexFile<<"M"<<littggrmu->addr<<"->"<<"T"<<littggrmu->tid<<";"<<'\n';
}

}

}


ModelMutexFile<<"}"<<'\n'<<endl;
MutexFile<<endl;

}

	if (!EnableExecutionDetection)
{
ModelExecFile<<"digraph G {"<<'\n';
ModelExecFile<<"node [style=filled,color=blue];"<<'\n';

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{ 
std::list<THREADID>::iterator litcr;		
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{
ModelExecFile<<"T"<<itcr->first<<";"<<'\n';
ModelExecFile<<"T"<<*litcr<<";"<<'\n';

}	
}

ModelExecFile<<"node [style=filled,color=red];"<<'\n'; 
for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)
{ 
				
std::list<ADDRINT>::iterator litcc;		

for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

ModelExecFile<<"C"<<*litcc<<";"<<'\n';			
	}	
}

  ModelExecFile<<"node [style=filled,color=green];"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{
for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{
if(*itg==itcgrin->first)
{ 

ModelExecFile<<"D"<<itcgrin->first<<";"<<'\n';		
	std::list<struct cglobalinregion>::iterator littcgrin1;	
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{ 

}
}
}
}

ModelExecFile<<"node [style=filled,color=yellow];"<<'\n';
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelExecFile<<"M"<<*itm<<";"<<'\n';
}

		
ModelExecFile<<"node [style=filled,color=orange];"<<'\n';        
  for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelExecFile<<"B"<<*itb<<";"<<'\n';
}







int i=1;
ADDRINT mut=0;
THREADID td=0;
//ADDRINT bar;
//THREADID 

for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{

 ExecFile<<'\n'<<"Event"<<" "<<it2->first<<" : ";				
std::list<struct threadregion>::iterator lit;
std::list<struct threadregion>::reverse_iterator lit1;
lit=it2->second.begin();
lit1=it2->second.rbegin();
//ExecFile<<lit->cont;
//ExecFile<<lit1->cont;
if(lit->order=="AFTER" && lit->cont=="STARTS" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="STARTS" and lit->node_type==6)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Requests for locking the mutex variable whose address is" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for mutex variable whose address is" and lit->node_type==0)

{
//for(itm=mlist.begin();itm!=mlist.end();++itm)
//{ 
//if(lit1->mutex_var==*itm)
//{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
//}
//}
td=lit->tid;
mut=lit1->mutex_var;

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';

}

if(lit->order=="BEFORE" && lit->cont=="Executes Main" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
 



if(lit->order=="BEFORE" && lit->cont=="Wakes up waiting threads" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Waits for acquiring lock" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="JOINS" and lit->node_type==0)



ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




if(lit->order=="BEFORE" && lit->cont=="Unlocks the mutex variable whose address is" and lit->node_type==0)
{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"unlocks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}

if(lit->order=="BEFORE" && lit->cont=="Unlocks the semaphore variable whose address is" and lit->node_type==0)
{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"unlocks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->sem_var <<"  "<<'\n';
}



if(lit->order=="BEFORE" && lit->cont=="Unlocks the rw_lock variable whose address is" and lit->node_type==0)
{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"unlocks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}



if(lit->order=="BEFORE" && lit->cont=="CREATES" and lit->node_type==0)

{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"creates]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"T"<<lit1->tid<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

}

if(lit->order=="BEFORE" && lit->cont=="Waits over condition variable" and lit->node_type==0)
{

ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"waits]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"C"<<lit1->cond_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->cond_var <<"  "<<'\n';

}


if(lit->order=="BEFORE" && lit->cont=="Signals" and lit->node_type==0)
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"signals]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"T"<<lit1->tid<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';
}


if(lit->order=="BEFORE" && lit->cont=="EXITS" and lit->node_type==0)


ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Writes to variable whose address is" and lit->node_type==0)

{





for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(lit1->global_var==*itg)
{
i++;
if(lit->tid==td)
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<mut<<";"<<'\n';
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelExecFile<<"M"<<mut<<"->"<<"D"<<lit1->global_var<<";"<<'\n';
td=99;
}
else
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelExecFile<<"T"<<lit->tid<<"->"<<"D"<<lit1->global_var<<";"<<'\n';
}
}
}




ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';


}


if(lit->order=="BEFORE" && lit->cont=="Reads from variable whose address is" and lit->node_type==0)


{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"reads]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"D"<<lit1->global_var<<";"<<'\n';

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';

}


if(lit->order=="BEFORE" && lit->node_type==9)



ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="RUNS EXIT HANDLERS" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




//if(lit->order=="BEFORE" && lit->cont=="Exits the Catch Block" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="Catches the Exception" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Unregisters fork handlers" and lit->node_type==0)


ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



//if(lit->order=="BEFORE" && lit->cont=="Completed Exception handling" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



//if(lit->order=="BEFORE" && lit->cont=="Detects Exception" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="Throws Exception" and lit->node_type==0)



//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Puts Memory bar" and lit->node_type==0)


{
//for(itb=blist.begin();itb!=blist.end();++itb)
//{ 
//if(lit1->mutex_var==*itb)
//{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"barriers]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"B"<<lit->global_var<<";"<<'\n';
//}
//}
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var<<'\n';
}


}

if(1==1)
{
ExecFile<<'\n'<<endl;
ModelExecFile<<"}"<<'\n'<<endl;
}
}





/*for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

 cout<< " : "<<"Global Variable "<<itgrin->first<<"Access Sequence Order "<<'\n';
	
std::list<struct globalinregion>::iterator littgrin3;
				
for(littgrin3=itgrin->second.begin(); littgrin3!=itgrin->second.end(); ++littgrin3)
{
 cout<<"thread "<<littgrin3->tid;
cout<<"    type "<<littgrin3->sync;
cout<<"   mutex "<<littgrin3->mutex_var;
cout<<"   sem "<<littgrin3->sem_var;
cout<<"   rw "<<littgrin3->rw_var<<'\n';


}		
}
*/







if(manyP==1)
   { *outf1<<"\nthere are many parents"<<endl;

cout<<"\nthere are many parents"<<endl;}
else if(manyP==0)
{
    *outf1<<"\nthere is a single parent"<<endl;
cout<<"\nthere is a single parent"<<endl;
}
int j= is_join();
if(j==1)
    *outf1<<"\nthere is join"<<endl;
else if(j==0)
    *outf1<<"\nthere is no join"<<endl;

int k= does_update();
if(k==1)
    *outf1<<"\nthere is update"<<endl;
else if(k==0)
    *outf1<<"\nthere is no update"<<endl;


int tf=0;
int update, join;
join = is_join();
update = does_update();
data_access();


{

ofstream visual;

visual.open("Concurrency_model.dot");
visual<< "digraph test{"<<endl;

/*Decision Tree for detecting the thread model*/
visual<<"   Decision1"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual<<"   Decision2"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual<<"   Decision3"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual<<"   Start"<<"[fillcolor = \"sienna\" style = filled];"<<endl;
visual<<"   node1"<<"[fillcolor = \"yellow\" style = filled];"<<endl;
visual<<"   node2"<<"[fillcolor = \"yellow\" style = filled];"<<endl;

visual<<"   Start"<<"-> Decision1"<<"[color = \"green\" label = \"Many Threads create child threads\"];"<<endl;
visual<<"   node1"<<"-> Decision2"<<"[color = \"green\" label = \"Each Thread updates global variables including parent threads\"];"<<endl;
visual<<"   node2"<<"-> Decision3"<<"[color = \"green\" label = \"Thread exits after task with joining\"];"<<endl;

if(manyP==1)
{

    visual<<"   Decision1"<<"-> node1"<<"[color = \"green\" label = \"YES\"];"<<endl;
    if(update==1)
    {

        visual<<"   Decision2"<<"-> node2"<<"[color = \"green\" label = \"YES\"];"<<endl;
        if(join==1)
            { 
                //cout<<"ONE"<<endl;
            visual<<"   Hybrid"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Hybrid"<<"[color = \"green\" label = \"YES\"];"<<endl;
cout <<"\nHybrid model"<<endl;
              *outf1<<"Hybrid"<<endl;
		}
        else
	     {
	       visual<<"   Pipeline"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Pipeline"<<"[color = \"green\" label = \"NO\"];"<<endl;
cout <<"\nPipeline model"<<endl;
            *outf1<<"Pipeline"<<endl;
 		}
    }
    else
    {
        visual<<"   Decision2"<<"-> node2"<<"[color = \"green\" label = \"NO\"];"<<endl;
        if(join==1)
            { 
              visual<<"   Boss_Worker"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Boss_Worker"<<"[color = \"green\" label = \"YES\"];"<<endl;
cout <<"\nBoss_Worker model"<<endl;
              *outf1<<"Boss_Worker"<<endl;
		}
        else
	     {
	       visual<<"   Hybrid"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Hybrid"<<"[color = \"green\" label = \"NO\"];"<<endl;
cout <<"\nHybrid model"<<endl;
            *outf1<<"Hybrid"<<endl;
 		}
    }
}
else
{
    visual<<"   Decision1"<<"-> node1"<<"[color = \"green\" label = \"NO\"];"<<endl;
    if(update==1)
    {
        visual<<"   Decision2"<<"-> node2"<<"[color = \"green\" label = \"YES\"];"<<endl;
        if(join==1)
            { 
                visual<<"   Peer_to_Peer"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Peer_to_Peer"<<"[color = \"green\" label = \"YES\"];"<<endl;
cout <<"\nPeer to Peer model"<<endl;
              *outf1<<"Peer to Peer"<<endl;
		}
        else
	     {
	           visual<<"   Peer_to_Peer"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Peer_to_Peer"<<"[color = \"green\" label = \"NO\"];"<<endl;
cout <<"\nPeer to Peer model"<<endl;
            *outf1<<"Peer to Peer"<<endl;
 		}
    }
    else
    {
        visual<<"   Decision2"<<"-> node2"<<"[color = \"green\" label = \"NO\"];"<<endl;
        if(join==1)
            { 
              visual<<"   Boss_Worker"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Boss_Worker"<<"[color = \"green\" label = \"YES\"];"<<endl;
cout <<"\nBoss_Worker model"<<endl;
              *outf1<<"Boss_Worker"<<endl;
		}
        else
	     {
	       visual<<"   Hybrid"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
            visual<<"   Decision3"<<"-> Hybrid"<<"[color = \"green\" label = \"NO\"];"<<endl;
cout <<"\nHybrid model"<<endl;
            *outf1<<"Hybrid"<<endl;
 		}
    }
}

visual<<"}"<<endl;
visual.close();



}


/*Decision tree to decide whether the program is thread safe or not*/



{

ofstream visual1;

visual1.open("Safety_model.dot");
visual1<< "digraph test{"<<endl;

visual1<<"   Decision1"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual1<<"   Start"<<"[fillcolor = \"sienna\" style = filled];"<<endl;


visual1<<"   Start"<<"-> Decision1"<<"[color = \"green\" label = \"Global Variables Present\"];"<<endl;

int safety = safety_decider();

if(safety==1)
{
    visual1<<"   node1"<<"[fillcolor = \"yellow\" style = filled];"<<endl;
    visual1<<"   Decision1"<<"-> node1"<<"[color = \"green\" label = \"YES\"];"<<endl;
    visual1<<"   Totally_Unsafe"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual1<<"   node1"<<"-> Totally_Unsafe"<<"[color = \"green\" label = \"No Synchronisation\"];"<<endl;
    cout<<"\nThe program is totally thread unsafe"<<endl;
	*outf2<<"\n\nThe program is totally thread unsafe"<<endl;
}
else if(safety==2)
{   
    visual1<<"   node1"<<"[fillcolor = \"yellow\" style = filled];"<<endl;
    visual1<<"   Decision1"<<"-> node1"<<"[color = \"green\" label = \"YES\"];"<<endl;
    visual1<<"   Partially_Safe"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual1<<"   node1"<<"-> Partially_Safe"<<"[color = \"green\" label = \"Some Threads access with Synchronisation\"];"<<endl;
    cout<<"\nThe program is partially thread safe"<<endl;
	*outf2<<"\n\nThe program is partially thread safe"<<endl;
}
else if(safety==3)
{
    visual1<<"   node1"<<"[fillcolor = \"yellow\" style = filled];"<<endl;
    visual1<<"   Decision1"<<"-> node1"<<"[color = \"green\" label = \"YES\"];"<<endl;
    visual1<<"   Totally_Safe"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual1<<"   node1"<<"-> Totally_Safe"<<"[color = \"green\" label = \"All Threads access with Synchronisation\"];"<<endl;
    cout<<"\nhe program is totally thread safe"<<endl;
	*outf2<<"\n\nThe program is totally thread safe"<<endl;
}
else if(safety==4)
{
    visual1<<"   Thread_Safe"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual1<<"   Decision1"<<"-> Thread_Safe"<<"[color = \"green\" label = \"NO\"];"<<endl;
    cout<<"\nNo global variables are present and the program is fully thread safe."<<endl;
    *outf2<<"No global variables are present and the program is fully thread safe."<<endl;
}

visual1<<"}"<<endl;
visual1.close();



}


/*Decision tree fto decide whether there is starvation among threads*/



{

ofstream visual2;

visual2.open("Starvation_model.dot");
visual2<< "digraph test{"<<endl;

visual2<<"   Decision1"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual2<<"   Start"<<"[fillcolor = \"sienna\" style = filled];"<<endl;


visual2<<"   Start"<<"-> Decision1"<<"[color = \"green\" label = \"Threads acquire mutex after more than "<<limit<<"trials\"];"<<endl;

int isStarving = starvation_decider();

if(isStarving==1)
{ 
    visual2<<"   Eventual_Starvation"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual2<<"   Decision1"<<"-> Eventual_Starvation"<<"[color = \"green\" label = \"YES\"];"<<endl; 
    cout<<"\nEventual starvation among specific threads"<<endl;
	*outf3<<"\n\nEventual starvation among specific threads"<<endl;
}
else if(isStarving==2)
{
    visual2<<"   No_Starvation"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual2<<"   Decision1"<<"-> No_Starvation"<<"[color = \"green\" label = \"NO\"];"<<endl; 
    cout<<"\nNo starvation"<<endl;
	*outf3<<"\n\nNo starvation"<<endl;
}
else
{
    visual2<<"   Eventual_Starvation"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual2<<"   Decision1"<<"-> Eventual_Starvation"<<"[color = \"green\" label = \"YES\"];"<<endl; 
    cout<<"\nEventual starvation among specific threads"<<endl;
	*outf3<<"\n\nEventual starvation among specific threads"<<endl;
}

visual2<<"}"<<endl;
visual2.close();



}

/*Decision tree to decide whether there is potential chance for live lock in the program*/



{

ofstream visual3;

visual3.open("Livelock_model.dot");
visual3<< "digraph test{"<<endl;

visual3<<"   Decision1"<<"[shape = diamond fillcolor = \"white\" style = filled];"<<endl;
visual3<<"   Start"<<"[fillcolor = \"sienna\" style = filled];"<<endl;


visual3<<"   Start"<<"-> Decision1"<<"[color = \"green\" label = \"Threads use try_lock and acquire mutex after more than "<<limit<<"trials\"];"<<endl;

int isLiveLock = live_lock_decider();
if(isLiveLock==1)
{
    visual3<<"   Potential_LiveLock"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual3<<"   Decision1"<<"-> Potential_LiveLock"<<"[color = \"green\" label = \"YES\"];"<<endl; 
    cout<<"\nPotential chance of live lock"<<endl;
    *outf4<<"\n\n\nPotential chance of live lock"<<endl;
}
else
{
    visual3<<"   No_Livelock"<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
    visual3<<"   Decision1"<<"-> No_Livelock"<<"[color = \"green\" label = \"NO\"];"<<endl;
    cout<<"\nNo live lock"<<endl;
    *outf4<<"\n\n\nNo live lock"<<endl;
}

visual3<<"}"<<endl;
visual3.close();



}

for(jq=joinmap.begin(); jq!=joinmap.end(); jq++) {

 //cout << "jq->first" << jq->first <<'\n';
 //cout << "jq->second.exit" << jq->second.exit <<'\n';
 //cout << "jq->second.join" << jq->second.join <<'\n';
     
    //join++;
 }


{
for (int i = 0; i < NTHREADS; i++)
  for (int j = 0; j < 9; j++)
    dataarray[i][j] = 0.25;


int thc = total_thread_counter();
//itcr = createmap.begin();
cout << "THC" <<thc<<'\n';
//itth=thlist.begin();itth!=thlist.end();++itth
for(int j=0;j< thc;j++)
   {
   
   
  for(int z=0; z< 1; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
           //cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
   
          dataarray[j][z] = thread_create_feature(*itth);
         // cout << t << '\n';
         cout << "INDI CREATE" << dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}





for(int j=0;j< NTHREADS;j++)
   {
   
   
  for(int z=1; z< 2; z++)
   {
       
         //  itth=thlist.begin();
         //  std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
   
          dataarray[j][z] = thread_create();
         // cout << t << '\n';
       cout << "TTAL CREATE" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}




for(int j=0;j< thc;j++)
   {
   
   
  for(int z=2; z< 3; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
          
          dataarray[j][z] = thread_signal_feature(*itth);
         // cout << t << '\n';
       cout << "SIGNAL" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}


for(int j=0;j< thc;j++)
   {
   
   
  for(int z=3; z< 4; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";

          dataarray[j][z] = thread_cond(*itth);
         // cout << t << '\n';
       cout << "COND" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}

for(int j=0;j< thc;j++)
   {
   
   
  for(int z=4; z< 5; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
  
          dataarray[j][z] = global_updates_thread(*itth);
         // cout << t << '\n';
       cout << "GLOBAL" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}



for(int j=0;j< NTHREADS;j++)
   {
   
   
  for(int z=5; z< 6; z++)
   {
       
          // itth=thlist.begin();
          // std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
  
          dataarray[j][z] = count_globals();
         // cout << t << '\n';
       cout << "TOTAL GLOBAL" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}
 
 
 for(int j=0;j< thc;j++)
   {
   
   
  for(int z=6; z< 7; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
          
          dataarray[j][z] = total_start_time(*itth);
         // cout << t << '\n';
       cout << "START" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}

for(int j=0;j< thc;j++)
   {
   
   
  for(int z=7; z< 8; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
       
          dataarray[j][z] = total_exit_time(*itth);
         // cout << t << '\n';
       cout << "EXIT" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}


for(int j=0;j< thc;j++)
   {
   
   
  for(int z=8; z< 9; z++)
   {
       
           itth=thlist.begin();
           std::advance (itth,j);
         //  cout << "ITCR_>FIRST" << *itth <<'\n';
          //cout << "Hii i am here";
          
          dataarray[j][z] = thread_join_counts(*itth);
         // cout << t << '\n';
       cout << "JOIN" <<dataarray[j][z] << '\n';
          //itcr++; 
          
          
       
       }

}

//FILE * fp; 	

ofstream myfile;

//filename = strcat(filename, ".csv");
//char *str[200];
//str = strcat(str, ".csv");
//fp =fopen("output3.csv", "a+");
myfile.open ("output5.csv");
myfile<<"\n";
//for (int i = 0; i < NTHREADS; i++)
//{
 // for (int j = 0; j < 9; j++)
 // {
      for (int i = 0; i < NTHREADS; i++){
      myfile << dataarray[i][0] <<"," << dataarray[i][1] << "," << dataarray[i][2] << "," << dataarray[i][3] << "," << dataarray[i][4] << "," << dataarray[i][5] << "," << dataarray[i][6] << "," << dataarray[i][7] << "," << dataarray[i][8] << ",";
      
      }
     //myfile.close();
  //}
//}
myfile<<"\n";
myfile.close();
    //fprintf(fp, ",%f ", dataarray[i][j]);
    //TrainFile << dataarray[i][j] << endl;
    //fprintf(fp, "\n");
 //fclose(fp);


//ofstream myfile;
     // myfile.open ("example.csv");
     // myfile << "This is the first cell in the first column.\n";
     // myfile << "a,b,c,\n";
     // myfile << "c,s,v,\n";
     // myfile << "1,2,3.456\n";
     // myfile << "semi;colon";
     // myfile.close();
}

//cout << "Displaying start sequence map";
//cout <<"-------------------------";

for(sq = startmap.begin(); sq!= startmap.end(); ++sq)
{

   // cout << sq->first <<'\n';
    // cout << sq->second <<'\n';
    //cout<< "START COUNT"<< start_count <<'\n';
    
  }

//cout << " Displaying Signal Map " << '\n';
//cout << "-------------------------------" << '\n';

cout << " Displaying GlobalMut Map " << '\n';

for(itgrmu = globalmutmap.begin(); itgrmu!= globalmutmap.end(); ++itgrmu)
{

cout << " First Element of globalmut Map " << '\n' << itgrmu->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<struct mutexregion>::iterator globalmut_2nd_itr;
               
for(globalmut_2nd_itr=itgrmu->second.begin(); globalmut_2nd_itr!=itgrmu->second.end(); ++globalmut_2nd_itr)
{

cout<<"flag "<<globalmut_2nd_itr->flag<<'\n';
cout<<"addr "<<globalmut_2nd_itr->addr<<'\n';
cout<<"thread "<<globalmut_2nd_itr->tid<<'\n';


}

}

for(itsr = signalmap.begin(); itsr!= signalmap.end(); ++itsr)
{

//cout << " First Element of Signal Map " << '\n' << itsr->first <<'\n';

//cout <<" ----- The second element lists are -----"<<'\n';               
std::list<THREADID>::iterator signal_2nd_itr;
               
for(signal_2nd_itr=itsr->second.begin(); signal_2nd_itr!=itsr->second.end(); ++signal_2nd_itr)
{

//cout<<"Thread "<<*signal_2nd_itr<<'\n';
}

}








printf("\nYou have chosen to detect thread model.\n Enter 1 for Concurrency Model. \n Enter 2 for Safety Model. \n Enter 3 for Starvation Model. \n Enter 4 for Livelock Model.\n Enter 5 to get all of the above.\n");
scanf("%d", &tf);


if (tf==1 || tf==5)
{
FILE *fp1 = popen("./graphviz1.sh","r");

                        fp1++;
}

if (tf==2 || tf==5)
{
FILE *fp1 = popen("./graphviz2.sh","r");

                        fp1++;
}

if (tf==3 || tf==5)
{
FILE *fp2 = popen("./graphviz3.sh","r");

                        fp2++;
}

if (tf==4 || tf==5)
{
FILE *fp3 = popen("./graphviz4.sh","r");

                        fp3++;
}





//cout << " *Displaying Create Map* " << '\n'<< '\n';
//cout << "-------------------------------" << '\n';

//double 

/*for(itcr = createmap.begin(); itcr!= createmap.end(); ++itcr)
{

cout << " First Element of Create Map " << '\n' << itcr->first << '\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<THREADID>::iterator create_2nd_itr;
               
for(create_2nd_itr=itcr->second.begin(); create_2nd_itr!=itcr->second.end(); ++create_2nd_itr)
{

cout<<"Thread "<<*create_2nd_itr<<'\n';
}

}*/



cout << " *Displaying globalin Map " << '\n' ;
cout << "-------------------------------" << '\n';


for(itgrin = globalinmap.begin(); itgrin!= globalinmap.end(); ++itgrin)
{

cout << " First Element of globalin Map " << '\n' << itgrin->first << '\n';
//cout << "Routine" << RTN_FindNameByAddress(6295584);

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<struct globalinregion>::iterator globalin_2nd_itr;
               
for(globalin_2nd_itr=itgrin->second.begin(); globalin_2nd_itr!=itgrin->second.end(); ++globalin_2nd_itr)
{

cout<<"Thread "<<globalin_2nd_itr->tid<<'\n';
//cout<<"global_variable "<<globalin_2nd_itr->global_var<<'\n';
//cout<<"mutex_variable "<<globalin_2nd_itr->mutex_var<<'\n';
//cout<<"sem_variable "<<globalin_2nd_itr->sem_var<<'\n';
//cout<<"rw_variable "<<globalin_2nd_itr->rw_var<<'\n';
//cout<<"sync "<<globalin_2nd_itr->sync<<'\n';
//cout<<"type "<<globalin_2nd_itr->type<<'\n';
}

}


cout << " *Displaying local map" << '\n' ;
cout << "-------------------------------" << '\n';
for(itl = lmap.begin(); itl!= lmap.end(); ++itl)
{

cout << " First Element of lmap Map " << '\n'<< itl->first <<'\n';

cout <<" ----- The second element lists are -----"<<'\n';               
std::list<THREADID>::iterator lmap_2nd_itr;
               
for(lmap_2nd_itr=itl->second.begin(); lmap_2nd_itr!=itl->second.end(); ++lmap_2nd_itr)
{

cout<<"lmap "<<*lmap_2nd_itr<<'\n';





}

}


cout << "*Displaying glist " << '\n';
cout << "-------------------------------" << '\n';
//cout << sizeof(int) <<"SIZEOF";

for(itg=glist.begin();itg!=glist.end();++itg)
{

cout <<"global "<<*itg << '\n';

}

for(sq = startmap.begin(); sq!= startmap.end(); ++sq)
{

    cout << "SQ 1st " << sq->first << '\n';
    cout << "SQ 2nd " << sq->second << '\n';
    
    }
 fprintf(trace, "#eof\n");
    fclose(trace);

}



INT32 Usage()
{
    PIN_ERROR("This Pintool prints the IPs of every instruction executed\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();


    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if(PIN_Init(argc,argv))
    {
        return Usage();
    }

   // const string fileName = KnobOutputFile.Value();

   // if (!fileName.empty())
    //{
    //    out = new std::ofstream(fileName.c_str());
   // }
    PIN_InitLock(&lock);
    OutFile.open(KnobOutputFile.Value().c_str());
    outf5 = new std::ofstream("Model5seq.out");
    outf4 = new std::ofstream("Model4live.out");
    outf3 = new std::ofstream("Model3starve.out");
    outf2 = new std::ofstream("Model2safety.out");
    outf1 = new std::ofstream("Model1conc.out");
 trace = fopen("itrace1.out", "w");
     
    MutexFile.open(KnobMutexOutputFile.Value().c_str());
     TrainFile.open(KnobTrainFile.Value().c_str());
    ExceptionFile.open(KnobExceptionOutputFile.Value().c_str());
    ModelMutexFile.open(KnobModelMutexOutputFile.Value().c_str());
    ExecFile.open(KnobExecutionOutputFile.Value().c_str());
    DataFile.open(KnobDataOutputFile.Value().c_str());
    ModelDataFile.open(KnobModelDataOutputFile.Value().c_str());
    DataseqFile.open(KnobDataSeqOutputFile.Value().c_str());
    ModelMapFile.open(KnobModelMapOutputFile.Value().c_str());

    ModelThreadFile.open(KnobModelThreadOutputFile.Value().c_str());
    ThreadFile.open(KnobThreadOutputFile.Value().c_str());
    ModelDataSeqFile.open(KnobModelDataSeqOutputFile.Value().c_str());
 RoutineFile.open(KnobRoutineFile.Value().c_str());
 FeatureFile.open(KnobFeatureFile.Value().c_str());
 
  //AddrOutputFile.open(KnobAddrOutputFile.Value().c_str());

    ModelExecFile.open(KnobModelExecutionOutputFile.Value().c_str());
    // Obtain  a key for TLS storage.
    tls_key = PIN_CreateThreadDataKey(0);


    PIN_AddDebugInterpreter(DebugInterpreter, 0);
    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(Routine, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, NULL);
       


    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;

}



