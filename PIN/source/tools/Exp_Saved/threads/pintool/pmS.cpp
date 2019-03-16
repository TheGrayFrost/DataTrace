#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <cctype>
#include <stdlib.h>
#include <stdlib.h>
#include <iomanip>
#include <vector>
#include <map> 
#include <list>
#include "pin.H"

VOID lock(char*, THREADID, ADDRINT );
VOID unlock( char *, THREADID, ADDRINT );
VOID slock(char*, THREADID, ADDRINT );
VOID sunlock( char *, THREADID, ADDRINT );
VOID wlock(char*, THREADID, ADDRINT );
VOID wunlock( char *, THREADID, ADDRINT );
const char * StripPath(const char *);


ofstream outFile, out;
ADDRINT lowImg,highImg,lowSec,highSec;
using namespace std;

struct thread{
	ADDRINT threadIdentity;
	int status;
};



std::map<int, std::vector<struct thread> > accessmap; 
std::map<int, std::vector<struct thread> >::iterator it;
std::map<int, std::vector<struct thread> >::iterator it2;

std::map<int, std::vector<struct thread> > saccessmap; 
std::map<int, std::vector<struct thread> >::iterator sit;
std::map<int, std::vector<struct thread> >::iterator sit2;

std::map<int, std::vector<struct thread> > waccessmap; 
std::map<int, std::vector<struct thread> >::iterator wit;
std::map<int, std::vector<struct thread> >::iterator wit2;

const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

vector<struct thread> allThreads;

int dflag=0;
int threadCount = 0;
int mutexCount = 0;
int sCount = 0;
int wCount = 0;

void Image(IMG img, void * v)
{
	if(IMG_IsMainExecutable(img))
	{
	lowImg = IMG_LowAddress(img);
	highImg = IMG_HighAddress(img);
	outFile<<IMG_Name(img)<< " Starting Address : "<<lowImg<<"  Ending Address : "<<highImg<<endl;
	for(SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
	{
		unsigned found = SEC_Name(sec).compare(".bss");
		if(found==0)
			{
				//printf("Starting Address of .bss section: %lu\n",SEC_Address(sec));
				outFile << "Starting Address of .bss section:" << setw(8) << SEC_Address(sec) << " " << SEC_Name(sec) << endl;
				lowSec=SEC_Address(sec);
				highSec=IMG_HighAddress(img);
				outFile << "Ending Address of .bss section: " << highSec << '\n';
				//printf("Ending Address of .bss section: %lu\n",highSec);
			}	
	}
	}
}

void print()
{
	//printf("_________________________________\n");
	std::vector<struct thread> ::iterator threadIt;
	
	for(threadIt=allThreads.begin(); threadIt!=allThreads.end(); threadIt++)
	{
		cout<<" "<<threadIt->threadIdentity<<" ";
	}
	cout<<endl;
	for(it=accessmap.begin(); it!=accessmap.end(); it++)
	{
		cout<<"  "<<it->first<<" : ";
		for(threadIt =it->second.begin(); threadIt!=it->second.end(); threadIt++)
		{
			cout<<" "<<threadIt->status<<" ";
		}
		cout<<endl;
	}
}

void Routine(RTN rtn, void * v)
{
	unsigned found,found1;
	//ADDRINT address=reinterpret_cast<ADDRINT>(RTN_Address(rtn));
	found=RTN_Name(rtn).compare("sem_wait");
	if(found==0)
	{	
               // if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
	
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)slock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		//cout<<"I returned\n";
		
		RTN_Close(rtn);
           }
	}

	found=RTN_Name(rtn).compare("sem_post");
	if(found==0)
	{	 
                //if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)sunlock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
           }
	}


	found=RTN_Name(rtn).compare("__pthread_rwlock_rdlock");
        found1=RTN_Name(rtn).compare("pthread_rwlock_wrlock");

	if(found==0 || found1==0)
	{	
               // if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
	
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		//cout<<"I returned\n";
		
		RTN_Close(rtn);
           }
	}

	

	found=RTN_Name(rtn).compare("__pthread_rwlock_unlock");
	if(found==0)
	{	
               // if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
	
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wunlock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		//cout<<"I returned\n";
		
		RTN_Close(rtn);
           }
	}



	found=RTN_Name(rtn).compare("pthread_mutex_lock");
	if(found==0)
	{	
               // if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
	
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		//cout<<"I returned\n";
		
		RTN_Close(rtn);
           }
	}


	found=RTN_Name(rtn).compare("pthread_mutex_unlock");
	if(found==0)
	{	 
                //if (address <= lowSec && address >= highSec)
           {
		outFile <<  RTN_Name(rtn) <<" : " << StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()) << '\n';
			 
		RTN_Open(rtn);   
		// Insert a call at the entry point of a routine to increment the call count
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)unlock,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);
		RTN_Close(rtn);
           }
	}
   
}

