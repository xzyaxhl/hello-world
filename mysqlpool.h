#pragma once
#ifndef __MYSQLPOOL_H__
#define __MYSQLPOOL_H__

#include <stdio.h>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <json/json.h>
#include "config.h"
//#include "mutex.h"
using namespace std;

#define MYSQL_CONN_NUM_MAX_VALUE 2;

enum _USE_STATUS
{
	US_USE = 0,//使用
	US_IDLE = 1 //空闲
};

typedef struct _sConStatus
{
	void* connAddr;
	int useStatus;
}sConStatus;

/*class IMysqlPool
{
public:
	~IMysqlPool() {};
	virtual int Init();// string& host, string& user, string& pass, string& db, int port, int nConnNum) = 0;
	virtual void* getOneConn() = 0;
	virtual void retOneConn(void* pMysql) = 0;
	virtual void checkConn() = 0;
	virtual void* createOneConn() = 0;
	virtual int mysqlinsert(const char* sql) = 0;
};*/

class CMysqlPool
{
public:

	~CMysqlPool();
	//-1,失败，0成功。
	int Init(string host, string user, string pass, string db, int port, int nConnNum);
	//成功取出一个连接则返回MYSQL* ,失败NULL。
	void* getOneConn();
	//归还一个连接池。
	void retOneConn(void* pMysql);
	void checkConn();
	void* createOneConn();
	static CMysqlPool* instance();
	//外部访问函数
	//std::string mysqlquery(Json::Value& result, const char* sql);
	//std::string mysqlquery(Json::Value& result, const char* sql, int& row);
	std::string mysqlquery(const char* sql, int& row);
	int mysqlquery(const char* sql, std::vector< std::vector<std::string>>& vec);


	int mysqlinsert(const char* sql);
	string GetErr();
	void help();
private:
	string m_sHost;
	string m_sUser;
	string m_sPass;
	string m_sDb;
	int m_iPort;
	int m_iConnNum;
private:
	CMysqlPool(int num);
	vector<void*>m_vectorConn;//push_back sConStatus。 MYSQl* mysql，和状态
	map<void*, int>m_mapVI;//sConStatus *,INT i(m_vectorConn 的索引)
	map<void*, void*>m_mapMysqlScs;//MYSQL*, sConStatus *
	string err;
	GlobalConfig* m_config;
	//Pthread::CMutex m_sMutex;

	/*
	conn之后调用
	my_bool reConnect = 1;

	mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reConnect);

	char value = 1;
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, (char *)&value);

	隔一段时间ping一次
	mysql_ping(m_mysql);
	*/
};

#endif // !__MYSQLPOOL_H__
// yum intall libmysqlclient-dev
//附加依赖项  `mysql_config --cflags --libs`
