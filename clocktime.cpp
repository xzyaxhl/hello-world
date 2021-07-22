#include "clocktime.h"
#include "log.h"
#include "xzypub.h"
#include <map>

#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "mysqlpool.h"
#include "XTime.h"

using namespace std;

ClockTime::ClockTime()
{
	m_imysqlpool = CMysqlPool::instance();
	int i;
}

ClockTime::~ClockTime()
{

}

void ClockTime::start() 
{
	WriteEnter;
	cout << xzy::nowtime() << endl;
	url::SetQuery(m_url, m_query);
	string type = path::ptfname(m_query);
	url::SetParse(m_query, m_parse_map);
	std::vector<std::string>vec;
	vec=str::split_1(m_url, '?');
	vec=str::split_1(vec[0], '/');
	std::string context;//���ܲ�ͬ����Ϣ
	run(vec[1],context);
	WriteExit;
	/*
	//xie
	string filename = getfilename(m_url);
	string time = xzy::now();
	time += "\r\n";
	writetofile(filename, time);
	//du
	string context = readtofile(filename);
	//Http::CHttpResponse * response =new Http::CHttpResponse();
	formatcontext(response, context);
	//return response;
	*/
};

void ClockTime::writetofile(string filename, string context)
{
	WriteLog(DEBUG, "enter %s", __FUNCTION__);

	//2020-11-20 19:33:00
	int fd = 0;
	mode_t f_attrib;
	f_attrib = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	if ((fd = open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, f_attrib)) != -1)
	{
		WriteLog(DEBUG, "%s    suceess,", __FUNCTION__);
	}
	else
	{
		WriteLog(DEBUG, "%s    failed, %d", __FUNCTION__, errno);
	}
	write(fd, context.c_str(), context.size());

	close(fd);
	WriteExit;
}

std::string ClockTime::readtofile(string filename)
{
	WriteLog(DEBUG, "enter %s", __FUNCTION__);

	if (-1 == access(filename.c_str(), F_OK))
	{
		WriteLog(DEBUG,"config file [%s] not find!\nPress any key to exit!", filename.c_str());
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

	}
	string context(fbuff);
	//cout << context << endl;
	delete[]fbuff;
	fclose(fp);
	WriteExit;
	return context;
}

void ClockTime::formatcontext(shared_ptr<Http::IHttpResponse> response, std::string& context)
{
	WriteLog(DEBUG, "enter %s", __FUNCTION__);
	WriteLog(DEBUG, "enter %s", context.data());
	string str1 = "\r\n";
	string str2 = "</a></td></tr>\n<tr><td><a></a>";
	xzy::replace(context, str1, str2);

	context = "<html><table border = \"1\">\n<meta charset=\"UTF-8\">\n""<tr><td><a>" + context + "</td></tr>\n";
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
	printf("%s\n", context.c_str());
	WriteExit;
	
}

string ClockTime::getfilename(const string& url)
{
	WriteLog(DEBUG, "enter %s", __FUNCTION__);

	string query;
	url::SetQuery(url, query);
	map<string, string> name_pass;
	url::SetParse(query, name_pass);
	string user = name_pass["user"];
	string s = xzy::curdate().substr(0, 7);
	s[4] = '_';
	string filename = user + s + ".json";
	WriteExit;
	return filename;
}

bool ClockTime::run(std::string& s,string &text)
{
	if (s == "work")
		Work(text);
	else if (s == "adduser")
		AddUser(text);
	else if (s == "deleteuser")
		DeleteUser(text);
	else if (s == "deleterecord")
		DeleteRecode(text);
	else
		return false;
	SetReponse200(m_response, text);
	WriteExit;
}

