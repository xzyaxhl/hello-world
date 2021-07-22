#include "thread.h"
#include "log.h"


namespace Pthread
{
	Thread::Thread() :m_pid(0), m_autoDelete(false)
	{

	}

	Thread::~Thread()
	{
	}

	void Thread::Start()
	{
		pthread_attr_init(&m_attr);
		if (pthread_create(&m_pid, &m_attr, RunThread, (void*)this) < 0) {
			WriteLog(INFO, "pthread_create is failed!");
		}
		pthread_attr_destroy(&m_attr);
	}
	void Thread::SetAutoDelete(bool autoDelete)
	{
		m_autoDelete = autoDelete;
	}
	void* Thread::RunThread(void* arg)
	{
		Thread* thread = static_cast<Thread*>(arg);//派生类指针转换成基类指针
		thread->Run();
		if (thread->m_autoDelete)
			delete thread;
		return NULL;
	}

	void Thread::Join()
	{
		pthread_join(m_pid, NULL);
	}

}