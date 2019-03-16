
# DataTrace

## Overview
[DataTrace] is a dynamic binary analyzer, developed as part of the collective efforts to build an intelligent query system on source code projects. In the grander scheme of things, DataTrace shall be used to detect data structure operations and strategies used in the code.
It traces the run-time behavior of binary executables, collecting information about memory accesses, data manipulation and threading and synchronization constructs used. It is based on [Intel PIN]<sup>TM</sup> dynamic instrumentation framework.
The run-time trace of the executable is combined with the static source code information (obtained via [LLVM]) to generate features for a learning-based detection framework. The framework uses these time-sequenced features to identify data structures and their operations used in the code.

## Usage
### Generating trace from executable  
##### **Note: Your executable should have been compiled to contain debugging information. And, its extension should be .out**
1. In DataTrace, run `cd Work; make </path/to/exec>.dump`

Congratulations! You're done. The trace is available in the .dump generated.  
### Generating trace from source files
If your executable is not compiled with debugging options, or you don't know if it is. Don't worry, we still got you if you have the source file. The trace generation stays the same. 
1. In DataTrace, run `cd Work; make </path/to/main/file>.dump`

Congratulations! You're done. The trace is available in the .dump generated.  
##### **Note: We currently support only C and C++ sources. More importantly, if your project requires a complicated build, involves linking multiple object files and what not... we'd suggest you _learn_ how to build the final executable with the debugging info. <br/>Here's a helpful link: [Debug Options in GCC]**

## Understanding the Trace
The trace has the events listed out in time-order. 
Events are basically assembly instructions we consider worth noting. 
There are 3 types of instructions we consider for our applications:
* Memory Read (denoted by flag, R)
* Memory Write (denoted by flag W)
* Procedure Calls (denoted by flag C)

A sample event might look like this:
> THREADID: 1, EVENT SEQ: 428
> LOCATION: /home/srijoni/Vishesh/DataTrace/Work/bares.c: 16
> FUNCTION: trythis
> INS: 0x4008ae: call 0x400680 - C
> TARGET: puts@plt
> SYNC: Has synchronous access with the following locks:
> 0x6010a0 as Pthread Mutex
> 0x4315a4 as Semaphore

All events have these basic attributes:
* THREADID, EVENTSEQ: Thread Id & Event Sequence No. (within the thread) - Uniquely identify each event.
* LOCATION: Source line information - Contains name of source file and line no. which generates the traced assembly.
* FUNCTION: The function containing the source line. 
* INS: Contains information in the format - Instruction Address, Instruction Assembly followed by Instruction Flag (as described earlier in event types).
* SYNC: Synchronization of memory access. Describes locks held by the executing thread. Currently, we support the following locking mechanisms: 
	* Pthread Mutex
	* Semaphore
	* RW locks

There are also the following event-specific attributes:
* For Memory Read \(R\) and Memory Write (W)
	* TARGET VAR: Variable description of the accessed memory.
* For Procedure Calls \(C\)
	* TARGET FUN: Name of the target function.

## DataTrace Internals

### Call Graph

[DataTrace]: <https://github.com/TheGrayFrost/DataTrace>
[Intel PIN]: <https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool>
[LLVM]: <https://llvm.org/>
[Debug Options in GCC]: <https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html>
