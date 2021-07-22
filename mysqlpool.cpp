#include <mysql/mysql.h>
#include "mysqlpool.h"
#include <json/json.h>
#include "log.h"
//#include "mutex.h"

CMysqlPool::CMysqlPool(int num)
{
	WriteLog(DEBUG, "enter");
	m_config = GlobalConfig::instance();
	m_sHost = m_config->mysql.host;
	m_sUser = m_config->mysql.user;
	m_sPass = m_config->mysql.pass;
	m_sDb = m_config->mysql.db;
	m_iPort = m_config->mysql.port;
	m_iConnNum = num;
	Init(m_sHost, m_sUser, m_sPass, m_sDb, m_iPort, m_iConnNum);
}

CMysqlPool::~CMysqlPool()
{
	for (int i = 0; i < m_vectorConn.size(); i++)
	{
		sConStatus* scs = (sConStatus*)m_vectorConn[i];

		mysql_close((MYSQL*)scs->connAddr);
		scs->connAddr = NULL;
		delete scs;
	}
	mysql_library_end();
}

int CMysqlPool::Init(string host, string user, string pass, string db, int port, int nConnNum)
{
	m_sHost = host;
	m_sUser = user;
	m_sPass = pass;
	m_sDb = db;
	m_iPort = port;
	m_iConnNum = nConnNum;

	//初始化数据库  
	if (0 == mysql_library_init(0, NULL, NULL))
		WriteLog(DEBUG, "mysql_library_init()succeed");
	else {
		WriteLog(DEBUG,"mysql_library_init()failed" );
		return -1;
	}

	MYSQL* mysql;
	for (int i = 0; i < m_iConnNum; i++)
	{
		mysql = (MYSQL*)this->createOneConn();
		if (mysql == NULL)
			return -1;
		sConStatus* scs = new sConStatus();
		scs->connAddr = mysql;
		scs->useStatus = US_IDLE;
		m_vectorConn.push_back(scs);
		m_mapVI[scs] = i;
		m_mapMysqlScs[mysql] = scs;
	}
	cout << m_vectorConn.size() << endl;
	return 0;
}
//取一个链接出来，并且标记使用状态
void* CMysqlPool::getOneConn()//get a connection
{
	int N = m_vectorConn.size();
	for (int i = 0; i < N; i++)
	{
		//Pthread::CGuard guard(m_sMutex);
		sConStatus* scs = (sConStatus*)m_vectorConn[i];
		if (scs->useStatus == US_IDLE)
		{
			scs->useStatus = US_USE;
			return scs->connAddr;
		}
	}
	return NULL;
}
//归还一个连接
void CMysqlPool::retOneConn(void* pMysql)//return a connection
{
	if (!pMysql)
		return;
	//Pthread::CGuard guard(m_sMutex);
	auto it1 = m_mapMysqlScs.find(pMysql);
	if (it1 == m_mapMysqlScs.end())
		return;
	auto it2 = m_mapVI.find(it1->second);
	if (it2 == m_mapVI.end())
		return;

	int nInx = it2->second;
	sConStatus* scs = (sConStatus*)m_vectorConn[nInx];
	scs->useStatus = US_IDLE;
}

void CMysqlPool::checkConn()//check the connection if is alive 
{

}