bool ClockTime::AddUser(std::string& str)
{
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string mail = m_parse_map["mail"];
	string ruser = m_parse_map["ruser"];		//root 
	string password = m_parse_map["password"];//clockintimexwx987785

	//查询root账户密码是不是正确
	std::vector<std::vector<std::string> >vec;
	if (m_ruser.size() != 0 && m_password.size() != 0)
	{
		if (ruser != m_ruser || password != m_password)
		{
			std::string sql = "SELECT pass FROM clockintime.user WHERE `name`='root'";
			if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
			{
				cout << m_imysqlpool->GetErr() << endl;
			}
			else
			{
				if (vec.size() >= 2)
				{
					std::string sqlpass = vec[1][0];
					if (sqlpass != password)
					{
						str = "password failed!";
						return false;
					}
					m_password = sqlpass;
				}
				else
				{
					str = "select not data!";
					return false;
				}
			}
		}
		
	}
	//
	std::string sql = "INSERT INTO clockintime.user (name,pass,mail)VALUE('"
		+ user + "','" + pass + "','" + mail + "');";
	WriteLog(DEBUG, sql.c_str());
	//m_imysqlpool->mysqlquery(root, sql.c_str());
	if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
	{
		m_UserPassFlag = false;//更新用户重新载入数据
		str = "create user is success!";
	}
	else
	{
		str = "create user is failed!";
	}
	return true;
}
/*
bool ClockTime::Work(std::string& str)
{
	//type= in out;
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string type = m_parse_map["type"];

	std::vector<std::vector<std::string>> vec;

	if (!isUser(user, pass))
	{
		str = "user " + user + " pass " + pass + " is not in databases!";
		return false;
	}

	string userid = getUserId(user);
	if (userid == "")
	{
		str = "userid not exist!";
		return false;
	}

	if (type == "in")
	{
		m_curtime = xzy::curdate();
		m_starttime = xzy::nowtime();
		std::string sql= "INSERT INTO clockintime.clock(user_id, rq, inclock)VALUE("+userid+",'"+xzy::curdate()+"','"+xzy::nowtime()+\
			"')ON DUPLICATE KEY UPDATE inclock = VALUES(inclock);";
		WriteLog(DEBUG, sql.c_str());
		if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
		{
			str = "insert intime success!";
		}
		else
		{
			str = "insert intime failed!";
			return false;
		}
		WriteLog(DEBUG, "flag");
	}
	else if (type == "off")
	{
		Time time1;
		if (m_curtime == xzy::curdate())
		{
			m_endtime = xzy::nowtime();
			Time timein = m_starttime;
			Time timeoff = m_endtime;
			time1 = calculatevalidtime(timein, timeoff);
			WriteLog(DEBUG, "1",timein.toString().c_str(),"2", timeoff.toString().c_str(),"3", time1.toString().c_str());
		}
		//INSERT INTO clockintime.clock(user_id, rq, inclock)VALUE(1, "2020-11-26", "06:12:19")ON DUPLICATE KEY UPDATE inclock = VALUES(inclock);
		//INSERT INTO clockintime.clock(user_id, rq, offclock)VALUE(1, "2020-11-26", "06:12:29")ON DUPLICATE KEY UPDATE offclock = VALUES(offclock);
		std::string sql = "INSERT INTO clockintime.clock(user_id, rq, offclock)VALUE(" + userid + ",'" + xzy::curdate() + "','" + xzy::nowtime() + \
			"')ON DUPLICATE KEY UPDATE offclock = VALUES(offclock);";
		WriteLog(DEBUG, sql.c_str());
		if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
		{
			//sql = "UPDATE clock SET priodtime = DATE_FORMAT(DATE_SUB(FROM_UNIXTIME(TIMESTAMPDIFF(SECOND, inclock, offclock)), INTERVAL 8 HOUR), '%%H:%%i:%%s') WHERE rq ='"+xzy::curdate()+"';";
			sql = "UPDATE clock SET priodtime ='" + time1.toString()+"' WHERE rq = '" + xzy::curdate() + "'; ";

			WriteLog(DEBUG, sql.c_str());
			if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
			{
				str = "insert offtime success! update priodtime success!";
			}
		}
		else
		{
			str = "insert offtime failed!";
			return false;
		}
	}
	else
	{
		//计算时间输出结果,
		string sql = "SELECT rq,inclock,offclock,priodtime FROM clock WHERE user_id= " + userid + " AND DATE_FORMAT(rq,'%Y-%m')=DATE_FORMAT(CURDATE(),'%Y-%m');";
		if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
		{
			cout << m_imysqlpool->GetErr() << endl;
		}
		else
		{
			calculatesub(vec);
			for (auto ve : vec)
			{
				cout << ve[0] << "---" << ve[1] << "---" << ve[2] << "---" << ve[3] << endl;
			}
			str += "<html>\r\n";
			str += "<head>\r\n";
			str += "</head>\r\n";
			str += "<body><p> 打卡记录 </p>\r\n";
			str += "<table border=\"0\"><tbody><table border=\"1\" cellspacing=\"0\">\r\n";
			str += "<tr>\r\n";
			str += "<td>--  日期  --</td>\r\n";
			str += "<td>--上班打卡--</td>\r\n";
			str += "<td>--下班打卡--</td>\r\n";
			str += "<td>--有效时长--</td>\r\n";
			str += "</tr>\r\n";

			for (int i = 1; i < vec.size(); i++)
			{
				str += "<tr>\r\n";
				for (int j = 0; j < vec[i].size(); j++)
				{
					str += "<td>";
					str += vec[i][j];
					str += "</td>\r\n";
				}
				str += "</tr>\r\n";
			}
			str += "</table>\r\n";
			str += "</tbody>\r\n";
			str += "</table>\r\n";
			str += "</body>\r\n";
			str += "</html>\r\n";

		}
		return true;
	}
	
	string sql = "SELECT rq,inclock,offclock,priodtime FROM clock WHERE user_id= " + userid + " AND DATE_FORMAT(rq,'%Y-%m')=DATE_FORMAT(CURDATE(),'%Y-%m');";

	
	if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
	{
		cout << m_imysqlpool->GetErr() << endl;
	}
	else
	{
		str += "<html>\r\n";
		str += "<head>\r\n";
		str += "</head>\r\n";
		str += "<body><p> 打卡记录 </p>\r\n";
		str += "<table border=\"0\"><tbody><table border=\"1\" cellspacing=\"0\">\r\n";
		str += "<tr>\r\n";
		str += "<td>  日期  </td>\r\n";
		str += "<td>上班打卡</td>\r\n";
		str += "<td>下班打卡</td>\r\n";
		str += "<td>有效时长</td>\r\n";
		str += "</tr>\r\n";

		for (int i = 1; i < vec.size(); i++)
		{
			str += "<tr>\r\n";
			for (int j = 0; j < vec[i].size(); j++)
			{
				str += "<td>";
				str += vec[i][j];
				str += "</td>\r\n";
			}
			str+= "</tr>\r\n";
		}
		str += "</table>\r\n";
		str += "</tbody>\r\n";
		str += "</table>\r\n";
		str += "</body>\r\n";
		str += "</html>\r\n";
		WriteLog(DEBUG, "7flag");
	}
	return true;
}
*/

