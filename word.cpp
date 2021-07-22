#include "word.h"
#include "xzypub.h"
#include "log.h"
#include "sys/stat.h"
#include <assert.h>
#include "http.h"

void replace(std::string& str, std::string str1, std::string str2)
{
	int npos = str.find(str1);
	while (npos>0)
	{
		str.replace(npos, str1.size(), str2);
		npos = str.find(str1, npos + 1);
	}
}
string readfile(string filename)
{
	if (-1 == access(filename.c_str(), F_OK))
	{
		printf("config file [%s] not find!\nPress any key to exit!", filename.c_str());
		getchar();
		exit(0);
	}

	struct stat filestat;
	stat(filename.c_str(), &filestat);
	const size_t filesize = filestat.st_size;

	char* fbuff = new char[filesize + 1]();

	assert(fbuff != NULL);
	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL || fread(fbuff, filesize, 1, fp) != 0x01)
	{
		delete[]fbuff;
		fclose(fp);
	}
	std::string context(fbuff);
	delete[]fbuff;
	fclose(fp);
	return context;
}


Word::Word()
{
	m_imysqlpool = CMysqlPool::instance();
}

Word::~Word()
{

}

void Word::start()
{
	WriteEnter;
	
	cout << xzy::nowtime() << endl;
	//url=word/select/?list=1&id=1.
	//word/setflag/?listid=1&wordid=1&flag=1 or flag=0//if id=0 设置全部list
	url::SetQuery(m_url, m_query);
	string type = path::ptfname(m_query);
	url::SetParse(m_query, m_parse_map);

	std::vector<std::string>vec;
	vec = str::split_1(m_url, '?');
	vec = str::split_1(vec[0], '/');
	std::string context;
	run(vec[1], context);

}

bool Word::run(std::string& s, string& text)
{
	m_listid = m_parse_map["listid"];
	m_wordid = m_parse_map["wordid"];
	if (s == "select" || s == "select/")
		Select(text);
	else if (s == "setflag" || s == "setflag/")
		SetFlag(text);
	else if (s == "selectall" || s == "selectall/")
		SelectAll(text);
	else if (s == "flag" || s == "flag/")
		Flag(text);
	else
		return false;
	SetReponse200(m_response, text);

}



