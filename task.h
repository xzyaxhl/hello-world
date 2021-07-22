#pragma once
/*

Http::CHttpRespose* response_tj(Http::CHttpRespose* response, std::string& text, GlobalConfig* globalconfig)
{
	text = "<html><table border = \"0\">\n<meta charset=\"UTF-8\">\n";
	auto iter = globalconfig->de_vpath.begin();
	for (iter; iter != globalconfig->de_vpath.end(); iter++)
	{
		text += "<tr><td><a href=" + iter->value + ">" + iter->key + "</a></td>";
		text += "<td><a href=" + iter->value + "?table=1>" + iter->key + "-table</a></td></tr>";
	}
	text += "</table></html>";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	printf("%s\n", text.c_str());
	response->set_version(HTTP_VERSION);
	response->set_status("200", "OK");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, "close");
	response->set_body(text.c_str(), text.size());
	return response;
}

Http::CHttpRespose* response_404(Http::CHttpRespose* response, std::string& text)
{
	text = "<html><head> <title>404 Not Found</title></head><body bgcolor = \"white\"><center> <h1>404 Not Found</h1></center></body></html>";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	response->set_version(HTTP_VERSION);
	response->set_status("404", "Not Found");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, HTTP_HEAD_KEEP_ALIVE);
	response->set_body(text.c_str(), text.size());
	return response;

}

Http::CHttpRespose* response_500(Http::CHttpRespose* response, std::string& text)
{
	WriteLog(DEBUG, "mysql init failed!");
	text = "<html><head> <title>500 Internal Server Error</title></head><body bgcolor = \"white\"><center> <h1>500 Internal Server Error</h1></center><center><p>mysql connection failed!</p></center></body></html>";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	response->set_version(HTTP_VERSION);
	response->set_status("500", "Internal Server Error");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, HTTP_HEAD_KEEP_ALIVE);
	response->set_body(text.c_str(), text.size());
	return response;
}

Http::CHttpRespose* response_table(Http::CHttpRespose* response, std::string& text)
{
	char sfilesize[16] = { 0x00 };
	text = "<html><table border = \"1\">\n<meta charset=\"UTF-8\">\n" + text;
	text += "</table></html>";
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	response->set_version(HTTP_VERSION);
	response->set_status("200", "OK");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, "close");
	response->set_body(text.c_str(), text.size());
	return response;
}

Http::CHttpRespose* response_json(Http::CHttpRespose* response, Json::Value& result)
{
	std::string text = result.toStyledString();
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	response->set_version(HTTP_VERSION);
	response->set_status("200", "OK");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_TXT_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, "close");
	response->set_body(text.c_str(), text.size());
	return response;
}

void parseparamtomap(map<string, string>& m_headers, string params)
{
	int nps2 = 0;
	int nps1 = 0;
	std::string param;
	std::string param1;
	std::string head;
	std::string attr;
	param = params;
	do {
		param1 = param;
		nps1 = param1.find("&");
		nps2 = param1.find("=");

		head = std::string(param1, 0, nps2);
		attr = std::string(param1, nps2 + 1, nps1 - nps2 - 1);

		param = std::string(param1, nps1 + 1, param1.size() - nps1 - 1);
		m_headers[head] = attr;
		WriteLog(DEBUG, "---\nhead: %s.\nattr: %s\nparam: %s\n---", head.c_str(), attr.c_str(), param.c_str());
	} while (nps1 > 0);
}
Http::CHttpRespose* insqltoios(CHttpRespose* response, map<string, string>& m_headers, GlobalConfig::Mysql& Mysql, string& text)
{
	map<string, string>::iterator itor;
	//idfa
	std::string imei = "";
	itor = m_headers.find("idfa");//find ios 
	if (itor == m_headers.end())
		return response;
	else
		imei = itor->second;

	std::string cid = "";
	itor = m_headers.find("cid");
	if (itor == m_headers.end())
		cid = "";
	else
		cid = itor->second;

	std::string callback_url = "";
	itor = m_headers.find("callback_url");
	if (itor == m_headers.end())
		return response;
	else
		callback_url = UrlDecode(itor->second);

	WriteLog(DEBUG, "callback:%s---%s\n", callback_url.c_str(), callback_url.c_str());

	std::string md5mac = "";
	itor = m_headers.find("mac");
	if (itor == m_headers.end())
		return response;
	else
		md5mac = itor->second;

	if (strncmp(md5mac.c_str(), "__", 2) == 0 || strncmp(imei.c_str(), "__", 2) == 0)
		return response;
	WriteLog(DEBUG, "mac:%s---%s\n", md5mac.c_str(), md5mac.c_str());

	Mysql::IMysql* m_imysql = NULL;
	m_imysql = new Mysql::CMysql(Mysql.host, Mysql.user, Mysql.pass, Mysql.port, Mysql.db);
	std::string sql = "insert into test.toutiao_imei(cid,md5_imei,md5_mac,url,os) values(\"" + cid + "\",\"" + imei + "\",\"" + md5mac + "\",\"" + callback_url + "\",\"ios\")";
	WriteLog(DEBUG, "---\n%s\n---", sql.c_str());
	if (!m_imysql->mysqlinit())
		return response_500(response, text);
	if (-1 == m_imysql->mysqlinsert(sql.c_str()))
		return NULL;
	delete m_imysql;

	return response;
}

Http::CHttpRespose* response_ad(CHttpRespose* response, const std::string& url, GlobalConfig::Mysql& Mysql)
{
	FILE* fp = NULL;

	fp = fopen("toutiao1.txt", "aw+");
	fprintf(fp, url.c_str(), '\n');
	fprintf(fp, "\n");
	fclose(fp);

	std::string text = "ok";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	response->set_version(HTTP_VERSION);
	response->set_status("200", "OK");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_TXT_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, "close");
	response->set_body(text.c_str(), text.size());


	//CMysqlPool::instance()->Init(Mysql.host,Mysql.user,Mysql.pass,Mysql.db,Mysql.port,1);
	WriteLog(DEBUG, "-------\n\n%s\n\n--------", url.c_str());
	int nps = url.find('?');

	std::string params = url.substr(nps + 1);
	//adid = __AID__ & cid = __CID__ & idfa = sfd
	WriteLog(DEBUG, "params: %s\n\n", params.c_str());

	std::map<string, string> m_headers;
	/*int nps2 = 0;
	int nps1 = 0;
	std::string param;
	std::string param1;
	std::string head;
	std::string attr;
	param = params;
	do {
		param1 = param;
		nps1 = param1.find("&");
		nps2 = param1.find("=");

		head = std::string(param1, 0, nps2);
		attr = std::string(param1, nps2 + 1, nps1 - nps2 - 1);

		param = std::string(param1, nps1 + 1, param1.size() - nps1 - 1);
		m_headers[head] = attr;
		WriteLog(DEBUG, "---\nhead: %s.\nattr: %s\nparam: %s\n---", head.c_str(), attr.c_str(), param.c_str());
	} while (nps1 > 0);
	* /
	parseparamtomap(m_headers, params);
	std::map<std::string, std::string>::iterator itor = m_headers.find("imei");
	if (itor == m_headers.end())//ios
	{
		return insqltoios(response, m_headers, Mysql, text);
	}

	std::string imei = "";
	if (itor == m_headers.end())
		return response;
	else
		imei = itor->second;

	std::string cid = "";
	itor = m_headers.find("cid");
	if (itor == m_headers.end())
		cid = "";
	else
		cid = itor->second;

	std::string callback_url = "";
	itor = m_headers.find("callback_url");
	if (itor == m_headers.end())
		return response;
	else
		callback_url = UrlDecode(itor->second);
	WriteLog(DEBUG, "callback:%s---%s\n", callback_url.c_str(), callback_url.c_str());

	std::string md5mac = "";
	itor = m_headers.find("mac");
	if (itor == m_headers.end())
		return response;
	else
		md5mac = itor->second;


	if (strncmp(md5mac.c_str(), "__", 2) == 0 || strncmp(imei.c_str(), "__", 2) == 0)
		return response;
	WriteLog(DEBUG, "mac:%s---%s\n", md5mac.c_str(), md5mac.c_str());

	Mysql::IMysql* m_imysql = NULL;
	m_imysql = new Mysql::CMysql(Mysql.host, Mysql.user, Mysql.pass, Mysql.port, Mysql.db);
	std::string sql = "insert into test.toutiao_imei(cid,md5_imei,md5_mac,url) values(\"" + cid + "\",\"" + imei + "\",\"" + md5mac + "\",\"" + callback_url + "\")";
	WriteLog(DEBUG, "---\n%s\n---", sql.c_str());
	if (!m_imysql->mysqlinit())
		return response_500(response, text);
	if (-1 == m_imysql->mysqlinsert(sql.c_str()))
		return NULL;
	delete m_imysql;

	return response;
}

//����request,���ؽ��
Http::IHttpRespose *HttpStream::handle_request(Http::IHttpRequest &request)
{
	const std::string &method = request.method();
	const std::string &url = request.url();

	std::string dname, bname;
	split_url(url, dname, bname);
	//printf("url:%s--dname-%s-----bname-%s", url, dname, bname);
	Http::CHttpRespose *response = new Http::CHttpRespose;
	std::string filepath = http_path_handle(dname, bname);
	//GlobalConfig *globalconfig=GlobalConfig::instance();
	//GlobalConfig::Mysql mysql = NULL; globalconfig->mysql;
	if (method == "GET")
	{
		std::string text;
		if (url == "/tj" || url == "/tj/")
			//return response_tj(response, text, globalconfig);
		//else if (strncmp(url.c_str(),"/ad/",4)==0)/*����ͷ������* /
		//return response_ad(response, url, mysql);
		{

		}
		else if (strncmp(url.c_str(), "/clockintime", 12) == 0)
		{
		return clocktime(response, text, url);
		}
		/*else
		{
			int flag = 0;
			int nps = 0;
			std::string url1 = url;
			if ((nps = url.rfind("?table=1")) > 0)
			{
				url1 = url.substr(0, nps);
				WriteLog(DEBUG, "%s\n\n", url1.c_str());
				flag = 1;
			}
			m_imysql = new Mysql::CMysql(mysql.host,mysql.user,mysql.pass,mysql.port,mysql.db);
			if (!m_imysql->mysqlinit())
				return response_500(response, text);

			Json::Value result;
			MYSQL_RES* results;
			auto iter = globalconfig->vp_sql.find(url1);
			if (iter != globalconfig->vp_sql.end())
			{
				if ((nps = url.rfind("?table=1")) > 0)
				{
					if (-1 == m_imysql->mysqlquery((iter->second).c_str(), text))
						return NULL;
					return response_table(response, text);
				}

				else
				{
					if (-1 == m_imysql->mysqlquery((iter->second).c_str(), result))
						return NULL;
					return response_json(response, result);
				}
			}
			return response_404(response, text);

		}* /
	}
	else if (method == "post" || method == "POST")
	{
	std::string text = "{\"dfa\": \"sdf\"}";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", text.size());
	WriteLog(DEBUG, "%s\n", text.c_str());
	response->set_version(HTTP_VERSION);
	response->set_status("200", "OK");
	response->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response->add_head(HTTP_HEAD_CONNECTION, "close");
	printf("%s\n", text.c_str());
	response->set_body(text.c_str(), text.size());
	}
	return response;
}


*/
#include <string>
#include <iostream>
#include <unistd.h>
#include <memory>
#include "http.h"

class Task
{
public:
	virtual ~Task() {};
	virtual void start() =0;
private:

public:
	//Http::IHttpResponse* m_response;
	std::shared_ptr<Http::IHttpResponse> m_response;
	std::string m_url;
};