bool ClockTime::Work(std::string& str)
{
	//type= in out;
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string type = m_parse_map["type"];

	std::vector<std::vector<std::string>> vec;

	if (!isUser(user, pass))
	{
		str = "user " + user + " pass " + pass + " is not in databases!";
		return false;
	}

	string userid = getUserId(user);
	if (userid == "")
	{
		str = "userid not exist!";
		return false;
	}

	if (type == "in")
	{
		m_curtime = xzy::curdate();
		m_starttime = xzy::nowtime();
		std::string sql = "INSERT INTO clockintime.clock(user_id, rq, inclock)VALUE(" + userid + ",'" + xzy::curdate() + "','" + xzy::nowtime() + \
			"')ON DUPLICATE KEY UPDATE inclock = VALUES(inclock);";
		WriteLog(DEBUG, sql.c_str());
		if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
		{
			str = "insert intime success!";
		}
		else
		{
			str = "insert intime failed!";
			return false;
		}
		WriteLog(DEBUG, "flag");
	}
	else if (type == "off")
	{
		Time time1;
		if (1)
		{
			m_endtime = xzy::nowtime();
			Time timein = m_starttime;
			Time timeoff = m_endtime;
			time1 = calculatevalidtime(timein, timeoff);
			WriteLog(DEBUG, "1", timein.toString().c_str(), "2", timeoff.toString().c_str(), "3", time1.toString().c_str());
		}
		//INSERT INTO clockintime.clock(user_id, rq, inclock)VALUE(1, "2020-11-26", "06:12:19")ON DUPLICATE KEY UPDATE inclock = VALUES(inclock);
		//INSERT INTO clockintime.clock(user_id, rq, offclock)VALUE(1, "2020-11-26", "06:12:29")ON DUPLICATE KEY UPDATE offclock = VALUES(offclock);
		std::string sql = "INSERT INTO clockintime.clock(user_id, rq, offclock)VALUE(" + userid + ",'" + xzy::curdate() + "','" + xzy::nowtime() + \
			"')ON DUPLICATE KEY UPDATE offclock = VALUES(offclock);";
		WriteLog(DEBUG, sql.c_str());
		if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
		{
			//sql = "UPDATE clock SET priodtime = DATE_FORMAT(DATE_SUB(FROM_UNIXTIME(TIMESTAMPDIFF(SECOND, inclock, offclock)), INTERVAL 8 HOUR), '%%H:%%i:%%s') WHERE rq ='"+xzy::curdate()+"';";
			sql = "UPDATE clock SET priodtime ='" + time1.toString() + "' WHERE rq = '" + xzy::curdate() + "'; ";

			WriteLog(DEBUG, sql.c_str());
			if (m_imysqlpool->mysqlinsert(sql.c_str()) != 0)
			{
				str = "insert offtime success! update priodtime success!";
			}
		}
		else
		{
			str = "insert offtime failed!";
			return false;
		}
	}
	else
	{
		//计算时间输出结果,
		string sql = "SELECT rq,inclock,offclock,priodtime FROM clock WHERE user_id= " + userid + " AND DATE_FORMAT(rq,'%Y-%m')=DATE_FORMAT(CURDATE(),'%Y-%m');";
		if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
		{
			cout << m_imysqlpool->GetErr() << endl;
		}
		else
		{
			calculatesub(vec);
			calculateweektime(vec);
			str += "<html>\r\n";
			str += "<head>\r\n";
			str += "</head>\r\n";
			str += "<body><p> 打卡记录 </p>\r\n";
			str += "<table border=\"0\"><tbody><table border=\"1\" cellspacing=\"0\">\r\n";
			str += "<tr>\r\n";
			str += "<td>--  日期  --</td>\r\n";
			str += "<td>--上班打卡--</td>\r\n";
			str += "<td>--下班打卡--</td>\r\n";
			str += "<td>--有效时长--</td>\r\n";
			str += "</tr>\r\n";

			for (int i = 1; i < vec.size(); i++)
			{
				str += "<tr>\r\n";
				for (int j = 0; j < vec[i].size(); j++)
				{
					str += "<td>";
					str += vec[i][j];
					str += "</td>\r\n";
				}
				str += "</tr>\r\n";
			}
			str += "</table>\r\n";
			str += "</tbody>\r\n";
			str += "</table>\r\n";
			str += "</body>\r\n";
			str += "</html>\r\n";

		}
		return true;
	}

	string sql = "SELECT rq,inclock,offclock,priodtime FROM clock WHERE user_id= " + userid + " AND DATE_FORMAT(rq,'%Y-%m')=DATE_FORMAT(CURDATE(),'%Y-%m');";


	if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
	{
		cout << m_imysqlpool->GetErr() << endl;
	}
	else
	{
		calculateweektime(vec);
		str += "<html>\r\n";
		str += "<head>\r\n";
		str += "</head>\r\n";
		str += "<body><p> 打卡记录 </p>\r\n";
		str += "<table border=\"0\"><tbody><table border=\"1\" cellspacing=\"0\">\r\n";
		str += "<tr>\r\n";
		str += "<td>  日期  </td>\r\n";
		str += "<td>上班打卡</td>\r\n";
		str += "<td>下班打卡</td>\r\n";
		str += "<td>有效时长</td>\r\n";
		str += "</tr>\r\n";

		for (int i = 1; i < vec.size(); i++)
		{
			str += "<tr>\r\n";
			for (int j = 0; j < vec[i].size(); j++)
			{
				str += "<td>";
				str += vec[i][j];
				str += "</td>\r\n";
			}
			str += "</tr>\r\n";
		}
		str += "</table>\r\n";
		str += "</tbody>\r\n";
		str += "</table>\r\n";
		str += "</body>\r\n";
		str += "</html>\r\n";
		WriteLog(DEBUG, "7flag");
	}
	return true;
}