int isLocked(int mute)
{
	it = accessmap.find(mute);
	std::vector<struct thread> ::iterator threadIt;
	for(threadIt=it->second.begin(); threadIt!=it->second.end(); threadIt++)
	{
		if(threadIt->status == 1)
		{
			return threadIt->threadIdentity;
		}
	}
  return 2;
}

int isWaiting(ADDRINT thread)
{
	std::vector<struct thread> ::iterator threadIt;
	for(it=accessmap.begin(); it!=accessmap.end(); it++)
	{
		for(threadIt =it->second.begin(); threadIt!=it->second.end(); threadIt++)
		{
			if(threadIt->threadIdentity == thread && threadIt->status ==0)
			{
				return it->first;
			}
		}
	}
	return 2;
}

int isLockeds(int mute)
{
	sit = saccessmap.find(mute);
	std::vector<struct thread> ::iterator threadIt;
	for(threadIt=sit->second.begin(); threadIt!=sit->second.end(); threadIt++)
	{
		if(threadIt->status == 1)
		{
			return threadIt->threadIdentity;
		}
	}
  return 2;
}

int isWaitings(ADDRINT thread)
{
	std::vector<struct thread> ::iterator threadIt;
	for(sit=saccessmap.begin(); sit!=saccessmap.end(); sit++)
	{
		for(threadIt =sit->second.begin(); threadIt!=sit->second.end(); threadIt++)
		{
			if(threadIt->threadIdentity == thread && threadIt->status ==0)
			{
				return sit->first;
			}
		}
	}
	return 2;
}


int isLockedw(int mute)
{
	wit = waccessmap.find(mute);
	std::vector<struct thread> ::iterator threadIt;
	for(threadIt=wit->second.begin(); threadIt!=wit->second.end(); threadIt++)
	{
		if(threadIt->status == 1)
		{
			return threadIt->threadIdentity;
		}
	}
  return 2;
}


int isWaitingw(ADDRINT thread)
{
	std::vector<struct thread> ::iterator threadIt;
	for(wit=waccessmap.begin(); wit!=waccessmap.end(); wit++)
	{
		for(threadIt =wit->second.begin(); threadIt!=wit->second.end(); threadIt++)
		{
			if(threadIt->threadIdentity == thread && threadIt->status ==0)
			{
				return wit->first;
			}
		}
	}
	return 2;
}



