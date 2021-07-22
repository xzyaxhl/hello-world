#pragma once
#include "task.h"
#include <string>
#include <map>
#include <memory>
#include "mutex.h"
class TaskServerDispatch
{
public:
	static TaskServerDispatch* GetInstance()
	{
		static TaskServerDispatch tf;
		return &tf;
	}

	virtual bool Init();
	void Regsiter(std::string key, Task* task);
	void server(std::string &url, std::shared_ptr<Http::IHttpResponse>,void*);
	~TaskServerDispatch() ;
private:
	TaskServerDispatch();
	std::string m_url;
	std::map<std::string, Task*> ts;
	std::map<Task*, int> calls_del;
	Pthread::CMutex m_factorymutex;
};
