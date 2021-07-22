
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include "mysql.h"
#include "event.h"
#include "log.h"
#include "utils.h"
#include <json/json.h>
#include "lcurl.h"
#include "xzypub.h"
using namespace xzy;

namespace Event
{
	//返回CEventproxy的注册事件
	int IEventHandle::register_event(int fd, EventType type)
	{
		WriteLog(DEBUG, "constructor ip %d, port %d\n", fd, type);
		return CEventProxy::instance()->register_event(fd, this, type);

	}
	//返回CEventproxy的注册事件
	int IEventHandle::register_event(Socket::ISocket& socket, EventType type)
	{
		return CEventProxy::instance()->register_event(socket, this, type);//CEventProxy *eventproxy 
	}
	//关闭CEventProxy 的事件
	int IEventHandle::shutdown_event(int fd)
	{
		return CEventProxy::instance()->shutdown_event(fd);
	}
	//关闭CEventProxy 的事件
	int IEventHandle::shutdown_event(Socket::ISocket& socket)
	{
		return CEventProxy::instance()->shutdown_event(socket);
	}


	CNetObserver::CNetObserver(INetObserver& obj, EventType regevent) :
		m_regevent(regevent), m_obj(obj), m_refcount(1)
	{
		WriteEnter;
	}

	CNetObserver::~CNetObserver()
	{
		WriteEnter;
	}
	//计数加一
	void CNetObserver::addref()
	{
		WriteEnter;
		Pthread::CGuard guard(m_refcount_mutex);
		m_refcount++;
		WriteLog(DEBUG, "%d\n", m_refcount);
	}
	//计数减一
	inline void CNetObserver::subref()
	{
		WriteEnter;
		Pthread::CGuard guard(m_refcount_mutex);
		m_refcount--;
		WriteLog(DEBUG, "%d\n", m_refcount);

	}
	//计数减一，当计数为0，返回true
	bool CNetObserver::subref_and_test()
	{
		WriteEnter;
		Pthread::CGuard guard(m_refcount_mutex);
		m_refcount--;
		WriteLog(DEBUG, "%d\n", m_refcount);

		return (m_refcount == 0x00);
	}
	//删除自身
	void CNetObserver::selfrelease()
	{
		WriteEnter;
		delete this;
	}
	//返回cnetobserver的eventtype
	EventType CNetObserver::get_regevent()
	{
		WriteEnter;
		return m_regevent;
	}
	//返回Inetobserver的地址
	const INetObserver* CNetObserver::get_handle()
	{
		WriteEnter;
		return &m_obj;
	}
	//inetobserver的读事件调用
	void CNetObserver::handle_in(int fd)
	{
		WriteEnter;
		m_obj.handle_in(fd);
	}
	//inetobserver的写事件调用
	void CNetObserver::handle_out(int fd)
	{
		WriteEnter;
		m_obj.handle_out(fd);
	}
	//inetobserver的关闭事件调用
	void CNetObserver::handle_close(int fd)
	{
		m_obj.handle_close(fd);
	}
	//inetobserver的错误事件调用
	void CNetObserver::handle_error(int fd)
	{
		WriteEnter;
		m_obj.handle_error(fd);
	}

	//事件初始化，m_epollfd生成，eventwait_thread线程开启
	CEvent::CEvent(size_t neventmax) :m_detectionthread(0)
	{
		WriteEnter;
		bzero((void*)&m_eventbuff, sizeof(m_eventbuff));

		m_epollfd = epoll_create(neventmax);
		assert(m_epollfd >= 0);

		m_ithreadpool = ThreadPool::CThreadPoolProxy::instance();//创建线程池

		pthread_t tid = 0;//创建线程阻塞在eventwait_thread的epoll_wait()监听socket。fd
		if (pthread_create(&tid, NULL, eventwait_thread, (void*)this) == 0)
			m_detectionthread = tid;
		//eventwait_thread((void *)this);

/*		pthread_t tid1 = 0;//创建线程阻塞在eventwait_thread的epoll_wait()监听socket。fd
		if (pthread_create(&tid1, NULL, get_imei, (void*)this) == 0)
			m_detectionthread1 = tid1;
			*/
		WriteExit;
	}

