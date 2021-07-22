#include "file.h"
#include "log.h"
#include <sys/stat.h>
#include <assert.h>


//返回.后面的字符串
std::string  extention_name(const std::string& basename)
{//返回.后面的字符串
	std::string extention("");
	size_t pos = basename.find_last_of('.');//返回最后一个.的位置
	if (pos == std::string::npos)
		return extention;
	return basename.substr(pos + 1);//返回.后面的字符
}

//根据extention返回类型
const char* http_content_type(const std::string& extension)
{
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

SelectFile::SelectFile()
{

}

SelectFile::~SelectFile()
{

}

void SelectFile::start()
{
	WriteEnter;
	int n = m_url.find("file");
	if (n == std::string::npos)
		return;
	std::string filename = m_url.substr(n + 4,m_url.size()-n-4);
	filename = ROOT + filename;
	readfile(filename);
	WriteExit;
}

void SelectFile::readfile(std::string& filepath)
{
	WriteEnter;
	std::string suffix;
	int npos = m_url.rfind('.');
	if (npos == std::string::npos)
	{
		WriteLog(INFO, "not find . !");
	}
	suffix = m_url.substr(npos + 1);

	if (suffix.empty() || access(filepath.c_str(), R_OK) < 0) {

		errsys("access %s error\n", filepath.c_str());

		m_response.get()->set_version(HTTP_VERSION);
		m_response.get()->set_status("404", "Not Found");
		m_response.get()->add_head(HTTP_HEAD_CONNECTION, "close");
		return;
	}

	struct stat filestat;
	stat(filepath.c_str(), &filestat);
	const size_t filesize = filestat.st_size;
	WriteLog(DEBUG, "filesize:%d", filesize);
	char* fbuff = new char[filesize];
	assert(fbuff != NULL);

	FILE* fp = fopen(filepath.c_str(), "rb");
	if (fp == NULL || fread(fbuff, filesize, 1, fp) != 0x01) {
		delete fbuff;

		m_response.get()->set_version(HTTP_VERSION);
		m_response.get()->set_status("500", "Internal Server Error");
		m_response.get()->add_head(HTTP_HEAD_CONNECTION, "close");
		return;
	}

	fclose(fp);

	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", filesize);
	if (m_response == NULL)
	{
		WriteLog(DEBUG, "m_response is null!");
	}
	WriteLog(INFO, "response %p,size: %d,count: %d,url:%s", m_response.get(), m_response.get()->size(), m_response.use_count(), filepath.c_str());

	m_response.get()->set_status("200", "OK");
	m_response.get()->set_version(HTTP_VERSION);
	m_response.get()->add_head(HTTP_HEAD_CONTENT_TYPE, http_content_type(suffix));
	m_response.get()->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	m_response.get()->add_head(HTTP_HEAD_CONNECTION, "close");
	m_response.get()->set_body(fbuff, filesize);
	WriteLog(INFO, "response %p,size: %d,count: %d", m_response.get(), m_response.get()->size(), m_response.use_count());
	delete fbuff;
	WriteExit;
}