bool ClockTime::DeleteRecode(std::string& text)
{
	string rq = m_parse_map["rq"];//删除的日期;//判断rq是否有效
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];

	std::vector<std::vector<std::string>> vec;

	if (!isUser(user, pass))
	{
		text = "user " + user + " pass " + pass + " is not in databases!";
		return false;
	}
	string userid = getUserId(user);
	if (!xzy::IsDate(rq))
	{
		text = "rq is error!";
		return false;
	}
	string sql = "delete from clock WHERE user_id=" + userid + " and rq='" + rq+"';";
	int result = m_imysqlpool->mysqlinsert(sql.c_str());
	if (result == 0)
	{
		text = "delete record success: value="+to_string(result)+"!";
		return false;
	}
	else
	{
		text += "delete record success: value=" + to_string(result) + "!\n";
	}
	sql = "SELECT rq,inclock,offclock,priodtime FROM clock WHERE user_id= " + userid + " AND DATE_FORMAT(rq,'%Y-%m')=DATE_FORMAT(CURDATE(),'%Y-%m');";
	if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
	{
		cout << m_imysqlpool->GetErr() << endl;
	}
	else
	{
		calculatesub(vec);
		calculateweektime(vec);
		text += "<html>\r\n";
		text += "<head>\r\n";
		text += "</head>\r\n";
		text += "<body><p> 打卡记录 </p>\r\n";
		text += "<table border=\"0\"><tbody><table border=\"1\" cellspacing=\"0\">\r\n";
		text += "<tr>\r\n";
		text += "<td>--  日期  --</td>\r\n";
		text += "<td>--上班打卡--</td>\r\n";
		text += "<td>--下班打卡--</td>\r\n";
		text += "<td>--有效时长--</td>\r\n";
		text += "</tr>\r\n";

		for (int i = 1; i < vec.size(); i++)
		{
			text += "<tr>\r\n";
			for (int j = 0; j < vec[i].size(); j++)
			{
				text += "<td>";
				text += vec[i][j];
				text += "</td>\r\n";
			}
			text += "</tr>\r\n";
		}
		text += "</table>\r\n";
		text += "</tbody>\r\n";
		text += "</table>\r\n";
		text += "</body>\r\n";
		text += "</html>\r\n";
	}
	return true;
}
bool ClockTime::DeleteUser(std::string&)
{

}