void lock(char * rtnName,THREADID threadId, ADDRINT mutexId)
{
       
	it = accessmap.find(mutexId);
	if(it==accessmap.end())
	{
		mutexCount++;
		vector<struct thread> threadVector(allThreads);
		std::vector<struct thread> ::iterator threadIt;
		int found =0;
		for(threadIt = threadVector.begin(); threadIt != threadVector.end(); threadIt ++)
		{
			if(threadIt->threadIdentity == threadId)
			{
				threadIt->status = 1;
				found =1;
			}
			else
			{
				threadIt->status = 2;
			}

		}
		if(found==1)
		{
			accessmap.insert( std::pair<int ,std::vector<struct thread> >(mutexId,threadVector));	
		}
		else
		{
			threadCount++;
			thread newThread = { threadId , 1};
			thread newThread_2 = {threadId , 2};
			allThreads.push_back(newThread_2);
			for(it2 = accessmap.begin() ; it2!=accessmap.end() ; it2++)
			{
				it2->second.push_back(newThread_2);
			}
			threadVector.push_back(newThread);
			accessmap.insert( std::pair<int ,std::vector<struct thread> >(mutexId,threadVector));	

		}
	}
	else
	{ 

		std::vector<struct thread> ::iterator threadIt;
		int found =0, alreadyLocked = 0; ADDRINT varthread = 2;
		for(threadIt = it->second.begin(); threadIt != it->second.end(); threadIt++)
		{
			if(threadIt->status==1)
			{
				varthread = threadIt->threadIdentity;
				alreadyLocked  =1;
			}
		}
		if(varthread!=threadId)
		{

			if(alreadyLocked==1)
			{

				for(threadIt = it->second.begin(); threadIt != it->second.end(); threadIt++)
				{
					if(threadIt->threadIdentity==threadId )
					{
						found  =1;
						threadIt->status = 0;
					}
				}
				if(found==0)
				{

					threadCount++;
					thread newThread = {threadId ,0 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(it2 = accessmap.begin() ; it2!=accessmap.end() ; it2++)
					{
						if(it2==it)
							it2->second.push_back(newThread);
						else
							it2->second.push_back(newThread_2);
					}	
				}	
			}
			else
			{

				for(threadIt = it->second.begin(); threadIt != it->second.end(); threadIt++)
				{
					if(threadIt->threadIdentity==threadId)
					{
						found  =1;
						threadIt->status = 1;
					}
				}
				if(found==0)
				{
					threadCount++;
					thread newThread = {threadId ,1 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(it2 = accessmap.begin() ; it2!=accessmap.end() ; it2++)
					{
						if(it2==it)
							it2->second.push_back(newThread);
						else
							it2->second.push_back(newThread_2);
					}
				}		
			}
			int i;
			for(i=0;i<=threadCount;i++)
			{
				int a; ADDRINT b;
				a = isWaiting(varthread);
				if(a == 2)
				{
					return;
				}
				else
				{

					//cout<<"Lock call-L1 entering DEAD\n";
					b = isLocked(a);
					if(b==threadId)
					{
						cout<<"DEADLOCK detected!!\n"; dflag=1;
							outFile<<"Dead Lock Created";
						ofstream visual;
						visual.open("dead.dot");
						visual<< "digraph test{"<<endl;
						std::vector<struct thread> ::iterator threadIt;
						for(it=accessmap.begin(); it!=accessmap.end(); it++)
						{
							visual<<"	Mutex"<<it->first<<"[fillcolor = \"sienna\" style = filled];"<<endl;
						}
						for(threadIt=allThreads.begin(); threadIt!=allThreads.end(); threadIt++)
						{
							visual<<"	Thread"<<threadIt->threadIdentity<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
						}
						for(it=accessmap.begin(); it!=accessmap.end(); it++)
						{
							for(threadIt =it->second.begin(); threadIt!=it->second.end(); threadIt++)
							{
								if(threadIt->status==0)
									visual<<"	Thread"<<threadIt->threadIdentity<<"->Mutex"<<it->first<<"[color = \"red\"];"<<endl;
								else if(threadIt->status==1)
									visual<<"	Thread"<<threadIt->threadIdentity<<"->Mutex"<<it->first<<"[color = \"green\"];"<<endl;
							}
						}
						visual<<"}"<<endl;
						visual.close();
						//system("./graphviz.sh");
						FILE *fp = popen("./graphD.sh","r");
						fp++;
						exit(0);
					}
					else
					{
						varthread = b;
					}
				}
			}
		}
		
	}
	
	
}

void slock(char * rtnName,THREADID threadId, ADDRINT sId)
{
       
	sit = saccessmap.find(sId);
	if(sit==saccessmap.end())
	{
		sCount++;
		vector<struct thread> sthreadVector(allThreads);
		std::vector<struct thread> ::iterator sthreadIt;
		int found =0;
		for(sthreadIt = sthreadVector.begin(); sthreadIt != sthreadVector.end(); sthreadIt ++)
		{
			if(sthreadIt->threadIdentity == threadId)
			{
				sthreadIt->status = 1;
				found =1;
			}
			else
			{
				sthreadIt->status = 2;
			}

		}
		if(found==1)
		{
			saccessmap.insert( std::pair<int ,std::vector<struct thread> >(sId,sthreadVector));	
		}
		else
		{
			threadCount++;
			thread newThread = { threadId , 1};
			thread newThread_2 = {threadId , 2};
			allThreads.push_back(newThread_2);
			for(sit2 = saccessmap.begin() ; sit2!=saccessmap.end() ; sit2++)
			{
				sit2->second.push_back(newThread_2);
			}
			sthreadVector.push_back(newThread);
			saccessmap.insert( std::pair<int ,std::vector<struct thread> >(sId,sthreadVector));	

		}
	}
	else
	{ 

		std::vector<struct thread> ::iterator sthreadIt;
		int found =0, alreadyLocked = 0; ADDRINT varthread = 2;
		for(sthreadIt = sit->second.begin(); sthreadIt != sit->second.end(); sthreadIt++)
		{
			if(sthreadIt->status==1)
			{
				varthread = sthreadIt->threadIdentity;
				alreadyLocked  =1;
			}
		}
		if(varthread!=threadId)
		{

			if(alreadyLocked==1)
			{

				for(sthreadIt = sit->second.begin(); sthreadIt != sit->second.end(); sthreadIt++)
				{
					if(sthreadIt->threadIdentity==threadId )
					{
						found  =1;
						sthreadIt->status = 0;
					}
				}
				if(found==0)
				{

					threadCount++;
					thread newThread = {threadId ,0 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(sit2 = saccessmap.begin() ; sit2!=saccessmap.end() ; sit2++)
					{
						if(sit2==sit)
							sit2->second.push_back(newThread);
						else
							sit2->second.push_back(newThread_2);
					}	
				}	
			}
			else
			{

				for(sthreadIt = sit->second.begin(); sthreadIt != sit->second.end(); sthreadIt++)
				{
					if(sthreadIt->threadIdentity==threadId)
					{
						found  =1;
						sthreadIt->status = 1;
					}
				}
				if(found==0)
				{
					threadCount++;
					thread newThread = {threadId ,1 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(sit2 = saccessmap.begin() ; sit2!=saccessmap.end() ; sit2++)
					{
						if(sit2==sit)
							sit2->second.push_back(newThread);
						else
							sit2->second.push_back(newThread_2);
					}
				}		
			}
			int i;
			for(i=0;i<=threadCount;i++)
			{
				int a; ADDRINT b;
				a = isWaitings(varthread);
				if(a == 2)
				{
					return;
				}
				else
				{

					//cout<<"Lock call-L1 entering DEAD\n";
					b = isLockeds(a);
					if(b==threadId)
					{
						cout<<"DEADLOCK detected!!\n"; dflag=1;
							outFile<<"Dead Lock Created";
						ofstream visual;
						visual.open("dead.dot");
						visual<< "digraph test{"<<endl;
						std::vector<struct thread> ::iterator sthreadIt;
						for(sit=saccessmap.begin(); sit!=saccessmap.end(); sit++)
						{
							visual<<"	Semaphore"<<it->first<<"[fillcolor = \"sienna\" style = filled];"<<endl;
						}
						for(sthreadIt=allThreads.begin(); sthreadIt!=allThreads.end(); sthreadIt++)
						{
							visual<<"	Thread"<<sthreadIt->threadIdentity<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
						}
						for(sit=saccessmap.begin(); sit!=saccessmap.end(); sit++)
						{
							for(sthreadIt =sit->second.begin(); sthreadIt!=sit->second.end(); sthreadIt++)
							{
								if(sthreadIt->status==0)
									visual<<"	Thread"<<sthreadIt->threadIdentity<<"->Semaphore"<<sit->first<<"[color = \"red\"];"<<endl;
								else if(sthreadIt->status==1)
									visual<<"	Thread"<<sthreadIt->threadIdentity<<"->Semaphore"<<sit->first<<"[color = \"green\"];"<<endl;
							}
						}
						visual<<"}"<<endl;
						visual.close();
						//system("./graphviz.sh");
						FILE *fp = popen("./graphD.sh","r");
						fp++;
						exit(0);
					}
					else
					{
						varthread = b;
					}
				}
			}
		}		
	}
	
}

void wlock(char * rtnName,THREADID threadId, ADDRINT wId)
{
       
	wit = waccessmap.find(wId);
	if(wit==waccessmap.end())
	{
		wCount++;
		vector<struct thread> wthreadVector(allThreads);
		std::vector<struct thread> ::iterator wthreadIt;
		int found =0;
		for(wthreadIt = wthreadVector.begin(); wthreadIt != wthreadVector.end(); wthreadIt ++)
		{
			if(wthreadIt->threadIdentity == threadId)
			{
				wthreadIt->status = 1;
				found =1;
			}
			else
			{
				wthreadIt->status = 2;
			}

		}
		if(found==1)
		{
			waccessmap.insert( std::pair<int ,std::vector<struct thread> >(wId,wthreadVector));	
		}
		else
		{
			threadCount++;
			thread newThread = { threadId , 1};
			thread newThread_2 = {threadId , 2};
			allThreads.push_back(newThread_2);
			for(wit2 = waccessmap.begin() ; wit2!=waccessmap.end() ; wit2++)
			{
				wit2->second.push_back(newThread_2);
			}
			wthreadVector.push_back(newThread);
			waccessmap.insert( std::pair<int ,std::vector<struct thread> >(wId,wthreadVector));	

		}
	}
	else
	{ 

		std::vector<struct thread> ::iterator wthreadIt;
		int found =0, alreadyLocked = 0; ADDRINT varthread = 2;
		for(wthreadIt = wit->second.begin(); wthreadIt != wit->second.end(); wthreadIt++)
		{
			if(wthreadIt->status==1)
			{
				varthread = wthreadIt->threadIdentity;
				alreadyLocked  =1;
			}
		}
		if(varthread!=threadId)
		{

			if(alreadyLocked==1)
			{

				for(wthreadIt = wit->second.begin(); wthreadIt != wit->second.end(); wthreadIt++)
				{
					if(wthreadIt->threadIdentity==threadId )
					{
						found  =1;
						wthreadIt->status = 0;
					}
				}
				if(found==0)
				{

					threadCount++;
					thread newThread = {threadId ,0 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(wit2 = waccessmap.begin() ; wit2!=waccessmap.end() ; wit2++)
					{
						if(wit2==wit)
							wit2->second.push_back(newThread);
						else
							wit2->second.push_back(newThread_2);
					}	
				}	
			}
			else
			{

				for(wthreadIt = wit->second.begin(); wthreadIt != wit->second.end(); wthreadIt++)
				{
					if(wthreadIt->threadIdentity==threadId)
					{
						found  =1;
						wthreadIt->status = 1;
					}
				}
				if(found==0)
				{
					threadCount++;
					thread newThread = {threadId ,1 };
					thread newThread_2 = {threadId , 2};
					allThreads.push_back(newThread_2);
					for(wit2 = waccessmap.begin() ; wit2!=waccessmap.end() ; wit2++)
					{
						if(wit2==wit)
							wit2->second.push_back(newThread);
						else
							wit2->second.push_back(newThread_2);
					}
				}		
			}
			int i;
			for(i=0;i<=threadCount;i++)
			{
				int a; ADDRINT b;
				a = isWaitingw(varthread);
				if(a == 2)
				{
					return;
				}
				else
				{

					//cout<<"Lock call-L1 entering DEAD\n";
					b = isLockedw(a);
					if(b==threadId)
					{
						cout<<"DEADLOCK detected!!\n"; dflag=1;
							outFile<<"Dead Lock Created";
						ofstream visual;
						visual.open("dead.dot");
						visual<< "digraph test{"<<endl;
						std::vector<struct thread> ::iterator sthreadIt;
						for(wit=waccessmap.begin(); wit!=waccessmap.end(); wit++)
						{
							visual<<"	RW_Lock"<<it->first<<"[fillcolor = \"sienna\" style = filled];"<<endl;
						}
						for(wthreadIt=allThreads.begin(); wthreadIt!=allThreads.end(); wthreadIt++)
						{
							visual<<"	Thread"<<wthreadIt->threadIdentity<<"[fillcolor = \"turquoise\" style = filled];"<<endl;
						}
						for(wit=waccessmap.begin(); wit!=waccessmap.end(); wit++)
						{
							for(wthreadIt =wit->second.begin(); wthreadIt!=wit->second.end(); wthreadIt++)
							{
								if(wthreadIt->status==0)
									visual<<"	Thread"<<wthreadIt->threadIdentity<<"-> RW_Lock"<<wit->first<<"[color = \"red\"];"<<endl;
								else if(wthreadIt->status==1)
									visual<<"	Thread"<<wthreadIt->threadIdentity<<"-> RW_Lock"<<wit->first<<"[color = \"green\"];"<<endl;
							}
						}
						visual<<"}"<<endl;
						visual.close();
						//system("./graphviz.sh");
						FILE *fp = popen("./graphD.sh","r");
						fp++;
						exit(0);
					}
					else
					{
						varthread = b;
					}
				}
			}
		}
		
	}
	
	
}


void unlock(char * rtnName,THREADID threadId, ADDRINT mutexId)
{
	it = accessmap.find(mutexId);
	std::vector<struct thread> ::iterator threadIt;
	for(threadIt = it->second.begin(); threadIt != it->second.end() ; threadIt++)
	{
		
		if(threadIt->threadIdentity==threadId)
		{
			threadIt->status = 2;
		}
	}
}

void sunlock(char * rtnName,THREADID threadId, ADDRINT sId)
{
	sit = saccessmap.find(sId);
	std::vector<struct thread> ::iterator sthreadIt;
	for(sthreadIt = sit->second.begin(); sthreadIt !=sit->second.end() ; sthreadIt++)
	{
		
		if(sthreadIt->threadIdentity==threadId)
		{
			sthreadIt->status = 2;
		}
	}
}

void wunlock(char * rtnName,THREADID threadId, ADDRINT wId)
{
	wit = waccessmap.find(wId);
	std::vector<struct thread> ::iterator wthreadIt;
	for(wthreadIt = wit->second.begin(); wthreadIt !=wit->second.end() ; wthreadIt++)
	{
		
		if(wthreadIt->threadIdentity==threadId)
		{
			wthreadIt->status = 2;
		}
	}
}


INT32 Usage()
{
    outFile<<"This Pintool detects deadlock.\n" <<endl;
    return 2;
}

VOID Fini(INT32 code, VOID *v)
{
if (dflag==0) 
	cout << "\nNo deadlock found!"<<endl;
}
int main(int argc, char * argv[])
{
	//int c ,d =1, threadId , mutexId;
	print();
	PIN_InitSymbols();

	outFile.open(StripPath(argv[argc-1]));
	out.open("Thread.out");
	



    if (PIN_Init(argc, argv)) return Usage();
   
	
    //PIN_InitLock(&lock);
    
    /*RegSkipNextR = PIN_ClaimToolRegister();
    if (!REG_valid(RegSkipNextR)){
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
    }
	
    RegSkipNextW = PIN_ClaimToolRegister();
    if (!REG_valid(RegSkipNextW)){
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
    }
*/
    

    IMG_AddInstrumentFunction(Image, 0);
    //INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(Routine, 0);
	
    // Register Analysis routines to be called when a thread begins/ends
   /* PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);*/
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    
     PIN_StartProgram();

	
	
	return 0;
}
