

#include "threadpool.h"
#include "log.h"
#include "utils.h"
#include <iostream>
//#include <sys/types.h>
//#include <sys/syscall.h>
namespace ThreadPool
{
CThreadPool::CThreadPool(size_t threadnum, size_t tasknum): 
	m_taskqueue(tasknum), m_hasleader(false)
{
	WriteEnter;
	m_threadnum = (threadnum > THREADNUM_MAX)? THREADNUM_MAX: threadnum;
	create_threadpool();
	WriteExit;
}

CThreadPool::~CThreadPool() 
{
	WriteEnter;
	destroy_threadpool();
	WriteExit;
}

int CThreadPool::pushtask(IThreadHandle *handle, bool block)
{
	WriteEnter;
	if(block)
		return m_taskqueue.push(handle);
	return m_taskqueue.push_nonblock(handle);
}

void CThreadPool::promote_leader()
{
	WriteEnter;
	Pthread::CGuard guard(m_identify_mutex);
	while(m_hasleader){	// more than one thread can return
		m_befollower_cond.wait(m_identify_mutex);
	}
	m_hasleader = true;
	WriteExit;
}

void CThreadPool::join_follwer()
{
	WriteEnter;
	Pthread::CGuard guard(m_identify_mutex);
	m_hasleader = false;
	WriteLog(DEBUG, "pid: , tid %d\n",pthread_self());
	m_befollower_cond.signal();
	WriteExit;
}

void CThreadPool::create_threadpool()
{
	WriteEnter;
	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	for(size_t i = 0; i < m_threadnum; i++){
		pthread_t tid = 0;
		if(pthread_create(&tid, &thread_attr, process_task, (void *)this) < 0){
			WriteLog(INFO, "create thread[%d] filed\n", (int)i);
			continue;
		}

		m_thread.push_back(tid);
	}
	pthread_attr_destroy(&thread_attr);
	WriteLog(DEBUG, "create thread pool, thread number %d\n", (int)m_thread.size());
	WriteExit;
}

void CThreadPool::destroy_threadpool()
{
	WriteEnter;
	void *retval = NULL;
	vector_tid_t::iterator itor = m_thread.begin();
	for(; itor != m_thread.end(); itor++){
		if(pthread_cancel(*itor) < 0 || pthread_join(*itor, &retval) < 0){
			WriteLog(INFO, "destroy thread[%d]\n", (int)(*itor));
			continue;
		}
	}
	m_thread.clear();
	WriteLog(DEBUG, "destroy thread pool... done\n");
	WriteExit;
}

void *CThreadPool::process_task(void * arg)
{
	CThreadPool &threadpool = *(CThreadPool *)arg;
	while(true){
		WriteEnter;
		threadpool.promote_leader();	
		IThreadHandle *threadhandle = NULL;
		int ret = threadpool.m_taskqueue.pop(threadhandle);//取任务，成功返回0，
		threadpool.join_follwer();		
		if(ret == 0 && threadhandle)
			threadhandle->threadhandle();
		WriteExit;
	}
	pthread_exit(NULL);//显示退出
}
}