void* CMysqlPool::createOneConn()
{
	MYSQL* mysql;

	mysql = mysql_init(0);
	if (mysql == NULL)
	{
		WriteLog(DEBUG,"mysql init failed...");
		return NULL;
	}
	if (0 == mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8"))
		WriteLog(DEBUG,"mysql_options charset utf8 ok!\n");
	//超时时间后，有链接自动重连
	my_bool reConnect = 1;
	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reConnect);

	if (!mysql_real_connect(mysql, m_sHost.c_str(), m_sUser.c_str(), m_sPass.c_str(), m_sDb.c_str(), 0, NULL, NULL))
	{
		WriteLog(DEBUG,"connect mysql failed!\n");
		err = mysql_error(mysql);
		WriteLog(DEBUG, err.c_str());
		exit(1);
		return NULL;
	}
	else
	{
		WriteLog(DEBUG,"connect mysql success!\n");
	}

	return mysql;
}
/*
std::string CMysqlPool::mysqlquery(Json::Value& result, const char* sql, int& r)
{
	MYSQL_RES* results;
	Json::Value node;
	Json::Value arraynode;
	MYSQL* conn = (MYSQL*)getOneConn();
	if (conn == NULL)
		return "";
	cout << m_mapVI[m_mapMysqlScs[conn]] << endl;
	std::string text = "<tr>";
	if (0 == mysql_query(conn, sql))
	{
		results = mysql_store_result(conn);
		int rows = mysql_num_rows(results);
		r = rows;
		unsigned int fieldcount = mysql_num_fields(results);//
		MYSQL_FIELD* field = NULL;
		std::string fields[fieldcount];
		for (unsigned int j = 0; j < fieldcount; j++)
		{
			field = mysql_fetch_field_direct(results, j);//
			fields[j] = field->name;
			text = text + "<td>" + field->name + "</td>";
		}
		text += "</tr>\n";

		for (unsigned int i = 0; i < rows; i++)
		{
			text += "<tr>";
			MYSQL_ROW row = NULL;
			row = mysql_fetch_row(results);//ָ
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				node[fields[j]] = rows;

				text = text + "<td>" + row[j] + "</td>";
			}

			arraynode[i] = node;
			text += "</tr>\n";
		}
		if (results != NULL)
			mysql_free_result(results);// 出错
		result["data"] = arraynode;
	}
	retOneConn(conn);
	return text;
}


std::string CMysqlPool::mysqlquery(Json::Value& result, const char* sql)
{
	MYSQL_RES* results;
	Json::Value node;
	Json::Value arraynode;
	MYSQL* conn = (MYSQL*)getOneConn();
	if (conn == NULL)
		return "";
	cout << m_mapVI[m_mapMysqlScs[conn]] << endl;

	std::string text = "<tr>";
	if (0 == mysql_query(conn, sql))
	{
		results = mysql_store_result(conn);
		int rows = mysql_num_rows(results);

		unsigned int fieldcount = mysql_num_fields(results);//

		MYSQL_FIELD* field = NULL;
		std::string fields[fieldcount];
		for (unsigned int j = 0; j < fieldcount; j++)
		{
			field = mysql_fetch_field_direct(results, j);//
			fields[j] = field->name;
			text = text + "<td>" + field->name + "</td>";
		}
		text += "</tr>\n";

		for (unsigned int i = 0; i < rows; i++)
		{
			text += "<tr>";
			MYSQL_ROW row = NULL;
			row = mysql_fetch_row(results);//ָ
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				node[fields[j]] = row[j];
				text = text + "<td>" + row[j] + "</td>";
				//printf("%s: %s\t\n", fields[j].c_str(), row[j]);
			}
			arraynode[i] = node;
			text += "</tr>\n";
		}
		if (results != NULL)
			mysql_free_result(results);//
		result["data"] = arraynode;
	}
	retOneConn(conn);
	return text;
}*/

std::string CMysqlPool::mysqlquery(const char* sql, int& r)
{
	MYSQL_RES* results;
	MYSQL* conn = (MYSQL*)getOneConn();
	if (conn == NULL)
		return "";
	cout << m_mapVI[m_mapMysqlScs[conn]] << endl;
	std::string text = "<tr>";
	if (0 == mysql_query(conn, sql))
	{
		results = mysql_store_result(conn);
		int rows = mysql_num_rows(results);
		r = rows;
		unsigned int fieldcount = mysql_num_fields(results);//
		MYSQL_FIELD* field = NULL;
		std::string fields[fieldcount];
		for (unsigned int j = 0; j < fieldcount; j++)
		{
			field = mysql_fetch_field_direct(results, j);//
			fields[j] = field->name;
			text = text + "<td>" + field->name + "</td>";
		}
		text += "</tr>\n";

		for (unsigned int i = 0; i < rows; i++)
		{
			text += "<tr>";
			MYSQL_ROW row = NULL;
			row = mysql_fetch_row(results);//ָ
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				text = text + "<td>" + row[j] + "</td>";
			}
			text += "</tr>\n";
		}
		if (results != NULL)
			mysql_free_result(results);//
	}
	retOneConn(conn);
	return text;
}

