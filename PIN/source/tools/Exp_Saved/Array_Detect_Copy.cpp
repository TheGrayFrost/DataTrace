#include "pin.H"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <set>
#include <list>
#include <iterator>
using namespace std;

//global usage
struct glove
{
	ADDRINT addr;
	string useType;					// read or write
};

//register usage
struct reg
{
	string name;
	int registerFlag;				//0 no comments(rbp,rsp), 1 computation, 2 accumulator, 3 parameter passing, 4 float
};

//instruction info
struct instruction
{
	string opcode;
	string addr;
	vector <string> operands;
	vector <glove> globals;
	vector <reg> registers;
	string disassemble;
	bool isValidJump;				//if instruction is a loopy jump and not a branchy jump 
	vector <string> locals;
	int times;
};

struct segment
{
	int start_ind;
	int end_ind;
};

struct ifSegment
{
	int start_ind;
	int end_ind;
	int loopVarCount;
};

struct insUse
{
	unsigned int index;
	string useType;
};

struct arrrayLocal
{
	unsigned int index;
	string parent;
	int offsetIndex;
};

struct loopVar
{
	string name;
	int forSegIndex;
};

struct parameter
{
	int size;
	bool isArray;
};

//my structure
struct array
{
	vector <string> base;					//all accessed elements in array (relative to rbp)
	vector <pair <string, int>> access;		//order of element access in terms of rtnInsList index
	int size;
	int flag;
};

struct link
{
	vector <pair <string,int>> base;		//all accessed elements in linked list
	vector <pair <string,int>> access;		//order of element access in terms of rtnInsList index
};


map <string, list <map <string, pair <int, list <insUse>>>>> local_map;
map <string, vector <arrrayLocal>> arrayLocal_map;
map <string, vector <instruction>> M;										//M: routine -> instructions in routine
map <string, vector <segment>> for_segment_map;								//for_segment_map: routine -> for loop segments in routine
map <string, vector <ifSegment>> if_segment_map;
map <string, vector <loopVar>> loopVarMap;
map <string, vector <segment>> nonForRegion;
map <string, string> rtnAddr;												//rtnAddr:	routine -> address
map <string, int> rtnCount;													//rtnCount:	routine -> no of times called
map <string, int> insCount;													//insCount:	instruction -> no of times executed
map <string, vector <parameter>> param_map;

//my map
map <string, array> array_map;												//array_map: routine -> array in routine
map <string, vector <pair <string, int>>> linkbase_map;						//linkbase_map: routine -> <linked list node, access>
map <string, vector <pair <string, int>>> linkmalloc_map;					//linkbase_map: routine -> <linked list node, malloc index>
map <string, vector <pair <string, int>>> linkaccess_map;					//linkaccess_map: routine -> <linked list node, access index>
map <string, vector <pair <string, int>>> linkupdateptr_map;
list <string> insList;														//insList: instruction addresses in order of instructions execution
//map <string, int> ins_map;

//converts hex int containing string to int
unsigned long long int convertToInt (string tmp)
{
	unsigned long long int x;
	stringstream ss;
	ss << hex << tmp;
	ss >> x;
	return x;
}

//records global read from memory into M
VOID RecordMemRead (VOID * ip, VOID * addr)
{
	//extract instruction and memory location (global) address
	stringstream ss, sss;
	string instruction_Addr = static_cast<ostringstream&>(ss << setw(8) << hex << (reinterpret_cast<ADDRINT>(ip))).str();
	ADDRINT global_Addr = reinterpret_cast<ADDRINT>(addr);
	
	//create global with the read usage
	string usageType = "read";
	glove tmp;
	tmp.addr = global_Addr;
	tmp.useType = usageType;

	//add to all corresponding instructions in the M map
	for (auto& map_it: M)
		for (auto& listIt: map_it.second)
			if (convertToInt(listIt.addr) == convertToInt(instruction_Addr))
				listIt.globals.push_back(tmp);
}

