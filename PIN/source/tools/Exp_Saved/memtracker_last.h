#ifndef __MEMTRACKER_H__
#define __MEMTRACKER_H__

#include "pin.H"
#include <iostream>
#include <string>
#include <set>
#include <sstream>
#include <iterator>

string stopinit[] = {"_fini", "_libc_csu_init", "frame_dummy", "register_tm_clones", "wctype_l", ".text", "__get_cpu_features",
					"explicit_bzero", "vprintf", "brk", "exit", "_exit", "deregister_tm_clones", "_Exit", "calloc", "malloc",
					"_start", "_init", "wctob", "btowc", "on_exit", "gnu_dev_makedev", "fwrite", "uselocale"};
string stopcheck[] = {"__", "@", ".", "_dl", "_IO"};
string lp[] = {"rip", "rbp", "rsp"};
int lenstopin = sizeof(stopinit)/sizeof(*stopinit);
int lenstopch = sizeof(stopcheck)/sizeof(*stopcheck);
int lenlp = sizeof(lp)/sizeof(*lp);
set <string> stop1 (stopinit, stopinit + lenstopin);
vector <string> stop2 (stopcheck, stopcheck + lenstopch);
set <string> globstop (lp, lp + lenlp);

// checks if routine is a common routine
bool isStopWord (string rtn_name)
{
	if (stop1.find(rtn_name) != stop1.end())
		return true;
	for (auto u: stop2)
		if (rtn_name.find(u) != -1)
			return true;
	return false;
}

// converts hex into to string
string to_string (ADDRINT u)
{
	stringstream ss;
	ss << hex << u;
	return ss.str();
}

// breaks memory access into list of registers involved
// dword ptr [rcx+rax*1] -> (rcx, rax)
set <string> membreak (string ma)
{
	set <string> u;
	int l = ma.find('[');
	string k = ma.substr(l+1, ma.length()-l-1);
	const char * str2 = k.c_str();
	char str[40];
	strcpy (str, str2);
	char * pch = strtok(str, "+-*");
	while (pch != NULL)
	{
		if (*pch >= 'a' && *pch <= 'z')
			u.insert(string(pch));
		pch = strtok (NULL, "+-*");
	}
	return u;
}

// parses disassembly into opcode and operands
// mov dword ptr [rcx+rax*1], edx -> (mov, dword ptr [rcx+rax*1], edx)
vector <string> parse (string& diss)
{
	vector <string> ops;
	int l = diss.find(" ");
	ops.push_back(diss.substr(0, l));
	if (l == -1)
		return ops;
	int r = diss.find(",");
	if (r == -1)
		ops.push_back(diss.substr(l+1, r));
	else
	{
		ops.push_back(diss.substr(l+1, r-l-1));
		ops.push_back(diss.substr(r+2, -1));
	}
	return ops;
}

// extract relative address part from memory addresses
// Examples:
// dword ptr [rbp-0x8] -> rbp-0x8
// ptr [rip+0x200859] -> rip+0x200859
// dword ptr [rbp+rax*4-0xfbc] -> rbp-0xfbc
// ptr [rax+rax*1] -> rax+rax*1
string extracter (string& diss)
{
	auto k = parse(diss);
	int idx = 1;
	if (k[2].find("ptr") != -1)
		idx = 2;
	string dpart = k[idx];
	string u;
	int l = dpart.find('[');

	if (dpart.find("rbp") != -1)
	{	
		int r = dpart.find('+');
		if (r != -1)
		{
			u = dpart.substr(l+1, r-l-1);
			r = dpart.find('-');
			u = u + dpart.substr(r, dpart.length()-r-1);
		}
		else
			u = dpart.substr(l+1, dpart.length()-l-2);
	}
	else
		u = dpart.substr(l+1, dpart.length()-l-2);
	return u;
}



// structure to hold variable information
struct VARINFO
{
//to be removed later
	string relAdd; 		// rbp/rip relative address
//to be removed later
	ADDRINT absAdd;		// absolute address in memory
	int stloc;			// stack in which it was first used/created
	string varname;		// variable name for humans to identify

	VARINFO() {};
	VARINFO(string rA, ADDRINT aA, int st)
	{relAdd = rA; absAdd = aA; stloc = st;}
	void print() {cout << stloc << " -> " << relAdd << ":0x" << hex << absAdd << " " << varname << "\n";}
};

// structure to hold instruction information
struct INSINFO
{
	string rtnName;				// parent routine
	string diss;				// instruction assembly
	char flag;					// instruction type: 'r/w'->memory read/write, 'i'-> rip relative value reference, 'n'-> none
	vector <string> regR, regW;	// registers read from and written to
	string extract;				// memory extract
	vector <VARINFO> vars;		// list of variables involved as accessed from extract
	ADDRINT memOp;				// actual memory reference in case of flag != 'n'

	INSINFO() {};
	INSINFO(INS&);
	void print();
};

INSINFO::INSINFO(INS& ins)
{
	diss = INS_Disassemble(ins);
	transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
	
	rtnName = RTN_FindNameByAddress(INS_Address(ins));
	bool memR = INS_IsMemoryRead(ins) && (diss.find("ptr") != -1);
	bool memW = INS_IsMemoryWrite(ins) && (diss.find("ptr") != -1);

	int rreg = INS_MaxNumRRegs(ins);
	int wreg = INS_MaxNumWRegs(ins);
	for (int i = 0; i < rreg; ++i)
	{
		regR.push_back(REG_StringShort(INS_RegR(ins, i)));
		if (REG_is_seg(INS_RegR(ins, i)))
			memR = memW = false;
	}
	for (int i = 0; i < wreg; ++i)
	{
		regW.push_back(REG_StringShort(INS_RegW(ins, i)));
		if (REG_is_seg(INS_RegW(ins, i)))
			memR = memW = false;
	}

	if (memR || memW || diss.find("rip") != -1)
		extract = extracter(diss);
	
	if (memR)
		flag = 'r';	//memory read
	else if (memW)
		flag = 'w';	//memory write
	else if (diss.find("rip") != -1)
		flag = 'i';	//ip-relative memory value
	else
		flag = 'n';	//none
}

void INSINFO::print()
{
	cout << rtnName << ": " << diss << " - " << flag;
	cout << "\nRR: " << regR.size() << " ";
	for (auto l: regR)
		cout << l << " ";
	cout << "WR: " << regW.size() << " ";
	for (auto l: regW)
		cout << l << " ";
	cout << "\n";
	if (memOp != -1)
		for (auto l: vars)
			l.print();
	cout << "\n";
}

#endif

