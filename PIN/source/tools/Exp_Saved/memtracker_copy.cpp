#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <set>
#include "pin.H"

using namespace std;

string to_string (long long int u)
{
	stringstream ss;
	ss << hex << u;
	return ss.str();
}

int countS = 0, countG = 0;
map <long int, string> vars;
map <long int, string> inst;

INT32 Usage()
{
	cerr << "This tool produces a trace of memory accesses." << endl;
	cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
	return -1;
}

string stopinit[] = {"_fini", "_libc_csu_init", "frame_dummy", "register_tm_clones", "wctype_l", ".text", "__get_cpu_features",
						"explicit_bzero", "vprintf", "brk", "exit", "_exit", "deregister_tm_clones", "_Exit", "calloc", "malloc",
						"_start", "_init", "wctob", "btowc", "on_exit", "gnu_dev_makedev", "fwrite", "uselocale"};
set <string> stop (stopinit, stopinit + 24);

string lp[] = {"rip", "rbp"};
set <string> globstop (lp, lp + 2);

//checks for common uninteresting functions
bool isStopWord (string rtn_name)
{
	if (rtn_name.find("__") != -1 || rtn_name.find("@") != -1 || rtn_name.find(".") != -1 
		|| rtn_name.find("_dl") != -1 || rtn_name.find("_IO") != -1)
		return true;
	if (stop.find(rtn_name) != stop.end())
		return true;
	return false;
}

VOID recordMemoryAccess(ADDRINT addr, UINT32 size, ADDRINT codeAddr, ADDRINT rtnAddr, bool read)
{
	string filename;
	INT32 column = 0, line = 0;
	string source = "<unknown>";

	string name = RTN_FindNameByAddress((ADDRINT)rtnAddr);
	if (name.length() == 0)
		return;

	PIN_LockClient();
	PIN_GetSourceLocation(codeAddr, &column, &line, &filename);
	PIN_UnlockClient();

	if(filename.length() > 0)
	{
		if (vars.find(addr) == vars.end())
			vars[addr] = "V" + to_string(size) + "_" + to_string(++countG);
		source = filename + ":" + to_string(line);
		cout << vars[addr] << " " << size << " " << (read ? 'R' : 'W') << " " 
		<< name << " " << source << ": " << inst[codeAddr] << "\n";
	}
}

/* ===================================================================== */
/* Instrumentation routines											  */
/* ===================================================================== */

//parses disassembly into opcode and operands
vector <string> parse (string& disassemble)
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

//checks if the instruction holds a true memory access
bool varcheck (string& diss)
{
	if (diss.find("ptr") == -1 || diss.find("fs") != -1 || diss.find("gs") != -1  || 
		diss.find("cs") != -1 || diss.find("ds") != -1 || diss.find("ss") != -1 || diss.find("es") != -1)
		return true;
}

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

string extracter (string& dpart)
{
	int l = dpart.find('[');
	int r = dpart.find('+');
	string u = "";
	if (r != -1)
	{
		u = dpart.substr(l+1, r-l-1);
		r = dpart.find('-');
		u = u + dpart.substr(r, dpart.length()-r-1);
	}
	else
		u = dpart.substr(l+1, dpart.length()-l-2);
	return u;
}

long long int baddr (string& dpart)
{
	int r = dpart.find('+');
	string u = dpart.substr(r+1, dpart.length()-r-2);
	stringstream ss;
	ss << u;
	long long int d;
	ss >> hex >> d;
	return d;
}

VOID Instruction2(INS ins, VOID *v)
{
	ADDRINT ina = INS_Address(ins);
	string name = RTN_FindNameByAddress(ina);
	if (name == "main")
	{
		string diss = INS_Disassemble(ins);
		transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
		cout << "0x" << hex << INS_Address(ins) << ": " << diss << "\n";
	}
}

