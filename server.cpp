

#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utils.h"
#include "sockopt.h"
#include "client.h"
#include "server.h"
#include "log.h"
namespace Socket
{
CStreamServer::CStreamServer(const std::string &addr, uint16_t port):
m_addr(addr), m_port(port), m_sockfd(-1)
{
}

CStreamServer::~CStreamServer()
{
	close();	// ignore error
}

int CStreamServer::fd()
{
	Pthread::CGuard guard(m_socketmutex);
	return m_sockfd;
}

int CStreamServer::set_nonblock(bool nonblock)
{
	Pthread::CGuard guard(m_socketmutex);
	if(INVALID_FD(m_sockfd))	
		return -1;
		
	return Socket::set_nonblock(m_sockfd, nonblock);
}

//socket,bind,listen
int CStreamServer::start(size_t backlog)
{
	WriteEnter;
	Pthread::CGuard guard(m_socketmutex);
 	if(!INVALID_FD(m_sockfd))	
		return 0;
		
	struct sockaddr_in sockaddr;
	//绑定地址
	if (convert_inaddr(m_addr, m_port, sockaddr) < 0)
	{
		WriteLog(DEBUG, "conver_inaddr failed!");
		return -1;
	}

	int sockfd = 0x00;
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		WriteLog(DEBUG, "socket failed!");
		return -1;
	}
	//设置地址复用 true 复用 false不设置
	set_resuseport(sockfd, true);	// ignore error
	if(bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0){
		::close(sockfd);
		WriteLog(DEBUG, "bind failed!");
		return -1;
	}
	
	if(listen(sockfd, backlog) < 0){
		::close(sockfd);
		WriteLog(DEBUG, "listen failed!");
		return -1;
	}
	
	m_sockfd = sockfd;
	WriteExit;
	return 0;	
}

//close
int CStreamServer::close()
{
	WriteEnter;
	Pthread::CGuard guard(m_socketmutex);
	if(INVALID_FD(m_sockfd))
		return -1;

	if(::close(m_sockfd) < 0);
		return -1;
		
	m_sockfd = -1;
	WriteExit;
	return 0;
}

//判断是不是close了
bool CStreamServer::isclose()
{
	WriteEnter;
	Pthread::CGuard guard(m_socketmutex);
	if(INVALID_FD(m_sockfd))
		return true;
	WriteExit;
	return false;
}

//返回accept()监听的socket套接字
IClient *CStreamServer::accept()
{
	WriteEnter;
	Pthread::CGuard guard(m_socketmutex);
	if (INVALID_FD(m_sockfd)) {
		WriteLog(INFO, "return ");
		return NULL;
	}
	struct sockaddr_in clientaddr;
	socklen_t clientaddrlen = sizeof(clientaddr);
	int clientfd = ::accept(m_sockfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
	trace("sock[%#X] accept::clientfd: %d\n", clientfd, clientfd);

	if(INVALID_FD(clientfd)) {
		WriteLog(INFO, "return ");
		return NULL;
	}
	IClient *client = new CStreamClient(clientfd);
	assert(client != NULL);
	WriteExit;
	return client;
}

}
