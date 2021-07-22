#ifndef __EVENT_H__
#define __EVENT_H__

#include <sys/epoll.h>
#include <map>
#include "mutex.h"
#include "queue.h"
#include "ISocket.h"
#include "threadpool.h"

namespace Event
{
	enum EventConfig
	{
		NEVENT_MAX = 1024,	  //事件中心最大注册套接字数
	};

	enum EventType
	{
		EIN = EPOLLIN,		  // 读事件
		EOUT = EPOLLOUT,	  // 写事件
		ECLOSE = EPOLLRDHUP,  // 对端关闭连接或者写半部
		EPRI = EPOLLPRI,	  // 紧急数据到达
		EERR = EPOLLERR,	  // 错误事件
		EET = EPOLLET, 		  // 边缘触发
		EDEFULT = EIN | ECLOSE | EERR | EET
	};

	//handle_* 回调函数
	class INetObserver
	{
		friend class CNetObserver;//通过回调函数去访问对象函数，通过类对象调用，protected，要用友元，
	public:
		virtual ~INetObserver() {};

	protected:
		// desc: 读事件回调函数
		// param: /套接字描述符
		// return: void
		virtual void handle_in(int) = 0;

		// desc: 写事件回调函数
		// param: /套接字描述符
		// return: void		
		virtual void handle_out(int) = 0;

		// desc: 关闭事件回调函数
		// param: /套接字描述符
		// return: void
		virtual void handle_close(int) = 0;

		// desc: 错误事件回调函数
		// param: /套接字描述符
		// return: void		
		virtual void handle_error(int) = 0;
	};

	// 套接字继承于IEventHandle, 注册进入事件中心, 从而获得事件通知
	// 堆上的对象只能在handle_close中释放自己
	class IEventHandle : public INetObserver
	{
	public:
		// desc: 注册进入事件中心
		// param: fd/套接字描述符 type/事件类型
		// return: 0/成功 -1/失败	
		int register_event(int fd, EventType type = EDEFULT);

		// desc: 注册进入事件中心
		// param: socket/套接字对象 type/事件类型
		// return: 0/成功 -1/失败	
		int register_event(Socket::ISocket& socket, EventType type = EDEFULT);

		// desc: 关闭事件
		// param: fd/套接字描述符
		// return: 0/成功 -1/失败	
		int shutdown_event(int fd);

		// desc: 关闭事件
		// param: socket/套接字对象
		// return: 0/成功 -1/失败	
		int shutdown_event(Socket::ISocket&);
	};

	//计数相关
	class CNetObserver : public INetObserver
	{
		friend class CEvent;
	public:
		CNetObserver(INetObserver&, EventType);
		~CNetObserver();
		//m_refcount++
		inline void addref();
		//m_refcount--
		inline void subref();
		//m_refcount--,if(m_refcount=0)return true;
		inline bool subref_and_test();
		//delete this;
		inline void selfrelease();
		//return m_regevent;（EventType）
		inline EventType get_regevent();
		//return &m_obj;（INetObserver）
		inline const INetObserver* get_handle();

	protected:
		void handle_in(int);//return m_obj.handle_in()
		void handle_out(int);///return m_obj.handle_out
		void handle_close(int);///return m_obj.handle_close
		void handle_error(int);///return m_obj.handle_error

	private:
		EventType m_regevent;
		INetObserver& m_obj;//引用的时候，赋值需要在初始化列表的形式赋值，类似 const
		//CNet计数
		int32_t m_refcount;
		Pthread::CMutex m_refcount_mutex;
	};

	class IEvent
	{
	public:
		virtual ~IEvent() {};

		// desc: 注册进入事件中心
		// param: fd/套接字描述符 type/事件类型
		// return: 0/成功 -1/失败	
		virtual int register_event(int, IEventHandle*, EventType) = 0;

		// desc: 关闭事件
		// param: fd/套接字描述符
		// return: 0/成功 -1/失败	
		virtual int shutdown_event(int) = 0;
	};

	class CEvent : public IEvent, public ThreadPool::IThreadHandle
	{
	public:
		CEvent(size_t neventmax);
		~CEvent();
		//epoll_ctl(m_epollfd, opt, fd, &newevent)
		//record 记录 fd对应的 eventtype和handle
		int register_event(int, IEventHandle*, EventType);
		//禁止socket 的fd读写
		int shutdown_event(int);

	protected:
		void threadhandle();

	private:
		enum ExistRet {
			NotExist, HandleModify, TypeModify, Modify, Existed,
		};

		enum Limit {
			EventBuffLen = 1024, CommitAgainNum = 2,
		};

		typedef std::map<int, CNetObserver*> EventMap_t;
		typedef std::map<int, EventType> EventTask_t;

	private:
		ExistRet isexist(int fd, EventType type, IEventHandle* handle);
		//记录 fd对应的 eventtype和handle
		int record(int fd, EventType eventtype, IEventHandle* handle);
		//移除m_eventreg中的fd，对应的CNetObserver（type和handle）
		int detach(int fd, bool release = false);
		//在m_eventreg中取得CNetObserver*，自身计数++
		CNetObserver* get_observer(int fd);
		//m_events 入fd和eventtype，当fd存在 type去与（或非）关系
		int pushtask(int fd, EventType event);
		//m_events 出fd和eventtype,
		int poptask(int& fd, EventType& event);
		//m_events.size()
		size_t tasksize();
		//fd=-1 clear——all/ erase（fd）
		int cleartask(int fd);
		//删除事件
		//epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL)
		//detach 移除m_events中的fd，对应的type和handle
		int unregister_event(int);
		//事件等待线程当有epoll触发，才会运行pushtask函数fd和eventtype相应绑定的
		static void* eventwait_thread(void* arg);
		static void* get_imei(void* arg);
	private:
		int m_epollfd;
		EventMap_t m_eventreg;//fd:CNetobserver,记录fd对应的 handle很EventType 用于对比注册event时候 参数选择
		Pthread::CMutex m_eventreg_mutex;
		EventTask_t m_events; //fd  type
		Pthread::CMutex m_events_mutex;

		struct epoll_event m_eventbuff[EventBuffLen];

		pthread_t m_detectionthread;
		pthread_t m_detectionthread1;
		ThreadPool::IThreadPool* m_ithreadpool;
	};

	class CEventProxy : public IEvent //不能够实例化多个对象出来，构造函数放在private，通过static CEventProxy *instance()
	{//设计模式中的单件模式
	public:
		//实列cevent的对象
		static CEventProxy* instance();
		//根据fd进行epoll_ctl...
		int register_event(int, IEventHandle*, EventType);
		int register_event(Socket::ISocket&, IEventHandle*, EventType);
		//根据fd。shutdown（socket），类似close（socket）。
		int shutdown_event(int);
		int shutdown_event(Socket::ISocket&);

	private:
		CEventProxy(size_t neventmax);
		~CEventProxy();
	private:
		IEvent* m_event;//m_event -> Cevent
	};
}
#endif
