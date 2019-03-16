#include "pin.H"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <set>
#include <sstream>
#include <list>
#include <iterator>

using namespace std;

struct variable
{
	set<long long int> absaddr;
	long long int baseaddr;
	string reladdr;
	string dtype;

	variable() {baseaddr = -1;}
	void print() 
	{
		cout << dtype << " " << reladdr << " " << baseaddr << "\n";
		for (auto v: absaddr)
			cout << v << " ";
		cout << "\n";
	}
};

struct container
{
	string dtype;
	set <long long int> elem;
};

map <string, vector <long long int>> memAcc;
map <long long int, variable> memrg, memwg, memrs, memws;
map <long long int, container> globArrays;

map <string, vector <set <long long int>>> people;

//utility function to convert int to string
string to_string(long long int u)
{
	string v;
	stringstream p;
	p << hex << u;
	p >> v;
	return v;
}

//checks for common uninteresting functions
bool isStopWord (string rtn_name)
{
	if (rtn_name.find("__") != -1 || rtn_name.find("@") != -1 || rtn_name.find(".") != -1 || rtn_name.find("_ZN") != -1
		|| rtn_name.find("_Zd") != -1 || rtn_name.find("_ZS") != -1 || rtn_name.find("_Zn") != -1)
		return true;
	string stop[] = {"_fini", "_libc_csu_init", "frame_dummy", "register_tm_clones", "deregister_tm_clones", "_start", "_init"};
	for (long long int i = 0; i < 7; i++)
		if (rtn_name == stop[i])
			return true;
	return false;
}

//parses disassembly into opcode and operands
vector <string> parse (string disassemble)
{
	istringstream iss(disassemble);
	vector <string> tokens;

	//break into space-separated tokens
	copy (istream_iterator<string> (iss), istream_iterator<string> (), back_inserter(tokens));

	vector <string> operands;
	operands.push_back(tokens[0]);
	if(tokens.size() == 1) 				//do nothing in 0 operands case
		;
	else if(tokens.size() == 2)			//unary operand and no comma
		operands.push_back(tokens[1]);
	else
	{
		bool found_comma = false;
		string tmp1 = "";
		string tmp2 = "";

		//iterate over tokens leaving opcode
		for (unsigned long long int i = 1; i < tokens.size(); i++)
		{
			if (!found_comma)													//do this until you don't find a comma
			{
				if (tokens[i].find(",") != string::npos)						//when you find the comma
				{
					tokens[i].resize(tokens[i].size()-1);						//ignore it
					found_comma = true;
				}
				tmp1.size() ? tmp1 += " " + tokens[i] : tmp1 += tokens[i];		//append token to tmp1
			}
			else
				tmp2.size() ? tmp2 += " " + tokens[i] : tmp2 += tokens[i];		//append remaining tokens to tmp2
		}
		operands.push_back(tmp1);
		operands.push_back(tmp2);
	}
	return operands;
}

// detect data type from parsed disassembly
string fdtype(vector<string>& h, long long int i)
{
	if (h[i].find("byte") != -1)
		return "char";
	if (h[i].find("dword") != -1)
	{
		if (h[0] == "movss")
			return "float";
		return "int";
	}
	if (h[i].find("qword") != -1)
	{
		if (h[0] == "movsd")
		{
			return "double";
		}
		return "pointer";
	}
	return "short";
}


//records global read from memory into M
VOID RecordMemRead (VOID * ip, VOID * addr)
{
	//extract instruction and memory location (global) address
	long long int instruction_Addr = reinterpret_cast<ADDRINT>(ip);
	long long int global_Addr = reinterpret_cast<ADDRINT>(addr);
	memrg[instruction_Addr].absaddr.insert(global_Addr);
}

//records global write from memory into M
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
	long long int instruction_Addr = reinterpret_cast<ADDRINT>(ip);
	long long int global_Addr = reinterpret_cast<ADDRINT>(addr);
	memwg[instruction_Addr].absaddr.insert(global_Addr);
}


// detect array base address from element access
long long int baddr (string u)
{
	long long int m = u.find("+");
	if (m == -1)
		return m;
	string l = u.substr(m);
	l = l.substr(0, l.length()-1);
	
	stringstream val;
	val << hex << l;
	val >> m;
	return m;
}

string STLCall[8], STL[8];