//records global write from memory into M
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
	//extract instruction and memory location (global) address
	stringstream ss,sss;
	string instruction_Addr = static_cast<ostringstream&>(ss << setw(8) << hex << (reinterpret_cast<ADDRINT>(ip))).str();
	ADDRINT global_Addr = reinterpret_cast<ADDRINT>(addr);

	//create global with the write usage
	string usageType = "write";
	glove tmp;
	tmp.addr=global_Addr;
	tmp.useType=usageType;

	//add to all corresponding instructions in the M map
	for (auto& map_it: M)
		for (auto& listIt: map_it.second)
			if (convertToInt(listIt.addr) == convertToInt(instruction_Addr))
				listIt.globals.push_back(tmp);
}

VOID Instruction(INS ins, VOID *v)
{
	//add instruction address to list
	stringstream ss;
	string addr = static_cast <ostringstream&> (ss << setw(8) << hex << (INS_Address(ins))).str();
	insList.push_back(addr);

	//update instruction execution count
	if (insCount.find(addr) == insCount.end())
		insCount.insert(make_pair(addr,1));
	else
		++(insCount.find(addr)->second);

	//count memory operands
	UINT32 memOperands = INS_MemoryOperandCount(ins);

	//iterate over each memory operand of the instruction
	//record each memory io
	//InsertPredicatedCall only executes analysis function if the predicated instruction makes changes to program state
	for (UINT32 memOp = 0; memOp < memOperands; memOp++)
	{
		if (INS_MemoryOperandIsRead(ins, memOp))
		{
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, memOp,
				IARG_END);
		}

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

//VOID docount(UINT64 * counter) { (*counter)++; }

//Routine level instrumentation function: maintains routine call count
VOID Routine(RTN rtn,VOID *v)
{
	//extract routine address and update call count
	stringstream ss;
	string addr = static_cast <ostringstream&> (ss << setw(8) << hex << (RTN_Address(rtn))).str();
	if (addr.size() <= 8)
	{
		if (rtnCount.find(addr) == rtnCount.end())
			rtnCount.insert(make_pair(addr, 1));
		else
			++(rtnCount.find(addr)->second);
	}

	//routine processing: completed in image load

	/*string rtn_name = RTN_Name(rtn);
	//cout <<rtn_name <<"\n";
	if(rtn_name=="main")
	{
		RTN_Open(rtn);
		// For each instruction of the routine
		int i=0;
		int *ptr=&i;
		for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
		{
			// Insert a call to docount to increment the instruction counter for this rtn
			// INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_icount), IARG_END);
			stringstream sss;
			string addr=static_cast <ostringstream&>(sss <<setw(8)<<hex << (INS_Address(ins))).str();
			INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &ptr, IARG_END);
			ins_map[addr]++;
			//cout <<addr <<" "<<i <<"\n";
			//i++;
		}
		// cout <<"ptr count: "<<*ptr <<"\n";
		RTN_Close(rtn);
	}*/
}

//checks for common uninteresting functions
bool isStopWord (string rtn_name)
{
	string stop[] = {"printf@plt", "puts@plt", "__gmon_start__@plt", "_fini", "_libc_csu_init", "frame_dummy", "__do_global_dtors_aux",
					"register_tm_clones", "deregister_tm_clones", "_start", "__libc_start_main@plt", ".plt", "_init", ".plt.got",\
					"__libc_csu_fini", "__libc_csu_init", "__stack_chk_fail@plt"};
	for (int i = 0; i <= 16; i++)
		if (rtn_name == stop[i])
			return true;
	return false;
}

// returns operands from an instruction disassembly
vector <string> parse (string opcode, string disassemble)
{
	istringstream iss(disassemble);
	vector <string> tokens;

	//break into space-separated tokens
	copy (istream_iterator<string> (iss), istream_iterator<string> (), back_inserter(tokens));

	vector <string> operands;
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
		for (int i = 1; i < tokens.size(); i++)
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

//Image level instrumentaion function: maintains rtnAddr map and M map
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

				if (isStopWord(routine_name))	//ignore common routines
				{
					RTN_Close(rtn);
					continue;
				}

				//extract routine address and add to map
				stringstream ss;
				string rtn_Addr = static_cast<ostringstream&>(ss << setw(8) << hex << (RTN_Address(rtn))).str();
				rtnAddr.insert(make_pair(routine_name, rtn_Addr));
				
				//loops on the instructions in the routine
				for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
				{
					instruction ist;
					string tmp;

					//printable version of the opcode
					string str = INS_Mnemonic(ins);	
					tmp.resize(str.size());
					transform(str.begin(), str.end(), tmp.begin(), ::tolower);
					ist.opcode = tmp;

					//get instruction address
					stringstream ss;
					string str_tmp = static_cast<ostringstream&>(ss << setw(8) << hex << (INS_Address(ins))).str();
					ist.addr = str_tmp;

					//get instruction disassembly and operands
					ist.disassemble = INS_Disassemble(ins);
					ist.operands = parse(tmp, ist.disassemble);

					//collect information about the instruction operand reg use
					reg regtmp;
					for (auto& op_it: ist.operands)
					{
						if (op_it.find("eax") != string::npos)		//eax -> computation
						{
							regtmp.name = "eax";
							regtmp.registerFlag = 1;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rax") != string::npos)	//rax -> computation
						{
							regtmp.name = "rax";
							regtmp.registerFlag = 1;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("ecx") != string::npos)	//ecx -> accumulator
						{
							regtmp.name = "ecx";
							regtmp.registerFlag = 2;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rcx") != string::npos)	//rcx -> accumulator
						{
							regtmp.name = "rcx";
							regtmp.registerFlag = 2;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("edi") != string::npos)	//edi -> parameter
						{
							regtmp.name = "edi";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("esi") != string::npos)	//esi -> parameter
						{
							regtmp.name = "esi";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rdi") != string::npos)	//rdi -> parameter
						{
							regtmp.name = "rdi";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rsi") != string::npos)	//rsi -> parameter
						{
							regtmp.name = "rsi";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("r8") != string::npos)	//r8 -> parameter
						{
							regtmp.name = "r8";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("r9") != string::npos)	//r9 -> parameter
						{
							regtmp.name = "r9";
							regtmp.registerFlag = 3;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rbp")!=string::npos)	//rbp -> base pointer
						{
							regtmp.name = "rbp";
							regtmp.registerFlag = 0;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("rsp") != string::npos)	//rsp -> stack pointer
						{
							regtmp.name = "rsp";
							regtmp.registerFlag = 0;
							ist.registers.push_back(regtmp);
						}
						else if (op_it.find("xm") != string::npos)	//xmm -> float register
						{
							regtmp.name = "xm";
							regtmp.registerFlag = 4;
							ist.registers.push_back(regtmp);
						}
					}

					//accumulate instructions from routine into M
					if (M.find(routine_name) == M.end())
					{
						vector <instruction> lis;
						lis.push_back(ist);
						M.insert (make_pair(routine_name, lis));
					}
					else
						M[routine_name].push_back(ist);
				}
				RTN_Close (rtn);
			}
		}
	}
}

//checks if this instruction has been encountered before
bool occurs_previously (string addr, vector <string> ins_Addr)
{
	addr = addr.substr(2, string::npos);	//ignore 0x
	for (auto& it: ins_Addr)
	{
		string tmp = string(it);
		if (convertToInt(tmp) == convertToInt(addr))
			return true;
	}
	return false;
}

//counts number of loops in a routine
//also updates M to indicate loopy jumps as compared to branchy jumps
int countNumberOfLoops(string rtn_name)
{
	int count = 0;
	vector <string> ins_Addr;
	
	for (auto& list_it: M[rtn_name])
	{
		if (list_it.opcode == "jmp" || list_it.opcode == "jle")		//if it is a jump instruction
		{
			if (occurs_previously(list_it.operands[0], ins_Addr))	//which jumps to an instruction encountered previously (indicates loop)
			{
				++count;											//then increment number of loops
				list_it.isValidJump = true;
			}
			else
				list_it.isValidJump = false;
		}
		else
		{
			ins_Addr.push_back(list_it.addr);
			list_it.isValidJump = false;
		}
	}

	return count;

}

//finds the address tmp in ins
int findIndex(string tmp, vector<instruction>& ins)
{
	int ind = 0;
	for (auto& it: ins)
	{
		if(convertToInt(it.addr) == convertToInt(tmp))
			return ind;
		ind++;
	}
	return ind;
}

/*
bool isRegister(string name)
{
	string registerNames[] = {"rdx", "rax", "eax", "ecx", "rcx", "edi", "esi", "rdi", "rsi", "r8", "r9"};
	bool flag = true;
	for (int i = 0; i < 11; i++)
		if (name == registerNames[i])
			flag=true;
	if (name.find("xm") != string::npos)
		flag = true;
	if (name.find("rbp") != string::npos)
		flag = false;
	return flag;
}
*/

/*
int findSizeOfOperand(string operand)
{
	int size=0;
	if (operand.find("qword")!=string::npos)
		size=8;
	else if (operand.find("dword")!=string::npos)
		size=4;
	else if (operand.find("word")!=string::npos)
		size=2;
	else if (operand.find("byte")!=string::npos)
		size=1;
	else
		;
	return size;
}
*/

//build the segment map for each for loop for each routine in M
VOID segmentizeFor()
{
	for (auto& map_it: M)									//iterate over each routine
	{
		string rtn_name = map_it.first;						//extract its name
		int num_for_loops = countNumberOfLoops(rtn_name);	//count number of loops
		
		vector <segment> segtmp;
		if (num_for_loops > 0)
		{
			for (auto list_it = map_it.second.rbegin(); list_it != map_it.second.rend(); list_it++)
			{
				if ((*list_it).opcode == "jle" && (*list_it).isValidJump == true)					//for each loop
				{
					int startInd = findIndex((*list_it).operands[0], map_it.second);				//loop start index in rtnInsList
					int endInd = findIndex((*list_it).addr, map_it.second);							//loop end index in rtnInsList
					
					//add to segment vector
					segment tmp;
					tmp.start_ind = startInd;
					tmp.end_ind = endInd;
					segtmp.push_back(tmp);
				}
			}
			for_segment_map.insert(make_pair(rtn_name, segtmp));
		}
	}
}


//extracts all local memory access from M and updates instruction info accordingly
VOID add_locals()
{
	for (auto& map_it: M)									//iterate over each routine
		for (auto& list_it: map_it.second)					//iterate over each instruction in the routine
			for (auto& op_it: list_it.operands)				//for all the operands in the instruction
				if(op_it.find("rbp-") != string::npos)		//if any accesses a local stack variable
					list_it.locals.push_back(op_it);		//add it to the instructions list of locals
}

//update instruction call times in M using insCount
VOID add_inscount()
{
	for (auto& map_it: M)									//iterate over each routine
	{
		for (auto& list_it: map_it.second)					//iterate over each instruction in the routine
		{
			string ins_Addr = list_it.addr;					//extract its address
			for (auto& ins_it: insCount)
				if(ins_it.first == ins_Addr)				//find in insCount
					list_it.times=ins_it.second;			//update in M
		}
	}
}

//detects arrays from memory accesses in M and add to array_map
VOID buildarraymap()
{
	for (auto& map_it: M)									//iterate over each routine
	{
		string rtn_name = map_it.first;						//extract its name
		array tmp;
		int index = 0;
		set <string> base;
		vector <pair <string, int>> acc;
		for (auto& list_it: map_it.second)					//iterate over each instruction in the routine
		{
			for (auto& op_it: list_it.operands)
			{
				if(op_it.find("rbp+rax*") != string::npos || op_it.find("rbp-rax*") != string::npos)	//indication of array element access
				{
					base.insert(op_it);
					acc.push_back(make_pair(op_it, index));
				}
			}
			index++;
		}

		//convert set of bases to vector
		//update base and access fields of tmp accordingly
		vector <string> vec;
		copy(base.begin(), base.end(), back_inserter(vec));
		tmp.base = vec;
		tmp.access = acc;

		//insert in array map
		array_map.insert(make_pair(rtn_name,tmp));
	}
}

//checks if an array is accessed inside a loop
VOID find_arrayinsidefor()
{
	for (auto& for_it: for_segment_map)														//for each routine
		for (auto& it: for_it.second)														//for each loop in the routine
			for (auto& ar_it: array_map)													//for each routine
				for (auto& acc_it : ar_it.second.access)									//for each array in routine
					if(acc_it.second >= it.start_ind && acc_it.second <= it.end_ind)		//if array access occus within the loop
						cout << "Array accessed inside the loop\n";							//declare the access
}

VOID print_map() //check in main map how many times instruction is executed
{
	for (auto& map_it: M)
	{
		string rtn_name=map_it.first;
		cout <<"rtn:"<<map_it.first <<" ";
		cout <<"-----------------------\n";
		for (auto& list_it: map_it.second)
		{
			cout <<list_it.addr <<" "<<list_it.opcode <<" "<<list_it.times <<"\n";

		}
	}
}
VOID print_inscount()
{

	for (auto& map_it: insCount)
	{
		cout <<map_it.first <<" "<<map_it.second <<"\n";
	}

}
VOID print_arraymap()
{
	for (auto& map_it: array_map)
	{
		string rtn_name=map_it.first;
		cout <<"rtn:"<<rtn_name <<" ";
		cout <<"-----------------------\n";
		array tmp=map_it.second;
		cout <<"------base---------\n";
		for (auto& it: tmp.base)
		{
			cout <<it <<"";
		}
		cout <<"\n------access---------\n";
		for (auto& it: tmp.access)
		{
			cout <<it.first <<" "<<it.second <<"";
		}
		/* cout <<"\n------base address---------\n";
		for (auto& it: tmp.Addr_base)
		{
			cout <<*it <<"";
		}
		cout <<"\n------access address---------\n";
		for (auto& it: tmp.Addr_access)
		{
			cout <<*it <<"";
		}*/
		cout <<"\n";
	}

}
VOID print_insList()
{
	for (auto& map_it: M)
	{
		string rtn_name=map_it.first;
		cout <<"rtn:"<<map_it.first <<" ";
		cout <<"-----------------------\n";
		for (auto& list_it: map_it.second)
		{
			string addr=list_it.addr;
			//cout <<list_it->addr <<" "<<list_it->opcode <<" "<<list_it->times <<"\n";
			int count =0;
			for (auto& it: insList)
			{
				if(it==addr)
				{
				count++;
				}
			}
			cout <<addr <<" "<<count <<"\n";


		}

	}
}

//finds all linked node access in a routine
VOID add_linkbase()
{
	for (auto& map_it: M)													//iterate over each routine
	{
		vector <pair <string, int>> tmp;
		int index = 0;
		for (auto& list_it: map_it.second)									//iterate over each instruction
		{
			for (auto& op_it: list_it.operands)
				if(op_it.find("qword ptr [rbp-") != string::npos)			//if there is linked list access
					tmp.push_back(make_pair(op_it, index));					//save access and corr rtnInsList index
			index++;
		}

		//update in linkbase_map
		linkbase_map.insert(make_pair(map_it.first, tmp));
	}
}

//finds if a called address is malloc
bool occur_inmalloc(string addr)
{
	addr = addr.substr(2,string::npos);
	for (auto& rtnit: rtnAddr)
		if(rtnit.first == "malloc@plt")								//of all the malloc calls
			if(convertToInt(rtnit.second) == convertToInt(addr))	//if any occur at the addr given
				return true;
	return false;
}

//finds where each linked node is malloc'd
VOID add_linkmalloc()
{
	for (auto& map_it: linkbase_map)														//iterate over each linked list in the routines
	{
		vector <pair <string, int>> tmp;
		vector <instruction> ins = M[map_it.first];
		for (auto& list_it: map_it.second)
		{
			int index = list_it.second;													//access index
			string str = list_it.first;													//accessed node
			if (ins[index-1].opcode == "call" or ins[index-1].opcode == "call_near")	//if a function is called before accesss
			{
				vector <string> op = ins[index-1].operands;
				if (occur_inmalloc(op[0]))												//and that function is malloc
					for (auto& opt_list: ins[index].operands)
						if (opt_list == "rax")											//if the malloc return value is used with a list node
							tmp.push_back(make_pair(str, index));						//that node has been malloc'd there
			}
		}

		//update linkmalloc map accordingly
		linkmalloc_map.insert(make_pair(map_it.first,tmp));
	}
}

VOID add_linkaccess()
{
	for (auto& map_it: M)								//iterate over each routine
	{
		vector <pair <string,int>> tmp;
		vector <instruction> ins = map_it.second;
		int n = ins.size();
		int prev = 0;
		for(int i = 0; i < n; i++)						//iterate over each instruction
		{
			if(ins[i].opcode == "mov")					//if it is a move instruction
			{
				vector <string> op = ins[i].operands;
				string str = "dword ptr [rax]";
				if (op[0] == str || op[1] == str)		//and moves a linked list node (indicated by dword)
				{
					int cur = i-1;
					while(cur >= prev)
					{
						if (ins[cur].opcode == "mov")	//find the first instruction before it that moves data into or out of rax
						{
							vector <string> oprnd = ins[cur].operands;
							string opr = "rax";

							if(oprnd[0] == opr || oprnd[1] == opr)		//find where rax gets its data from or into
							{
								if(oprnd[0] == opr)
									opr = oprnd[1];
								else
									opr = oprnd[0];

								if(linkmalloc_map.find(map_it.first) != linkmalloc_map.end())	//in the linkmalloc map, find
									for (auto& m_it: linkmalloc_map[map_it.first])
										if(m_it.first == opr)
											tmp.push_back(make_pair(str,i));
								break;
							}
						}
						cur--;
					}
					prev = i+1;
				}
			}
		}
		linkaccess_map.insert(make_pair(map_it.first,tmp));
	}
}
VOID add_linkptr()
{
	for (auto& map_it: M)
	{
		//cout <<"rtn:"<<map_it->first <<" ";
		//cout <<"-----------------------\n";
		vector <pair <string,int> > tmp;
		vector <instruction > ins=M.find(map_it.first)->second;
		int n=ins.size();
		int prev=0;
		for(int i=0;i <n;i++)
		{
				if(ins[i].opcode=="mov")
				{
					// cout <<ins[i].opcode <<" "<<i <<"\n";
					vector <string> op=ins[i].operands;
					if(op[0].find("qword ptr [rax")!=string::npos)
					{

						//cout <<ins[i].opcode <<" -- "<<op[0]<<" -- "<<op[1]<<" "<<i <<"\n";
						string str=op[0];
						int cur=i-1;
						int flag1=0;
						int flag2=0;

						while(cur>=prev)
						{
							if(ins[cur].opcode=="mov")
							{
								vector <string> oprnd=ins[cur].operands;
								if(flag1==0 and oprnd[0]=="rax")
								{
									string opr=oprnd[1];

									//cout <<cur <<" "<<oprnd[0]<<" "<<oprnd[1]<<"\n";
									if(linkmalloc_map.find(map_it.first)!=linkmalloc_map.end())
									{
										for (auto& m_it: linkmalloc_map.find(map_it.first)->second)
										{
											if(m_it.first==opr)
											{

												flag1=1;

											}
										}


									}


								}
								if(op[1]=="0x0")
								{
									flag2=1;
									// cout <<op[0]<<"	"<<op[1]<<" \n";
								}
								if(flag2==0 and oprnd[0]==op[1])
								{
									string opr=oprnd[1];

									//cout <<cur <<" "<<oprnd[0]<<" "<<oprnd[1]<<"\n";
									if(linkmalloc_map.find(map_it.first)!=linkmalloc_map.end())
									{
										for (auto& m_it: linkmalloc_map.find(map_it.first)->second)
										{
											if(m_it.first==opr)
											{

												flag2=1;

											}
										}


									}


								}
								if(flag1==1 and flag2==1)
								{
									tmp.push_back(make_pair(str,i));
									break;

								}


							}


							cur--;
						}
						prev=i+1;




					}
				}

		}
		linkupdateptr_map.insert(make_pair(map_it.first,tmp));

	}

}

//checks if s is a pointer in a linked list
bool is_ptr (string rtn, string s)
{
	vector <pair<string,int>>& tmp = linkbase_map.find(rtn)->second;

	for (auto& list_it: tmp)
		if (list_it.first == s)
			return true;
	return false;
}

//check if s is a local variable
bool is_local(string s)
{
	if(s.find("dword ptr [rbp-") != string::npos)
		return true;
	return false;
}

//checks if s points to data in the linked list
bool is_data_ptr(string s)
{
	if(s.find("dword ptr [rax]") != string::npos)
		return true;
	return false;

}

//checks if name is a register
bool isReg(string name)
{
	string registerNames[] = {"rdx","rax","eax","ecx","rcx","edi","esi","rdi","rsi","r8","r9","rbp","rsp","edx"};
	for(int i = 0; i < 14; i++)
		if (name == registerNames[i])
			return true;
	return false;
}

//checks if s is a hex constant
bool isconst(string s)
{
	if(s[0] == '0' && s[1] == 'x')
		return true;
	return false;
}

//creates data flow graph of the code run
VOID make_graph()
{
	ofstream fs;
	fs.open("array.dot");
	fs << "digraph G {\n";
	//fs <<"welcome"<<" -> "<<"To\n";
	//fs <<"To"<<" -> "<<"Web\n";
	//fs <<"To"<<" -> "<<"Graphviz\n";
	for (auto& map_it: M)
	{
		if(map_it.first != "malloc@plt")	//not a malloc routine
		{
			cout << "rtn:" << map_it.first << " ";
			cout << "-----------------------\n";
			vector<instruction> ins = map_it.second;
			int n = ins.size();
			int prev = 0;
			string array_ptr;
			int flag = 0;
			for(int i = 0; i < n; i++)
			{
				string opcode = ins[i].opcode;
				if (opcode == "mov" || opcode == "add")
				{
					string opr1 = ins[i].operands[0];
					string opr2 = ins[i].operands[1];

					//data exchange between two registers
					if (isReg(opr1) && isReg(opr2))	
						fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";

					//data exchange between a register and an array element
					else if(opr2.find("rbp+rax*") != string::npos || opr1.find("rbp+rax*") != string::npos)
					{
						cout << "array -> " << opr1 << " -- " << opr2 << "\n";
						string edx, node;							//array element in node and register in edx					
						if(opr2.find("rbp+rax*") != string::npos)
						{
							edx = opr1;
							node = opr2;
						}
						else
						{
							node = opr1;
							edx = opr2;
						}

						if(flag == 0)
						{
							int cur = i-1;
							while(cur >= prev)
							{
								if(ins[cur].opcode == "mov")
								{
									vector <string> oprnd = ins[cur].operands;
									if(oprnd[1] == "rax" and oprnd[0].find("qword ptr [rbp-") != string::npos) //accumulator to array element
									{
										cout << "arrayprev -> " << oprnd[0] << " -- " << oprnd[1] << "\n";
										fs << "\"" << edx << "\" -> \"" << oprnd[0] << "\" [ label= \"write " << i << "\" color=\"red\" ]\n";
										array_ptr = oprnd[0];
										cur = cur + 1;
										flag = 1;
										while (cur < i)
										{
											if(ins[cur].opcode == "mov")
											{
												oprnd = ins[cur].operands;
												if (oprnd[1].find("dword ptr [rbp-") != string::npos && oprnd[0] == edx) //array element to data reg
												{
													cout << "arraydata -> " << oprnd[0] << " -- " << oprnd[1] << "\n";
													fs << "\"" << array_ptr << "\" -> \"" << oprnd[1]
														<< "\" [ label= \"write " << i << "\" color=\"green\" ]\n";
												}
											}
											cur++;
										}
									break;
									}
								}
								cur--;
							}
							prev = i+1;
						}

						else
						{
							cout << "else " << node << "->" << edx << "\n";
							int cur = i-1;
							while(1)
							{
								if(ins[cur].opcode == "mov")
								{
									vector<string> oprnd = ins[cur].operands;
									if(oprnd[0] == edx)
									{
										cout << "arraynext ->" << oprnd[0] << " -- " << oprnd[1] << "\n";
										fs << "\"" << array_ptr << "\" -> \"" << oprnd[0] << "\" [ label= \"write " << i << "\"color=\"red\" ]\n";
										break;
									}
								}
								cur--;
							}
							prev = i+1;
						}
					}

					else if (opr1.find("qword ptr [rax+") != string::npos)
					{
						cout << "link ptr -> " << i << "->" << opr1 << " -- " << opr2 << "\n";
						int cur = i-1;
						int flag1 = 0, flag2 = 0;
						if(opr2 == "0x0")
						{
							cout << "link null -> "<< i << "->" << opr1 << " -- " << opr2 << "\n";
							cur = prev-1;
						}
						string srax, srdx;
						while(cur >= prev)
						{
							if(ins[cur].opcode == "mov")
							{
								vector<string> oprnd = ins[cur].operands;
								cout << "linkprev ->" << cur << "->" << oprnd[0] << " -- " << oprnd[1] << "\n";
								if (flag1 == 0 && oprnd[0] == "rax")
								{
									if(oprnd[1].find("qword ptr [rbp-") != string::npos)
									{
										srax = oprnd[1];
										flag1 = 1;
									}
									else
										break;
								}
								else if (flag1 == 0 && oprnd[0] == "rdx")
								{
									if(oprnd[1].find("qword ptr [rbp-") != string::npos)
									{
										srdx = oprnd[1];
										flag2 = 1;
									}
									else
										break;
								}
								else if(flag1 == 1 && flag2 == 1)
								{
									cout << "link update ->" << "->" << srax << " -- " << srdx << "\n";
									fs << "\"" << srdx << "\" -> \"" << srax << "\" [ label= \"write " << i << "\" color=\"blue\" ]\n";
									break;
								}
							}
							cur--;
						}
						prev=i+1;
					}

					else if (is_ptr(map_it.first, opr1) || is_ptr(map_it.first, opr2))
					{
						if(is_ptr(map_it.first,opr1))
						{
							if(isReg(opr2))
								fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";
							else if (isconst(opr2))
								fs << "\"" << opr1 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << " : " << opr2 << "\" ]\n";
						}
						else if(isReg(opr1))
							fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";

					}

					else if (is_data_ptr(opr1) && isReg(opr2))
						fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";

					else if (is_local(opr1) || is_local(opr2))
					{
						if(is_local(opr1))
						{
							if(isReg(opr2))
								fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";
							else if(isconst(opr2))
								fs << "\"" << opr1 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << " : " << opr2 << "\" ]\n";
						}
						else if(isReg(opr1))
							fs << "\"" << opr2 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";
					}

					else if (isReg(opr1) && isconst(opr2))
					{
						cout << "const -> " << opr1 << " -- " << opr2 << "\n";
						fs << "\"" << opr1 << "\" -> \"" << opr1 << "\" [ label = \"write " << i << "\" ]\n";
					}
				}
			}
		}
	}
	fs << "}";
	fs.close();
}

VOID print_tmpmap()
{

	for (auto& map_it: linkupdateptr_map)
	{
		cout <<"rtn:"<<map_it.first <<" ";
		cout <<"-----------------------\n";
		for (auto& list_it: map_it.second)
		{
		cout <<list_it.first <<" "<<list_it.second <<"\n";
		}
		// cout <<map_it->first <<" "<<map_it->second <<"\n";
	}
}

VOID Fini (INT32 code, VOID *v)
{
	add_locals();			//find and update local access in M
	add_inscount();			//update instruction count in M using insCount
	buildarraymap();		//find array access inside of routine
	segmentizeFor();		//find loops in each routine and build segment map
	find_arrayinsidefor();	//map array access inside of loops

	//print_insList();
	//print_arraymap();
	//print_inscount();
	//print_map();
	//print_mainmap();

	//function calls for identifying linked lists
	add_linkbase();
	add_linkmalloc();
	add_linkaccess();
	add_linkptr();

	//final data flow graph of the runtime
	make_graph();

	//print_tmpmap();
	//segmentizeIf();
	//for (auto& map_it: M)
	//buildArrayLocalMap(map_it->first);
	//buildparam_map();

	//cout << "\nThis is the fini function\n";
}

/* ===================================================================== */
/* 							Print Help Message							*/
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
	RTN_AddInstrumentFunction(Routine, 0);
	INS_AddInstrumentFunction(Instruction, 0);

	PIN_AddFiniFunction(Fini, 0);

	//OutFile.open(KnobOutputFile.Value().c_str());
	//OutFile.setf(ios::showbase);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