VOID Instruction(INS ins, VOID *v)
{
	ADDRINT ina = INS_Address(ins);
	string name = RTN_FindNameByAddress(ina);
	if (name.length() == 0)
		return;

	if (!isStopWord(name))
	{
		int column, line;
		string filename;
		PIN_LockClient();
		PIN_GetSourceLocation(ina, &column, &line, &filename);
		PIN_UnlockClient();

		if(filename.length() > 0)
		{
			//get instruction disassembly
			string diss = INS_Disassemble(ins);
			transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
			cout << "0x" << hex << INS_Address(ins) << ": " << diss;
			
			int rreg = INS_MaxNumRRegs(ins);
			int wreg = INS_MaxNumWRegs(ins);

			cout << " RR: " << rreg << " ";
			for (int i = 0; i < rreg; ++i)
				cout << REG_StringShort(INS_RegR(ins, i)) << " ";
			cout << " WR: " << wreg << " ";
			for (int i = 0; i < wreg; ++i)
				cout << REG_StringShort(INS_RegW(ins, i)) << " ";
			
			// detect variable access
			if (varcheck(diss))
			{
				cout << "\n";
				return;
			}

			bool flag = false;
			if (INS_IsMemoryRead(ins))
			{
				cout << " -> R\n";
				flag = true;
			}
			else if (INS_IsMemoryWrite(ins))
			{
				cout << " -> W\n";
				flag = true;
			}

			if (flag)
			{
				auto m = parse(diss);
				int idx = 1;
				if (m[2].find("ptr") != -1)
					idx = 2;
				auto e = membreak(m[idx]);

				vector <string> mems;
				
				set <string> f;
				f.insert (e.begin(), e.end());
				f.insert (globstop.begin(), globstop.end());
				if (m[idx].find("rbp") != -1)
					mems.push_back(extracter(m[idx]));
				else if (m[idx].find("rip") != -1)
					mems.push_back(to_string(INS_Address(ins) + baddr(m[idx])));

				INS iseek = ins;
				while (f != globstop)
				{
					// cout << "( ";
					// for (auto l: e)
					// 	cout << l << " ";
					// cout << ")\n";

					iseek = INS_Prev(iseek);
					int rreg = INS_MaxNumRRegs(iseek);
					int wreg = INS_MaxNumWRegs(iseek);

					// cout << INS_Disassemble(iseek);
					// cout << " RR: " << rreg << " ";
					// for (int i = 0; i < rreg; ++i)
					// 	cout << REG_StringShort(INS_RegR(iseek, i)) << " ";
					// cout << " WR: " << wreg << " ";
					// for (int i = 0; i < wreg; ++i)
					// 	cout << REG_StringShort(INS_RegW(iseek, i)) << " ";
					// cout << "\n";

					for (int i = 0; i < wreg; ++i)
					{
						string reg = REG_StringShort(INS_RegW(iseek, i));
						auto it = e.find(reg);
						if (it != e.end())
						{
							e.erase(it);
							for (int j = 0; j < rreg; ++j)
							{
								string var = REG_StringShort(INS_RegR(iseek, j));
								e.insert(var);
								if (var == "rbp" || var == "rip")
								{
									string lotus = INS_Disassemble(iseek);
									auto trial = parse(lotus);
									int idx = 1;
									if (trial[2].find("ptr") != -1)
										idx = 2;
									if (trial[idx].find("rbp") != -1)
										mems.push_back(extracter(trial[idx]));
									else if (trial[idx].find("rip") != -1)
										mems.push_back(to_string(INS_Address(iseek) + baddr(trial[idx])));
								}
							}
						}
					}

					f.clear();
					f.insert (e.begin(), e.end());
					f.insert (globstop.begin(), globstop.end());
				}
				// cout << "( ";
				// for (auto l: e)
				// 	cout << l << " ";
				// cout << ")\n";

				for (auto l: mems)
					cout << l << " ";
				cout << "\n";
			}
			cout << "\n";
		}
	}
}