VOID initSTL()
{
	STLCall[0] = "_ZNSt6vector"; STL[0] = "vector";
	STLCall[1] = "_ZNSt3map"; STL[1] = "map";
	STLCall[2] = "_ZNSt7__cxx114list"; STL[2] = "list";
	STLCall[3] = "_ZNSt3set"; STL[3] = "set";
	STLCall[4] = "_ZNSt5stack"; STL[4] = "stack";
	STLCall[5] = "_ZNSt5queue"; STL[5] = "queue";
	STLCall[6] = "_ZNSt14priority_queue"; STL[6] = "priority_queue";
	STLCall[7] = "_ZNSt5deque"; STL[7] = "deque";
}


//record STL container base
VOID vbase (ADDRINT addr, ADDRINT base, int u, int v)
{
	if ((v == 0) || (v == 2))
	{
		memrg[(long long int)addr].baseaddr = (long long int)base;
		memrg[(long long int)addr].dtype = STL[u];
	}
	if ((v == 2) || (v == 1))
	{
		memwg[(long long int)addr].baseaddr = (long long int)base;
		memwg[(long long int)addr].dtype = STL[u];
	}
	//cout << "\n VBASE CALLED " << addr << ": " << base << "\n";
}

//identify STL container and instrument accordingly	
bool identifySTL (INS ins)
{
	bool ans = false;
	if (INS_IsDirectBranchOrCall(ins))
	{
		ADDRINT ina = INS_Address(ins);
		ADDRINT m = INS_DirectBranchOrCallTargetAddress(ins);
		string call = RTN_FindNameByAddress(m);
		cout << "Call " << call << "\n";
		for (int u = 0; u < 8; ++u)
		{
			if (call.find(STLCall[u]) != -1)
			{
				cout << STL[u] << "ed call\n";
				INS prev = INS_Prev(ins);
				long long int inp = INS_Address(prev);
				string dissp = INS_Disassemble(prev);
				transform(dissp.begin(), dissp.end(), dissp.begin(), ::tolower);
				if (dissp.find("rip") != -1)
				{
					//memrg[ina].dtype = u.second;
					//memwg[ina].dtype = u.second;								
					ans = true;
					//cout << "Global " << STL[u] << "ed Call\n";
					INS next = INS_Next(ins);
					string dissn = INS_Disassemble(next);
					transform(dissn.begin(), dissn.end(), dissn.begin(), ::tolower);
					cout << INS_Address(next) << ": " << dissn << "\n";
					if (dissn.find("[rax]") != -1)
					{
						//cout << "\nInstrumenting Next";
						if (INS_IsMemoryRead(next))
						{
							INS_InsertCall(
								next, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
								IARG_ADDRINT, ina,
								IARG_MEMORYOP_EA, 0,
								IARG_END);
							INS_InsertCall(
								prev, IPOINT_AFTER, (AFUNPTR) vbase,
								IARG_ADDRINT, ina,
								IARG_REG_VALUE, INS_RegW(prev, 0),
								IARG_UINT32, u,
								IARG_UINT32, 0,
								IARG_END);
						}
						if (INS_IsMemoryWrite(next))
						{
							INS_InsertCall(
								next, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
								IARG_ADDRINT, ina,
								IARG_MEMORYOP_EA, 0,
								IARG_END);
							INS_InsertCall(
								prev, IPOINT_AFTER, (AFUNPTR) vbase,
								IARG_ADDRINT, ina,
								IARG_REG_VALUE, INS_RegW(prev, 0),
								IARG_UINT32, u,
								IARG_UINT32, 1,
								IARG_END);
						}
					}
					else
					{
						INS_InsertCall(
							prev, IPOINT_AFTER, (AFUNPTR) vbase,
							IARG_ADDRINT, ina,
							IARG_REG_VALUE, INS_RegW(prev, 0),
							IARG_UINT32, u,
							IARG_UINT32, 2,
							IARG_END);
					}
				}
				//break;
			}
		}
	}
	return ans;
}

//checks if the instruction holds a true memory access
bool varcheck (string& diss)
{
	if (diss.find("ptr") == -1 || diss.find("fs") != -1 || diss.find("gs") != -1  || 
		diss.find("cs") != -1 || diss.find("ds") != -1 || diss.find("ss") != -1 || diss.find("es") != -1)
		return true;
}