int CMysqlPool::mysqlquery(const char* sql, std::vector< std::vector<std::string>>& vec)
{
	//WriteLog(INFO, "11");
	MYSQL_RES* results;
	MYSQL* conn = (MYSQL*)getOneConn();
	if (conn == NULL)
	{
		//WriteLog(INFO, "112");
		err = "getOneConn is NULL!";
		WriteLog(INFO, "getOneConn is NULL!");
		return -1;

	}
	cout << m_mapVI[m_mapMysqlScs[conn]] << endl;
	//WriteLog(INFO, "113");
	if (0 == mysql_query(conn, sql))
	{
		results = mysql_store_result(conn);
		int rows = mysql_num_rows(results);
		
		unsigned int fieldcount = mysql_num_fields(results);//
		MYSQL_FIELD* field = NULL;
		std::string fields[fieldcount];
		//WriteLog(INFO, "114");
		std::vector<std::string> temp;
		for (unsigned int j = 0; j < fieldcount; j++)
		{
			//WriteLog(INFO, "115");
			field = mysql_fetch_field_direct(results, j);//
			//fields[j] = field->name;
			temp.push_back(field->name);
		}
		vec.push_back(temp);
		for (unsigned int i = 1; i < rows+1; i++)
		{
			//WriteLog(INFO, "116,%d,%d",i,rows+1);
			temp.clear();
			MYSQL_ROW row = NULL;
			row = mysql_fetch_row(results);//ָ
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				//WriteLog(INFO, "117,%d,%d,%s",j, fieldcount, row[j]);
				if (row[j] == NULL)
				{
					temp.push_back("");
				}	
				else
				{
					temp.push_back(row[j]);
				}
			}
			vec.push_back(temp);
		}
		//WriteLog(INFO, "118");
		if (results != NULL)
			mysql_free_result(results);//
		retOneConn(conn);
		return 0;
	}
	err = mysql_error(conn);
	retOneConn(conn);
	return -1;
}

int CMysqlPool::mysqlinsert(const char* sql)
{
	MYSQL* conn = (MYSQL*)getOneConn();
	if (conn == NULL)
		return 0;
	int ret = mysql_query(conn, sql);
	if (ret != 0)
	{
		err = mysql_error(conn);
		WriteLog(DEBUG,"mysqlinsert failed: %s,ret: %d" , err.c_str() ,ret );
		return 0;
	}
	my_ulonglong affected_row = mysql_affected_rows(conn);

	WriteLog(DEBUG,"%d rows affected.\n", (int)affected_row);
	retOneConn(conn);
	return (int)affected_row;
}

CMysqlPool* CMysqlPool::instance()
{
	static CMysqlPool* cmysqlpool = NULL;
	if (cmysqlpool == NULL)
	{
		/*std::string host = "127.0.0.1";
		std::string user = "root";
		std::string pass = "123456";
		std::string db = "test";
		int port = 3306;*/
		cmysqlpool = new CMysqlPool(4);
	}
	return cmysqlpool;
}

string CMysqlPool::GetErr()
{
	return err;
}

void CMysqlPool::help()
{
	printf(
		R"(	CMysqlPool* mysqlpool = CMysqlPool::instance();
			mysqlpool->Init("127.0.0.1", "root", "123456", "test", 3306, 10);
		)"
	);
}

/*

sprintf(select_user, "select * from user where UserName='%s'", body.userName);
*/