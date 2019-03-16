#include "memtracker.h"
#include <map>

std::map <ADDRINT, INSINFO> inslist;
std::map <std::string, int> locks;
std::map <int, std::string> unlocks;
std::map <ADDRINT, std::map<ADDRINT, int>> syncs;

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
	std::cout << "THREADID: " << tid << "\n";
	inslist[ina].print();
	std::cout << "SYNC: ";
	if (syncs.find(tid) != syncs.end())
	{
		std::cout << "Has synchronous access with the following locks:\n";
		for (auto l: syncs[tid])
			std::cout << "0x" << hex << l.first << " as " << unlocks[l.second] << "\n\n";
	}
	else
		std::cout << "Has asynchronous access.\n\n";
}

VOID callP (THREADID tid, ADDRINT ina)
{
	std::cout << "THREADID: " << tid << "\n";
	std::cout << "MILA MILA!!!!! CHAUTHA!!\n";
	inslist[ina].print();
	std::cout << "\n\n";
}

VOID Instruction(INS ins, VOID * v)
{
	ADDRINT ina = INS_Address(ins);
	int column, line;
	std::string fname;

	PIN_LockClient();
	PIN_GetSourceLocation(ina, &column, &line, &fname);
	PIN_UnlockClient();

	// if (INS_IsDirectCall(ins))
	// {
	// 	inslist[ina] = INSINFO(ins, fname, column, line);
	// 	if (inslist[ina].target != "")
	// 		INS_InsertCall(
	// 		ins, IPOINT_BEFORE, (AFUNPTR)callP,
	// 		IARG_THREAD_ID, IARG_ADDRINT, ina,
	// 		IARG_END);
	// }

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

VOID lock_func_bf (THREADID tid, ADDRINT addr, int type, ADDRINT rta)
{
	PIN_LockClient();
	RTN rtn = RTN_FindByAddress(rta);
	RTN_Open(rtn);
	INS ins = RTN_InsHead(rtn);
	RTN_Close(rtn);
	PIN_UnlockClient();
	INSINFO l(ins);
	std::cout << "MILA MILA!!!!! DOOSRA!!\n";
	l.print();
	syncs[tid][addr] = type;
	std::cout << "setting syncs " << tid << " " << addr << "\n\n";
}

// VOID lock_func_af (THREADID tid, ADDRINT addr, int type, ADDRINT rta)
// {
// 	PIN_LockClient();
// 	RTN rtn = RTN_FindByAddress(rta);
// 	RTN_Open(rtn);
// 	INS ins = RTN_InsHead(rtn);
// 	RTN_Close(rtn);
// 	PIN_UnlockClient();
// 	INSINFO l(ins);
// 	std::cout << "MILA MILA!!!!! DOOSRA!!\n";
// 	l.print();
// 	std::cout << "\n\n";

// 	syncs[tid][addr] = type;
// 	std::cout << "setting syncs " << tid << " " << addr << "\n";
// }

VOID unlock_func(THREADID tid, ADDRINT addr, ADDRINT rta)
{
	PIN_LockClient();
	RTN rtn = RTN_FindByAddress(rta);
	RTN_Open(rtn);
	INS ins = RTN_InsHead(rtn);
	RTN_Close(rtn);
	PIN_UnlockClient();
	INSINFO l(ins);
	std::cout << "MILA MILA PEHLA!!!!!\n";
	l.print();
	std::cout << "\n\n";

	if (syncs.find(tid) == syncs.end())
		std::cout << "DAFUQ!!\n";
	else
	{
		auto loc = syncs[tid].find(addr);
		syncs[tid].erase(loc);
		if (syncs[tid].size() == 0)
			syncs.erase(syncs.find(tid));
		std::cout << "unsetting syncs " << tid << " " << addr << "\n";		
	}
}

VOID Routine (RTN rtn, VOID * v)
{
	std::string rname = RTN_Name(rtn), pat = "";
	if (rname == "pthread_mutex_lock@plt")
		pat = "mutex";
	else if (rname == "sem_wait")
		pat = "semaphore";
	else if (rname == "__pthread_rwlock_rdlock")
		pat = "reader lock";
	else if (rname == "__pthread_rwlock_wrlock")
		pat = "writer lock";

	if (pat != "")
	{
		// std::cout << "gotcha: " << hex << RTN_Address(rtn) << " " << rname << " " << pat << "\n";
		RTN_Open(rtn);
		RTN_InsertCall(rtn, IPOINT_BEFORE,
			(AFUNPTR)lock_func_bf,
			IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_UINT64, locks[pat], IARG_INST_PTR,
			IARG_END);
		// RTN_InsertCall(rtn, IPOINT_AFTER,
		// 	(AFUNPTR)lock_func_af,
		// 	IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
		// 	IARG_UINT64, locks[pat], IARG_INST_PTR,
		// 	IARG_END);
		RTN_Close(rtn);
	}

	if (rname == "pthread_mutex_unlock@plt" || rname == "thread_rwlock_unlock" || rname == "sem_post")
	{
	 	//std::cout << "gotcha: " << hex << RTN_Address(rtn) << " " << rname << "\n";
	 	RTN_Open(rtn);
		RTN_InsertCall (rtn, IPOINT_BEFORE,
			(AFUNPTR) unlock_func,
			IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_INST_PTR, IARG_END);
		RTN_Close(rtn);
	}
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

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


