#include "taskfactory.h"
#include "log.h"
#include "clocktime.h"
#include "xzypub.h"
#include "word.h"
#include "file.h"
#include <vector>
#include "PassWord.h"
#include <memory>
using namespace std;

TaskServerDispatch::TaskServerDispatch()
{
	Regsiter("clockintime", new ClockTime);
	Regsiter("word", new Word);
	Regsiter("file", new SelectFile);
	Regsiter("password", new PassWord);
}

TaskServerDispatch::~TaskServerDispatch()
{
	WriteEnter;
	for (auto ptr = calls_del.begin(); ptr != calls_del.end(); ptr++)
	{
		delete ptr->first;
	}
}

void TaskServerDispatch::Regsiter(std::string key, Task* task)
{
	if (key.empty()){
		WriteLog(INFO, "TaskServerDispatch key is NULL!");
		return;
	}
	if(!task){
		WriteLog(INFO, "TaskServerDispatch task is NULL!");
		return;
	}
	else {
		WriteLog(INFO, "Task ptr: %p",task);
	}
	if (ts.find(key) != ts.end()){
		WriteLog(DEBUG, "key:%s! register",key.c_str());
		delete this;
		return;
	}
	ts[key] = task;
	//�ռ�����
	calls_del[task] = 0;
}

bool TaskServerDispatch::Init()
{

}

void TaskServerDispatch::server(std::string &url, std::shared_ptr<Http::IHttpResponse> response,void*)
{
	WriteEnter;
	WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());

	m_url = url;
	WriteLog(DEBUG, "url: %s", m_url.c_str());
	std::vector<std::string> cuturl;
	str::split(m_url, cuturl,"/?");
	std::string key = cuturl[0];
	WriteLog(DEBUG, "key: %s", key.c_str());
	
	if (ts.find(key) != ts.end())
	{
		Task* t = ts[key];
		t->m_url = url;
		t->m_response = response;
		WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());
		WriteLog(INFO, "m_response %p,size: %d,count: %d", t->m_response.get(), t->m_response.get()->size(), t->m_response.use_count());

		t->start();
		WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());
		WriteLog(INFO, "m_response %p,size: %d,count: %d", t->m_response.get(), t->m_response.get()->size(), t->m_response.use_count());

	}
	return;
}