VOID ImageLoad(IMG img, VOID * v)
{
	//on the main image only
	if(IMG_IsMainExecutable(img))
	{
		//loop on the sections in the image
		for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
		{
			//loop on the routines called in each section
			for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
			{
				RTN_Open (rtn);
				string routine_name = (RTN_Name(rtn));

				//ignore common routines
				if (isStopWord(routine_name))
				{
					RTN_Close(rtn);
					continue;
				}

				//extract routine address and add to map
				long long int rtn_Addr = RTN_Address(rtn);
				cout << "\nROUTINE: " << hex << rtn_Addr << ": " << routine_name << "\n";
				
				//loops on the instructions in the routine
				for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
				{
					//get instruction address
					long long int ina = INS_Address(ins);
					//get instruction disassembly and operands
					string diss = INS_Disassemble(ins);
					transform(diss.begin(), diss.end(), diss.begin(), ::tolower);

					cout << ina << ": " << diss << "\n";

					//identifying STL containers
					initSTL();
					if (identifySTL(ins))
					{
						memAcc[routine_name].push_back(ina);
						ins = INS_Next(ins);
						continue;
					}

					// detect variable access
					if (varcheck(diss))
						continue;

					// if (!INS_IsMemoryRead(ins) && !INS_IsMemoryWrite(ins))
					// {
					// 	cout << "No Mem\n";
					// 	continue;
					// }

					memAcc[routine_name].push_back(ina);
					//extract variable access
					auto k = parse(diss);
					long long int idx = 1;
					if (k.size() == 3 && k[2].find("ptr") != -1)
						idx = 2;
					
					// if local variable access
					if (INS_IsStackRead(ins) || INS_IsStackWrite(ins))
					{
						if (INS_IsStackRead(ins))
						{
							cout << "Stack Read\n";
							memrs[ina].reladdr = k[idx];
							memrs[ina].dtype = fdtype(k, idx);
						}
						if (INS_IsStackWrite(ins))
						{
							memws[ina].reladdr = k[idx];
							memws[ina].dtype = fdtype(k, idx);
						}
					}
					// else if global variable
					else
					{
						if (INS_IsMemoryRead(ins))
						{
							// direct access
							cout << "Global Read\n";
							memrg[ina].reladdr = k[idx];
							memrg[ina].dtype = fdtype(k, idx);
							cout << k[idx] << " " << fdtype(k, idx) << "\n";
							// indirect access via pointer
							if (!INS_IsIpRelRead(ins))
								memrg[ina].baseaddr = baddr(k[idx]);
							UINT32 memOperands = INS_MemoryOperandCount(ins);
							for (UINT32 memOp = 0; memOp < memOperands; memOp++)
							{
								if (INS_MemoryOperandIsRead(ins, memOp))
								{
									INS_InsertCall(
										ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
										IARG_INST_PTR,
										IARG_MEMORYOP_EA, memOp,
										IARG_END);
								}
							}
						}
						if (INS_IsMemoryWrite(ins))
						{
							// direct access
							memwg[ina].reladdr = k[idx];
							memwg[ina].dtype = fdtype(k, idx);
							cout << k[idx] << " " << fdtype(k, idx) << "\n";
							// indirect access via pointer -> could be array or struct
							if (!INS_IsIpRelWrite(ins))
								memwg[ina].baseaddr = baddr(k[idx]);
							UINT32 memOperands = INS_MemoryOperandCount(ins);
							for (UINT32 memOp = 0; memOp < memOperands; memOp++)
							{
								if (INS_MemoryOperandIsWritten(ins, memOp))
								{
									INS_InsertCall(
										ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
										IARG_INST_PTR,
										IARG_MEMORYOP_EA, memOp,
										IARG_END);
								}
							}
						}
					}
				}
				RTN_Close (rtn);
			}
		}
	}
}

void collectArrays(variable j)
{
	if (j.baseaddr != -1)
	{
		globArrays[j.baseaddr].dtype = j.dtype;
		globArrays[j.baseaddr].elem.insert(j.absaddr.begin(), j.absaddr.end());
	}
}

auto inArrays (variable x)
{
	for (auto u = globArrays.begin(); u != globArrays.end(); ++u)
		if (u->second.dtype == x.dtype)
			if (u->second.elem.find(*x.absaddr.begin()) != u->second.elem.end())
				return u;
	return globArrays.end();
}

VOID Fini2 (INT32 code, VOID * v)
{
	cout << "\nMEMRG\n";
	for (auto j: memrg)
	{
		{
			cout << j.first << "\n";
			j.second.print();
		}
	}
	cout << "\nMEMWG\n";
	for (auto j: memwg)
	{
		{
			cout << j.first << "\n";
			j.second.print();
		}
	}
}

bool basics (string r)
{
	if (r == "int" || r == "int" || r == "char" || r == "float" || r == "double" || r == "pointer")
		return true;
}

