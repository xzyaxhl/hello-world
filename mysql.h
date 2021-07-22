#pragma once
#ifndef __MYSQL_H__
#define __MYSQL_H__

#include <mysql/mysql.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"
#include <json/json.h>
#include <vector>

namespace Mysql
{
	class IMysql
	{
	public:
		virtual ~IMysql() {};
		virtual bool mysqlinit() = 0;
		virtual int mysqlquery(const char* sql, Json::Value &result) = 0;
		virtual int mysqlquery(const char* sql, std::string &) = 0;
		virtual int mysqlinsert(const char *sql) = 0;
		virtual int mysqlqueryb(const char* sql, Json::Value &result) = 0;
	};
	class CMysql :public IMysql
	{
	public:
		CMysql();
		CMysql(std::string &host, std::string &user, std::string &pass, uint16_t port, std::string &db);
		~CMysql();
		bool mysqlinit();

		int mysqlquery(const char* sql, Json::Value &result);
		int mysqlqueryb(const char* sql, Json::Value &result);

		int mysqlquery(const char* sql, std::string &);
		int mysqlinsert(const char *sql);


	private:
		MYSQL conn;
		std::string m_host;
		std::string m_user;
		std::string m_pass;
		uint16_t m_port;
		std::string m_db;


	};
}

#endif // __MYSQL_H__
