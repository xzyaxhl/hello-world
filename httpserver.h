
#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "server.h"
#include "event.h"
#include "http.h"
#include "config.h"
#include "mysql.h"

#include <memory>

namespace Http
{
#define HTTP_ROOT "/root/Html"
#define HTTP_DEFAULTFILE 	"index.html"
#define HTTP_SLASH 			"/"
#define HTTP_CURRENT_DIR	"."
#define HTTP_ABOVE_DIR 		".."
	//根据url合成路径寻找文件
	std::string http_path_handle(const std::string& dname, const std::string& bname);

	inline void split_url(const std::string& url, std::string& dir, std::string& base);
/*	CHttpRespose* response_tj(CHttpRespose* response, std::string& text, GlobalConfig* globalconfig);
	CHttpRespose* response_404(CHttpRespose* response, std::string& text);
	CHttpRespose* response_500(CHttpRespose* response, std::string& text);
	CHttpRespose* response_table(CHttpRespose* response, std::string& text);
	CHttpRespose* response_json(CHttpRespose* response, Json::Value& result);
	CHttpRespose* response_ad(CHttpRespose* response, const std::string& text, GlobalConfig::Mysql&);

	void* get_url(void* arg);
*/
	class HttpStream : public Event::IEventHandle //回调函数handle_*()和注册事件函数
	{
	public:
		HttpStream(Socket::IClient*);
		~HttpStream();
		int close();
	protected:
		void handle_in(int fd);
		void handle_out(int fd);
		void handle_close(int fd);
		void handle_error(int fd);

	private:
		//根据request返回response
		void handle_request(Http::IHttpRequest& request, std::shared_ptr<Http::IHttpResponse>);

	private:
		enum CONFIG {
			READBUFF_LEN = 2048,
		};
	private:
		Socket::IClient* m_client;

		Pthread::CMutex m_readbuffmutex;
		char* m_readbuff;

		Mysql::IMysql* m_imysql;
	};

	class HttpServer : public Event::IEventHandle
	{
	public:
		HttpServer(const std::string& addr, uint16_t port);
		~HttpServer();
		int start(size_t backlog);
		int close();

	protected:
		void handle_in(int fd);
		void handle_out(int fd);
		void handle_close(int fd);
		void handle_error(int fd);

	private:
		Pthread::CMutex m_sockmutex;
		uint16_t m_port;
		const std::string m_addr;
		Socket::CStreamServer m_server;
	};
}

#endif