bool ClockTime::DeleteId(std::string&)
{

}

bool ClockTime::SetTime(std::string&)
{

}

/*
bool ClockTime::AddUser(std::string &str)//
{
	//write user file
	
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string mail = m_parse_map["mail"];
	string ruser = m_parse_map["ruser"];		//root ����û�
	string password = m_parse_map["password"];//clockintimexwx987785

	if (ruser != "wx987785" && password != "clockintimexwx987785")
	{
		str = "ruse or password is failed!";
		WriteLog(INFO, "ruser or password failed", ruser.c_str(),password.c_str());
		return false;
	}
	string userfile = USERFILE;
	//����Ŀ¼
	string dirname = ABSPATH + user;
	mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	if (mkdir(dirname.c_str(), mode)!=0)
	{
		WriteLog(DEBUG, "mkdir %s failed!,errno: %d", dirname.c_str(), errno);
		if (!file::existsfile(dirname.c_str()))
		{
			str = "create user faild!mkdir failed!";
			return false;
		}
		WriteLog(DEBUG, "%s is exist", dirname.c_str());
	}

	Json::Reader reader;
	Json::Value root;
	if (!file::readfiletojson(userfile, root))
	{
		WriteLog(INFO, "readfiletojosn failed,filename %s,errno: %d", userfile.c_str(), errno);
		str = "readfiletojson failed!";
		return false;
	}
	Json::Value node = root["info"];
	Json::Value node1;
	if (node.isArray())
	{
		//�ж��û��Ƿ����
		bool userexists = false;
		for (int i = 0; i < node.size(); i++)
		{
			node1 = node[i];
			if (user == node1["name"].asString())
			{
				userexists = true;
				str = user + " is exist!";
				return true;
			}
		}
		if (!userexists)
		{
			Json::Value node2;
			node2["name"] = user;
			node2["pass"] = pass;
			node2["mail"] = mail;
			node.append(node2);
		}
		root["info"] = node;
	}
	//д���ݵ��ļ�
	file::jsontofile(userfile.c_str(), root);
	str = "create user: " + user + " success.\n";
	str += "http://119.45.184.103:5189/clockintime/work?user=" + user + "&pass=" + pass;

	return true;
}

bool ClockTime::Work(std::string& str)
{
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	
	std::string dir= ABSPATH + user;

	//�ж��û��治����
	if (!file::existsfile(dir.c_str()))
	{
		str = "user is not exist!";
		return false;
	}
	string filename = dir+"/"+getfilename(m_url);
	if (!file::existsfile(filename.c_str()))
	{
		//�½�һ���ļ�
		std::string cmd = "touch " + filename;
		system(cmd.c_str());
	}
	Json::Reader reader;
	Json::Value root;
	if (!file::readfiletojson(filename, root))
	{
		WriteLog(INFO, "readfiletojosn failed,filename %s,errno: %d", filename.c_str(), errno);
		str = "readfiletojson "+filename+" failed!";
		return false;
	}
	Json::Value node = root["data"];
	string rq = xzy::curdate();
	string t1 = xzy::nowtime();
	if (node.isNull())
	{
		
		Json::Value node1;
		node1.append(t1);
		node[rq].append(node1);
	}
	else if (node.isArray())
	{
		bool flag = false;
		for (int i = 0; i < node.size(); i++)
		{
			string key = node[i]["rq"].asString();
			if (key == rq)
			{
				flag = true;
				node[i]["clock"].append(t1);
			}
		}
		if (!flag)
		{
			Json::Value node1;
			node1["rq"] = rq;
			node1["clock"].append(t1);
			node.append(node1);

		}
	}
	root["data"] = node;
	file::jsontofile(filename.c_str(), root);
	//readfile
	str = jsontohtml(root);
}

bool ClockTime::DeleteUser(std::string& str)
{
	str = "";
	bool flag = false;
	string user = m_parse_map["user"];	
	string pass = m_parse_map["pass"];

	Json::Reader reader;
	Json::Value root;
	if (!file::readfiletojson(USERFILE, root))
	{
		WriteLog(INFO, "readfiletojosn failed,filename %s,errno: %d", USERFILE, errno);
		str = "readfiletojson failed!";
		return false;
	}

	Json::Value node = root["info"];

	Json::Value node1;
	Json::Value copynode;
	if (node.isArray())
	{
		//�ж��û��Ƿ����
		for (int i = 0; i < node.size(); i++)
		{
			node1 = node[i];
			if (user != node1["name"].asString())
			{
				copynode.append(node[i]);

			}
			else
			{
				if (pass == node1["pass"].asString())
				{
					str = "user: " + user + "exist!" + " delete success. ";
					//ɾ��Ŀ¼��user.json�µ�����;//中
					string dir = ABSPATH;
					std::string deletestr = "rm -rf " + dir + user;
					system(deletestr.c_str());
					flag = true;
				}
				else
				{
					str = "user: " + user + " exist!" + "pass is error!" + "delete failed!";
				}
			}
		}
	}
	if (str.size() == 0)
	{
		str = "user: " + user + " is not exist!";
	}
	else if (flag)
	{
		root["info"] = copynode;
		file::jsontofile(USERFILE, root);
	}
	return true;
}

bool ClockTime::DeleteId(std::string& str)
{
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string date = m_parse_map["date"];
	int id= atoi(m_parse_map["id"].c_str());
	//�ж�id ��û����û������ �����, dateû�о�Ĭ�ϵ���,

}

bool ClockTime::SetTime(std::string& str)
{
	string user = m_parse_map["user"];
	string pass = m_parse_map["pass"];
	string type = m_parse_map["type"];
	string starttime = m_parse_map["starttime"];
	string endtime = m_parse_map["endtime"];

}
*/