	string readfile(const char* file)
	{
		WriteEnter;
		printf("%s\n", file);
		struct stat filestat;
		stat(file, &filestat);
		const size_t filesize1 = filestat.st_size;
		char* fbuff1 = new char[filesize1 + 1]();
		assert(fbuff1 != NULL);
		FILE* fp1 = fopen(file, "rb+");
		if (fp1 == NULL || fread(fbuff1, filesize1, 1, fp1) != 0x01)
		{
			delete[]fbuff1;
			fbuff1 = NULL;
			if (fp1 != NULL)
				fclose(fp1);
		}
		std::string id1 = "";
		if (fbuff1 != NULL)
		{
			id1 = fbuff1;
			delete[]fbuff1;
		}
		if (fp1 != NULL)
			fclose(fp1);
		WriteExit;
		return id1;
	}

	//写数据到file中
	void writefile(const char* file, string text)
	{
		WriteEnter;
		FILE* fp1 = fopen(file, "rb+");
		fseek(fp1, 0, SEEK_SET);
		fprintf(fp1, text.c_str());
		int ret = fclose(fp1);
		WriteLog(DEBUG, "ret:%d\n", ret);
	}
/*
	void* select_sql(Mysql::IMysql* m_imysql, Json::Value& result, string sql)
	{
		WriteLog(DEBUG, "sqlid:%s\n", sql.c_str());

		if (-1 == m_imysql->mysqlqueryb(sql.c_str(), result))
			return NULL;
	}
	//从json文件中读数据
	string getvalue(Json::Value& result, const char* key, string id)
	{
		string mpreid = id;
		if (result.isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
		{
			Json::Value node;

			std::string url;
			for (int i = 0; i < result.size(); i++)
			{
				//cout << result.size() << endl;
				WriteLog(DEBUG, "result:%d\n", result.size());
				node = result[i];

				if (node.isObject())
				{
					if (node[key].isNull())
						mpreid = id;
					else
						mpreid = node[key].asString();

					WriteLog(DEBUG, "key:%s\n", mpreid.c_str());
				}
			}
		}
		return mpreid;
	}
/*
	void callbackandroidpre(Json::Value& result, string id1, string mpreid, FILE* fpw)
	{
		if (result.isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
		{
			Json::Value node1;
			std::string md5_imei;
			std::string url1;
			std::string id;
			for (int i = 0; i < result.size(); i++)
			{
				node1 = result[i];

				if (node1.isObject())
				{
					if (node1["md5_imei"].isNull())
						break;
					md5_imei = node1["md5_imei"].asString();
					url1 = node1["url"].asString();
					id = node1["id"].asString();
					printf("%s--md5:%s,:%s\n", id.c_str(), url1.c_str(), md5_imei.c_str());

					url1 += "&imei=" + md5_imei + "&event_type=0";
					WriteLog(INFO, "urllog:---->  %s:%s\n", url1.c_str(), id.c_str());
					//string strURL = "http://www.wwewei.com/ad/jhit?adid=1636137251643405&cid=1636138844047368&imei=4155014b012a95ec6e2f031918096bd6&mac=88:5A:06:2C:27:AF&androidid=&os=0&timestamp=1560399675000&csite=900000000&ctype=3&ip=27.154.133.25&callback_url=http%3A%2F%2Fad.toutiao.com%2Ftrack%2Factivate%2F%3Fcallback%3DCI3AwJ3ugfQCEIiQ6ZT0gfQCGMvQoJqMtMEHIMvQoJqMtMEHKAAwDjiv3-SuA0IpMDMyZWIwMmEtNTkyMy00Nzc2LWFjN2ItYWMzZjE3NDcyZmI3dTE0NDBIgNKTrQNQAA%3D%3D%26os%3D0%26muid%3D4155014b012a95ec6e2f031918096bd6";
					string strResponse;
					CURLcode nRes = CommonTools::HttpGet(url1, strResponse, 300);
					fprintf(fpw, "%s %s\ntoutiaoid:%s\tmin:%s\tmax:%s\n%s\n", xzy::now(), url1.c_str(), id.c_str(), id1.c_str(), mpreid.c_str(), strResponse.c_str());
				}
			}
		}

	}

	void callbackiospre(Json::Value result, string id1, string mpreid, FILE* fpw)
	{
		if (result.isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
		{
			Json::Value node1;
			std::string md5_imei;
			std::string url1;
			std::string id;
			for (int i = 0; i < result.size(); i++)
			{
				node1 = result[i];

				if (node1.isObject())
				{
					if (node1["md5_imei"].isNull())
						break;
					md5_imei = node1["md5_imei"].asString();
					url1 = node1["url"].asString();
					id = node1["id"].asString();
					printf("%s--md5:%s,:%s\n", id.c_str(), url1.c_str(), md5_imei.c_str());

					url1 += "&idfa=" + md5_imei + "&event_type=0";
					WriteLog(INFO, "urllog:---->  %s:%s\n", url1.c_str(), id.c_str());
					//string strURL = "http://www.wwewei.com/ad/jhit?adid=1636137251643405&cid=1636138844047368&imei=4155014b012a95ec6e2f031918096bd6&mac=88:5A:06:2C:27:AF&androidid=&os=0&timestamp=1560399675000&csite=900000000&ctype=3&ip=27.154.133.25&callback_url=http%3A%2F%2Fad.toutiao.com%2Ftrack%2Factivate%2F%3Fcallback%3DCI3AwJ3ugfQCEIiQ6ZT0gfQCGMvQoJqMtMEHIMvQoJqMtMEHKAAwDjiv3-SuA0IpMDMyZWIwMmEtNTkyMy00Nzc2LWFjN2ItYWMzZjE3NDcyZmI3dTE0NDBIgNKTrQNQAA%3D%3D%26os%3D0%26muid%3D4155014b012a95ec6e2f031918096bd6";
					string strResponse;
					CURLcode nRes = CommonTools::HttpGet(url1, strResponse, 300);
					fprintf(fpw, "%s %s\ntoutiaoid:%s\tmin:%s\tmax:%s\n%s\n", xzy::now(), url1.c_str(), id.c_str(), id1.c_str(), mpreid.c_str(), strResponse.c_str());
				}
			}
		}

	}

	void callbackandroidreg(Json::Value result, string id, string mid, FILE* fpw)
	{
		if (result.isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
		{
			Json::Value node;
			std::string md5_imei;
			std::string url;
			std::string idm;
			for (int i = 0; i < result.size(); i++)
			{
				node = result[i];

				if (node.isObject())
				{
					if (node["md5_imei"].isNull())
						break;
					md5_imei = node["md5_imei"].asString();
					url = node["url"].asString();
					idm = node["id"].asString();
					WriteLog(DEBUG, "md5:%s,:%s\n", url.c_str(), mid.c_str());

					url += "&imei=" + md5_imei + "&event_type=1";
					WriteLog(INFO, "urllog--->:%s,:%s\n\n\n", url.c_str(), idm.c_str());
					//string strURL = "http://www.wwewei.com/ad/jhit?adid=1636137251643405&cid=1636138844047368&imei=4155014b012a95ec6e2f031918096bd6&mac=88:5A:06:2C:27:AF&androidid=&os=0&timestamp=1560399675000&csite=900000000&ctype=3&ip=27.154.133.25&callback_url=http%3A%2F%2Fad.toutiao.com%2Ftrack%2Factivate%2F%3Fcallback%3DCI3AwJ3ugfQCEIiQ6ZT0gfQCGMvQoJqMtMEHIMvQoJqMtMEHKAAwDjiv3-SuA0IpMDMyZWIwMmEtNTkyMy00Nzc2LWFjN2ItYWMzZjE3NDcyZmI3dTE0NDBIgNKTrQNQAA%3D%3D%26os%3D0%26muid%3D4155014b012a95ec6e2f031918096bd6";
					string strResponse;
					CURLcode nRes = CommonTools::HttpGet(url, strResponse, 300);
					fprintf(fpw, "%s %s\ntoutiaoid:%s\t ev_loginminid:%s\t ev_loginmaxid:%s\n%s\n", xzy::now(), url.c_str(), idm.c_str(), id.c_str(), mid.c_str(), strResponse.c_str());
				}
			}
		}
	}

	void callbackiosreg(Json::Value result, string id, string mid, FILE* fpw)
	{
		if (result.isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
		{
			Json::Value node;
			std::string md5_imei;
			std::string url;
			std::string idm;
			for (int i = 0; i < result.size(); i++)
			{
				node = result[i];

				if (node.isObject())
				{
					if (node["md5_imei"].isNull())
						break;
					md5_imei = node["md5_imei"].asString();
					url = node["url"].asString();
					idm = node["id"].asString();
					WriteLog(DEBUG, "md5:%s,:%s\n", url.c_str(), mid.c_str());

					url += "&idfa=" + md5_imei + "&event_type=1";
					WriteLog(INFO, "urllog--->:%s,:%s\n\n\n", url.c_str(), idm.c_str());
					//string strURL = "http://www.wwewei.com/ad/jhit?adid=1636137251643405&cid=1636138844047368&imei=4155014b012a95ec6e2f031918096bd6&mac=88:5A:06:2C:27:AF&androidid=&os=0&timestamp=1560399675000&csite=900000000&ctype=3&ip=27.154.133.25&callback_url=http%3A%2F%2Fad.toutiao.com%2Ftrack%2Factivate%2F%3Fcallback%3DCI3AwJ3ugfQCEIiQ6ZT0gfQCGMvQoJqMtMEHIMvQoJqMtMEHKAAwDjiv3-SuA0IpMDMyZWIwMmEtNTkyMy00Nzc2LWFjN2ItYWMzZjE3NDcyZmI3dTE0NDBIgNKTrQNQAA%3D%3D%26os%3D0%26muid%3D4155014b012a95ec6e2f031918096bd6";
					string strResponse;
					CURLcode nRes = CommonTools::HttpGet(url, strResponse, 300);
					fprintf(fpw, "%s %s\ntoutiaoid:%s\t ev_loginminid:%s\t ev_loginmaxid:%s\n%s\n", xzy::now(), url.c_str(), idm.c_str(), id.c_str(), mid.c_str(), strResponse.c_str());
				}
			}
		}
	}

	void* CEvent::get_imei(void* arg)
	{
		GlobalConfig* globalconfig = GlobalConfig::instance();
		GlobalConfig::Mysql mysql = globalconfig->mysql;

		Mysql::IMysql* m_imysql = NULL;
		m_imysql = new Mysql::CMysql(mysql.host, mysql.user, mysql.pass, mysql.port, mysql.db);
		if (!m_imysql->mysqlinit())
			return NULL;
		for (;;)
		{
			string file1 = GetName("off1.txt");
			string file2 = GetName("log.txt");
			FILE* fpw = fopen(file2.c_str(), "ab+");
			if (fpw == NULL)
			{
				printf("file failed!\n");
			}
			Json::Value result;
			std::string id1 = readfile(file1.c_str());//取得数据库中的id
			std::string sql = "SELECT max(pre.id) mpreid FROM app.ev_prelogin pre WHERE pre.id>= " + id1;

			select_sql(m_imysql, result, sql);//查询数据库

			std::string mpreid = getvalue(result, "mpreid", id1);
			cout << "endl;" << mpreid << endl;
			cout << "endl;" << mpreid << endl;

			writefile(file1.c_str(), mpreid);//把新的id值写回文件中。


			//android
			//id 记录的是 范围id，现在文件中记录的是符合标准的激活id便于后续查询结果
			std::string sqlandroid = "select id,md5_imei,url from (SELECT id,md5_imei,md5_mac,url FROM test.toutiao_imei WHERE md5_mac IN(SELECT md5(imei) FROM app.ev_prelogin WHERE id >"
				+ id1 + " and id <= " + mpreid + " and channel<>'ios' )  AND md5_imei<>\"\" AND id>(SELECT id FROM test.toutiao_imei WHERE DATE(udt)>=DATE_SUB(CURDATE(),INTERVAL 2 DAY)limit 1) ORDER BY id DESC)a GROUP BY md5_mac;";
			printf("sqlx:%s\n---", sqlandroid.c_str());
			select_sql(m_imysql, result, sqlandroid);

			callbackandroidpre(result, id1, mpreid, fpw);//回传安卓数据
			//ios
			std::string sqlios = "select id,md5_imei,url from (SELECT id,md5_imei,md5_mac,url FROM test.toutiao_imei WHERE md5_imei IN(SELECT imei FROM app.ev_prelogin WHERE id >"
				+ id1 + " and id <= " + mpreid + " and channel='ios' )  AND md5_imei<>\"\" and os='ios' AND id>(SELECT id FROM test.toutiao_imei WHERE DATE(udt)>=DATE_SUB(CURDATE(),INTERVAL 2 DAY)limit 1) ORDER BY id DESC)a GROUP BY md5_mac;";
			printf("sqlios:%s\n---", sqlios.c_str());
			select_sql(m_imysql, result, sqlios);
			callbackiospre(result, id1, mpreid, fpw);//回传ios数据

			//*********************************注册数据**************************** //
			string file = GetName("off.txt");
			std::string id = readfile(file.c_str());//注册表上次记录的id值

			string sqlid = "SELECT max(ev.id) mid FROM app.ev_login ev WHERE ev.id>= " + id;
			WriteLog(INFO, "sqlidt1:%s\n", sqlid.c_str());
			select_sql(m_imysql, result, sqlid);
			string mid = getvalue(result, "mid", id);
			callbackandroidpre(result, id, sqlid, fpw);//回传安卓注册数据

			mid = getvalue(result, "mid", id);
			writefile(file.c_str(), mid);//把最新的注册id写回文件中

			//android 注册
			string sqlandroidreg = "select id,md5_imei,url from ( SELECT id,md5_imei,md5_mac,url FROM test.toutiao_imei WHERE md5_mac IN ( SELECT MD5(au.imei) FROM app.ev_login ev,app.user au WHERE ev.id > "
				+ id + " and ev.id <=" + mid +
				" AND au.uid=ev.uid AND ev.event in (\"register\",\"wxreg\") GROUP BY ev.uid )  AND md5_imei<>\"\"  and os='android' AND id>(SELECT id FROM test.toutiao_imei WHERE DATE(udt)>=DATE_SUB(CURDATE(),INTERVAL 2 DAY)limit 1) ORDER BY id DESC)a GROUP BY md5_mac;";
			select_sql(m_imysql, result, sqlandroidreg);
			callbackandroidreg(result, id, mid, fpw);
			//ios 注册

			string sqliosreg = "select id,md5_imei,url from ( SELECT id,md5_imei,md5_mac,url FROM test.toutiao_imei WHERE md5_imei IN ( SELECT au.imei FROM app.ev_login ev,app.user au WHERE ev.id > "
				+ id + " and ev.id <=" + mid +
				" AND au.uid=ev.uid AND ev.event in (\"register\",\"wxreg\") GROUP BY ev.uid )  AND md5_imei<>\"\"  and os='ios' AND id>(SELECT id FROM test.toutiao_imei WHERE DATE(udt)>=DATE_SUB(CURDATE(),INTERVAL 2 DAY)limit 1) ORDER BY id DESC)a GROUP BY md5_mac;";
			select_sql(m_imysql, result, sqliosreg);
			callbackiosreg(result, id, mid, fpw);

			fclose(fpw);
			sleep(60);
		}
		delete m_imysql;
	}
*/
	CEvent::~CEvent()
	{
		WriteEnter;
		if (m_detectionthread != 0 && pthread_cancel(m_detectionthread) == 0) {	//cancel thread
			pthread_join(m_detectionthread, (void**)NULL);
		}
		if (!INVALID_FD(m_epollfd))
			close(m_epollfd);
		WriteExit;
	}