VOID Image(IMG img, VOID *v)
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

				if (!isStopWord(routine_name))
				{
					cout << "ROUTINE: " << hex << RTN_Address(rtn) << ": " << routine_name << "\n";
					
					//loops on the instructions in the routine
					for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
					{
						//get instruction disassembly
						string diss = INS_Disassemble(ins);
						transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
						cout << "0x" << hex << INS_Address(ins) << ": " << diss;
						
						// int rreg = INS_MaxNumRRegs(ins);
						// int wreg = INS_MaxNumWRegs(ins);

						// cout << " RR: " << rreg << " ";
						// for (int i = 0; i < rreg; ++i)
						// 	cout << REG_StringShort(INS_RegR(ins, i)) << " ";
						// cout << " WR: " << wreg << " ";
						// for (int i = 0; i < wreg; ++i)
						// 	cout << REG_StringShort(INS_RegW(ins, i)) << " ";
						// cout << "\n";

						// detect variable access
						if (varcheck(diss))
						{
							cout << "\n";
							continue;
						}

						bool flag = false;
						if (INS_IsMemoryRead(ins))
						{
							cout << " -> R\n";
							flag = true;
						}
						else if (INS_IsMemoryWrite(ins))
						{
							cout << " -> W\n";
							flag = true;
						}

						if (flag)
						{
							auto m = parse(diss);
							int idx = 1;
							if (m[2].find("ptr") != -1)
								idx = 2;
							auto e = membreak(m[idx]);

							vector <string> mems;
							
							set <string> f;
							f.insert (e.begin(), e.end());
							f.insert (globstop.begin(), globstop.end());
							if (m[idx].find("rbp") != -1)
								mems.push_back(extracter(m[idx]));
							else if (m[idx].find("rip") != -1)
								mems.push_back(to_string(INS_Address(ins) + baddr(m[idx])));

							INS iseek = ins;
							while (f != globstop)
							{
								// cout << "( ";
								// for (auto l: e)
								// 	cout << l << " ";
								// cout << ")\n";

								iseek = INS_Prev(iseek);
								int rreg = INS_MaxNumRRegs(iseek);
								int wreg = INS_MaxNumWRegs(iseek);

								// cout << INS_Disassemble(iseek);
								// cout << " RR: " << rreg << " ";
								// for (int i = 0; i < rreg; ++i)
								// 	cout << REG_StringShort(INS_RegR(iseek, i)) << " ";
								// cout << " WR: " << wreg << " ";
								// for (int i = 0; i < wreg; ++i)
								// 	cout << REG_StringShort(INS_RegW(iseek, i)) << " ";
								// cout << "\n";

								for (int i = 0; i < wreg; ++i)
								{
									string reg = REG_StringShort(INS_RegW(iseek, i));
									auto it = e.find(reg);
									if (it != e.end())
									{
										e.erase(it);
										for (int j = 0; j < rreg; ++j)
										{
											string var = REG_StringShort(INS_RegR(iseek, j));
											e.insert(var);
											if (var == "rbp" || var == "rip")
											{
												string lotus = INS_Disassemble(iseek);
												auto trial = parse(lotus);
												int idx = 1;
												if (trial[2].find("ptr") != -1)
													idx = 2;
												if (trial[idx].find("rbp") != -1)
													mems.push_back(extracter(trial[idx]));
												else if (trial[idx].find("rip") != -1)
													mems.push_back(to_string(INS_Address(iseek) + baddr(trial[idx])));
											}
										}
									}
								}

								f.clear();
								f.insert (e.begin(), e.end());
								f.insert (globstop.begin(), globstop.end());
							}
							// cout << "( ";
							// for (auto l: e)
							// 	cout << l << " ";
							// cout << ")\n";

							for (auto l: mems)
								cout << l << " ";
							cout << "\n";
						}
						cout << "\n";
					}
					cout << "\n";
				}
				RTN_Close (rtn);
			}
		}
	}
}

VOID Fini(INT32 code, VOID *v)
{
	cout << "\nDONE\n";
}

/* ===================================================================== */
/* Main																  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
	// Initialize pin & symbol manager
	PIN_InitSymbols();

	if (PIN_Init(argc,argv) )
	{
		return Usage();
	}

	/* Instrument tracing memory accesses */
	INS_AddInstrumentFunction(Instruction, 0);
	//IMG_AddInstrumentFunction(Image, 0);
	PIN_AddFiniFunction(Fini, 0);

	// Never returns
	PIN_StartProgram();

	return 0;
}