void Word::SetReponse200(std::shared_ptr<Http::IHttpResponse> response, std::string& context)
{
	WriteEnter;
	//WriteLog(DEBUG, "enter %s", context.data());
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

/*bool Word::run(std::string& s, string& text)
{
	if (s == "select")
		return Select(text);
	else if (s == "set")
		return SetFlag(text);
	else
		return false;
	//SetReponse200(m_response, text);
}*/

bool Word::Select(std::string& text)
{
	WriteEnter;


	if (m_listid.size() < 0 || m_wordid.size() <= 0)
	{
		text = "listid: [" + m_listid + "] and wordid: [" + m_wordid + "]!";
		WriteLog(INFO, text.c_str());
		return false;
	}
	//std::string sql = "select pngurl,mp3url from word where listid = " + m_listid + \
		" and wordid = "+ m_wordid +"  and flag = 0;";
	std::string sql;
	if (m_flag != "")
	{
		sql = "select pngurl,mp3url from word where listid = " + m_listid + \
			" order by flag desc, wordid asc ; ";
	}
	else
	{
		sql = "select pngurl,mp3url from word where listid = " + m_listid + \
			" order by wordid asc; ";
	}

	WriteLog(DEBUG, sql.c_str());
	m_sqlvec.clear();
	//m_imysqlpool->mysqlquery(sql.c_str(), m_sqlvec);
	if (m_imysqlpool->mysqlquery(sql.c_str(), m_sqlvec) == -1)
	{
		
		text = "mysql failed! %s" + sql;
		WriteLog(INFO, text.c_str());
		return false;
	}
	//构造html
	return SelectHtml(text,m_sqlvec);	
}

bool Word::SelectAll(std::string& text)
{
	WriteEnter;
	text = readfile("/home/xuzhiyuan/all.html");
	std::string data1= "http://119.45.184.103:5888/word/select/?listid=" + m_listid + "&wordid=" + m_wordid;
	replace(text, "$$data1", data1);
	cout << text << endl;
	return true;
}

bool Word::Flag(std::string& text)
{
	WriteEnter;
	if (m_flag != "")
		m_flag = "";
	else
		m_flag = "1";
	text = m_flag+": ok";
	return true;
}

bool Word::SelectHtml(std::string& text, std::vector<std::vector<std::string> >& vecv)
{
	WriteEnter;
	std::vector<std::string> vec;
	std::string url = "http://119.45.184.103:5888/file/";

	if (vecv.size() <= 1 || vecv[1].size() != 2)
	{
		text = "SelectHtml false! select data is NULL or vecv[1].size: not 2";
		return false;
	}
	int n = stoi(m_wordid);
	if (n >= vecv.size())
	{
		text = "wordid out";
		return false;
	}
	std::string page1 = to_string(n - 1);
	std::string page2 = to_string(n + 1);

	std::string url1 = "http://119.45.184.103:5888/word/select/?listid=" + m_listid + "&wordid=" + page1;
	std::string url2 = "http://119.45.184.103:5888/word/select/?listid=" + m_listid + "&wordid=" + page2;
	std::string data1 = url + vecv[n][0];
	std::string data2 = url + vecv[n][1];
	std::string seturl = "http://119.45.184.103:5888/word/setflag/?listid=" + m_listid + "&wordid=" + m_wordid +"&flag=1";
	std::string seturl1 = "http://119.45.184.103:5888/word/setflag/?listid=" + m_listid + "&wordid=" + m_wordid + "&flag=0";
	text = readfile("/home/xuzhiyuan/word.html");

	replace(text, "$$data1", data1);
	replace(text, "$$data4", data2);
	replace(text, "$$data2", url1);
	replace(text, "$$data3", url2);
	replace(text, "$$data5", seturl);
	replace(text, "$$data6", seturl1);
	WriteLog(DEBUG, text.c_str());
	return true;

}


/*
bool Word::SelectHtml(std::string& text, std::vector<std::vector<std::string> >&vecv)
{
	WriteEnter;
	std::vector<std::string> vec;
	std::string url = "http://119.45.184.103:5888/file/";
	if (vecv.size() != 2 || vecv[1].size() != 2)
	{
		text = "SelectHtml false! vecv.size: or vecv[1].size: not 2";
		return false;
	}
	int n = stoi(m_wordid);
	std::string page1 = to_string(n - 1);
	std::string page2 = to_string(n + 1);
	std::string url1 = "http://119.45.184.103:5888/word/select/?listid=" + m_listid + "&wordid=" + page1;
	std::string url2 = "http://119.45.184.103:5888/word/select/?listid=" + m_listid + "&wordid=" + page2;

	text="<!DOCTYPE html>\
		<html>\
		<head>\
		<tite>Audio Player</title>"

		//<style>\
		  //  .btn {position: absolute;text-align: center;bottom: 0;font-size: 16px;color: #FFFFFF;margin: 25px 0;width: 300px;height: 45px;line-height: 45px;background: #E0E0E0;border-radius: 0px;background-color: #1B68FD;left: 50%;transform: translateX(-30%);}\
		//</style>"

		"</head>\
		<meta charset=\"utf-8\">\
		<body>\
		<img src=\""+url+vecv[1][0]+"style =\"max-width:100%\" \"></br>"
		"<div >"
		"<button class =\"btn\" style=\"width:300px; height:150px\" onclick = \"click01()\">播放</button>"
		"</div>"
		"<div align= \"left\">"
		"<a style=\"font-size: 100px;\" href=\"" + url1 + "\"> 上一页</a>"
		//"</div>"
		//"<div align= \"right\">"
		"<a align= \"right\" style=\"font-size: 100px;\" href=\"" + url2 + "\"> 下一页</a>"
		"</div>"
		//"<button onclick=\"click02()\">暂停</button>"
		"<audio id=\"audio01\" src=\""+ url+vecv[1][1] +"\"></audio>"
		"<script>"
			"var c=document.getElementById(\"audio01\");"
			"function click01(){"
				"if(c.paused){"
					"c.play();"
				"}"
			"}"
			"function click02(){"
				"if(c.paused){"
					"c.pause();"
				"}"
			"}"
		"</script>"
		"</body>"
		"</html>";
	WriteLog(DEBUG, text.c_str());
	return true;
}
*/
bool Word::SetFlag(std::string& text)
{
	WriteEnter;
	if (m_listid.size() < 0 || m_wordid.size() <= 0)
	{
		text = "listid: [" + m_listid + "] and wordid: [" + m_wordid + "]!";
		WriteLog(INFO, text.c_str());
		return false;
	}
	std::string flag = m_parse_map["flag"];
	std::string sql;
	if (flag == "1")
	{
		sql = "INSERT INTO word(listid,wordid) VALUE ("+m_listid+" , "+m_wordid+" ) ON DUPLICATE KEY UPDATE flag = flag + 1; ";
	}
	else
	{
		sql = "INSERT INTO word(listid, wordid) VALUE(" + m_listid + " , " + m_wordid + ") ON DUPLICATE KEY UPDATE flag = flag - (IF(flag = 0, 0, 1)); ";
	}
	WriteLog(DEBUG, sql.c_str());

	if (m_imysqlpool->mysqlinsert(sql.c_str()))
	{
		WriteLog(DEBUG, "update success! sql: ", sql.c_str());
		return true;
	}
	m_wordid=to_string((atoi(m_wordid.c_str()))+1);

	Select(text);
	return false;
}

/*
table = word
id
listid
wordid
pngurl
mp3url
number	select的次数
flag	是不是已经会了 0 不会 1 记住了 记住的少出现甚至不出现
lasttime datetime 出现的最近一次时间 长时间不出现需要显示一次 暂定一个月
baidump3url 文字识别 暂时不处理
*/