	//注册事件  ,保存observer和eventType
	int CEvent::register_event(int fd, IEventHandle* handle, EventType type)
	{
		WriteEnter;
		if (INVALID_FD(fd) || INVALID_FD(m_epollfd) || INVALID_POINTER(handle)) {
			seterrno(EINVAL);
			return -1;
		}

		struct epoll_event newevent;
		memset(&newevent, 0, sizeof(newevent));
		newevent.data.fd = fd;
		newevent.events = type;
		//newevent.data.u64 = 0;

		ExistRet ret = isexist(fd, type, handle);//用于判断handle和event的type 没有add，不同 则修改EPOLL_CTL_MOD
		if (ret == Existed)
			return 0;

		/*
		* epoll_ctl先执行会出现注册后立即产生事件
		* 但是此时未执行到record记录导致丢失事件的问题
		*/
		record(fd, type, handle);
		if (ret == HandleModify)//epoll 关心fd和type,只改变了handle,说明fd已存在,可以直接返回.
			return 0;

		int opt;
		if (ret == TypeModify || ret == Modify)
			opt = EPOLL_CTL_MOD;
		else if (ret == NotExist)
			opt = EPOLL_CTL_ADD;
		WriteLog(INFO, "fd %d,opt:%d \n", fd, opt);
		if (epoll_ctl(m_epollfd, opt, fd, &newevent) < 0) {
			WriteLog(INFO, "epoll op %d, fd %#x error\n", opt, fd);
			detach(fd, true);
			return -1;
		}
		WriteExit;
		return 0;
	}

