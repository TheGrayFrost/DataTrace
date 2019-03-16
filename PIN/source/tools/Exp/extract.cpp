struct thread
{
	THREADID tid;
	OS_THREAD_ID oid;
	int node_type;  	// 1 for mutexes, 2 for semaphore, 3 for rw_lock
	ADDRINT lock_var;
	char * name;		// to specify routine
};


std::map <ADDRINT tid, std::map<ADDRINT, int>> syncs;

VOID lock_func_af (THREADID tid, ADDRINT addr, int type)
{
	syncs[tid][addr] = type;
}

if (RTN_Name(rtn)== "pthread_mutex_lock")
{
	RTN_Open(rtn);
	RTN_InsertCall(rtn, IPOINT_AFTER, 
		(AFUNPTR)lock_func_af,
		IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
		IARG_ADDRINT, 1,
		IARG_END);
	RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_mutex_unlock")
{
 	RTN_Open(rtn);
	RTN_InsertCall (rtn, IPOINT_BEFORE,
		(AFUNPTR)unlock_func, 
		IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
		IARG_ADDRINT, 1, 
		IARG_END);
	RTN_Close(rtn);
}

if (RTN_Name(rtn)== "sem_wait")
{
 
RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)slock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);


RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)slock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



 RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "sem_post")
{
 
RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)sunlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

 RTN_Close(rtn);
}



 if (RTN_Name(rtn)== "__pthread_rwlock_rdlock" )
 
{
RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);


RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



 RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_rwlock_wrlock" )


{
RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);


RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wlock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



 RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__pthread_rwlock_unlock")
{
 
RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)wunlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

 RTN_Close(rtn);
}





PIN_GetLock(&lock, threadid+1);
//OS_THREAD_ID td;
//td=PIN_GetPid();
THREADID th;

    numThreads++;


  // accessmap.insert(std::pair<THREADID,OS_THREAD_ID>(threadid, td) );
    

  thread_data_tsu* tdata = new thread_data_tsu;
   tdata->_rtncount=0;

  
/*for(tmp=threadmap.begin(); tmp!=threadmap.end(); ++tmp)
{
   if (tmp->second==PIN_GetParentTid())
   {
 tdata->oid=tmp->first;
cout<<"DEKH"<<tmp->first;}
   // if (tmp->second==PIN_GetPid())
    //tdata->oid=tmp->first;
}*/

   
    tdata->oid=PIN_GetParentTid();
  tdata->_rdcount=0;
  tdata->_wrcount=0;
   tdata->_fcount=0;
   strcat(tdata->rtnlist," ");
  
  tdata->t=PIN_GetTid();
//cout<< "THREAD OS ID" <<tdata->t;
   tdata->_fcount=0;  
th=threadid;  


	thlist.push_back(th);

    PIN_SetThreadData(tls_key, tdata, threadid);


   PIN_ReleaseLock(&lock);
}

