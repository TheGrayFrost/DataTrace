#include "memtracker.h"
#include <map>
#include <list>
#include <stack>

#define HISTLEN 1000

using namespace std;

// reduce memory requirements later
map <ADDRINT, INSINFO> gmap;			// instruction saves during analysis for recall during instrumentation
map <ADDRINT, int> globals;				// global address -> variable index
map <pair<ADDRINT, int>, int> locals; 	// local address + stack no. -> variable index

// need not be optimized
stack <int> stacks;						// current stack structure

// try to remove now
list <INSINFO> history;
// using
// map <string, vector<VARINFO>> cRegs;

int stno = 0, countG = 0, countL = 0;

VOID dataman (ADDRINT ina, ADDRINT memOp)
{
	INSINFO& r = gmap[ina];
	if (r.diss == "push rbp")
		stacks.push(++stno);
	else if (r.diss == "ret")
		stacks.pop();

	r.memOp = memOp;
	
	if (r.flag != 'n')
	{
		// variable update
		r.vars.push_back(VARINFO(r.extract, r.memOp, stacks.top()));
		
		// backtrack
		auto m = parse(r.diss);
		int idx = 1;
		if (m[2].find("ptr") != -1)
			idx = 2;
		auto e = membreak(m[idx]);		
		auto f = e;
		f.insert(globstop.begin(), globstop.end());

		auto curP = history.begin();
		while (f != globstop)
		{
			for (auto reg: curP->regW)
			{
				auto it = e.find(reg);
				if (it != e.end())
				{
					e.erase(it);
					for (auto var: curP->regR)
					{
						e.insert(var);
						if (var == "rbp" || var == "rip")
							r.vars.insert(r.vars.end(), curP->vars.begin(), curP->vars.end());
					}
					break;
				}
			}
			
			f = e;
			f.insert (globstop.begin(), globstop.end());
			++curP;
		}

		// identifying variables
		if (e.find("rip") != e.end())
		{
			if (globals.find(r.memOp) == globals.end())
				globals[r.memOp] = ++countG;
			r.vars.begin()->varname = "G_" + to_string(globals[r.memOp]);
		}
		else
		{
			auto k = make_pair(r.memOp, stacks.top());
			if (locals.find(k) == locals.end())
				locals[k] = ++countL;
			r.vars.begin()->varname = "L_" + to_string(locals[k]);
		}
	}

	history.push_front(r);
	cout << "0x" << hex << ina << ": ";
	gmap[ina].print();
}

map <string, vector<string>> cRegs;

void print (set <string> u)
{
	for (auto l: u)
		cout << l << ". ";
	cout << "\n";
}

VOID Instruction(INS ins, VOID * v)
{
	ADDRINT ina = INS_Address(ins);
	string name = RTN_FindNameByAddress(ina);
	if (name.length() == 0 || isStopWord(name))
		return;

	int column, line;
	string filename;
	PIN_LockClient();
	PIN_GetSourceLocation(ina, &column, &line, &filename);
	PIN_UnlockClient();

	if(filename.length() > 0)
	{
		//get instruction info
		INSINFO u(ins);
		u.print();

		set <string> der;
		string v;

		if (u.flag == 'w')
		{
			v = u.mem;
			for (auto l: u.regR)
				if (globstop.find(l) == globstop.end())
					der.insert(l);
		}
		else if (u.regW.size() == 1)
		{
			v = *u.regW.begin();
			if (v.find("flag") == -1 && globstop.find(v) == globstop.end())
			{
				for (auto l: u.regR)
				{
					if (globstop.find(l) != globstop.end())
						der.insert(u.extract);
					else
						der.insert(l);
					if (u.mem != "")
						der.insert(u.mem);
				}
			}
		}
		else if (u.regW.size() > 1)
		{
			for (auto x: u.regW)
				if (x.find("flag") == -1 && globstop.find(x) == globstop.end())
					cout << x << " cleared\n\n";
		}

		if (der.size())
		{
			cout << v << " derives from ";
			print(der);
			cout << "\n";
		}
		der.clear();
		//gmap[ina] = u;


		// if (u.flag == 'r' || u.flag == 'w')
		// 	INS_InsertCall(
		// 	ins, IPOINT_BEFORE, (AFUNPTR)dataman,
		// 	IARG_ADDRINT, ina,
		// 	IARG_MEMORYOP_EA, 0,
		// 	IARG_END);
		// else if (u.flag == 'i')
		// 	INS_InsertCall(
		// 	ins, IPOINT_AFTER, (AFUNPTR)dataman,
		// 	IARG_ADDRINT, ina,
		// 	IARG_REG_VALUE, INS_RegW(ins, 0),
		// 	IARG_END);
		// else
		// 	INS_InsertCall(
		// 	ins, IPOINT_BEFORE, (AFUNPTR)dataman,
		// 	IARG_ADDRINT, ina,
		// 	IARG_ADDRINT, -1,
		// 	IARG_END);
	}
}

INT32 Usage()
{
	cerr << "This is the invocation plong long intool" << endl;
	cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
	return -1;
}


int main(int argc, char * argv[])
{
	// Initialize pin & symbol manager
	if (PIN_Init(argc, argv))
		return Usage();
	PIN_InitSymbols();

	INS_AddInstrumentFunction(Instruction, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


