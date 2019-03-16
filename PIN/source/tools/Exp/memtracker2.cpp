#include "memtracker.h"
#include <map>
#include <fstream>

std::map <ADDRINT, INSINFO> inslist;
std::map <std::string, int> locks;
std::map <int, std::string> unlocks;
std::map <ADDRINT, std::map<ADDRINT, int>> syncs;
std::ofstream outp[10];

VOID init()
{
	locks["mutex"] = 1;
	locks["semaphore"] = 2;
	locks["reader lock"] = 3;
	locks["writer lock"] = 4;
	unlocks[1] = "mutex";
	unlocks[2] = "semaphore";
	unlocks[3] = "reader lock";
	unlocks[4] = "writer lock";
}

VOID dataman (THREADID tid, ADDRINT ina, ADDRINT memOp)
{
	inslist[ina].memOp = memOp;
	outp[tid] << "THREADID: " << tid << "\n";
	inslist[ina].print(outp[tid]);
	outp[tid] << "SYNC: ";
	if (syncs.find(tid) != syncs.end())
	{
		outp[tid] << "Has synchronous access with the following locks:\n";
		for (auto l: syncs[tid])
			outp[tid] << hex << l.first << " as " << unlocks[l.second] << "\n";
	}
	else
		outp[tid] << "Has asynchronous access.\n";
	outp[tid] << "\n";
	// inslist[ina].shortPrint();
}

VOID Instruction(INS ins, VOID * v)
{
	ADDRINT ina = INS_Address(ins);
	int column, line;
	std::string fname;

	PIN_LockClient();
	PIN_GetSourceLocation(ina, &column, &line, &fname);
	PIN_UnlockClient();

	if (fname.length() > 0)
	{
		if (inslist.find(ina) == inslist.end())
			inslist[ina] = INSINFO(ins, fname, column, line);

		if (inslist[ina].flag != 'n')
			INS_InsertCall(
			ins, IPOINT_BEFORE, (AFUNPTR)dataman,
			IARG_THREAD_ID, IARG_ADDRINT, ina,
			IARG_MEMORYOP_EA, 0,
			IARG_END);
		else
			INS_InsertCall(
			ins, IPOINT_BEFORE, (AFUNPTR)dataman,
			IARG_THREAD_ID, IARG_ADDRINT, ina,
			IARG_ADDRINT, -1,
			IARG_END);
	}
}

VOID lock_func_af (THREADID tid, ADDRINT addr, int type)
{
	syncs[tid][addr] = type;
	cout << "setting syncs " << tid << " " << addr << "\n";
}

VOID unlock_func(THREADID tid, ADDRINT addr)
{
	if (syncs.find(tid) == syncs.end())
		cout << "DAFUQ!!\n";
	else
	{
		auto loc = syncs[tid].find(addr);
		syncs[tid].erase(loc);
		if (syncs[tid].size() == 0)
			syncs.erase(syncs.find(tid));
		cout << "unsetting syncs " << tid << " " << addr << "\n";		
	}
}

VOID Routine (RTN rtn, VOID * v)
{
	std::string rname = RTN_Name(rtn), pat = "";
	if (rname == "pthread_mutex_lock")
		pat = "mutex";
	else if (rname == "sem_wait")
		pat = "semaphore";
	else if (rname == "__pthread_rwlock_rdlock")
		pat = "reader lock";
	else if (rname == "__pthread_rwlock_wrlock")
		pat = "writer lock";

	if (pat != "")
	{
		cout << "gotcha: " << hex << RTN_Address(rtn) << " " << rname << " " << pat << "\n";
		RTN_Open(rtn);
		RTN_InsertCall(rtn, IPOINT_AFTER,
			(AFUNPTR)lock_func_af,
			IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_ADDRINT, locks[pat],
			IARG_END);
		RTN_Close(rtn);
	}

	if (rname == "pthread_mutex_unlock" || rname == "__pthread_rwlock_unlock" || rname == "sem_post")
	{
	 	cout << "gotcha: " << hex << RTN_Address(rtn) << " " << rname << "\n";
	 	RTN_Open(rtn);
		RTN_InsertCall (rtn, IPOINT_BEFORE,
			(AFUNPTR) unlock_func,
			IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_END);
		RTN_Close(rtn);
	}
}

VOID ThreadStart (THREADID tid, CONTEXT * ctxt, INT32 flags, VOID * v)
{
	//cout << tid << "\n";
	outp[tid].open(("Thread" + decstr(tid) + ".dump").c_str());
	//outp[tid].open("ok.txt");
	//cout << "Opened file.\n";
}

VOID ThreadFini (THREADID tid, const CONTEXT * ctxt, INT32 flags, VOID * v)
{
	outp[tid].close();
}


int main(int argc, char * argv[])
{
	// Initialize pin & symbol manager
	PIN_Init(argc, argv);
	PIN_InitSymbols();

	// Instruction level Instrumentation
	init();
	RTN_AddInstrumentFunction(Routine, 0);
	INS_AddInstrumentFunction(Instruction, 0);
	PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


