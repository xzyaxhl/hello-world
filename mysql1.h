/*#pragma once
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
		virtual std::string mysqlquery(Json::Value &result, const char* sql) = 0;
	};

	class CMysql :public IMysql
	{
	public:
		CMysql();
		CMysql(GlobeConfig &);
		CMysql(const CMysql&);
		~CMysql();
		int connect(std::string &host, std::string &user, std::string &pass, std::string &db, int port);
		void closeconnect();
		std::string mysqlquery(Json::Value &result, const char* sql);
		int SelectQuery(const char* sql);
		int ModifyQuery(const char* sql);
		
		//��ȡ��һ�εĴ���
		const char *GetErr();

		char **FetchRow();

		char *getField(const char *filedname);
	public:
		void setused();
		void setidle();
		bool isidle();
	private:
		bool isConnected();
		void setConnected(bool bTrueFalse);
		char* GetField(unsigned int iFieldIndex);
		void FreePreResult();
		int reconnect();
	private:
		bool m_buseidle;  //conn״̬��true:use;false:idle;
		bool m_connstatu;//conn״̬��true:������;false:δ����;
		char m_errmsg[1024];//������Ϣ;
		int m_iFields;//�ֶθ���;
		std::map<std::string, int> m_mapFieldNameIndex;//��һ��map,  key���ֶ���,  value���ֶ�����
	private:
		MYSQL conn;
		MYSQL_RES* m_result;
		MYSQL_ROW m_row;
	private:
		std::string m_host;
		std::string m_user;
		std::string m_pass;
		std::string m_db;
		int m_port;
	};
}

#endif // __MYSQL_H__*/