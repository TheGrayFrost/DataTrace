#include "pin.H"
#include <iostream>

using namespace std;

VOID Routine (RTN rtn, VOID * v)
{
	string routine_name = (RTN_Name(rtn));
	//extract routine address and add to map
	long long int rtn_Addr = RTN_Address(rtn);
	cout << "\nROUTINE: " << hex << rtn_Addr << ": " << routine_name << "\n";
	RTN_Open (rtn);
	for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
	{
		if (INS_IsDirectBranchOrCall(ins))
		{
			long long int ina = INS_Address(ins);
			string diss = INS_Disassemble(ins);
			transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
			cout << ina << ": " << diss;
			ADDRINT m = INS_DirectBranchOrCallTargetAddress(ins);
			string call = RTN_FindNameByAddress(m);
			cout << " -> " << call << "\n";
		}
	}
	RTN_Close(rtn);
}

VOID printer (ADDRINT ina)
{
	cout << ina << "\n";
}

VOID Routine2 (RTN rtn, VOID * v)
{
	string routine_name = (RTN_Name(rtn));
	//if (routine_name == "main")
	{
		long long int rtn_Addr = RTN_Address(rtn);
		cout << "\nROUTINE: " << hex << rtn_Addr << ": " << routine_name << "\n";
		RTN_Open (rtn);
		for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
		{
			long long int ina = INS_Address(ins);
			string diss = INS_Disassemble(ins);
			transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
			cout << ina << ": " << diss << " R ";
			int count = 0;
			for (int k = 0; k < 2; ++k)
			{
				if (REG_is_gr(REG_FullRegName(INS_RegR(ins, k))))
				{
					cout << REG_StringShort(INS_RegR(ins, k)) << ", ";
					count++;
				}
			}
			cout << "W ";
			for (int k = 0; k < 2; ++k)
			{
				if (REG_is_gr(REG_FullRegName(INS_RegW(ins, k))))
				{
					cout << REG_StringShort(INS_RegW(ins, k)) << ", ";
					count++;
				}
			}
			if (INS_IsDirectBranchOrCall(ins))
			{
				ADDRINT m = INS_DirectBranchOrCallTargetAddress(ins);
				string call = RTN_FindNameByAddress(m);
				cout << " -> " << call;
			}
			cout << "\n";
			//INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) printer, IARG_INST_PTR, IARG_END);
		}
		RTN_Close(rtn);
	}
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

				//extract routine address and add to map
				long long int rtn_Addr = RTN_Address(rtn);
				
				cout << "\nROUTINE: " << hex << rtn_Addr << ": " << routine_name << "\n";
				
				//loops on the instructions in the routine
				for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
				{
					//get instruction address
					//long long int ina = INS_Address(ins);
					//get instruction disassembly and operands
					string diss = INS_Disassemble(ins);
					transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
				}

				RTN_Close (rtn);
			}
		}
	}
}

VOID Instruction (INS ins, VOID * u)
{
	if (INS_IsProcedureCall(ins))
	{
		long long int ina = INS_Address(ins);
		string diss = INS_Disassemble(ins), call;
		transform(diss.begin(), diss.end(), diss.begin(), ::tolower);
		cout << ina << ": " << diss;
		//ADDRINT m = INS_DirectBranchOrCallTargetAddress(ins);
		//string call = RTN_FindNameByAddress(m);
		cout << " -> " << call << "\n";
	}
}


/* ===================================================================== */
/* 							Prlong long int Help Message							 */
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

	// Multiple level instrumentation analysis
	//IMG_AddInstrumentFunction(ImageLoad, 0);
	RTN_AddInstrumentFunction(Routine2, 0);
	//INS_AddInstrumentFunction(Instruction, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}


