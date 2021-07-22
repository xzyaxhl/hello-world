

#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "sockopt.h"
#include "log.h"
namespace Socket
{
	//设置端口复用
int set_resuseport(int sockfd, bool en)
{
	int sbuflen = 1024;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&sbuflen, sizeof(int));
	int enable = en? 0x01: 0x00;
	/*//是否启用keepAlive心跳机制，为0关闭。
	int keepAlive = 1;
	//start第一次发送心跳包的空闲时间 
	int start = 10;
	//两次心跳侦测包之间的间隔时间
	int interval = 10;
	//发送次数，几次探测失败判定为TCP断开连接
	int count = 10;
	setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void *)&start, sizeof(start));
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));*/
	return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (void *)&enable, sizeof(enable));
}
int set_nonblock(int sockfd, bool en)
{
	int flags = fcntl(sockfd, F_GETFL, 0);//O_RDONLY 0 读， O_WRONLY 1 写， O_RDWR 2 可读可写
	if(en)
		flags |= O_NONBLOCK;//非阻塞
	else
		flags &=~ O_NONBLOCK;
		
	return fcntl(sockfd, F_SETFL, flags);
}
//绑定地址0.0.0.0 8080 到sockaddr
int convert_inaddr(const std::string &addr, uint16_t port, struct sockaddr_in &sockaddr)
{
	bzero((void *)&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	//将一个32的字符串的IP地址，转化为32位的网络字节序ip地址
	if(inet_aton(addr.c_str(), &sockaddr.sin_addr) < 0)
		return -1;
	return 0;
}
}