#ifndef __MEMTRACKER_H__
#define __MEMTRACKER_H__

#include "pin.H"
#include <iostream>
#include <string>
#include <vector>

// parses disassembly into opcode and operands
// mov dword ptr [rcx+rax*1], edx -> (mov, dword ptr [rcx+rax*1], edx)
std::vector <std::string> parse (std::string& diss)
{
	std::vector <std::string> ops;
	int l = diss.find(" ");
	ops.push_back(diss.substr(0, l));
	if (l == diss.length()-1)
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

// structure to hold instruction information
struct INSINFO
{
	std::string fname;						// file name
	int column, line;						// column and line no.
	std::string rtnName;					// parent routine
	ADDRINT ina;							// instruction address
	std::string diss;						// instruction assembly
	char flag;								// instruction type: 'r/w'->memory read/write, 'n'-> none
	std::vector <std::string> regR, regW;	// registers read from and written to
	ADDRINT memOp;							// actual memory reference in case of flag != 'n'
	string target;							// if direct branch or call;
	
	INSINFO() {}
	INSINFO(INS&, std::string = "", int = 0, int = 0);
	void print(std::ostream& = std::cout);
	void shortPrint();
};

INSINFO::INSINFO(INS& ins, std::string fn, int col, int lno): fname(fn), column(col), line(lno)
{
	rtnName = RTN_FindNameByAddress(INS_Address(ins));
	ina = INS_Address(ins);
	target = "";
	if (INS_IsProcedureCall(ins))
	{
		int taddr = INS_DirectBranchOrCallTargetAddress(ins);
		target = RTN_FindNameByAddress(taddr);
	}


	// disassembly
	diss = INS_Disassemble(ins);
	transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
	
	// memory access flags
	bool memR = INS_IsMemoryRead(ins) && (diss.find("ptr") != -1);
	bool memW = INS_IsMemoryWrite(ins) && (diss.find("ptr") != -1);

	// registers
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
	
	if (memR)
		flag = 'r';	//memory read
	else if (memW)
		flag = 'w';	//memory write
	else
		flag = 'n';	//none
}

void INSINFO::shortPrint()
{
	std::cout << fname.substr(fname.length()-12) << ": " << diss << "\n";
}

// void INSINFO::print()
// {
// 	std::cout << "LOCATION: " << fname << ": " << dec << column << ", " << dec << line << "\n";
// 	std::cout << "FUNCTION: " << rtnName << "\n";
// 	std::cout << "INS: 0x" << hex << ina << ": " << diss << " - " << flag << " ";
// 	if (flag != 'n') std::cout << "0x" << hex << memOp;
// 	std::cout << "\nRR: " << regR.size() << " ";
// 	for (auto l: regR)
// 		std::cout << l << " ";
// 	std::cout << "\nWR: " << regW.size() << " ";
// 	for (auto l: regW)
// 		std::cout << l << " "; 
// 	std::cout << "\n";
// }

void INSINFO::print(std::ostream& outf)
{
	outf << "LOCATION: " << fname << ": " << dec << column << ", " << dec << line << "\n";
	outf << "FUNCTION: " << rtnName << "\n";
	outf << "INS: 0x" << hex << ina << ": " << diss << " - " << flag << " ";
	if (flag != 'n') outf << "0x" << hex << memOp;
	if (target != "") outf << "\nTARGET: " << target;
	outf << "\nRR: " << regR.size() << " ";
	for (auto l: regR)
		outf << l << " ";
	outf << "\nWR: " << regW.size() << " ";
	for (auto l: regW)
		outf << l << " "; 
	outf << "\n";
}

#endif