	//删除事件
	int CEvent::unregister_event(int fd)
	{
		WriteEnter;
		if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
			WriteLog(INFO, "epoll delete fd %d failed\n", fd);
			return -1;
		}
		WriteExit;
		return detach(fd);
	}

	int CEvent::shutdown_event(int fd)
	{
		WriteLog(DEBUG, "sock[%#X] shutdown event\n", fd);
		return ::shutdown(fd, SHUT_WR);//禁止该套接口写操作
		/*close把描述符的引用计数减1，仅在该计数变为0时关闭套接字。shutdown可以不管引用计数就激发TCP的正常连接终止序列*/
	}

	//返回结果，用于后续绑定事件类型
	CEvent::ExistRet CEvent::isexist(int fd, EventType type, IEventHandle* handle)
	{
		WriteEnter;
		Pthread::CGuard guard(m_eventreg_mutex);
		EventMap_t::iterator itor = m_eventreg.find(fd);
		if (itor == m_eventreg.end()) {	// not existed
			return NotExist;//ctl(EPOLL_CTL_ADD)
		}

		CNetObserver& eventhandle = *(itor->second);
		EventType oldregtype = eventhandle.get_regevent();
		const INetObserver* oldreghandle = eventhandle.get_handle();
		if (oldregtype != type && oldreghandle != handle) //Whole modify
			return Modify;
		else if (oldregtype != type)
			return TypeModify;
		else if (oldreghandle != handle)
			return HandleModify;
		WriteExit;
		return Existed;
	}
	//记录 fd和CNetObserver
	int CEvent::record(int fd, EventType type, IEventHandle* handle)
	{
		WriteEnter;
		CNetObserver* newobserver = new CNetObserver(*handle, type);
		assert(newobserver != NULL);

		Pthread::CGuard guard(m_eventreg_mutex);
		m_eventreg[fd] = newobserver;
		WriteExit;
		return 0;
	}
	//移除 fd和CNetObserver
	int CEvent::detach(int fd, bool release)
	{
		WriteEnter;
		Pthread::CGuard guard(m_eventreg_mutex);
		EventMap_t::iterator itor = m_eventreg.find(fd);
		if (itor == m_eventreg.end()) {	// not existed
			return -1;
		}

		if (release)
			itor->second->selfrelease();

		m_eventreg.erase(itor);
		WriteExit;
		return 0;
	}

	//返回 fd对于的CNetObserver
	CNetObserver* CEvent::get_observer(int fd)
	{
		WriteEnter;
		Pthread::CGuard guard(m_eventreg_mutex);
		EventMap_t::iterator itor = m_eventreg.find(fd);
		if (itor == m_eventreg.end())
			return NULL;

		CNetObserver* observer = m_eventreg[fd];
		observer->addref();
		WriteExit;
		return observer;
	}
	//push fd和 eventtype
	int CEvent::pushtask(int fd, EventType event)
	{
		WriteEnter;
		Pthread::CGuard guard(m_events_mutex);
		EventTask_t::iterator itor = m_events.find(fd);
		if (itor == m_events.end()) {
			m_events[fd] = event;
			return 0;
		}
		// exist, update it
		itor->second = (EventType)(itor->second | event);
		WriteExit;
		return 0;
	}
	//移除 fd和eventtype
	int CEvent::poptask(int& fd, EventType& event)
	{
		WriteEnter;
		Pthread::CGuard guard(m_events_mutex);
		EventTask_t::iterator itor = m_events.begin();
		if (itor == m_events.end())
			return -1;

		fd = itor->first;
		event = itor->second;

		m_events.erase(itor);
		WriteExit;
		return 0;
	}

	//移除fd和对应的eventtype// fd=-1 可以全部清空
	int CEvent::cleartask(int fd)
	{
		WriteEnter;
		Pthread::CGuard guard(m_events_mutex);
		if (fd == -1)
		{	// clear all
			m_events.clear();
			return 0;

		}
		else if (fd >= 0)
		{
			EventTask_t::iterator itor = m_events.find(fd);
			if (itor == m_events.end())
				return -1;

			m_events.erase(itor);
			return 0;
		}
		WriteExit;
		return -1;
	}
	//返回task size
	size_t CEvent::tasksize()
	{
		//LOG1("-----\n");
		WriteEnter;
		Pthread::CGuard guard(m_events_mutex);
		return m_events.size();
	}

	void CEvent::threadhandle()
	{
		WriteEnter;
		// LOG1("-----\n");
		int fd = 0x00;
		EventType events;
		if (poptask(fd, events) < 0) {
			return;
		}
		CNetObserver* observer = get_observer(fd);
		if (observer == NULL)
			return;

		/*
		* 关闭时递减引用计数。在对象的所有回调处理完时真正释放
		*/
		if (events & ECLOSE) {//对等方关闭连接
			WriteLog(DEBUG,"close");
			cleartask(fd);		//移除fd 计数-1
			observer->subref();

		}
		else {
			if (events & EERR) {
				WriteLog(DEBUG, "EERR");
				observer->handle_error(fd);
			}
			if (events & EIN) {
				WriteLog(DEBUG, "EIN");
				observer->handle_in(fd);
			}
			if (events & EOUT) {
				WriteLog(DEBUG, "EOUT");
				observer->handle_out(fd);
			}

		}

		/*
		* unregister_event 执行后于handle_close将会出现当前套接字关闭后
		* 在仍未执行完unregister_event时新的连接过来，得到一样的描述符
		* 新的连接调用register_event却未注册进入epoll。同时han_close中
		* 关闭了套接字，unregister_event中epoll删除关闭的套接字报错
		*/
		WriteLog(DEBUG, "m_refcount::  %d", observer->m_refcount);
		if (observer->subref_and_test()) {
			WriteLog(DEBUG, "subref_and_test");
			unregister_event(fd);
			observer->handle_close(fd);
			observer->selfrelease();
		}
		WriteExit;
	}
	//事件等待线程当有epoll触发，才会运行pushtask函数fd和eventtype相应绑定的
	void* CEvent::eventwait_thread(void* arg)
	{
		WriteEnter;
		CEvent& cevent = *(CEvent*)(arg);
		if (INVALID_FD(cevent.m_epollfd)) {
			seterrno(EINVAL);
			pthread_exit(NULL);
		}

		for (;;) {
			int nevent = epoll_wait(cevent.m_epollfd, &cevent.m_eventbuff[0], EventBuffLen, -1);
			if (nevent < 0 && errno != EINTR) {
				WriteLog(INFO, "epoll wait error\n");
				break;
			}

			for (int i = 0; i < nevent; i++) {

				int fd = cevent.m_eventbuff[i].data.fd;
				
				EventType events = static_cast<EventType>(cevent.m_eventbuff[i].events);
				WriteLog(DEBUG, "sock[%#X] epoll_wait: %d,EventType: %d\n", fd, fd,events);
				if (cevent.pushtask(fd, events) == 0x00) {
					cevent.m_ithreadpool->pushtask(&cevent);
				}
			}
		}
		pthread_detach(pthread_self());
		pthread_exit(NULL);
	}



	//申请CEvent（1024）个空间  m_event
	CEventProxy::CEventProxy(size_t neventmax)
	{
		m_event = new CEvent(neventmax);
	}
	//删除相应的Cevent  m_event
	CEventProxy::~CEventProxy()
	{
		if (m_event)
			delete m_event;
	}
 
	//第一次申请空间，之后返回一个CEventProxy的地址
	CEventProxy* CEventProxy::instance()
	{
		static CEventProxy* eventproxy = NULL;//第一次进入函数的时候会被初始化，之后不会
		if (eventproxy == NULL)
			eventproxy = new CEventProxy(NEVENT_MAX);
		return eventproxy;
	}
	//CEventproxy注册事件,根据fd进行epoll_ctl...
	int CEventProxy::register_event(int fd, IEventHandle* handle, EventType type)
	{
		return m_event->register_event(fd, handle, type);
	}
	//CEventproxy注册事件
	int CEventProxy::register_event(Socket::ISocket& socket, IEventHandle* handle, EventType type)
	{
		return m_event->register_event(socket.fd(), handle, type);
	}
	//CEventproxy关闭事件
	int CEventProxy::shutdown_event(int fd)
	{
		return m_event->shutdown_event(fd);
	}
	//CEventproxy关闭事件
	int CEventProxy::shutdown_event(Socket::ISocket& socket)
	{
		return m_event->shutdown_event(socket.fd());
	}
}