void ClockTime::SetReponse200(shared_ptr<Http::IHttpResponse> response, std::string& context)
{
	WriteEnter;
	WriteLog(DEBUG, "enter %s", context.data());
	context = "<html><table border = \"0\">\n<meta charset=\"UTF-8\">\n" + context ;
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

std::string ClockTime::jsontohtml(Json::Value& root)
{
	string text = "<html>\
		<head>\
		</head>\
		<meta charset=\"utf-8\">\
		<body><p> 打卡记录 </p>\
		<table border = \"0\"><tbody> <table border = \"1\" cellspacing = \"0\">";

	text += "<tr>	\
		<td>日期</td>\
		<td>上班打卡</td>\
		<td>下班打卡</td>\
		<td>时长</td>\
		</tr>";
	Json::Value node = root["data"];

	string rq;
	string stime;
	string etime;
	string priodtime = "0";
	for (int i = 0; i < node.size(); i++)
	{
		text += "<tr>	\
			<td> ";
		text += node[i]["rq"].asString() + "</td><td>";
		Json::Value node1 = node[i]["clock"];
		for (int j = 0; j < node1.size(); j++)
		{
			cout << node1[i] << endl;
			string st = node1[j].asString();
			text+= st + "</td><td>";
		}
		text += priodtime + "</td></tr>";
	}
	text += "</table>\
		</tbody>\
		</table>\
		</body>\
		</html>";
	cout << text << endl;
	return text;
}

void ClockTime::SelectUser()
{
	std::string sql = "SELECT `name`,pass,mail,id FROM clockintime.`user`;";
	std::vector<std::vector<std::string>> vec;
	if (m_imysqlpool->mysqlquery(sql.c_str(), vec) == -1)
	{
		cout << m_imysqlpool->GetErr() << endl;
	}
	else
	{
		m_user_pass.clear();
		int n = vec.size();
		cout << vec.size() << endl;
		cout << vec[1].size() << endl;
		
		for (int i = 1; i < vec.size(); i++)
		{
			for (int j = 0; j < vec[i].size(); j++)
			{
				cout << "vec[" << i << "][" << j << "]" << vec[i][j] << endl;
			}
			
			UserInfo userinfo;
			userinfo.user = vec[i][0];
			userinfo.pass = vec[i][1];
			userinfo.mail = vec[i][2];
			userinfo.userid = vec[i][3];
			m_user_pass[userinfo.user] = userinfo;
			
		}
		m_UserPassFlag = true;
	}
}

bool ClockTime::isUser(std::string& user, std::string& pass, std::string mail)
{
	if (!m_UserPassFlag)
		SelectUser();
	std::map<string, UserInfo>::iterator iter = m_user_pass.find(user);
	if (iter != m_user_pass.end())
	{
		if (mail.size() != 0)
		{
			if (mail != iter->second.mail)
				return false;
		}
		if (pass == iter->second.pass)
		{
			return true;
		}		
	}
	return false;
}

std::string ClockTime::getUserId(const std::string& user)
{
	if (!m_UserPassFlag)
		SelectUser();
	std::map<string, UserInfo>::iterator iter = m_user_pass.find(user);
	if (iter != m_user_pass.end())
	{	
		return iter->second.userid;
	}
	return "";
}

void calculatesub(std::vector<std::vector<std::string>>& vec)
{
	//rq time1 time2 proidtime
	std::vector<std::vector<std::string>> vec1;
	for (auto ve : vec)
	{
		Time timein = ve[1];
		Time timeoff = ve[2];
		Time time=calculatevalidtime(timein,timeoff);

		ve[3] = time.toString();
		vec1.push_back(ve);
		cout << ve[0] << "---" << ve[1] << "---" << ve[2] << "---" << ve[3] << endl;
	}	
	vec = vec1;
}

Time calculatevalidtime(Time time1, Time time2)
{
	Time t1;
	Time t2;
	Time ct1;
	Time ct2;
	Time sum;
	for (auto i : validtime)// 8-12 13:17 18 24
	{
		ct1 = time1;	//16
		ct2 = time2;	//17
		t1 = i.time1;  //8	13:30
		t2 = i.time2;  //12	17:30
		cout << t1.toString() << "  " << t2.toString() << endl;
		cout << time1.toString() << "  " << time2.toString() << endl;

		if (time2 < t1)
			break;
		if (t2 < time1)
			continue;
		if (t1 > time1)
			ct1 = t1;
		if (t2 < time2)
			ct2 = t2;
		sum = ct2 - ct1 + sum;
		cout << sum.toString() << endl;
	}
	return sum;
}

void calculateweektime(std::vector<std::vector<std::string>>& vec)
{
	std::vector<std::vector<std::string>> tempvec(vec);
	vec.erase(vec.begin()+1,vec.end());
	//vec[0] = tempvec[0];
	int days=0;//次数 day
	int values=0;//总时长秒 s
	int daysum = 0;
	int valuesum = 0;
	int week = -1;
	for (int i = 1; i < tempvec.size(); i++)
	{
		vector<std::string> ve;
		ve = tempvec[i];
		
		int weeknum=xzy::GetWeekNum(ve[0]);//这一年第几周;
		cout << "week:" << weeknum << "date:" << ve[0] << endl;
		if (week == weeknum || week == -1)
		{
			days++;
			values += xzy::GetSecond(ve[3]);
			vec.push_back(ve);
			week = weeknum;
		}
		else
		{
			vector<std::string> ve1;
			ve1.push_back("当周总计");  // 周总计,total,周平均
			ve1.push_back(xzy::GetTime(values));
			ve1.push_back("当周平均");
			ve1.push_back(xzy::GetTime(values / days));
			vec.push_back(ve1);

			daysum += days;
			valuesum += values;
			days = 1;
			values = xzy::GetSecond(ve[3]);
			vec.push_back(ve);
			week = weeknum;
		}
		WriteLog(DEBUG,"%s,%s,%s,%s", ve[0].c_str(), ve[1].c_str(), ve[2].c_str(), ve[3].c_str());
		WriteLog(DEBUG, "%d,%d,%d,%d", days, values, daysum, valuesum);
	}
	daysum += days;
	valuesum += values;
	//最后的没有统计
	vector<std::string> ve1;
	ve1.push_back("当周总计");  // 周总计,total,周平均
	ve1.push_back(xzy::GetTime(values));
	ve1.push_back("当周平均");
	ve1.push_back(xzy::GetTime(values / days));
	vec.push_back(ve1);

	//总计当月
	ve1.clear();
	ve1.push_back("当月总计");  // 周总计,total,周平均
	ve1.push_back(xzy::GetTime(valuesum));
	ve1.push_back("当月平均");
	ve1.push_back(xzy::GetTime(valuesum / daysum));
	vec.push_back(ve1);
}