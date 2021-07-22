#include <unistd.h>
#include <libgen.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "log.h"
#include "httpserver.h"
#include <json/json.h>
#include "mysqlpool.h"

#include "taskfactory.h"
namespace Http
{

std::string http_path_handle(const std::string &dname, const std::string &bname)
{
	WriteEnter;
	std::string filepath(HTTP_ROOT);
	if(bname == HTTP_SLASH || bname == HTTP_CURRENT_DIR || \
		bname == HTTP_ABOVE_DIR){	// limit in root dir
		filepath += HTTP_SLASH;// /
		filepath += HTTP_DEFAULTFILE; // /indexhtml.
	}else if(dname == HTTP_CURRENT_DIR){
		filepath += HTTP_SLASH;
		filepath += bname;
	}else if(dname == HTTP_SLASH){
		filepath += dname;
		filepath += bname;
	}else{
		filepath += dname;
		filepath += HTTP_SLASH;
		filepath += bname;
	}
	WriteExit;
	return filepath;
}
//dir："/""/img"   base:文件名
void split_url(const std::string &url, std::string &dir, std::string &base)
{
	WriteEnter;
	char *dirc = strdup(url.c_str());
	dir = dirname(dirc);
	free(dirc);
	
	char *basec = strdup(url.c_str());
	base = basename(basec);	
	free(basec);
	WriteExit;
}

HttpStream::HttpStream(Socket::IClient *iclient): m_client(iclient), m_imysql(NULL)
{
	WriteEnter;
	m_readbuff = new char[READBUFF_LEN + 1];
	assert(m_readbuff != NULL);
	//aceept() 后面的非阻塞
	m_client->set_nonblock(true);

	/*
	* 构造最后注册进入epoll
	* 防止未构造完全收到事件回调
	*/
	//handle_in(m_client->fd());
	register_event(*m_client);
	
	WriteLog(DEBUG, "constructor socket[%#X]\n", m_client->fd());
	WriteExit;
}

HttpStream::~HttpStream()
{
	WriteEnter;
	WriteLog(DEBUG, "distructor socket[%#X]\n", m_client->fd());

	delete m_client;
	m_client = NULL;
	
	delete []m_readbuff;
	m_readbuff = NULL;


	delete m_imysql;
	m_imysql = NULL;
	WriteExit;
}

int HttpStream::close()
{
       // LOG1("-----\n");
	return shutdown_event(*m_client);
}
//fd的recv和send()
void HttpStream::handle_in(int fd)
{
	WriteEnter;
	Pthread::CGuard guard(m_readbuffmutex);
	ssize_t nread = m_client->recv(m_readbuff, READBUFF_LEN, MSG_DONTWAIT);
	printf("++++++++++++++++++:--");
	if((nread < 0 && errno != EAGAIN) || nread == 0){	// error or read EOF // nread == EAGAIN 非阻塞操作,没数据可读是返回EAGAIN,提示在读一次
		WriteLog(INFO, "errno:%d",errno);
		close();
		return;
	}else if(nread < 0 && errno == EAGAIN){
		WriteLog(INFO, "non data to read\n");
		return; 
	}
	WriteLog(INFO, "len:%d,errno:%d\n", nread,errno);
	m_readbuff[nread] = 0x00;

	Http::CHttpRequest httprequest;
	//解析request
	if(httprequest.load_packet(m_readbuff, nread) < 0){
		WriteLog(INFO, "parse package error\n");
		return;
	}

	WriteLog(DEBUG, "socket[%d] receive <--- %ld bytes\n", fd, nread);
	//得到response的值
	shared_ptr<Http::IHttpResponse> response(new Http::CHttpResponse);
	WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(),response.use_count());
	handle_request(httprequest, response);
	WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());
	//printf("%s", respose->serialize());

	if(response != NULL){
		WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());

		m_client->send(response->getdata(), response->size(), 0);
		response == NULL;
	}
	WriteExit;

}
void HttpStream::handle_out(int fd)
{
	WriteEnter;
	WriteExit;
}
void HttpStream::handle_close(int fd)
{
	WriteLog(DEBUG, "socket[%d] handle close\n", fd);
	delete this;//自身对象应该是在堆上，释放new申请的对象空间
}

void HttpStream::handle_error(int fd)
{
	close();
}

void HttpStream::handle_request(Http::IHttpRequest& request, shared_ptr<Http::IHttpResponse> response)
{
	WriteEnter;
	WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());
	TaskServerDispatch *taskserver = TaskServerDispatch::GetInstance();
	string url = request.url();
	taskserver->server(url, response, NULL);
	WriteLog(INFO, "response %p,size: %d,count: %d", response.get(), response.get()->size(), response.use_count());
	WriteExit;
	return;
}

int HttpServer::start(size_t backlog)
{
	WriteEnter;
	if(!m_server.isclose())	 //has start
		return 0;
		//server端socket监听  m_sockfd=server段sockfd
	if(m_server.start(backlog) < 0)
		return -1;
	//设置非阻塞模式 true  false 阻塞模式
	if(m_server.set_nonblock(true) < 0)
		//errsys("set socket non block failed\n");
		WriteLog(INFO, "set socket non block failed\n");
	//注册事件
	WriteExit;
	return register_event(m_server);
}
//关闭套接口socket
int HttpServer::close()
{
	return shutdown_event(m_server);
}

HttpServer::HttpServer(const std::string &addr, uint16_t port):

	m_port(port), m_addr(addr), m_server(addr, port)
{
	WriteLog(INFO, "constructor ip %s, port %d\n", addr.c_str(), port);
}

HttpServer::~HttpServer()
{
	WriteLog(INFO, "distructor ip %s, port %d\n", m_addr.c_str(), m_port);
}
//accept()套接字并且注册事件
void HttpServer::handle_in(int fd)
{
	/*
	* 读取所有建立的连接
	*/
	WriteEnter;
	do{
		Socket::IClient *newconn = m_server.accept();
		if(newconn == NULL)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{/*EWOULDBLOCK：用于非阻塞模式，不需要重新读或者写EAGAIN\EINTR：指操作被中断唤醒，需要重新读/写*/
				break;
			}
			else
			{
				errsys("sockfd[%d] accept error\n", fd);
				close();
				return;
			}
		}
		WriteLog(INFO, "socket[%d] accept a conn aceept %d\n", fd, newconn);
		//注册事件
		HttpStream *httpstream = new HttpStream(newconn);// , g);	// self release 
		assert(httpstream != NULL);
	}while(true);
	WriteExit;
}
void HttpServer::handle_out(int fd)
{
}
void HttpServer::handle_close(int fd)
{
	WriteLog(INFO, "socket[%d] handle close\n", fd);
}
void HttpServer::handle_error(int fd)
{	
	close();
}
}