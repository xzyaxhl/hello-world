#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "log.h"
#include "http.h"

namespace Http
{
//返回二者之间的长度
size_t addrlen(const char *start, const char *end)
{
	WriteEnter;
	return (size_t)(end - start);
}

//返回.后面的字符串
std::string  extention_name(const std::string &basename)
{//返回.后面的字符串
	WriteEnter;
	std::string extention("");
	size_t pos = basename.find_last_of('.');//返回最后一个.的位置
	if(pos == std::string::npos)
		return extention;
	return basename.substr(pos + 1);//返回.后面的字符
}
//start和end之间endc的位置，输出contlen和sumlen
const char *find_content(const char *start, const char *end, char endc, size_t &contlen, size_t &sumlen)
{
	WriteEnter;
	size_t contentlen = 0;
	const char *contstart = NULL;
	for(const char *mstart = start; mstart < end; mstart++){
		if(contstart == NULL)
		{

			if(*mstart != ' ')
			{
				contstart = mstart;
				contentlen = 1;
			}
		}

		else 
		{
			if(*mstart == endc)
			{
				contlen = contentlen;
				sumlen = addrlen(start, mstart);
				return contstart;
			}
			contentlen++;
		}
	}
	return NULL;
}
//返回下一行的开头
const char * find_line(const char *start, const char *end)
{
	WriteEnter;
	for(const char *lstart = start; lstart < (end - 1); lstart++){
		if(lstart[0] == '\r' && lstart[1] == '\n'){
			return &lstart[2];
		}
	}

	return NULL;
}
//返回body的开头
const char * find_headline(const char *start, const char *end)
{
	WriteEnter;
	for(const char *hstart = start; hstart < (end - 3); hstart++){
		if(hstart[0] == '\r' && hstart[1] == '\n' && \
			hstart[2] == '\r' && hstart[3] == '\n'){
			return &hstart[4];
		}
	}

	return NULL;
}
//根据extention返回类型
const char *http_content_type(const std::string &extension)
{
	WriteEnter;
	if (extension.compare("html") == 0x00)
		return HTTP_HEAD_HTML_TYPE;
	else if (extension.compare("css") == 0x00)
		return HTTP_HEAD_CSS_TYPE;
	else if (extension.compare("gif") == 0x00)
		return HTTP_HEAD_GIF_TYPE;
	else if (extension.compare("jpg") == 0x00)
		return HTTP_HEAD_JPG_TYPE;
	else if (extension.compare("png") == 0x00)
		return HTTP_HEAD_PNG_TYPE;
	else if (extension.compare("txt") == 0x00)
		return HTTP_HEAD_TXT_TYPE;
	else if (extension.compare("mp3") == 0x00)
		return HTTP_HEAD_MP3_TYPE;
	return NULL;
}

CHttpRequest::CHttpRequest(): m_strerr("success"), m_body(NULL), m_bodylen(0)
{
}
CHttpRequest::~CHttpRequest()
{
	WriteEnter;
	if(m_body != NULL)
		delete m_body;
}
//解析request到各个成员中（请求行，请求头部，body）
int CHttpRequest::load_packet(const char *msg, size_t msglen)
{
	WriteEnter;
	const char *remainmsg = msg;
	const char *endmsg = msg + msglen;
	const char *endline = find_line(remainmsg, endmsg);	//parse start line

	if(endline == NULL){
		WriteLog(INFO, "endline error\n");
		set_strerror("startline not found");
		return -1;
	}
	
	if(parse_startline(remainmsg, endline) < 0){
		WriteLog(INFO, "parse_startline error\n");
		set_strerror("invalid startline");
		return -1;
	}

	remainmsg = endline;
	const char *headline_end = find_headline(remainmsg, endmsg);
	if(headline_end == NULL){
		WriteLog(INFO, "headline_end error\n");
		set_strerror("headers not found");
		return -1;
	}
	if(parse_headers(remainmsg, headline_end) < 0){//parse_headers(remainmsg, endmsg)
		WriteLog(INFO, "parse_headers error\n");
		set_strerror("parse headers error");
		return -1;
	}

	remainmsg = headline_end;
	if(parse_body(remainmsg, endmsg) < 0){
		WriteLog(INFO, "parse_body error\n");
		set_strerror("parse body error");
	}
	return 0;
}

const std::string &CHttpRequest::start_line()
{
	WriteEnter;
	return m_startline;
}
const std::string &CHttpRequest::method()
{
	WriteEnter;
	return m_method;
}
const std::string &CHttpRequest::url()
{
	WriteEnter;
	return m_url;
}
const std::string &CHttpRequest::version()
{
	WriteEnter;
	return m_version;
}
const HttpHead_t &CHttpRequest::headers()
{
	return m_headers;
}
//是不是有请求头部是否有name元素
bool CHttpRequest::has_head(const std::string &name)
{
	WriteEnter;
	HttpHead_t::iterator itor = m_headers.find(name);
	if(itor == m_headers.end())
		return false;
	return true;
}
//取请求头部中元素name的值
const std::string &CHttpRequest::head_content(const std::string &name)
{
	WriteEnter;
	const static std::string nullstring("");
	HttpHead_t::iterator itor = m_headers.find(name);
	if(itor == m_headers.end())
		return nullstring;

	return itor->second;
}

const size_t CHttpRequest::body_len()
{
	WriteEnter;
	return m_bodylen;
}

const char *CHttpRequest::body()
{
	WriteEnter;
	return m_body;
}

const char *CHttpRequest::strerror()
{
	WriteEnter;
	return m_strerr.c_str();
}
//设置m_strerr的值

void CHttpRequest::set_strerror(const char *str)
{
	m_strerr = str;
}
//解析request请求行
int CHttpRequest::parse_startline(const char *start, const char *end)
{
	size_t contlen = 0, sumlen = 0;
	const char *cont = NULL, *remainbuff = start;
	cont = find_content(remainbuff, end, '\r', contlen, sumlen);
	if(cont == NULL)
		return -1;
	m_startline = std::string(cont, contlen);
	
	
	cont = find_content(remainbuff, end, ' ', contlen, sumlen);
	if(cont == NULL)
		return -1;
	m_method = std::string(cont, contlen);
	remainbuff += sumlen;

	cont = find_content(remainbuff, end, ' ', contlen, sumlen);
	if(cont == NULL)
		return -1;
	m_url = std::string(cont, contlen);
	remainbuff += sumlen;

	cont = find_content(remainbuff, end, '\r', contlen, sumlen);
	if(cont == NULL)
		return -1;
	m_version = std::string(cont, contlen);	
	return 0;
}
//解析请求头部
int CHttpRequest::parse_headers(const char *start, const char *end)
{	
	WriteEnter;
	size_t contlen = 0, sumlen = 0;
	const char *line_start = start;
	std::string head, attr;
	m_headers.clear();
	for(;;){
		const char *line_end = find_line(line_start, end);
		if(line_end == NULL)
			return -1;
		else if(line_end == end)	// end
			break;

		const char *headstart = find_content(line_start, line_end, ':', contlen, sumlen);
		if(headstart == NULL)
			return -1;
		head = std::string(headstart, contlen);

		const char *attrstart = line_start + sumlen + 0x01;
		attrstart = find_content(attrstart, line_end, '\r', contlen, sumlen);
		if(attrstart == NULL)
			attrstart="";
		attr = std::string(attrstart, contlen);

		line_start = line_end;
		m_headers[head] = attr;
		WriteLog(DEBUG,"%s:  %s\n", head.data(), attr.data());
	}

	return 0;
}
//解析body到buf中
int CHttpRequest::parse_body(const char *start, const char *end)
{
	size_t bodylen = addrlen(start, end);
	if(bodylen == 0x00)
		return 0;

	char *buff = new char[bodylen];
	if(buff == NULL)
		return -1;
	memcpy(buff, start, bodylen);
	
	if(m_body != NULL)
		delete m_body;
	m_body = buff;
	m_bodylen = bodylen;
	return 0;
}



CHttpResponse::CHttpResponse()
{
	WriteEnter;
	m_package.body = NULL;
	m_package.bodylen = 0x00;
	m_package.data = NULL;
	m_package.datalen = 0x00;
	m_package.dirty = true;
	WriteExit;
}
CHttpResponse::~CHttpResponse()
{
	WriteEnter;
	if(m_package.body != NULL)
		delete [] m_package.body;
	if(m_package.data != NULL)
		delete [] m_package.data;
	WriteExit;
}
//set HTTP版本
int CHttpResponse::set_version(const std::string &version)
{
	WriteEnter;
	m_package.version = version;
	m_package.dirty = true;
	WriteExit;
	return 0;
}
//set 状态码 reason  
int CHttpResponse::set_status(const std::string &status, const std::string &reason)
{
	WriteEnter;
	m_package.status = status;
	m_package.reason = reason;
	m_package.dirty = true;
	WriteExit;
	return 0;
}
//增加head中的name元素
int CHttpResponse::add_head(const std::string &name, const std::string &attr)
{
	WriteEnter;
	if (name.empty() || attr.empty()) {
		WriteExit;
		return -1;
	}

	m_package.headers[name] = attr;
	m_package.dirty = true;
	WriteExit;
	return 0;
}
//删除head中的name元素
int CHttpResponse::del_head(const std::string &name)
{
	WriteEnter;
	HttpHead_t::iterator itor = m_package.headers.find(name);
	if (itor == m_package.headers.end()) {
		WriteExit;
		return -1;
	}
	m_package.headers.erase(itor);
	m_package.dirty = true;
	WriteExit;
	return 0;
}
//set 消息
int CHttpResponse::set_body(const char *body, size_t bodylen)
{
	WriteEnter;
	if (body == NULL || bodylen == 0x00 || bodylen > BODY_MAXSIZE) {
		WriteExit;
		return -1;
	}

	char *buff = new char[bodylen];
	assert(buff != NULL);
	
	memcpy(buff, body, bodylen);

	if (m_package.body != NULL)
	{
		WriteEnter;
		delete[] m_package.body;
	}
	m_package.body = buff;
	m_package.bodylen = bodylen;
	m_package.dirty = true;
	serialize();
	WriteExit;
	return 0;
}
//response消息总长度
size_t CHttpResponse::size()
{
	WriteLog(INFO, "%p,%d", &totalsizeflag, totalsizeflag);

	if (totalsizeflag)
	{
		WriteLog(INFO, "totalsize: %d", totalsize);
		return totalsize;
	}
	WriteLog(INFO,"%p,%d", &totalsizeflag, totalsizeflag);
	WriteExit;
	return 0;
}

const char* CHttpResponse::getdata()
{
	WriteEnter;
	if (!m_package.dirty) {
		WriteExit;
		return m_package.data;
	}
	return NULL;
}


//返回response的数据地址
const char *CHttpResponse::serialize()
{
	WriteEnter;
	if (!m_package.dirty) {
		WriteExit;
		return m_package.data;
	}
	m_package.totalsize = startline_stringsize() + headers_stringsize();
	m_package.totalsize += m_package.bodylen;
	totalsize = m_package.totalsize;
	totalsizeflag = true;
	char *buffreserver = new char[totalsize];
	assert(buffreserver != NULL);

	char *buff = buffreserver;
	int nprint = snprintf(buff, totalsize, "%s %s %s\r\n", m_package.version.c_str(), \
		m_package.status.c_str(), m_package.reason.c_str());
	if(nprint < 0)
		goto ErrorHandle;

	totalsize -= nprint;
	buff += nprint;

	for(HttpHead_t::iterator itor = m_package.headers.begin(); itor != m_package.headers.end(); itor++){
		const std::string &name = itor->first;
		const std::string &attr = itor->second;

		nprint = snprintf(buff, totalsize, "%s: %s\r\n", name.c_str(), attr.c_str());
		if(nprint < 0)
			goto ErrorHandle;
		totalsize -= nprint;
		buff += nprint;
	}	

	nprint = snprintf(buff, totalsize, "\r\n");
	if(nprint < 0)
		goto ErrorHandle;
	totalsize -= nprint;
	buff += nprint;
	
	memcpy(buff, m_package.body, totalsize);
	if(totalsize != m_package.bodylen){
		WriteLog(INFO, "body copy error, target %ld, actually %ld\n", m_package.bodylen, \
				totalsize);
	}
	if(m_package.data != NULL)
		delete m_package.data;
	m_package.data = buffreserver;

	m_package.dirty = false;

	WriteExit;
	return m_package.data;
ErrorHandle:
	delete buffreserver;
	WriteExit;
	return NULL;
}
//状态行长度
size_t CHttpResponse::startline_stringsize()
{
	WriteEnter;
	const size_t otherchar_size = 1 * 2 + 2;  // black * 2 + CRLF
	size_t total_size = otherchar_size + m_package.version.size();
	total_size += m_package.status.size()+ m_package.reason.size();
	WriteExit;
	return total_size;
}
//消息报头长度
size_t CHttpResponse::headers_stringsize()
{
	WriteEnter;
	const size_t otherchar_size = 2 + 2;	// ': ' + CRLF
	const size_t head_terminatorsize = 2;	// CRLF

	size_t stringsize = 0;
	HttpHead_t::iterator itor = m_package.headers.begin();
	for(; itor != m_package.headers.end(); itor++){
		const std::string &name = itor->first;
		const std::string &attr = itor->second;

		stringsize += name.size() + attr.size() + otherchar_size;
	}

	stringsize += head_terminatorsize;
	WriteExit;
	return stringsize;
}

}