VOID Fini (INT32 code, VOID *v)
{
	for (auto j: memrg)
		collectArrays(j.second);
	for (auto j: memwg)
		collectArrays(j.second);

	ofstream fs;
	fs.open("array.dot");
	fs << "digraph G {\n";
	long long int i = 1;
	for (auto u: globArrays)
	{
		string node = "";
		if (basics(u.second.dtype))
			node = "array ";
		node = node + u.second.dtype + " G_" + to_string(u.first);
		for (auto v: u.second.elem)
			fs << "\"" <<  node << "\" -> \"" << hex << v << "\"\n";
		i++;
	}
	
	i = 1;
	for (auto k: memAcc)
	{
		set <string> accrs, accws, accrg, accwg;
		fs << "R_" << i << " [label = \"" << k.first << "\" shape = \"box\"]\n";
		for (auto j: k.second)
		{
			auto l = memrg.find(j);
			if (l != memrg.end())
			{
				string node = "";
				if (l->second.baseaddr == -1)
				{
					auto r = inArrays(l->second);
					if (r == globArrays.end())
					{
						node = l->second.dtype + " G_" + to_string(*(l->second.absaddr.begin()));
						if (accrg.find(node) == accrg.end())
						{
							accrg.insert(node);
							fs << "\"" << node << "\" -> R_" << i << " [color = \"red\"]\n";
						}						
					}
					else
					{
						if (basics(r->second.dtype))
							node = "array ";
						node = node + r->second.dtype + " G_" + to_string(r->first);
						if (accrg.find(node) == accrg.end())
						{
							accrg.insert(node);
							fs << "\"" << node << "\" -> R_" << i << " [color = \"green\"]\n";
						}
					}
				}

				else
				{
					if (basics(l->second.dtype))
						node = "array ";
					node = node + l->second.dtype + " G_" + to_string(l->second.baseaddr);
					if (accrg.find(node) == accrg.end())
					{
						accrg.insert(node);
						fs << "\"" << node << "\" -> R_" << i << " [color = \"green\"]\n";
					}
				}
			}

			// l = memrs.find(j);
			// if (l != memrs.end() && accrs.find(l->second.reladdr) == accrs.end())
			// {
			// 	accrs.insert(l->second.reladdr);
			// 	fs << "\"" << l->second.dtype << " S_" << l->second.reladdr << "\" -> R_" << i << " [color = \"red\"]\n";
			// }

			// l = memws.find(j);
			// if (l != memws.end() && accws.find(l->second.reladdr) == accws.end())
			// {
			// 	accws.insert(l->second.reladdr);
			// 	fs << "R_" << i << " -> \"" << l->second.dtype << " S_" << l->second.reladdr << "\" [color = \"blue\"]\n";
			// }

			l = memwg.find(j);
			if (l != memwg.end())
			{
				string node = "";
				if (l->second.baseaddr == -1)
				{
					auto r = inArrays(l->second);
					if (r == globArrays.end())
					{
						node = l->second.dtype + " G_" + to_string(*(l->second.absaddr.begin()));
						if (accwg.find(node) == accwg.end())
						{
							accwg.insert(node);
							fs << "R_" << i << " -> \"" << node << "\" [color = \"blue\"]\n";
						}						
					}
					else
					{
						if (basics(r->second.dtype))
							node = "array ";
						node = node + r->second.dtype + " G_" + to_string(r->first);
						if (accwg.find(node) == accwg.end())
						{
							accwg.insert(node);
							fs << "R_" << i << " -> \"" << node << "\" [color = \"yellow\"]\n";
						}
					}
				}

				else
				{
					if (basics(l->second.dtype))
						node = "array ";
					node = node + l->second.dtype + " G_" + to_string(l->second.baseaddr);
					if (accwg.find(node) == accwg.end())
					{
						accwg.insert(node);
						fs << "R_" << i << " -> \"" << node << "\" [color = \"yellow\"]\n";
					}
				}
			}
		}
		i++;
	}
	fs << "}\n";
	fs.close();
}

/* ===================================================================== */
/* 							Print Help Message							 */
INT32 Usage()
{
	cerr << "This is the invocation pintool" << endl;
	cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
	return -1;
}


int main(int argc, char * argv[])
{
	// Initialize pin & symbol manager
	if (PIN_Init(argc, argv))
		return Usage();
	PIN_InitSymbols();

	// Multiple level instrumentation analysis
	IMG_AddInstrumentFunction(ImageLoad, 0);
	//INS_AddInstrumentFunction(Instruction, 0);

	PIN_AddFiniFunction(Fini, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


