#pragma once
#ifndef __THREAD_H__
#define __THREAD_H__


#include <pthread.h>
#include <iostream>


namespace Pthread
{
	class Thread
	{
	public:
		Thread();
		virtual ~Thread();
		void Start();
		void Join();
		void SetAutoDelete(bool autoDelete);
	private:
		static void* RunThread(void* arg);
		virtual void Run() = 0;
		bool m_autoDelete;
		pthread_t m_pid;
		pthread_attr_t m_attr;
	};
}


#endif // !__THREAD_H__