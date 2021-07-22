#include "PassWord.h"
#include "xzypub.h"
#include "log.h"
#include <memory>
void SetReponse200(std::shared_ptr<Http::IHttpResponse> response, std::string& context);


PassWord::PassWord()
{
	m_imysqlpool = CMysqlPool::instance();
}

PassWord::~PassWord()
{
	
}

void PassWord::start()
{
	WriteEnter;
	cout << xzy::nowtime() << endl;
	url::SetQuery(m_url, m_query);
	//string type = path::ptfname(m_query);
	url::SetParse(m_query, m_parse_map);
	std::vector<std::string>vec;
	vec = str::split_1(m_url, '?');
	vec = str::split_1(vec[0], '/');
	std::string context;//���ܲ�ͬ����Ϣ
	run(vec[1], context);
}

bool PassWord::run(std::string& s, string& text)
{
	if (s == "select")
		selectpassword(text);
	else if (s == "set")
		setpassword(text);
	else if(s=="delete")
		deletepassword(text);
	else
		return false;
	SetReponse200(m_response, text);
}

void SetReponse200(std::shared_ptr<Http::IHttpResponse> response, std::string& context)
{
	WriteEnter;
	WriteLog(DEBUG, "enter %s", context.data());
	context = "<html><table border = \"0\">\n<meta charset=\"UTF-8\">\n" + context;
	context += "</table></html>";
	char sfilesize[16] = { 0x00 };
	snprintf(sfilesize, sizeof(sfilesize), "%ld", context.size());
	//printf("%s\n", context.c_str());
	response.get()->set_version(HTTP_VERSION);
	response.get()->set_status("200", "OK");
	response.get()->add_head(HTTP_HEAD_CONTENT_TYPE, HTTP_HEAD_HTML_TYPE);
	response.get()->add_head(HTTP_HEAD_CONTENT_LEN, sfilesize);
	response.get()->add_head(HTTP_HEAD_CONNECTION, "close");
	response.get()->set_body(context.c_str(), context.size());
}

bool PassWord::selectpassword(std::string&text)
{
	m_key = m_parse_map["key"];
	string sql = "SELECT password FROM clockintime.password WHERE `key` LIKE '%" + m_key + "%' ;";
	std::vector<std::vector<std::string>> vec;
	if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
	{
		cout << m_imysqlpool->GetErr() << endl;
	}
	else
	{
		text = vec[1][0];
	}
}
bool PassWord::setpassword(std::string&text)
{
	m_key = m_parse_map["key"];
	m_password = m_parse_map["password"];
	string sql = "insert into clockintime.password ( `key`,`password` ) values ("+ m_key +","+m_password+"); ";
	if (m_imysqlpool->mysqlinsert(sql.c_str()) > 0)
	{
		text = "key: " + m_key + ", password insert success!";
	}
	else
	{
		text = "key: " + m_key + ", password insert failed!";
	}
}
bool PassWord::deletepassword(std::string&text)
{
	m_key = m_parse_map["key"];
	m_password = m_parse_map["password"];
	string sql = "delete form clockintime.password where `key`= '" +m_key +"' and `password` = '" + m_password + "'";


}